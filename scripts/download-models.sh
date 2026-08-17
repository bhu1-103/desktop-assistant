#!/usr/bin/sh

set -e
mkdir -p models

echo "Syncing friday.onnx, melspectrogram.onnx and embedding_model.onnx"
curl -L --fail -o models/friday.onnx https://github.com/bhu1-103/desktop-assistant/releases/download/v-0.1/friday.onnx
curl -L --fail -o models/melspectrogram.onnx https://raw.githubusercontent.com/CLFML/lowwi/46409651a9ed78912167ce3f24e7d5df97e24c81/models/melspectrogram.onnx
curl -L --fail -o models/embedding_model.onnx https://raw.githubusercontent.com/CLFML/lowwi/46409651a9ed78912167ce3f24e7d5df97e24c81/models/embedding_model.onnx
echo "Done syncing models"
