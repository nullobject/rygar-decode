.PHONY: clean view

view: rygar_sprites.png
	open rygar_sprites.png

rygar_sprites.png: rygar_decode
	./rygar_decode

rygar_decode: rygar_decode.c
	gcc rygar_decode.c -o rygar_decode

clean:
	rm rygar_decode rygar_sprites.png
