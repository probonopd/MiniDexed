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
cd circle-stdlib/libs/circle
git reset --hard
git checkout --recurse-submodules f18c60fa38042ea7132533e658abfafd5bd63435
cd -
#cd circle-stdlib/libs/circle-newlib
#git checkout develop
#cd -

# Use fixed master branch of Synth_Dexed
cd Synth_Dexed/
git reset --hard
git checkout 3c683fc801 -f
cd -
