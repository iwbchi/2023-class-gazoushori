// No.09-01 : 濃度変換(線形変換)

#include <stdio.h>
#include <string.h>
#include <opencv/highgui.h>


// double型のピクセル値を、unsigned char型で返す。0未満の値は0に、255以上の値は255にクリップする
unsigned char clip(double v) {
    return (v > 255) ? 255 : (v < 0) ? 0 : (unsigned char)v;
}



// 線形変換(Linear Stretch)
// dst: 変換結果の画像
// src: 変換元の画像
// dst = src * a + b 
void linear(IplImage* dst, IplImage* src, double a, double b) {
    double v;
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            for (int ch = 0; ch < src->nChannels; ch++) {   // グレイスケールとカラー両方まとめて処理するためのループ
                unsigned char n = (unsigned char)src->imageData[src->widthStep * y + x * src->nChannels + ch];
                v = a * n + b; // 一旦 double で計算
                dst->imageData[dst->widthStep * y + x * dst->nChannels + ch] = clip(v);
            }
        }
    }
}

void testClip() {
    for (double d = -1; d<256; d++) {
        int i = clip(d);
        printf("%d\n", i);
    }
    return;
}


void main() {
    IplImage* src;
    IplImage* dst;
    double a, b;  // 係数

    int mode = 0;  // この値でモードを切り替える

    char filename[] = "Mandrill.bmp";



    switch (mode) {
    case 0:  // 無変換 ---------------------
        a = 1.0;
        b = 0.0;
        break;

    case 1:  // ①明るく -------------------
        a = 1.0; // ここを調整
        b = 100.0; // ここを調整
        break;

    case 2:  // ②暗く ---------------------
        a = 1.0; // ここを調整
        b = -100.0; // ここを調整
        break;

    case 3:  // ③コントラストを強く -------
        a = 2.0; // ここを調整
        b = -126.0; // ここを調整
        break;

    case 4:  // ④コントラストを弱く -------
        a = 0.5; // ここを調整
        b = 63.0; // ここを調整
        break;

    case 5:  // ⑤階調反転 -----------------
        a = -1.0; // ここを調整
        b = 255.0; // ここを調整
        break;

    default:  // ---------------------------
        printf("modeが範囲外の値です\n");
        return;
    }


    // 画像データの読み込み
    if ((src = cvLoadImage(filename, CV_LOAD_IMAGE_UNCHANGED)) == NULL) {
        printf("画像ファイルの読み込みに失敗しました。\n");
        return;
    }
    printf("%s\nnChannels = %d\n\nMode = %d\n\ta = %f\n\tb = %f\n", filename, src->nChannels, mode, a, b);

    // 読み込んだ画像と同じサイズ, 同じチャンネル数(nChannels)の画像を生成
    dst = cvCreateImage(cvSize(src->width, src->height), src->depth, src->nChannels);

    cvNamedWindow("Source", 1);
    cvShowImage("Source", src);

    linear(dst, src, a, b);

    char outputname[100];
    sprintf(outputname,"linear_output%d.bmp", mode);
    cvNamedWindow("Destination", 1);
    cvShowImage("Destination", dst);
    cvSaveImage(outputname, dst, 0);


    cvWaitKey(0);
    cvDestroyAllWindows();
    cvReleaseImage(&dst);
    cvReleaseImage(&src);
    return;
}
