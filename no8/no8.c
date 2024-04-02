#include <stdio.h>
#include <opencv/highgui.h>

// グレイスケール画像を二値化
void gray2bin(IplImage* bin, IplImage* gray, unsigned char th) {
    for (int y = 0; y < gray->height; y++) {
        for (int x = 0; x < gray->width; x++) {
            unsigned char a = (unsigned char)gray->imageData[gray->widthStep * y + x];
            if (a < th) {
                bin->imageData[bin->widthStep * y + x] = 0;
            }
            else {
                bin->imageData[bin->widthStep * y + x] = 255;
            }
        }
    }
}

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

int main()
{
    IplImage* img;
    IplImage* img_gray;
    IplImage* img_bin;
    unsigned char th = 127;   // 閾値	

    char filename[] = "Mandrill.bmp";

    // 画像データの読み込み
    if ((img = cvLoadImage(filename, CV_LOAD_IMAGE_UNCHANGED)) == NULL) {
        printf("画像ファイルの読み込みに失敗しました。\n");
        return 0;
    }
    // 読み込んだ画像と同じサイズのグレイスケール画像(nChannels=1)を生成
    img_gray = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);

    cvNamedWindow("Original", 1);
    cvShowImage("Original", img);
    cvSaveImage("Original.bmp", img, 0);

    printf("nChannels = %d\n", img->nChannels);
    if (img->nChannels == 3) {
        // カラー画像だった場合、グレイスケール化した画像を表示
        printf("---Color Image\n");
        bgr2gray(img_gray, img);  // グレイスケール化

        cvNamedWindow("Grayscale", 1);
        cvShowImage("Grayscale", img_gray);
        cvSaveImage("Grayscale.bmp", img_gray, 0);
    }
    else if (img->nChannels == 1) {
        // グレイスケール画像だった場合はは、img_gray に元画像をコピーする
        printf("---Grayscale Image\n");
        cvCopy(img, img_gray, NULL);
    }

    // グレイスケール画像と同じ大きさの画像を生成 (※0,1 は グレイスケール画像の 0,255 で表現)
    img_bin = cvCreateImage(cvSize(img->width, img->height), img->depth, img_gray->nChannels);
    gray2bin(img_bin, img_gray, th);

    cvNamedWindow("Binary", 1);
    cvShowImage("Binary", img_bin);
    cvSaveImage("Binary.bmp", img_bin, 0);
    printf("Threshold = %u\n", th);

    cvWaitKey(0);
    cvDestroyAllWindows();
    cvReleaseImage(&img);
    return 1;
}
