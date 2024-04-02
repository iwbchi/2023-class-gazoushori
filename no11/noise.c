#include <stdio.h>
#include <string.h>

#include <opencv/highgui.h>

void writeFile(IplImage* img, char* imgFilename, const char* addName);
unsigned char clip(int gray);
void addSoltAndPepper(IplImage* img, double noiseLv);
void addWhite(IplImage* img, double noiseLv);
void addNoise(IplImage* img, int noiseType, double noiseLv);


// ===============================================================================

// ノイズ付加画像の書き出し(ファイル名も自動生成)
void writeFile(IplImage* img, char* imgFilename, const char* addName) {
    char filename[256];
    char* ext_p;

    strcpy(filename, imgFilename); // 読み込んだ画像のファイル名をコピー
    ext_p = strrchr(filename, '.');
    *ext_p = '\0';   // ファイル名から拡張子を消す
    sprintf(filename, "%s+%s.bmp", filename, addName); // ノイズ名を付加したファイル名を生成

    cvSaveImage(filename, img, 0);
}


// 値を 0-255 の範囲にクリップし、unsigned char で返す
unsigned char clip(int gray) {
    return (gray < 0) ? 0 : (gray > 255) ? 255 : gray;
}


// ごま塩ノイズの付加
void addSoltAndPepper(IplImage* img, double noiseLv) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            for (int ch = 0; ch < img->nChannels; ch++) {   // グレイスケール画像・カラー画像どちらにも対応
                if (rand() < RAND_MAX * noiseLv) {
                    if (rand() < RAND_MAX / 2) {
                        img->imageData[y * img->widthStep + x * img->nChannels + ch] = 0;
                    }
                    else {
                        img->imageData[y * img->widthStep + x * img->nChannels + ch] = 255;
                    }
                }
            }
        }
    }
}


// ホワイトノイズ(簡易)の付加
void addWhite(IplImage* img, double noiseLv) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            for (int ch = 0; ch < img->nChannels; ch++) {   // グレイスケール画像・カラー画像どちらにも対応
                img->imageData[y * img->widthStep + x * img->nChannels + ch]
                    = clip(((unsigned char)img->imageData[y * img->widthStep + x * img->nChannels + ch]) + ((float)rand() / RAND_MAX * 511 - 255) * noiseLv);

            }
        }
    }
}


// ノイズの種類により呼び出す関数を振り分け
void addNoise(IplImage* img, int noiseType, double noiseLv) {
    switch (noiseType) {
    case 0:
    default:
        addWhite(img, noiseLv);  // ホワイトノイズを付加
        break;

    case 1:
        addSoltAndPepper(img, noiseLv);  // ごま塩ノイズを付加
        break;
    }
}


int main(int argc, char* argv[])
{
    IplImage* img1;
    IplImage* img2;
    int noiseType = 0;       // ノイズタイプ [0(Default):ホワイトノイズ, 1:ごま塩ノイズ]
    double noiseLv = 0.05;   // ノイズレベル [noiseType==0 : ノイズの強度, noiseType==1 : ノイズの生起割合] (Default=0.05)
    const char* noiseStr[] = { "WhiteNoise", "Salt&PepperNoise" };

    // 起動オプションのチェック
    printf("argc = %d\n", argc);
    for (int k = 0; k < argc; k++) {
        printf("argv[%d] = %s\n", k, argv[k]);
    }
    printf("\n\n");

    if (argc < 2) {
        printf("ファイル名を指定してください。\n");
        return 1;
    }

    if (argc >= 3) {
        noiseType = atoi(argv[2]);  // ノイズタイプの設定
    }
    printf("Noise Type = %d (%s)\n", noiseType, noiseStr[noiseType]);


    if (argc >= 4) {
        noiseLv = atof(argv[3]);  // ノイズレベルの設定
    }
    printf("Noise Lv = %f\n", noiseLv);

    // 画像データの読み込み
    if ((img1 = cvLoadImage(argv[1], CV_LOAD_IMAGE_UNCHANGED)) == NULL) {   // 読み込んだ画像はカラーの場合も、グレイスケール画像の場合もある
        printf("画像ファイルの読み込みに失敗しました。\n");
        return 1;
    }
    // 読み込んだ画像の表示
    cvNamedWindow("Original", 1);
    cvShowImage("Original", img1);

    // -------------------------------------------------------------
    img2 = cvCreateImage(cvSize(img1->width, img1->height), img1->depth, img1->nChannels);   // 読み込んだ画像と同じ大きさの画像を生成
    cvCopy(img1, img2, NULL);  // 画像データをコピー

    addNoise(img2, noiseType, noiseLv);


    cvNamedWindow(noiseStr[noiseType], 1);
    cvShowImage(noiseStr[noiseType], img2);

    writeFile(img2, argv[1], noiseStr[noiseType]);         // ノイズ付加画像をファイルに出力

    cvWaitKey(0);
    cvDestroyAllWindows();
    return 0;
}

