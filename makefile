ifeq ($(OS), Windows_NT)
	LIBS=iup/lib/mingw4/libiup.a -lgdi32 -lcomdlg32 -lcomctl32 -luuid -loleaut32 -lole32 $(shell pkg-config --libs libsodium)
	CFLAGS=$(shell pkg-config --cflags libsodium) -Iiup/include -static -Os
	ENV=MINGW4=/mingw64 TEC_UNAME=mingw4
else
	echo "Platform not supported!"
	exit 1
endif

main: iup extract main.c concat
	$(CC) main.c  -o tmp_main $(CFLAGS) $(LIBS) 
	./concat tmp_main.exe extract.exe self-decrypt.exe

concat: concat.c
	$(CC) -Wall -Werror concat.c -o concat

concat-test: concat
	./concat makefile concat.c test.out | tee test.log
	hexdump -C test.out
	cat test.log

.PHONY: iup
iup:
	$(ENV) CFLAGS=-Os make -C iup/src

.PHONY: clean
clean:
	rm -rf *.exe *.o iup/obj

test: main
	./self-decrypt

extract: extract.c
	$(CC) extract.c -o extract $(CFLAGS) $(LIBS)

extract-test: extract
	./extract
