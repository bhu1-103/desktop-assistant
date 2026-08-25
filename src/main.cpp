/*
 *  Copyright 2024 (C) Jeroen Veen <ducroq> & Victor Hogeweij <Hoog-V>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * This file is part of the Lowwi library
 *
 * Author:          Victor Hogeweij <Hoog-V>
 *
 */
#include <lowwi.hpp>
#include <audio_async.hpp>
#include <whisper.h>
#include <SDL.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

constexpr int SAMPLE_RATE = 16000;
constexpr int COMMAND_CHUNK_MS = 500;
constexpr int MIN_RECORDING_MS = 1000;
constexpr int SILENCE_CHUNKS = 4;
constexpr int MAX_COMMAND_MS = 5000;
constexpr int REARM_MS = 2000;
constexpr float SILENCE_THRESHOLD = 0.015f;

std::atomic<bool> running(true);
std::atomic<bool> recording(false);

std::chrono::steady_clock::time_point command_start;
std::chrono::steady_clock::time_point rearm_until =
    std::chrono::steady_clock::now();

void stop(int) {
    running = false;
}

bool silent(const std::vector<float>& pcm) {
    if (pcm.empty()) return true;

    double energy = 0;
    for (float x : pcm)
        energy += static_cast<double>(x) * x;

    return std::sqrt(energy / pcm.size()) < SILENCE_THRESHOLD;
}

void tone(float freq, int ms) {
    SDL_AudioSpec spec{};
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_F32SYS;
    spec.channels = 1;
    spec.samples = 512;

    auto dev = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    if (!dev) return;

    int count = SAMPLE_RATE * ms / 1000;
    std::vector<float> buf(count);

    constexpr float PI = 3.14159265359f;

    for (int i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float fade = 1.0f;

        if (i < 160)
            fade = static_cast<float>(i) / 160.0f;
        else if (i > count - 160)
            fade = static_cast<float>(count - i) / 160.0f;

        buf[i] = 0.18f * fade *
                 std::sin(2.0f * PI * freq * t);
    }

    SDL_QueueAudio(dev, buf.data(), buf.size() * sizeof(float));
    SDL_PauseAudioDevice(dev, 0);
    SDL_Delay(ms + 10);
    SDL_CloseAudioDevice(dev);
}

void listening_tone() {
    tone(880.0f, 100);
}

void done_tone() {
    tone(440.0f, 120);
}

std::string shell_escape(const std::string& s) {
    std::string out = "'";

    for (char c : s)
        out += (c == '\'') ? "'\\''" : std::string(1, c);

    return out + "'";
}

std::string run_music(const std::string& text) {
    std::string cmd =
        "luajit music.lua " + shell_escape(text);

    std::array<char, 256> buf{};
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buf.data(), buf.size(), pipe))
        result += buf.data();

    pclose(pipe);

    while (!result.empty() &&
           (result.back() == '\n' ||
            result.back() == '\r' ||
            result.back() == ' '))
        result.pop_back();

    return result;
}

void wakeword_callback(
    CLFML::LOWWI::Lowwi_ctx_t,
    std::shared_ptr<void> arg)
{
    if (recording) return;

    auto now = std::chrono::steady_clock::now();
    if (now < rearm_until) return;

    auto audio = std::static_pointer_cast<audio_async>(arg);

    std::cout << "\n[Friday]: Wake word detected! Listening...\n";

    recording = true;
    command_start = now;

    audio->clear();
    listening_tone();
}

int main() {
    std::signal(SIGINT, stop);

    auto audio = std::make_shared<audio_async>(10000);

    CLFML::LOWWI::Lowwi wake;
    CLFML::LOWWI::Lowwi_word_t word;

    word.cbfunc = wakeword_callback;
    word.cb_arg = audio;
    word.model_path = std::filesystem::path("models/friday.onnx");
    word.phrase = "Friday";
    word.threshold = 0.05f;

    wake.add_wakeword(word);

    std::cout << "Loading Whisper model...\n";

    auto params = whisper_context_default_params();
    auto* ctx = whisper_init_from_file_with_params(
        "models/ggml-tiny.en-q5_1.bin", params);
        //"models/ggml-small.en-q5_1.bin", params);

    if (!ctx) {
        std::cerr << "Failed to initialize whisper context\n";
        return 1;
    }

    if (!audio->init(-1, SAMPLE_RATE) || !audio->resume()) {
        std::cerr << "Failed to initialize microphone\n";
        whisper_free(ctx);
        return 1;
    }

    audio->clear();

    std::vector<float> audio_buffer;
    std::vector<float> command_buffer;

    std::cout << "Ready. Say 'Friday'...\n";

    int silence_chunks = 0;

    while (running) {
        if (!recording) {
            if (std::chrono::steady_clock::now() < rearm_until) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100));
                continue;
            }

            audio->get(1000, audio_buffer);

            if (!audio_buffer.empty())
                wake.run(audio_buffer);

            audio_buffer.clear();

            std::this_thread::sleep_for(
                std::chrono::milliseconds(500));

            continue;
        }

        audio->get(COMMAND_CHUNK_MS, audio_buffer);

        command_buffer.insert(
            command_buffer.end(),
            audio_buffer.begin(),
            audio_buffer.end());

        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - command_start
            ).count();

        if (elapsed >= MIN_RECORDING_MS) {
            if (silent(audio_buffer))
                ++silence_chunks;
            else
                silence_chunks = 0;
        }

        audio_buffer.clear();

        bool stop_recording =
            silence_chunks >= SILENCE_CHUNKS ||
            command_buffer.size() >=
                static_cast<size_t>(SAMPLE_RATE * MAX_COMMAND_MS / 1000);

        if (!stop_recording) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(COMMAND_CHUNK_MS));
            continue;
        }

        std::cout << "[Friday]: Processing command...\n";

        std::string transcript;

        if (!command_buffer.empty()) {
            auto wp =
                whisper_full_default_params(
                    WHISPER_SAMPLING_GREEDY);

            wp.print_progress = false;
            wp.print_realtime = false;
            wp.language = "en";
            wp.no_context = true;
            wp.single_segment = true;

            if (whisper_full(
                    ctx,
                    wp,
                    command_buffer.data(),
                    command_buffer.size()) == 0)
            {
                for (int i = 0;
                     i < whisper_full_n_segments(ctx);
                     ++i)
                {
                    const char* text =
                        whisper_full_get_segment_text(ctx, i);

                    if (text)
                        transcript += text;
                }
            }
        }

        if (transcript.empty()) {
            std::cout << "[User Said]: [BLANK_AUDIO]\n";
        } else {
            std::cout << "[User Said]: "
                      << transcript << "\n";

            std::string intent = run_music(transcript);

            if (intent.empty())
                std::cout << "[Friday]: Lua execution failed\n";
            else
                std::cout << "[Friday]: " << intent << "\n";
        }

        command_buffer.clear();
        silence_chunks = 0;
        recording = false;

        audio->clear();

        rearm_until =
            std::chrono::steady_clock::now() +
            std::chrono::milliseconds(REARM_MS);

        done_tone();

        std::cout << "\n[Friday]: Ready. Say 'Friday'...\n";
    }

    whisper_free(ctx);
    return 0;
}
