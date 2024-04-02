gcc $1 -lm `pkg-config --cflags --libs opencv`
./a.out ${@:2}
rm -rf ./a.out
