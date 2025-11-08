if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

cc -g -fPIC -c -o wren.o wren.c &

if [ ! -d "stb" ]; then
    git clone https://github.com/nothings/stb.git
fi

wait

cc -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren main.c wren.o -lm -ldl &
cc -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -std=c++17 -fPIC -shared -o file.so file.cpp wren.o -lstdc++ -lm -ldl &
cc -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o image.so image.c wren.o -lm -ldl &
cc -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vector.so vector.c wren.o -lm -ldl &

wait
