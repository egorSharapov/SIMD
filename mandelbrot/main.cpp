#include "mndlbrot.hpp"
#include "stdio.h"
int main ()
{
    sf::RenderWindow window (sf::VideoMode(1200, 1000), "SSE+AVX");

	  Model model = {};
    init_model (&model, 1200, 1000);

    show_model (test_vector_sse, &model, &window, "vector sse");
    show_model (test_scalar, &model, &window, "scalar");
    return 0;
}

