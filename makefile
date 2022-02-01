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
