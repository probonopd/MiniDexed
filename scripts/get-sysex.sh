#!/bin/bash

echo "Fetching sysex files..."

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

mkdir -p sysex/voice/

DIR="https://yamahablackboxes.com/patches/dx7/factory"
FILES=(
  # "rom1a"
  # "rom1b"
  # "rom2a"
  # "rom2b"
  "rom3a"
  "rom3b"
  "rom4a"
  "rom4b"
)
for i in "${!FILES[@]}"; do
    index=$((i + 1))
    echo "  Downloading ${FILES[$i]}..."
    wget -cq "${DIR}/${FILES[$i]}.syx" -O "sysex/voice/$(printf '%06d' $index)_${FILES[$i]}.syx"
done

DIR="https://yamahablackboxes.com/patches/dx7/vrc"
FILES=(
  "vrc101b"
  "vrc102a"
  "vrc102b"
  "vrc103a"
  "vrc103b"
  "vrc104a"
  "vrc104b"
  "vrc105a"
  "vrc105b"
  "vrc106a"
  "vrc106b"
  "vrc107a"
  "vrc107b"
  "vrc108a"
  "vrc108b"
  "vrc109a"
  "vrc109b"
  "vrc110a"
  "vrc110b"
  "vrc111a"
  "vrc111b"
  "vrc112a"
  "vrc112b"
)
for i in "${!FILES[@]}"; do
    index=$((i + 5)) # Adjust the starting index as needed
    echo "  Downloading ${FILES[$i]}..."
    wget -cq "${DIR}/${FILES[$i]}.syx" -O "sysex/voice/$(printf '%06d' $index)_${FILES[$i]}.syx"
done

echo "  Done."
