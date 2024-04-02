// No.10-02 : 各種画像統計量の計算

#include <stdio.h>
#include <opencv/highgui.h>
#include <stdlib.h>
#include <math.h>


// カラー画像をグレイスケール化  
void bgr2gray(IplImage* gray, IplImage* bgr) {
    for (int y = 0; y < gray->height; y++) {
        for (int x = 0; x < gray->width; x++) {
            unsigned char b = (unsigned char)bgr->imageData[bgr->widthStep * y + x * 3 + 0];
            unsigned char g = (unsigned char)bgr->imageData[bgr->widthStep * y + x * 3 + 1];
            unsigned char r = (unsigned char)bgr->imageData[bgr->widthStep * y + x * 3 + 2];
            gray->imageData[gray->widthStep * y + x] = (0.299 * r + 0.587 * g + 0.114 * b);
        }
    }
}

void calc_histogram(unsigned int hist[], IplImage* gray) {
    for (int y = 0; y < gray->height; y++) {
        for (int x = 0; x < gray->width; x++) {
            hist[(unsigned char)gray->imageData[gray->widthStep * y + x]]++;
        }
    }
    return;
}

double calc_std(double mean, IplImage* gray) {
    double tmp_sum = 0.0;
    for (int y = 0; y < gray->height; y++) {
        for (int x = 0; x < gray->width; x++) {
            unsigned int n = (unsigned char)gray->imageData[gray->widthStep * y + x];
            double tmp = n- mean;
            tmp_sum += tmp * tmp;
        }
    }
    return sqrt(tmp_sum / (gray->height * gray->width));
}

double calc_median(unsigned int hist[], IplImage* gray) {
    int half_data_len = (gray->height * gray->width) / 2;
    int total = 0;
    for (int i = 0; i < 256; i++) {
        total += hist[i];
        if (total > half_data_len) {
            return i;
        }
        // データ数が偶数の時
        if (total == half_data_len) {
            if ((gray->height * gray->width) % 2 == 0) {
                return i + 0.5;
            }
        }
    }
}

int calc_mode(unsigned int hist[], IplImage* gray) {
    int mode;
    int max_num = 0;

    for (int i = 0; i < 256; i++) {
        if (hist[i] > max_num) {
            max_num = hist[i];
            mode = i;
        }
    }    
    return mode;
}


void main()
{
    IplImage* img;
    IplImage* gray;

    char filename[] = "Mandrill.bmp";

    // 画像データの読み込み
    if ((img = cvLoadImage(filename, CV_LOAD_IMAGE_UNCHANGED)) == NULL) {
        printf("画像ファイルの読み込みに失敗しました。\n");
        return;
    }
    // 読み込んだ画像と同じサイズのグレイスケール画像(nChannels=1)を生成
    gray = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);

    cvNamedWindow("Original", 1);
    cvShowImage("Original", img);

    if (img->nChannels == 3) {
        // カラー画像だった場合、グレイスケール化した画像を表示
        bgr2gray(gray, img);  // グレイスケール化

        cvNamedWindow("Grayscale", 1);
        cvShowImage("Grayscale", gray);
    }
    else if (img->nChannels == 1) {
        // グレイスケール画像だった場合はは、img_gray に元画像をコピーする
        cvCopy(img, gray, NULL);
    }


    // ここまでで、カラー画像であればグレイスケール化したものが、
    // もとからグレイスケール画像であれば、それをコピーしたものが、
    // gray に入っている状態になる。


    int minV = 255; // 最小値 : とりあえず最大値で初期化(より小さな値が見つかったら更新する)
    int maxV = 0;   // 最大値 : とりあえず最小値で初期化(より大きな値が見つかったら更新する)
    int range=0;      // 範囲   : 未初期化なので要注意
    double mean=0;    // 平均値 : 未初期化なので要注意
    double sum=0.0;
    // その他計算に必要な変数をここで宣言する

    double std;     // 標準偏差 : 未初期化なので要注意
    int median;     // 中央値: 未初期化なので要注意
    int mode;       // 最頻値: 未初期化なので要注意
                    // ※中央値や最頻値の計算には、ヒストグラムを使うと簡単

    int data_num = (gray->height * gray->width);

    for (int y = 0; y < gray->height; y++) {
        for (int x = 0; x < gray->width; x++) {
            unsigned int n = (unsigned char)gray->imageData[gray->widthStep * y + x];
            sum += n;
            if (n < minV) minV = n;
            if (n > maxV) maxV = n;
        }
    }
    mean = sum / (gray->height * gray->width);
    range = maxV - minV;

    std = calc_std(mean, gray);

    unsigned int hist[256] = { 0 };
    calc_histogram(hist, gray);

    median = calc_median(hist, gray);
    mode = calc_mode(hist, gray);

    
    // 計算結果の表示
    printf("minV  = %d\n", minV);
    printf("maxV  = %d\n", maxV);
    printf("range = %d\n", range);
    printf("mean  = %f\n\n", mean);

    printf("std    = %f\n", std);
    printf("median = %d\n", median);
    printf("mode   = %d\n", mode);


    cvWaitKey(0);
    cvDestroyAllWindows();
    cvReleaseImage(&gray);
    cvReleaseImage(&img);
    return;
}
