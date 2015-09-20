echo Starting to compile Dirty-Bit in debug mode
mkdir bin 2>/dev/null
cd bin
rm dirty-bit 2>/dev/null
c++ -g -O0 -arch x86_64 -stdlib=libc++ -o dirty-bit \
 ../perft.cpp \
 ../engine.cpp \
 ../evaluate.cpp \
 ../bitboards.cpp \
 ../SEE.cpp \
 ../main.cpp \
 ../hash.cpp \
 ../board.cpp \
 ../nextmove.cpp \
 ../movegen.cpp \
 ../uci.cpp \
 ../utilities.cpp \
 ../search.cpp \
 ../debug.cpp \
 ../precompute.cpp \
 ../move.cpp \
 ../makemove.cpp
cd ..
./bin/dirty-bit
