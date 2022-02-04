concat: concat.c
	$(CC) -Wall -Werror concat.c -o concat

concat-test: concat
	./concat makefile concat.c test.out | tee test.log
	hexdump -C test.out
	cat test.log

tomcrypt:
	make -C libtomcrypt-1.18.2/

iup:
	make -C iup/

ui:
	$(CC) ui_test.c  -o ui_test iup/lib/Linux510_64/libiup.a -Iiup/include/ -lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangocairo-1.0 -lcairo -lgdk_pixbuf-2.0 -lm -lX11

run_ui: ui
	./ui_test

play:
	$(CC) button_test.c  -o button_test iup/lib/Linux510_64/libiup.a -Iiup/include/ -lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangocairo-1.0 -lcairo -lgdk_pixbuf-2.0 -lm -lX11
	./button_test
main:
	$(CC) main.c  -o self-decrypt iup/lib/Linux510_64/libiup.a -Iiup/include/ -lgtk-3 -lgdk-3 -lglib-2.0 -lgobject-2.0 -lpango-1.0 -lpangocairo-1.0 -lcairo -lgdk_pixbuf-2.0 -lm -lX11

test: main
	./self-decrypt
