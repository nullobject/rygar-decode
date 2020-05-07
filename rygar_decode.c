#include <stdint.h>
#include "rygar_roms.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define BYTES_PER_PIXEL (1) // RGB
#define NUM_TILES 128
#define CHAR_ROM_SIZE (8*8*NUM_TILES)

/* step macros */
#define STEP2(start, step) (start), (start)+(step)
#define STEP4(start, step) STEP2(start, step), STEP2((start)+2*(step), step)
#define STEP8(start, step) STEP4(start, step), STEP4((start)+4*(step), step)
#define STEP16(start, step) STEP8(start, step), STEP8((start)+8*(step), step)
#define STEP32(start, step) STEP16(start, step), STEP16((start)+16*(step), step)
#define STEP64(start, step) STEP32(start, step), STEP32((start)+32*(step), step)

uint8_t buf[256 * 256 * BYTES_PER_PIXEL];
uint8_t buf_x4[1024 * 1024 * BYTES_PER_PIXEL];

typedef struct {
  /* tile dimensions */
  int tile_width;
  int tile_height;

  /* number of bit planes */
  int planes;

  /* offset arrays */
  int plane_offsets[8];
  int x_offsets[32];
  int y_offsets[32];

  /* tile size (in bytes) */
  int tile_size;
} tile_decode_desc_t;

/**
 * Reads a single bit value from the tile ROM at the given offset.
 *
 * The offset value is specified in bits, and can span across multiple bytes.
 */
static inline int read_bit(const uint8_t *rom, int offset) {
	return rom[offset / 8] & (0x80 >> (offset % 8));
}

/**
 * Decodes the given tile ROM to 8-bit pixel data.
 *
 * The decoded data takes up more space that the original tile ROM, but the
 * advantage is that you don't have to jump around to get the pixel data. You
 * can just iterate through the pixels sequentially, as each pixel is
 * represented by only one byte.
 */
void tile_decode(const tile_decode_desc_t *desc, uint8_t *rom, uint8_t *dst, int count) {
  for (int tile = 0; tile < count; tile++) {
    uint8_t *ptr = dst + (tile * desc->tile_width * desc->tile_height);

    /* clear the bytes for the next tile */
		memset(ptr, 0, desc->tile_width * desc->tile_height);

    for (int plane = 0; plane < desc->planes; plane++) {
      int plane_bit = 1 << (desc->planes - 1 - plane);
			int plane_offset = (tile * desc->tile_size * 8) + desc->plane_offsets[plane];

			for (int y = 0; y < desc->tile_height; y++) {
				int y_offset = plane_offset + desc->y_offsets[y];
				ptr = dst + (tile * desc->tile_width * desc->tile_height) + (y * desc->tile_width);

				for (int x = 0; x < desc->tile_width; x++) {
					if (read_bit(rom, y_offset + desc->x_offsets[x])) {
            ptr[x] |= plane_bit;
          }
				}
			}
    }

    ptr = dst + (tile * desc->tile_width * desc->tile_height);

    for (int i = 0; i < 8*8; i++) {
      // Multiply the pixel value by 32.
      ptr[i] = ptr[i] << 5;
    }
  }
}

void decode_rom(uint8_t *data, char *name) {
  /* decode descriptor for a 8x8 tile */
  tile_decode_desc_t tile_decode_8x8 = {
    .tile_width = 8,
    .tile_height = 8,
    .planes = 4,
    .plane_offsets = { STEP4(0, 1) },
    .x_offsets = { STEP8(0, 4) },
    .y_offsets = { STEP8(0, 4 * 8) },
    .tile_size = 4 * 8 /* 32 bytes */
  };

  uint8_t char_rom[CHAR_ROM_SIZE];
  tile_decode(&tile_decode_8x8, data, (uint8_t *)&char_rom, NUM_TILES);

  FILE *fptr;
  if ((fptr = fopen("tiles.hex", "wb")) == NULL) {
    printf("Error opening file");
    exit(1);
  }
  fwrite(&char_rom, CHAR_ROM_SIZE, 1, fptr);
  fclose(fptr);
}

int main() {
  decode_rom(dump_cpu_8k, "cpu_8k.png");

  /* decode_rom(dump_vid_6k, "vid_6k.png"); */
  /* decode_rom(dump_vid_6j, "vid_6j.png"); */
  /* decode_rom(dump_vid_6h, "vid_6h.png"); */
  /* decode_rom(dump_vid_6g, "vid_6g.png"); */

  /* decode_rom(dump_vid_6p, "vid_6p.png"); */
  /* decode_rom(dump_vid_6o, "vid_6o.png"); */
  /* decode_rom(dump_vid_6n, "vid_6n.png"); */
  /* decode_rom(dump_vid_6l, "vid_6l.png"); */

  /* decode_rom(dump_vid_6f, "vid_6f.png"); */
  /* decode_rom(dump_vid_6e, "vid_6e.png"); */
  /* decode_rom(dump_vid_6c, "vid_6c.png"); */
  /* decode_rom(dump_vid_6b, "vid_6b.png"); */

  return 0;
}
