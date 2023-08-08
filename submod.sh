#!/bin/bash
set -ex
git submodule update --init --recursive
cd circle-stdlib/
git checkout e318f89 # Needed to support Circle develop?
cd -
cd circle-stdlib/libs/circle
git checkout ec09d7e # develop
cd -
cd circle-stdlib/libs/circle-newlib
git checkout 48bf91d # needed for circle ec09d7e
cd -
cd Synth_Dexed 
git checkout c9f5274
cd -
