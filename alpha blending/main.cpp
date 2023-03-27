#include <stdio.h>
#include "alpha_func.hpp"

const char *file1_path = "images/sample1.bmp";
const char *file2_path = "images/sample2.bmp";


int main ()
{
    const int max_iter = 2048;
    Image background = {};
    Image foreground = {};
    Image test_out = {};

    uint8_t errors = load_bmp_from_file (&background, file1_path); 
    if (errors) printf ("%d\n", errors);
    
    errors = load_bmp_from_file (&foreground, file2_path);
    if (errors) printf ("%d\n", errors);


    testing (vector_sse,  "vector sse",  max_iter, &test_out, &foreground, &background);
    testing (vector_avx2, "vector avx2", max_iter, &test_out, &foreground, &background);
    testing (scalar,      "scalar",      max_iter, &test_out, &foreground, &background);

    construct_bmp (&test_out, "images/test512.bmp");
    return 0;

}