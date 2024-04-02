// No.09-02 : 濃度変換(γ変換)

#include <stdio.h>
#include <opencv/highgui.h>
#include <math.h>


// double型のピクセル値を、unsigned char型で返す。0未満の値は0に、255以上の値は255にクリップする
unsigned char clip(double v) {
    return (v > 255) ? 255 : (v < 0) ? 0 : (unsigned char)v;
}



// γ変換(Gamma Stretch)
// dst: 変換結果の画像
// src: 変換元の画像
// dst = 255 * (src/255)^(1/g)
void gamma1(IplImage* dst, IplImage* src, double g) {
    double v;
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            for (int ch = 0; ch < src->nChannels; ch++) {   // グレイスケールとカラー両方まとめて処理するためのループ
                unsigned char n = (unsigned char)src->imageData[src->widthStep * y + x * src->nChannels + ch];
                // printf("%d\n", n);
                v = 255 * pow((n/255.0), (1/g)); // 一旦 double で計算
                // printf("%f\n", v);
                dst->imageData[dst->widthStep * y + x * dst->nChannels + ch] = clip(v);
            }
        }
    }
    return;
}


void main() {

    IplImage* src;
    IplImage* dst;
    double g = 0.5;  // 係数(ここを調整)

    char filename[] = "Mandrill.bmp";

    // 画像データの読み込み
    if ((src = cvLoadImage(filename, CV_LOAD_IMAGE_UNCHANGED)) == NULL) {
        printf("画像ファイルの読み込みに失敗しました。\n");
        return;
    }
    printf("%s\nnChannels = %d\n\ngamma = %f\n", filename, src->nChannels, g);

    // 読み込んだ画像と同じサイズ, 同じチャンネル数(nChannels)の画像を生成
    dst = cvCreateImage(cvSize(src->width, src->height), src->depth, src->nChannels);

    cvNamedWindow("Source", 1);
    cvShowImage("Source", src);

    gamma1(dst, src, g);

    cvNamedWindow("Destination", 1);
    cvShowImage("Destination", dst);

    char outputname[100];
    sprintf(outputname,"gamma_output%f.bmp", gamma);
    
    cvSaveImage(outputname, dst, 0);


    cvWaitKey(0);
    cvDestroyAllWindows();
    cvReleaseImage(&dst);
    cvReleaseImage(&src);
    return;
}