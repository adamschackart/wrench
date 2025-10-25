if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

if [ ! -d "stb" ]; then
    git clone https://github.com/nothings/stb.git
fi

cc -g -fPIC -c -o wren.o wren.c

cc -g -lm -ldl -I. -Iwren/src/include -o run_wren main.c wren.o &
cc -g -lm -ldl -lc++ -I. -Iwren/src/include -std=c++17 -fPIC -shared -o file.so file.cpp wren.o &
cc -g -lm -ldl -I. -Iwren/src/include -fPIC -shared -o image.so image.c wren.o &
