#!/bin/bash
set -ex
#
# Update top-level modules as a baseline
git submodule update --init --recursive
#
# Use fixed master branch of circle-stdlib then re-update
cd circle-stdlib/
git checkout v16.6
git submodule update --init --recursive
cd -
#
# Optional update submodules explicitly
cd circle-stdlib/libs/circle
git checkout fff3764
cd -
cd circle-stdlib/libs/circle-newlib
#git checkout develop
cd -
#
# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git checkout c9f5274
cd -
