#include <assert.h>
#include <x86intrin.h>
#include <string.h>
#include <chrono>
#include "alpha_func.hpp"

#define    ADD4(a, b) _mm_add_epi32  (a, b) 
#define    AND4(a, b) _mm_and_si128  (a, b)
#define    SRL4(a, b) _mm_srl_epi32  (a, b)
#define    SUB4(a, b) _mm_sub_epi32  (a, b)
#define     OR4(a, b) _mm_or_si128   (a, b)
#define  MULLO4(a, b) _mm_mullo_epi32(a, b)
#define    DUP4(a)    a, a, a, a

#define    ADD8(a, b) _mm256_add_epi32  (a, b) 
#define    AND8(a, b) _mm256_and_si256  (a, b)
#define    SRL8(a, b) _mm256_srl_epi32  (a, b)
#define    SUB8(a, b) _mm256_sub_epi32  (a, b)
#define     OR8(a, b) _mm256_or_si256  (a, b)
#define  MULLO8(a, b) _mm256_mullo_epi32(a, b)
#define    DUP8(a)    a, a, a, a, a, a, a, a

uint8_t load_bmp_from_file (Image *image, const char *file_path)
{
    assert(image);
    assert(file_path);

    uint8_t error = 0;
    size_t  size = 0;

    File_header        file_header = {};
    Bitmap_info_header bi_header  {};
    ARGB_info          argb_info  {};

    FILE *bmp_file = fopen(file_path, "rb");
    if (!bmp_file)
      return error | (1 << 1);

    fseek(bmp_file, 0, SEEK_SET);
    size = fread(&file_header, 1, sizeof(File_header), bmp_file);

    if (file_header.file_type != 0x424D) error | (1 << 0);

    image->size = file_header.file_size;
    image->offset = file_header.offset;
    size = fread (&bi_header, 1, sizeof(Bitmap_info_header), bmp_file);

    if (bi_header.bi_width  <= 0) error | (1 << 1);
    if (bi_header.bi_height <= 0) error | (1 << 2);

    if (bi_header.bi_planes   != 1)  error | (1 << 3);
    if (bi_header.bi_bitcount != 32) error | (1 << 4);

    if (error)
    {
        fclose (bmp_file);
        return error;
    }

    size = fread(&argb_info, 1, sizeof(ARGB_info), bmp_file);

    image->pixels = (uint32_t *)
      calloc((size_t) (bi_header.bi_width*bi_header.bi_height), sizeof(uint32_t));
    
    if (!image->pixels) return error | (1 << 5);

    image->height = (unsigned) bi_header.bi_height;
    image->width  = (unsigned) bi_header.bi_width;

    image->bi_size = bi_header.bi_size;
    image->red_mask   = argb_info.red_mask;
    image->green_mask = argb_info.green_mask;
    image->blue_mask  = argb_info.blue_mask;
    image->alpha_mask = argb_info.alpha_mask;

    size = fread(image->pixels, (size_t) (image->width*image->height), sizeof(uint32_t), bmp_file);

    printf ("size:   %08x %d\n", bi_header.bi_size, bi_header.bi_size);
    printf ("width:  %08x %d\n", bi_header.bi_width, bi_header.bi_width);
    printf ("height: %08x %d\n\n", bi_header.bi_height, bi_header.bi_height);

    fclose(bmp_file);
    return  error;
}



uint8_t scalar (Image *out, const Image *foreground, const Image *background, const int max_iter)
  {
    assert(out);
    assert(foreground);
    assert(background);

    printf ("size: %d %d\n", foreground->width, background->width);
    assert(foreground->width  == background->width );
    assert(foreground->height == background->height);

    unsigned width  = background->width;
    unsigned height = background->height;

    out->width  = width;
    out->height = height;

    out->bi_size    = foreground->bi_size;
    out->size       = foreground->size;
    out->offset     = foreground->offset;

    out->pixels = (uint32_t *) calloc((size_t) (width*height), sizeof(uint32_t));
    
    if (!out->pixels)
        return 1 << 1;

    uint32_t foreground_pxl = 0;
    uint32_t background_pxl = 0;
    uint32_t out_pxl = 0;

    uint32_t red_blue = 0;
    uint32_t green    = 0;

    for (int iter = 0; iter < max_iter; iter++)
    {
        for (unsigned y = 0; y < height; ++y)
            for (unsigned x = 0; x < width; ++x)
            {
                foreground_pxl = foreground->pixels[x + y*width];
                background_pxl = background->pixels[x + y*width];

                uint8_t alpha = foreground_pxl >> 24;

                red_blue  = background_pxl & 0xFFFF00FF;
                green     = background_pxl & 0xFF00FF00;
                red_blue += (((foreground_pxl & 0xFFFF00FF) - red_blue) * alpha) >> 8;
                green    += (((foreground_pxl & 0xFF00FF00) -    green) * alpha) >> 8;

                out_pxl = (red_blue & 0xFFFF00FF)  | (green & 0xFF00FF00);

                out->pixels[x + y*width] = out_pxl;
            }
    }
    return 0;
}


