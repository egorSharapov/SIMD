include <stdio.h>
#include "alpha_func.hpp"


const char *file1_path = "images/image1.bmp";
const char *file2_path = "images/image2.bmp";


int main ()
{
    const int max_iter = 2048;
    Image image1 = {};
    Image image2    = {};
    Image test_out = {};

    uint8_t errors = load_bmp_from_file (&image1, file1_path); 
    if (errors) printf ("%d\n", errors);
    
    errors = load_bmp_from_file (&image2, file2_path);
    if (errors) printf ("%d\n", errors);


    testing (test_vector_intrin, "vector intrinsic", max_iter, &test_out, &image1, &image2);
    testing (test_scalar, "scalar", max_iter, &test_out, &image1, &image2);

    construct_bmp (&test_out, "images/test512.bmp");
    return 0;

}
