if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

clang -g -c -o wren.o wren.c

clang -g -lm -ldl -I. -Iwren/src/include -o run_wren main.c wren.o
clang -g -lm -ldl -lc++ -I. -Iwren/src/include -std=c++17 -fPIC -shared -o file.so file.cpp wren.o
