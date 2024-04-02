#include <stdio.h>
#include <opencv/highgui.h>
#include <math.h>

int main(void){
    unsigned char c = 64;
    double g = 2.0;
    double b = 0.0;
    double v;

    v = c/255.0;
    printf("%f ", v);    

    // for (unsigned char c = 0;c< 256; c++ ){
    //     v = 255 * pow((c/255), (1/g));
    //     printf("%f ", v);

    // }

    return 0;
}