#!/bin/bash
echo Starting to compile Dirty-Bit
mkdir bin
clang++ -arch x86_64 -std=c++1y -stdlib=libc++ -w -O2 \
    -o bin/dirty-bit \
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

