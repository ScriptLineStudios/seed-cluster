main: main.c
	gcc -o main main.c binaries/libcubiomes.a binaries/libcjson.a -lm -fwrapv