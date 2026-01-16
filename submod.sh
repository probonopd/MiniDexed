#!/bin/bash
set -ex

# Update top-level modules as a baseline
git submodule update --init --recursive -f

# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git checkout -f --recurse-submodules v17.2
cd -

# Optional update submodules explicitly
cd circle-stdlib/libs/circle
git checkout -f --recurse-submodules b42d060
cd -
#cd circle-stdlib/libs/circle-newlib
#git checkout develop
#cd -

# Use fixed master branch of Synth_Dexed
#cd Synth_Dexed/
#git checkout -f a02b5c0bf2da132f49a923f9c69796220a8ea93f
#cd -
