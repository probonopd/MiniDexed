#!/bin/bash

echo "Updating submodules..."

# Update top-level modules as a baseline
git submodule update --init --recursive

# Use fixed master branch of circle-stdlib then re-update
cd external/circle-stdlib/
git checkout 3bd135d
git submodule update --init --recursive
cd -

# Optional update submodules explicitly
cd external/circle-stdlib/libs/circle
git checkout 4155f43
cd -

# Use fixed master branch of Synth_Dexed
cd external/Synth_Dexed/
git checkout c9f5274
cd -

echo "  Done."
