#!/bin/sh

build_make() {
    BuildDir=$1
    if [ -d ${BuildDir} ]; then
	rm -fr ${BuildDir}
    fi
    mkdir ${BuildDir}
    cd ${BuildDir}
    cmake ..
    make
    cd ..
    if [ -d ${BuildDir} ]; then
	rm -fr ${BuildDir}
    fi
}

build_make build

