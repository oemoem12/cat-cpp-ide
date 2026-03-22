#!/bin/bash
set -euo pipefail
source /home/cat/Documents/trae_projects/trae/kitten-cpp/parts/cat-cpp-ide/run/environment.sh
set -x
make -j"16"
make -j"16" install DESTDIR="/home/cat/Documents/trae_projects/trae/kitten-cpp/parts/cat-cpp-ide/install"
