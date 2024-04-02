#include <stdio.h>
#include <opencv/highgui.h>
#include <math.h>

double calc_median(unsigned int hist[]) {
    // int half_data_len = (gray->height * gray->width) / 2;
    int half_data_len = 3;
    int total = 0;
    for (int i = 0; i < 256; i++) {
        total += hist[i];
        if (total > half_data_len) {
            return i;
        }
        // データ数が偶数の時
        if (total == half_data_len) {
            if (1) {
                return i + 0.5;
            }
        }
    }
}

int main(void)
{
    int data[] = {1, 2, 2, 1};
    printf("%f", calc_median(data));

    return 0;
}
