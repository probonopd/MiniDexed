#!/bin/sh

# Get voices from
# https://yamahablackboxes.com/collection/yamaha-dx7-synthesizer/patches/

mkdir -p sysex/voice/

DIR="https://yamahablackboxes.com/patches/dx7/factory"

# wget -c "${DIR}"/rom1a.syx -O sysex/voice/000000_rom1a.syx
# wget -c "${DIR}"/rom1b.syx -O sysex/voice/000001_rom1b.syx
# wget -c "${DIR}"/rom2a.syx -O sysex/voice/000002_rom2a.syx
# wget -c "${DIR}"/rom2b.syx -O sysex/voice/000003_rom2b.syx
wget -c "${DIR}"/rom3a.syx -O sysex/voice/000000_rom3a.syx
wget -c "${DIR}"/rom3b.syx -O sysex/voice/000001_rom3b.syx
wget -c "${DIR}"/rom4a.syx -O sysex/voice/000002_rom4a.syx
wget -c "${DIR}"/rom4b.syx -O sysex/voice/000003_rom4b.syx

DIR="https://yamahablackboxes.com/patches/dx7/vrc"

wget -c "${DIR}"/vrc101b.syx -O sysex/voice/000004_vrc101b.syx
wget -c "${DIR}"/vrc102a.syx -O sysex/voice/000005_vrc102a.syx
wget -c "${DIR}"/vrc102b.syx -O sysex/voice/000006_vrc102b.syx
wget -c "${DIR}"/vrc103a.syx -O sysex/voice/000007_vrc103a.syx
wget -c "${DIR}"/vrc103b.syx -O sysex/voice/000008_vrc103b.syx
wget -c "${DIR}"/vrc104a.syx -O sysex/voice/000009_vrc104a.syx
wget -c "${DIR}"/vrc104b.syx -O sysex/voice/000010_vrc104b.syx
wget -c "${DIR}"/vrc105a.syx -O sysex/voice/000011_vrc105a.syx
wget -c "${DIR}"/vrc105b.syx -O sysex/voice/000012_vrc105b.syx
wget -c "${DIR}"/vrc106a.syx -O sysex/voice/000013_vrc106a.syx
wget -c "${DIR}"/vrc106b.syx -O sysex/voice/000014_vrc106b.syx
wget -c "${DIR}"/vrc107a.syx -O sysex/voice/000015_vrc107a.syx
wget -c "${DIR}"/vrc107b.syx -O sysex/voice/000016_vrc107b.syx
wget -c "${DIR}"/vrc108a.syx -O sysex/voice/000017_vrc108a.syx
wget -c "${DIR}"/vrc108b.syx -O sysex/voice/000018_vrc108b.syx
wget -c "${DIR}"/vrc109a.syx -O sysex/voice/000019_vrc109a.syx
wget -c "${DIR}"/vrc109b.syx -O sysex/voice/000020_vrc109b.syx
wget -c "${DIR}"/vrc110a.syx -O sysex/voice/000021_vrc110a.syx
wget -c "${DIR}"/vrc110b.syx -O sysex/voice/000022_vrc110b.syx
wget -c "${DIR}"/vrc111a.syx -O sysex/voice/000023_vrc111a.syx
wget -c "${DIR}"/vrc111b.syx -O sysex/voice/000024_vrc111b.syx
wget -c "${DIR}"/vrc112a.syx -O sysex/voice/000025_vrc112a.syx
wget -c "${DIR}"/vrc112b.syx -O sysex/voice/000026_vrc112b.syx
