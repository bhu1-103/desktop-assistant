# desktop-assistant

A super lightweight desktop assistant (not a coworker) which works with an **embedding model based router** (for decision making), **lowwi** (wake word detection) and **whisper.cpp** (for STT)

it is very messy, i need to find a better way to do all this...

## Specs

- C++ based
- **wakeword detection** -> [lowwi](https://github.com/CLFML/lowwi)
- ~**wakeword trainer** -> [dscripka's trainer](https://github.com/dscripka/openWakeWord) with [dscripka's synthetic data generator](https://github.com/dscripka/piper-sample-generator)~
- **wakeword trainer** -> [livekit's trainer](https://github.com/livekit/livekit-wakeword)
- ~**router** -> [Google Bert base model](https://huggingface.co/google/bert_uncased_L-2_H-128_A-2)~ -> to-do
- **music controller** -> [Snowflake Arctic Embed](https://ollama.com/library/snowflake-arctic-embed)

## How to build

### Quick Way

- clone the repo
```bash
git clone --recurse-submodules https://github.com/bhu1-103/desktop-assistant.git
cd desktop-assistant
./scripts/download-models.sh
./scripts/build.sh
./build/friday
```

### Wake Word -> LOWWI

- clone the repo
```bash
git clone --recurse-submodules https://github.com/bhu1-103/desktop-assistant.git
```
- move into the directory
```bash
cd desktop-assistant
```
- download the required models for lowwi, whisper and lua
```bash
./scripts/download-models.sh
```
- run the build command
```bash
./scripts/build.sh
```
- run the program
```bash
./build/friday
```

### ASR -> Whisper.cpp

- clone whisper.cpp
```bash
git clone https://github.com/ggml-org/whisper.cpp
```
- move into the directory
```bash
cd whisper.cpp
```
- build whisper.cpp
```bash
cmake -B build && cmake --build build -j --config Release
```
- download the tiny model with 5 bit quantization
```bash
sh ./models/download-ggml-model.sh tiny.en-q5_1
```
or
```bash
wget -P models/ https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.en-q5_1.bin
```
- run a demo to make sure it is working
```bash
./whisper.cpp/build/bin/whisper-cli -m ./whisper.cpp/models/ggml-tiny.en-q5_1.bin -f ./whisper.cpp/samples/jfk.wav
cd ..
```

### Router Engine -> Arctic Embed using Lua
- install LuaJIT from [here](https://luajit.org/luajit.html)
- install required packages using LuaRocks and set path temporarily
```bash
luarocks --lua-version=5.1 --local install lua-cjson
luarocks --lua-version=5.1 --local install luafilesystem
eval "$(luarocks --lua-version=5.1 --local path)"
```
- run the inference
```bash
luajit music.lua "play music"
```