uint8_t vector_sse (Image *out, const Image *foreground, const Image *background, int max_iter)
{
    assert(out);
    assert(foreground);
    assert(background);

    printf ("size: %d %d\n", foreground->width, background->width);
    assert(foreground->width  == background->width );
    assert(foreground->height == background->height);

    unsigned width  = background->width;
    unsigned height = background->height;

    out->width  = width;
    out->height = height;

    out->bi_size    = foreground->bi_size;
    out->size       = foreground->size;
    out->offset     = foreground->offset;

    out->pixels = (uint32_t *) calloc((size_t) (width*height), sizeof(uint32_t));
    
    if (!out->pixels)
        return 1 << 1;

    __m128i red_blue = _mm_set_epi32 (DUP4 (0));
    __m128i eight    = _mm_set_epi32 (0, 0, 0, 8);
    __m128i green    = _mm_set_epi32 (DUP4 (0));
    __m128i out_pxl  = _mm_set_epi32 (DUP4 (0));

    __m128i twenty_four    = _mm_set_epi32 (0, 0, 0, 24);
    __m128i foreground_pxl = _mm_set_epi32 (DUP4 (0));
    __m128i background_pxl = _mm_set_epi32 (DUP4 (0));
    __m128i rb_mask        = _mm_set_epi32 (DUP4 (0x00FF00FF));
    __m128i g_mask         = _mm_set_epi32 (DUP4 (0xFF00FF00));
    __m128i alpha          = _mm_set_epi32 (DUP4 (0));


    for (int iter = 0; iter < max_iter; iter++)
    {
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width - 4; x += 4)
        {
            foreground_pxl = _mm_loadu_si128 ((__m128i *) (foreground->pixels + x + y*width));
            background_pxl = _mm_loadu_si128 ((__m128i *) (background->pixels + x + y*width));

            alpha = _mm_srl_epi32 (foreground_pxl, twenty_four);
            
            red_blue  = AND4 (background_pxl, rb_mask);
            green     = AND4 (background_pxl, g_mask);
            
            // red_blue += (((foreground_pxl & 0xFFFF00FF) - red_blue) * alpha) >> 8;
            red_blue = ADD4 (red_blue, SRL4 (MULLO4 (SUB4 (AND4 (foreground_pxl, rb_mask), red_blue), alpha), eight));
            
            //green    += (((foreground_pxl & 0xFF00FF00) -    green) * alpha) >> 8;
            green    = ADD4 (green,    SRL4 (MULLO4 (SUB4 (AND4 (foreground_pxl,  g_mask),    green), alpha), eight));
            
            //out_pxl = (red_blue & 0xFFFF00FF)  | (green & 0xFF00FF00);
            out_pxl  = OR4 (AND4 (red_blue, rb_mask), AND4 (green, g_mask));

            _mm_storeu_si128 ((__m128i *) (out->pixels + x + y*width),out_pxl);
            //out->pixels[x + y*width] = out_pxl;
        }
    }
    return 0;
}

void print128_int(__m128i var)
{
    uint32_t val[4];
    memcpy(val, &var, sizeof(val));
    printf("index: %08x %08x %08x %08x\n", 
           val[0], val[1], val[2], val[3]);
}


