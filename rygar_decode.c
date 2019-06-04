#include <stdint.h>
#include "rygar_roms.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define BYTES_PER_PIXEL (3) // RGB
#define TILE_WIDTH (8)
#define TILE_HEIGHT (8)

uint8_t buf[256 * 256 * BYTES_PER_PIXEL];
uint8_t buf_x4[1024 * 1024 * BYTES_PER_PIXEL];

/* 	8,8, // pixel width of each element */
/* 	RGN_FRAC(1,1), // total number of elements, or RGN_FRAC( */
/* 	4, // number of bitplanes */
/* 	{ 0, 1, 2, 3 }, // bit offset of each bitplane */
/* 	{ 0, 4, 8, 12, 16, 20, 24, 28 }, // bit offset of each horizontal pixel */
/* 	{ 0, 32, 64, 96, 128, 160, 192, 256 }, // bit offset of each vertical pixel */
/* 	4*8*8 // distance between two consecutive elements (in bits) */
void draw_tile(int width, int height, int x, int y, int tile_index, uint8_t* data) {
  for (int yy = 0; yy < TILE_HEIGHT; yy++) {
    // Each tile is 32 bytes apart
    // Each row is 4 bytes apart
    int tile_addr = tile_index*32 + yy*4;

    int py = y*TILE_HEIGHT + yy;
    int px = x*TILE_WIDTH;
    uint8_t* ptr = buf + (py*width*8 + px) * BYTES_PER_PIXEL;

    for (int xx = 0; xx < TILE_WIDTH/2; xx++) {
      // Low byte is the first pixel.
      uint8_t lo = data[tile_addr + xx] & 0xf;

      // High byte is the second pixel.
      uint8_t hi = (data[tile_addr + xx] >> 4) & 0xf;

      uint8_t c = hi << 5;
      *ptr++ = c;
      *ptr++ = c;
      *ptr++ = c;

      uint8_t d = lo << 5;
      *ptr++ = d;
      *ptr++ = d;
      *ptr++ = d;
    }
  }
}

void decode_tiles(int width, int height, uint8_t* data) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int tile_index = y*width + x;
      draw_tile(width, height, x, y, tile_index, data);
    }
  }
}

void resize() {
  for (uint32_t y = 0; y < 1024; y++) {
    for (uint32_t x = 0; x < 1024; x++) {
      uint32_t x0 = x>>2;
      uint32_t y0 = y>>2;
      uint8_t* src = buf + (y0*256 + x0) * BYTES_PER_PIXEL;
      uint8_t* dst = buf_x4 + (y*1024 + x)  * BYTES_PER_PIXEL;
      for (int i = 0; i < BYTES_PER_PIXEL; i++) {
        *dst++ = *src++;
      }
    }
  }
}

void decode_rom(uint8_t* data, char* name) {
  decode_tiles(32, 32, data);
  resize();
  stbi_write_png(name, 1024, 1024, 3, buf_x4, 1024*3);
}

int main() {
  decode_rom(dump_cpu_8k, "cpu_8k.png");

  decode_rom(dump_vid_6k, "vid_6k.png");
  decode_rom(dump_vid_6j, "vid_6j.png");
  decode_rom(dump_vid_6h, "vid_6h.png");
  decode_rom(dump_vid_6g, "vid_6g.png");

  decode_rom(dump_vid_6p, "vid_6p.png");
  decode_rom(dump_vid_6o, "vid_6o.png");
  decode_rom(dump_vid_6n, "vid_6n.png");
  decode_rom(dump_vid_6l, "vid_6l.png");

  decode_rom(dump_vid_6f, "vid_6f.png");
  decode_rom(dump_vid_6e, "vid_6e.png");
  decode_rom(dump_vid_6c, "vid_6c.png");
  decode_rom(dump_vid_6b, "vid_6b.png");

  return 0;
}
