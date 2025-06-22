decode: rygar_decode
	./rygar_decode

rygar_decode: rygar_decode.c
	gcc -Wall -Wextra -ggdb -o rygar_decode rygar_decode.c

clean:
	rm *.hex *.png rygar_decode

.PHONY: clean decode
