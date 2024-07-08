echo "Compiling main.c"
gcc -c ./src/main.c -o ./build/main.o

echo "Compiling spng.c"
gcc -c ./src/spng.c -o ./build/spng.o

echo "Linking"
gcc ./build/main.o ./build/spng.o -lm -lz -o sobel


