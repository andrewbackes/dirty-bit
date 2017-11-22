#!/bin/bash -xe
cd "$(dirname "$0")"
echo Starting to compile Dirty-Bit
mkdir -p build
if [[ -z "${TRAVIS_OS_NAME}" ]]; then
    SUFFIX=-${TRAVIS_OS_NAME}
fi
clang++ -arch x86_64 -std=c++1y -stdlib=libc++ -w -O2 \
    -o build/dirty-bit${SUFFIX} \
    perft.cpp \
    engine.cpp \
    evaluate.cpp \
    bitboards.cpp \
    SEE.cpp \
    main.cpp \
    hash.cpp \
    board.cpp \
    nextmove.cpp \
    movegen.cpp \
    uci.cpp \
    utilities.cpp \
    search.cpp \
    debug.cpp \
    precompute.cpp \
    move.cpp \
    makemove.cpp

