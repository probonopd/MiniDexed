#!/bin/bash
set -ex

# Update top-level modules as a baseline
git submodule update --init --recursive -f

# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git checkout -f --recurse-submodules 1111eee # Matches Circle Step49
cd -

# Optional update submodules explicitly
cd circle-stdlib/libs/circle
git checkout -f --recurse-submodules f18c60fa38042ea7132533e658abfafd5bd63435
cd -
#cd circle-stdlib/libs/circle-newlib
#git checkout develop
#cd -

# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git checkout -f 8ae8bb1cea7cffa07d2d258218130b90b744893e
cd -
