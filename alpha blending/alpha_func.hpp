#pragma once

#include <stdint.h>
#include <malloc.h>
#include <x86intrin.h>

struct __attribute__ ((packed)) File_header 
{
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

struct  __attribute__ ((packed)) Bitmap_info_header 
{
    uint32_t bi_size;
    uint32_t bi_width;
    uint32_t bi_height;
    uint16_t bi_planes;
    uint16_t bi_bitcount;
    uint32_t bi_compression;
    uint32_t bi_size_image;
    int32_t bi_xpxl_per_meter;
    int32_t bi_ypxl_per_meter;
    uint32_t bi_clr_used;
    uint32_t bi_clr_important;
};

struct  __attribute__ ((packed)) ARGB_info 
{
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t window_color_space;
    uint32_t unused_rgb[16];
};


struct Image
{
    uint32_t bi_size;
    uint32_t size;
    uint32_t offset;
  
    uint32_t *pixels;
    uint32_t height;
    uint32_t width;

    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
};

uint8_t load_bmp_from_file (Image *image, const char *file_path);
void    print128_int       (__m128i var);
bool    construct_bmp      (const Image *image, const char *file_path);
uint8_t scalar             (Image *out, const Image *foreground, const Image *background, const int max_iter);
uint8_t vector_sse         (Image *out, const Image *foreground, const Image *background, const int max_iter);
uint8_t vector_avx2        (Image *out, const Image *foreground, const Image *background, const int max_iter);

void testing (uint8_t (*test_func) (Image *out, const Image *foreground, const Image *background, const int max_iter),
             const char *test_name,                                                                                   
             int test_count,                                                                                          
             Image *out,                                                                                               
             Image *foreground,
             Image *background);