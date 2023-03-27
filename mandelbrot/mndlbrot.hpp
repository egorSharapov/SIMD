#pragma once 

#include <stdint.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Color.hpp>
#include <x86intrin.h>

struct 
{
    float Re_min;
    float Re_max;
    float Im_min;
    float Im_max;
    float Re_scalar;
    float Im_scalar;

} Mandelbrot_init;


struct Model 
{
    sf::Uint8* dots;
    unsigned  width;
    unsigned height;
};

typedef struct complex
{
	float Re = 0.0f;
	float Im = 0.0f;
} complex;


complex next_number     (complex number, complex c);
complex complex_square  (complex cfloat);
float   commplex_modul2 (complex number);

void    print128_int    (__m128i var);
void    new_get_color   (uint8_t *reference, unsigned current_iteration, unsigned max_iterations);
void    get_color       (uint8_t *reference, unsigned current_iteration, unsigned max_iterations);
void    init_model      (Model *model, unsigned width, unsigned height);
void    test_scalar     (Model *model, const unsigned max_iterations);
void    test_vector_sse (Model *model, const unsigned max_iterations);
void    test_vector_avx (Model *model, const unsigned max_iterations);
void    show_model      (void (*test_func) (Model *, const unsigned),
                         Model *model, 
                         sf::RenderWindow *Window,
                         const char *test_name);



