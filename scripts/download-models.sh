#!/usr/bin/sh

set -e #for reroll
mkdir -p models

RESET='\033[0m'
YELLOW='\033[93m'
GREEN='\033[32m'
CYAN='\033[36m'

echo -e "${GREEN}Syncing friday.onnx${RESET}"
curl -sSL --fail -o models/friday.onnx https://github.com/bhu1-103/desktop-assistant/releases/download/v-0.1/friday.onnx
echo -e "${GREEN}Syncing melspectrogram.onnx${RESET}"
curl -sSL --fail -o models/melspectrogram.onnx https://raw.githubusercontent.com/CLFML/lowwi/46409651a9ed78912167ce3f24e7d5df97e24c81/models/melspectrogram.onnx
echo -e "${GREEN}Syncing embedding_model.onnx${RESET}"
curl -sSL --fail -o models/embedding_model.onnx https://raw.githubusercontent.com/CLFML/lowwi/46409651a9ed78912167ce3f24e7d5df97e24c81/models/embedding_model.onnx
echo -e "${CYAN}Done syncing models${RESET}"
echo -e "${YELLOW}Run ./scripts/build.sh now${RESET}"
echo -e "${GREEN}Syncing ggml-tiny.en-q5_1.bin${RESET}"
wget -P models/ https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.en-q5_1.bin
echo -e "${GREEN}Syncing music-controls_embeddings.lua ${RESET}"
curl -sSL --fail -o models/friday.onnx https://github.com/bhu1-103/desktop-assistant/releases/download/v-0.1/music-controls_embeddings.lua
