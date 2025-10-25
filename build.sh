if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

cc -g -fPIC -c -o wren.o wren.c &

if [ ! -d "stb" ]; then
    git clone https://github.com/nothings/stb.git
fi

wait

cc -g -I. -Iwren/src/include -o run_wren main.c wren.o -lm -ldl &
cc -g -I. -Iwren/src/include -std=c++17 -fPIC -shared -o file.so file.cpp wren.o -lc++ -lm -ldl &
cc -g -I. -Iwren/src/include -fPIC -shared -o image.so image.c wren.o -lm -ldl &
