#include "mndlbrot.hpp"
#include <assert.h>
#include <stdio.h>
#include <chrono>
#include <string.h>


#define DUP4(a) a, a, a, a
#define DUP8(a) a, a, a, a, a, a, a, a


sf::Color mapping[16]
{
	sf::Color(66, 30, 15),
	sf::Color(25, 7, 26),
	sf::Color(9, 1, 47),
	sf::Color(4, 4, 73),
	sf::Color(0, 7, 100),
	sf::Color(12, 44, 138),
	sf::Color(24, 82, 177),
	sf::Color(57, 125, 209),
	sf::Color(134, 181, 229),
	sf::Color(211, 236, 248),
	sf::Color(241, 233, 191),
	sf::Color(248, 201, 95),
	sf::Color(255, 170, 0),
	sf::Color(204, 128, 0),
	sf::Color(153, 87, 0),
	sf::Color(106, 52, 3)
};

complex complex_square (complex number)
{

    complex temp = {};

    temp.Re = number.Re * number.Re  - number.Im * number.Im;
    temp.Im = 2 * number.Re * number.Im;
    return temp;
}

float commplex_modul2 (complex number)
{
	return number.Re*number.Re + number.Im*number.Im;
}

complex next_number (complex number, complex c)
{
    complex temp = {};

    temp = complex_square (number);
    temp.Re += c.Re;
	temp.Im += c.Im;
    return temp;
}


void test_vector_sse (Model *model, const unsigned max_iterations)
{
    assert (model);
    assert (max_iterations);

	__m128 re_min   = _mm_set_ps1 (Mandelbrot_init.Re_min);
	__m128 im_min   = _mm_set_ps1 (Mandelbrot_init.Im_min);
	__m128 re_scale = _mm_set_ps1 (Mandelbrot_init.Re_scalar);
	__m128 Im_scale = _mm_set_ps1 (Mandelbrot_init.Im_scalar);
	__m128 incr		= _mm_set_ps  (DUP4 (1));
	__m128 four     = _mm_set_ps  (DUP4 (4));
    __m128i fifteen = _mm_set_epi32 (DUP4 (15));

    for (unsigned y = 0; y < model->height; y++)
    {
        for (unsigned x = 0; x < model->width; x += 4)
        {
			__m128 x_4 = _mm_set_ps (x + 3, x + 2, x + 1, x);
			__m128 y_4 = _mm_set_ps (DUP4 (y));
			__m128 cr  = _mm_add_ps (re_min, _mm_mul_ps (x_4, re_scale));
			__m128 ci  = _mm_add_ps (im_min, _mm_mul_ps (y_4, Im_scale));

			__m128 zr = _mm_setzero_ps();
			__m128 zi = _mm_setzero_ps();

			__m128 mmax_iter = _mm_set_ps ((float) max_iterations, (float) max_iterations, \
										   (float) max_iterations, (float) max_iterations);
		
			unsigned iter_count = 0;
			size_t cur_index = 0;

			__m128 counter = _mm_setzero_ps();
	
			while (iter_count++ < max_iterations)
			{
				__m128 zr_square = _mm_mul_ps (zr, zr);
				__m128 zi_square = _mm_mul_ps (zi, zi);
				__m128 zi_zr     = _mm_mul_ps (zr, zi);

				__m128 z_square = _mm_add_ps (zr_square, zi_square);   // a^2 + b^2
				__m128 mask     = _mm_cmplt_ps (z_square, four);

				counter = _mm_add_ps (_mm_and_ps (mask, incr), counter);

				if (_mm_movemask_ps(mask) == 0x00)
					break;

				zr = _mm_add_ps (_mm_sub_ps (zr_square, zi_square), cr);
				zi = _mm_add_ps (_mm_add_ps (zi_zr, zi_zr), ci);
			}

			__m128i index  = _mm_and_si128 (_mm_cvtps_epi32 (counter), fifteen);
			uint32_t *index_32 = (uint32_t *) &index;

			cur_index = (x + y*model->width)*4;
			int mask = _mm_movemask_ps (_mm_cmplt_ps (counter, mmax_iter));
	
			for (int i = 0; i < 4; i++)
				if (mask & (1 << i) != 0)
					*((uint32_t *) (model->dots + cur_index + i*4)) = *((uint32_t *) (mapping + index_32[i]));
        }
    }
}



// void test_vector_avx (Model *model, const unsigned max_iterations)
// {
//     assert (model);
//     assert (max_iterations);

// 	__m256 re_min   = _mm256_set_ps1 (Mandelbrot_init.Re_min);
// 	__m256 im_min   = _mm256_set_ps1 (Mandelbrot_init.Im_min);
// 	__m256 re_scale = _mm256_set_ps1 (Mandelbrot_init.Re_scalar);
// 	__m256 Im_scale = _mm256_set_ps1 (Mandelbrot_init.Im_scalar);
// 	__m256 incr		= _mm256_set_ps  (DUP8 (1));
// 	__m256 four     = _mm256_set_ps  (DUP8 (4));
//     __m256i fifteen = _mm256_set_epi32 (DUP8 (15));

//     for (unsigned y = 0; y < model->height; y++)
//     {
//         for (unsigned x = 0; x < model->width - 8; x += 8)
//         {
// 			__m256 x_8 = _mm256_set_ps (x + 7, x + 6, x + 5, x + 4, x + 3, x + 2, x + 1, x);
// 			__m256 y_8 = _mm256_set_ps (DUP8 (y));
// 			__m256 cr  = _mm256_add_ps (re_min, _mm256_mul_ps (x_8, re_scale));
// 			__m256 ci  = _mm256_add_ps (im_min, _mm256_mul_ps (y_8, Im_scale));

