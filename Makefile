.PHONY: clean view

view: rygar_tx_tiles.png
	open rygar_tx_tiles.png

rygar_tx_tiles.png: rygar_decode
	./rygar_decode

rygar_decode: rygar_decode.c
	gcc rygar_decode.c -o rygar_decode

clean:
	rm *.png rygar_decode
