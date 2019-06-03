#include <stdint.h>
#include "rygar_roms.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define BYTES_PER_PIXEL (3)

uint8_t buf[256 * 256 * BYTES_PER_PIXEL];
uint8_t buf_x4[1024 * 1024 * BYTES_PER_PIXEL];

void decode(int width, int height, uint8_t* rom0, uint8_t* rom1, uint8_t* rom2, uint8_t* rom3) {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int tile_index = (y * width) + x;
      for (int yy = 0; yy < 8; yy++) {
        int tile_addr = (tile_index * 32) + (yy * 4);
        uint8_t bm0 = rom0[tile_addr + 0];
        uint8_t bm1 = rom0[tile_addr + 1];
        uint8_t bm2 = rom0[tile_addr + 2];
        uint8_t bm3 = rom0[tile_addr + 3];
        int py = y*8 + yy;
        int px = x*8;
        uint8_t* ptr = buf + (py*width*8 + px) * BYTES_PER_PIXEL;
        for (int xx = 0; xx < 8; xx++) {
          uint8_t pen = ((bm3>>xx)&1) | ((bm2>>xx)&1<<1) | (((bm1>>xx)&1)<<2) | (((bm0>>xx)&1)<<3);
          uint8_t c = pen << 5;
          *ptr++ = c;
          *ptr++ = c;
          *ptr++ = c;
        }
      }
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

int main() {
  decode(32, 32, dump_vid_6k, dump_vid_6j, dump_vid_6h, dump_vid_6g);
  resize();
  stbi_write_png("rygar_sprites.png", 1024, 1024, 3, buf_x4, 1024*3);
  return 0;
}