// 			__m256 zr = _mm256_setzero_ps();
// 			__m256 zi = _mm256_setzero_ps();

// 			__m256 mmax_iter = _mm256_set_ps ((float) max_iterations, (float) max_iterations,
// 										   (float) max_iterations, (float) max_iterations);
		
// 			unsigned iter_count = 0;
// 			size_t    cur_index = 0;

// 			__m256 counter = _mm256_setzero_ps();
	
// 			while (iter_count++ < max_iterations)
// 			{
// 				__m256 zr_square = _mm256_mul_ps (zr, zr);
// 				__m256 zi_square = _mm256_mul_ps (zi, zi);
// 				__m256 zi_zr     = _mm256_mul_ps (zr, zi);

// 				__m256 z_square = _mm256_add_ps (zr_square, zi_square); 		  	  // a^2 + b^2
// 				__m256 mask     = _mm256_cmp_ps (z_square, four);        			  // ? z^2 == 4
 
// 				counter = _mm256_add_ps (_mm256_and_ps (mask, incr), counter);

// 				if (_mm256_movemask_ps(mask) == 0x00)
// 					break;

// 				zr = _mm256_add_ps (_mm256_sub_ps (zr_square, zi_square), cr);
// 				zi = _mm256_add_ps (_mm256_add_ps (zi_zr, zi_zr), ci);
// 			}

// 			__m256i index  = _mm_and_si256 (_mm256_cvtps_epi32 (counter), fifteen);
// 			uint32_t *index_32 = (uint32_t *) &index;                                    // unsafe

// 			cur_index = (x + y*model->width)*8;
// 			int mask = _mm256_movemask_ps (_mm256_cmp_ps (counter, mmax_iter));
	
// 			for (int i = 0; i < 8; i++)
// 				if (mask & (1 << i) != 0)
// 					*((uint32_t *) (model->dots + cur_index + i*8)) = *((uint32_t *) (mapping + index_32[i]));
//         }
//     }
// }


void print128_int(__m128i var)
{
    uint32_t val[4];
    memcpy(val, &var, sizeof(val));
    printf("index: %3i %3i %3i %3i\n", 
           val[0], val[1], val[2], val[3]);
}


void test_scalar (Model *model, const unsigned max_iterations)
{
    assert (model);
    assert (max_iterations);

    for (unsigned i = 0; i < model->height; i++)
    {
        for (unsigned j = 0; j < model->width; j++)
        {
            float x = Mandelbrot_init.Re_min + Mandelbrot_init.Re_scalar * j;
			float y = Mandelbrot_init.Im_min + Mandelbrot_init.Im_scalar * i;
        
			complex z0 = {x, y};
			complex z  = {0, 0};

			unsigned iter_count = 0;
			size_t cur_index = 0;

			while (iter_count < max_iterations)
			{
				if (commplex_modul2(z) >= 2.f*2.f)
					break;
    
				z = next_number(z, z0);
				++iter_count;
			}

			cur_index = (j + i*model->width)*4;
			new_get_color (&model->dots[cur_index], iter_count, max_iterations);
        }
    }
}


void init_model(Model *model, unsigned width, unsigned height)
{
	assert(model);
	assert(width & height);

	model->dots = new sf::Uint8 [width*height*4];
	assert(model->dots);

	model->width  = width;
	model->height = height;
	
	float Re_min = -1.50;
	float Im_min = -1.12;

	float Re_max =  0.47;
	float Im_max =  1.12; 

	float Re_scalar = (Re_max - Re_min)/(width  - 1);
	float Im_scalar = (Im_max - Im_min)/(height - 1);

	Mandelbrot_init = { Re_min,    Re_max,
						Im_min,    Im_max,
						Re_scalar, Im_scalar
   	};
}
   

void destroy_model(Model *model)
{
    assert(model);
    free(model->dots);
}



void show_model (void (*test_func) (Model *, const unsigned),
                 Model *model, 
                 sf::RenderWindow *Window,
                 const char *test_name)
{
	unsigned width = model->width;
	unsigned height = model->height;

    sf::Texture texture;
	sf::Event event {};

    texture.create (width, height);
    sf::Sprite sprite (texture);


    printf ("calculation... %s\n" \
			"W: %d H: %d\n", test_name, width, height);

	bool is_visible = false;
	unsigned test_count = 512;
	printf ("Test count %d\n", test_count);

	Window->display();
	auto begin = std::chrono::high_resolution_clock::now();

	while (test_count--)
	{
		while (Window->pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::S)
				{
					is_visible = !is_visible;
				}
			}
		}
		test_func (model, 100);
		Window->clear();

		if (is_visible)
		{
			texture.update (model->dots);
			Window->draw (sprite);
		}
		Window->display();
	}

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    printf("Result: %.5f\n", elapsed.count() * 1e-9);
    printf("Result: %ld\n", elapsed.count());
}



void new_get_color (uint8_t *reference, unsigned current_iteration, unsigned max_iterations)
{
	if (current_iteration < max_iterations && current_iteration > 0) 
	{
		int i = current_iteration % 16;
		*((uint32_t *) reference) = *((uint32_t *) (mapping + i));
	}
	else
		*((uint32_t *) reference) = 0;
}



// model->dots[cur_index + 0 + i*4] = mapping[index_32[i]].r;
// model->dots[cur_index + 1 + i*4] = mapping[index_32[i]].g;
// model->dots[cur_index + 2 + i*4] = mapping[index_32[i]].b;
// model->dots[cur_index + 3 + i*4] = mapping[index_32[i]].a;