# desktop-assistant

A super lightweight desktop assistant which works with an embedding model based router (for decision making), lowwi (wake word detection) and whisper (for STT)

## specs

- C++ based
- **wakeword detection** -> [lowwi](https://github.com/CLFML/lowwi)
- ~**wakeword trainer** -> [dscripka's trainer](https://github.com/dscripka/openWakeWord) with [dscripka's synthetic data generator](https://github.com/dscripka/piper-sample-generator)~
- **wakeword trainer** -> [livekit's trainer](https://github.com/livekit/livekit-wakeword)

## how to build

- clone the repo -> `git clone --recurse-submodules https://github.com/bhu1-103/desktop-assistant.git`
- move into the directory -> `cd desktop-assistant`
- download the required models -> `./scripts/download-models.sh`
- run the build command -> `./scripts/build.sh`
- run the program -> `./build/friday`
