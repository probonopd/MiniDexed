#!/bin/bash
set -ex

# Update top-level modules as a baseline
git submodule update --init --recursive -f

# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git reset --hard
git checkout 1111eee -f # Matches Circle Step49
git submodule update --init --recursive -f
cd -

# Optional update submodules explicitly
#cd circle-stdlib/libs/circle
#git reset --hard
#git checkout tags/Step49
#cd -
#cd circle-stdlib/libs/circle-newlib
#git checkout develop
#cd -

# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git reset --hard
git checkout 2ad9e43095 -f
cd -
