ifeq ($(OS), Windows_NT)
	LIBS=iup/lib/mingw4/libiup.a -Iiup/include -lgdi32 -lcomdlg32 -lcomctl32 -luuid -loleaut32 -lole32
	ENV=MINGW4=/mingw64 TEC_UNAME=mingw4
else
	ENV=""
	LIBS=iup/lib/linux510_64/libiup.a -Iiup/include/ -lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangocairo-1.0 -lcairo -lgdk_pixbuf-2.0 -lm -lx11
endif

concat: concat.c
	$(CC) -Wall -Werror concat.c -o concat

concat-test: concat
	./concat makefile concat.c test.out | tee test.log
	hexdump -C test.out
	cat test.log

.PHONY: tomcrypt
tomcrypt:
	make -C libtomcrypt-1.18.2/

.PHONY: iup
iup:
	echo "building for "
	echo $(OS)
	$(ENV) make -C iup/src
	#$(ENV) make -C iup/srcfiledlg/

ui:
	$(CC) ui_test.c  -o ui_test $(LIBS)

run_ui: ui
	./ui_test

play:
	$(CC) button_test.c  -o button_test $(LIBS)
	./button_test
main: tomcrypt iup
	$(CC) main.c  -o self-decrypt $(LIBS) 

test: main
	./self-decrypt
