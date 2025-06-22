#include <stdint.h>
#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "rygar_roms.h"
#include "stb_image_write.h"

#define ROM_SIZE 65536

/* step macros */
#define STEP2(start, step) (start), (start) + (step)
#define STEP4(start, step) STEP2(start, step), STEP2((start) + 2 * (step), step)
#define STEP8(start, step) STEP4(start, step), STEP4((start) + 4 * (step), step)
#define STEP16(start, step)                                                    \
  STEP8(start, step), STEP8((start) + 8 * (step), step)
#define STEP32(start, step)                                                    \
  STEP16(start, step), STEP16((start) + 16 * (step), step)
#define STEP64(start, step)                                                    \
  STEP32(start, step), STEP32((start) + 32 * (step), step)

typedef struct {
  /* tile dimensions */
  size_t tile_width;
  size_t tile_height;

  /* number of bit planes */
  size_t planes;

  /* offset arrays */
  int plane_offsets[8];
  int x_offsets[32];
  int y_offsets[32];

  /* tile size (in bytes) */
  size_t tile_size;
} tile_decode_desc_t;

/* decode descriptor for a 8x8 tile */
tile_decode_desc_t tile_8x8 = {
    .tile_width = 8,
    .tile_height = 8,
    .planes = 4,
    .plane_offsets = {STEP4(0, 1)},
    .x_offsets = {STEP8(0, 4)},
    .y_offsets = {STEP8(0, 4 * 8)},
    .tile_size = 4 * 8 /* 32 bytes */
};

/* decode descriptor for a 16x16 tile, made up of four 8x8 tiles */
tile_decode_desc_t tile_16x16 = {
    .tile_width = 16,
    .tile_height = 16,
    .planes = 4,
    .plane_offsets = {STEP4(0, 1)},
    .x_offsets = {STEP8(0, 4), STEP8(4 * 8 * 8, 4)},
    .y_offsets = {STEP8(0, 4 * 8), STEP8(4 * 8 * 8 * 2, 4 * 8)},
    .tile_size = 4 * 4 * 8 /* 128 bytes */
};

uint8_t tile_data[ROM_SIZE];
uint8_t pixel_data[ROM_SIZE];

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
void decode_tile(const tile_decode_desc_t *desc, const uint8_t *rom,
                 uint8_t *dst, size_t count) {
  for (size_t tile = 0; tile < count; tile++) {
    uint8_t *ptr = dst + (tile * desc->tile_width * desc->tile_height);

    /* clear the bytes for the next tile */
    memset(ptr, 0, desc->tile_width * desc->tile_height);

    for (size_t plane = 0; plane < desc->planes; plane++) {
      int plane_bit = 1 << (desc->planes - 1 - plane);
      int plane_offset =
          (tile * desc->tile_size * 8) + desc->plane_offsets[plane];

      for (size_t y = 0; y < desc->tile_height; y++) {
        int y_offset = plane_offset + desc->y_offsets[y];
        ptr = dst + (tile * desc->tile_width * desc->tile_height) +
              (y * desc->tile_width);

        for (size_t x = 0; x < desc->tile_width; x++) {
          if (read_bit(rom, y_offset + desc->x_offsets[x])) {
            ptr[x] |= plane_bit;
          }
        }
      }
    }

    ptr = dst + (tile * desc->tile_width * desc->tile_height);
  }
}

void write_hex(const char *filename, const void *data, size_t size) {
  FILE *fptr;
  if ((fptr = fopen(filename, "wb")) == NULL) {
    printf("Error opening file");
    exit(1);
  }
  fwrite(data, size, 1, fptr);
  fclose(fptr);
}

void decode_tiles(const tile_decode_desc_t *desc, const uint8_t *data,
                  const char *name) {
  char buf[255];
  size_t rows = 256 / desc->tile_height;
  size_t cols = 256 / desc->tile_width;
  size_t tile_bytes = desc->tile_width * desc->tile_height;
  size_t stride_bytes = 256;

  decode_tile(desc, data, (uint8_t *)&tile_data, ROM_SIZE / tile_bytes);

  // Increase the brightness of each pixel
  for (int i = 0; i < ROM_SIZE; i++) {
    tile_data[i] = tile_data[i] << 5;
  }

  // Copy tile data to pixel buffer
  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      for (size_t y = 0; y < desc->tile_height; y++) {
        void *src = tile_data + (j * cols * tile_bytes) + (i * tile_bytes) +
                    (y * desc->tile_width);
        void *dest = pixel_data + (j * cols * tile_bytes) +
                     (i * desc->tile_width) + (y * stride_bytes);
        memcpy(dest, src, desc->tile_width);
      }
    }
  }

  // Write pixel data
  snprintf(buf, sizeof(buf), "%s.png", name);
  stbi_write_png(buf, 256, 256, 1, pixel_data, stride_bytes);

  // Write raw tile data
  snprintf(buf, sizeof(buf), "%s.hex", name);
  write_hex(buf, data, ROM_SIZE);
}

int main() {
  // char
  decode_tiles(&tile_8x8, dump_cpu_8k, "cpu_8k");

  // sprite
  decode_tiles(&tile_8x8, dump_vid_6k, "vid_6k");
  decode_tiles(&tile_8x8, dump_vid_6j, "vid_6j");
  decode_tiles(&tile_8x8, dump_vid_6h, "vid_6h");
  decode_tiles(&tile_8x8, dump_vid_6g, "vid_6g");

  // bg0
  decode_tiles(&tile_16x16, dump_vid_6p, "vid_6p");
  decode_tiles(&tile_16x16, dump_vid_6o, "vid_6o");
  decode_tiles(&tile_16x16, dump_vid_6n, "vid_6n");
  decode_tiles(&tile_16x16, dump_vid_6l, "vid_6l");

  // bg1
  decode_tiles(&tile_16x16, dump_vid_6f, "vid_6f");
  decode_tiles(&tile_16x16, dump_vid_6e, "vid_6e");
  decode_tiles(&tile_16x16, dump_vid_6c, "vid_6c");
  decode_tiles(&tile_16x16, dump_vid_6b, "vid_6b");

  return 0;
}