uint8_t vector_avx2 (Image *out, const Image *foreground, const Image *background, int max_iter)
{
    assert(out);
    assert(foreground);
    assert(background);

    printf ("size: %d %d\n", foreground->width, background->width);
    assert(foreground->width  == background->width );
    assert(foreground->height == background->height);

    unsigned width  = background->width;
    unsigned height = background->height;

    out->width  = width;
    out->height = height;

    out->bi_size    = foreground->bi_size;
    out->size       = foreground->size;
    out->offset     = foreground->offset;

    out->pixels = (uint32_t *) calloc((size_t) (width*height), sizeof(uint32_t));
    
    if (!out->pixels)
        return 1 << 1;

    __m256i red_blue = _mm256_set_epi32 (DUP8 (0));
    __m128i eight    = _mm_set_epi32 (0, 0, 0, 8);
    __m256i green    = _mm256_set_epi32 (DUP8 (0));
    __m256i out_pxl  = _mm256_set_epi32 (DUP8 (0));

    __m128i twenty_four    = _mm_set_epi32 (0, 0, 0, 24);
    __m256i foreground_pxl = _mm256_set_epi32 (DUP8 (0));
    __m256i background_pxl = _mm256_set_epi32 (DUP8 (0));
    __m256i rb_mask        = _mm256_set_epi32 (DUP8 (0xFFFF00FF));
    __m256i g_mask         = _mm256_set_epi32 (DUP8 (0xFF00FF00));
    __m256i alpha          = _mm256_set_epi32 (DUP8 (0));


    for (int iter = 0; iter < max_iter; iter++)
    {
    for (unsigned y = 0; y < height; ++y)
        for (unsigned x = 0; x < width - 8; x += 8)
        {
            foreground_pxl = _mm256_loadu_si256 ((__m256i *) (foreground->pixels + x + y*width));
            background_pxl = _mm256_loadu_si256 ((__m256i *) (background->pixels + x + y*width));

            alpha = SRL8 (foreground_pxl, twenty_four);
            
            red_blue  = AND8 (background_pxl, rb_mask);
            green     = AND8 (background_pxl, g_mask);
            
            // red_blue += (((foreground_pxl & 0xFFFF00FF) - red_blue) * alpha) >> 8;
            red_blue = ADD8 (red_blue, SRL8 (MULLO8 (SUB8 (AND8 (foreground_pxl, rb_mask), red_blue), alpha), eight));
            //green    += (((foreground_pxl & 0xFF00FF00) -    green) * alpha) >> 8;
            green    = ADD8 (green,    SRL8 (MULLO8 (SUB8 (AND8 (foreground_pxl,  g_mask),    green), alpha), eight));
            
            //out_pxl = (red_blue & 0xFFFF00FF)  | (green & 0xFF00FF00);
            out_pxl = OR8 (AND8 (red_blue, rb_mask), AND8 (green, g_mask));

            _mm256_storeu_si256 ((__m256i *) (out->pixels + x + y*width),out_pxl);
            //out->pixels[x + y*width] = out_pxl;
        }
    }
    return 0;
}


bool construct_bmp (const Image *image, const char *file_path)
{
    File_header        file_header{};
    Bitmap_info_header bi_header  {};
    ARGB_info          argb_info  {};


    FILE *bmp_file = fopen(file_path, "wb");
    if (!bmp_file)
        return false;

    file_header.file_type = 0x4D42;

    file_header.file_size = image->size;
    file_header.offset    = image->offset;
    
    fwrite (&file_header, 1, sizeof (File_header), bmp_file);

    bi_header.bi_width  = (int32_t) image->width;
    bi_header.bi_height = (int32_t) image->height;

    bi_header.bi_size   = image->bi_size;
    bi_header.bi_planes = 1;
    bi_header.bi_bitcount = 32;
    bi_header.bi_compression = 0;

    fwrite (&bi_header, 1, sizeof(Bitmap_info_header), bmp_file);

    argb_info.red_mask   = 0x00FF0000;
    argb_info.green_mask = 0x000000FF;
    argb_info.blue_mask  = 0x0000FF00;
    argb_info.alpha_mask = 0xFF000000;

    fwrite (&argb_info, 1, sizeof(ARGB_info), bmp_file);

    fwrite (image->pixels, (size_t) (image->width*image->height), sizeof (uint32_t), bmp_file);

    fclose(bmp_file);
    return true;

}

void testing (uint8_t (*test_func) (Image *out, const Image *foreground, const Image *background, const int max_iter), \
             const char *test_name,                                                                                    \
             int test_count,                                                                                           \
             Image *out,                                                                                               \
             Image *foreground,
             Image *background)
{
    auto begin = std::chrono::high_resolution_clock::now();
    printf ("calculating... %s (n = %d)\n", test_name, test_count); 

    test_func (out, foreground, background, test_count);

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    printf("result: %.5f s\n", elapsed.count() * 1e-9);
    printf("result: %ld us\n\n", elapsed.count());
}