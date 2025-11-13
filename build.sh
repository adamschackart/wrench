if [ ! -d "stb" ]; then
    git clone https://github.com/nothings/stb.git
fi

if [ ! -d "tinydir" ]; then
    git clone https://github.com/cxong/tinydir.git
fi

if [ ! -d "wren" ]; then
    git clone https://github.com/wren-lang/wren.git
    python3 -B wren/util/generate_amalgamation.py >> wren.c
fi

if [ ! -d "zip" ]; then
    git clone https://github.com/kuba--/zip.git
fi

cc -O0 -g -fPIC -c -o wren.o wren.c

cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -o run_wren main.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o file.so file.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o image.so image.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o util.so util.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vector.so vector.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o vm.so vm.c wren.o -lm -ldl &
cc -O0 -g -I. -Iwren/src/include -Wno-format-zero-length -Wno-format-truncation -fPIC -shared -o zip.so zip.c wren.o -lm -ldl &

wait
