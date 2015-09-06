echo Starting to compile Dirty-Bit
mkdir bin
c++ -O2 -w -c -o bin/perft.o perft.cpp
c++ -O2 -w -c -o bin/engine.o engine.cpp
c++ -O2 -w -c -o bin/evaluate.o evaluate.cpp
c++ -O2 -w -c -o bin/bitboards.o bitboards.cpp
c++ -O2 -w -c -o bin/SEE.o SEE.cpp
c++ -O2 -w -c -o bin/main.o main.cpp
c++ -O2 -w -c -o bin/hash.o hash.cpp
c++ -O2 -w -c -o bin/board.o board.cpp
c++ -O2 -w -c -o bin/nextmove.o nextmove.cpp
c++ -O2 -w -c -o bin/movegen.o movegen.cpp
c++ -O2 -w -c -o bin/uci.o uci.cpp
c++ -O2 -w -c -o bin/utilities.o utilities.cpp
c++ -O2 -w -c -o bin/search.o search.cpp
c++ -O2 -w -c -o bin/debug.o debug.cpp
c++ -O2 -w -c -o bin/precompute.o precompute.cpp
c++ -O2 -w -c -o bin/move.o move.cpp
c++ -O2 -w -c -o bin/makemove.o makemove.cpp
echo Done compiling each cpp file
echo Now putting them all together
cd bin
c++ -arch x86_64 -stdlib=libc++ -w -O2 -o dirty-bit perft.o engine.o evaluate.o bitboards.o SEE.o main.o hash.o board.o nextmove.o movegen.o uci.o utilities.o search.o debug.o precompute.o move.o makemove.o 
cd ..
