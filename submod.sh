#!/bin/bash
set -ex
#
# Update top-level modules as a baseline
git submodule update --init --recursive
#
# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git reset --hard
git checkout 3bd135d
git submodule update --init --recursive
cd -
#
# Optional update submodules explicitly
cd circle-stdlib/libs/circle
git reset --hard
git checkout tags/Step49
cd -
cd circle-stdlib/libs/circle-newlib
#git checkout develop
cd -
#
# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git reset --hard
git checkout c9f5274
cd -
