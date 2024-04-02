// No.12 : 近傍演算による画像処理(輪郭抽出、鮮鋭化)
//
#include <stdio.h>
#include <opencv/highgui.h>

// フィルタ係数等のフィルタのパラメータを保持する構造体
typedef struct {
    int size;           // フィルタの大きさ。テキストでいうところの w
    int widthStep;       // imageData[] と同様の方法で係数配列にアクセスが出来るようにするための変数
    double* weight;     // 係数の配列が入る
    double sigma;       // ガウシアンフィルタ用(σ)
    char stretch;       // 最小値=0, 最大値=255 になるように変換するかどうか (0:変換しない / 0以外:変換する)
} Operator;

// プロトタイプ宣言
void writeFile(IplImage* img, char* imgFilename, const char* addName);
IplImage* subImage(IplImage* subImg, IplImage* img, int x, int y, int w, int h);
double linearFilter(IplImage* subImg, int ch, Operator* ope);  // No.11 から、返り値の型を変更
unsigned char clip(double v);
void filterImageDbl(IplImage* img1, IplImage* img2, Operator ope);
void makeXDiffOpe(Operator* ope);
void makeYDiffOpe(Operator* ope);
void makeXPrewittOpe(Operator* ope);
void makeYPrewittOpe(Operator* ope);
void makeXSobelOpe(Operator* ope);
void makeYSobelOpe(Operator* ope);
void makeLaplacianOpe(Operator* ope);
void makeSharpenOpe(Operator* ope);
void showWeight(Operator* ope);


void writeFile(IplImage* img, char* imgFilename, const char* addName) {
    char filename[256];
    char* ext_p;

    strcpy(filename, imgFilename); // 読み込んだ画像のファイル名をコピー
    ext_p = strrchr(filename, '.');
    *ext_p = '\0';   // ファイル名から拡張子を消す
    sprintf(filename, "%s+%s.bmp", filename, addName); // ノイズ名を付加したファイル名を生成

    cvSaveImage(filename, img, 0);
}

// 画像の一部分(画像ブロック)を切り出す
IplImage* subImage(IplImage* subImg, IplImage* img, int x, int y, int w, int h) {

    // いったん0で初期化(imgの幅や高さを超えた場合の対策)
    cvSetZero(subImg);

    for (int i = y; i < y + h && i < img->height; i++) {
        for (int j = x; j < x + w && j < img->width; j++) {
            for (int ch = 0; ch < img->nChannels; ch++) {
                subImg->imageData[(i - y) * subImg->widthStep + (j - x) * img->nChannels + ch] = img->imageData[img->widthStep * i + j * img->nChannels + ch];
            }
        }
    }
    return subImg;
}



// 畳み込み積分を行う (カラー／グレイスケール両対応のため、ch を指定。(BGRのチャネル毎に処理する))
// ※グレイスケール画像(nChannel=1) の場合は、ch は常に0
// ヒント: (x,y)の位置のフィルタ係数は、「　ope->weight[y * ope->widthStep + x]　」 としてアクセスできる
double linearFilter(IplImage* subImg, int ch, Operator* ope) {
    double sum = 0;

    for (int y = 0; y < subImg->height; y++) {
        for (int x = 0; x < subImg->width; x++) {
            // ここを作る!! ・・・のは無しにしました。
            sum += (unsigned char)subImg->imageData[y * subImg->widthStep + subImg->nChannels * x + ch] * ope->weight[y * ope->widthStep + x];
        }
    }
    return sum;
}



unsigned char clip(double v) {
    return (v > 255.0) ? 255 : (v < 0.0) ? 0 : v;
}



// img1 に ope で設定したフィルタをかけたものを img2 として出力
// 内部計算を double で行い、ピクセル値の範囲がch毎に 最小値0, 最大値255となるよう引き伸ばす(※カラー画像の場合色が変わる場合あり)
void filterImageDbl(IplImage* img1, IplImage* img2, Operator ope) {

    IplImage* buff = cvCreateImage(cvSize(ope.size * 2 + 1, ope.size * 2 + 1), img1->depth, img1->nChannels);


    
    if (ope.stretch) { // 0-255 の値に変換する場合
        double* tmp = (double*)malloc(sizeof(double) * img2->width * img2->height * img2->nChannels);
        double v;

        double* minV = (double*)malloc(sizeof(double) * img2->nChannels);
        double* maxV = (double*)malloc(sizeof(double) * img2->nChannels);
        for (int ch = 0; ch < img2->nChannels; ch++) {
            minV[ch] = DBL_MAX;  // 取りうる最大値で初期化
            maxV[ch] = DBL_MIN;  // 取りうる最小値で初期化
        }

        // いったん double で計算し、chごとの最大値と最小値も同時に求める
        for (int y = ope.size; y < img1->height - ope.size - 1; y++) {
            for (int x = ope.size; x < img1->width - ope.size - 1; x++) {
                for (int ch = 0; ch < img2->nChannels; ch++) {    // グレイスケール／カラー両対応のためのループ
                    v = linearFilter(subImage(buff, img1, x - ope.size, y - ope.size, ope.size * 2 + 1, ope.size * 2 + 1), ch, &ope); // いったんdoubleで計算
                    tmp[y * img2->widthStep + x * img2->nChannels + ch] = v;
                    minV[ch] = (minV[ch] > v) ? v : minV[ch];
                    maxV[ch] = (maxV[ch] < v) ? v : maxV[ch];
                }
            }
        }

        // ch毎に、最大値=255、最小値=0、になるように変換して、img2 に代入
        for (int ch = 0; ch < img2->nChannels; ch++) {
            for (int y = 0; y < img2->height; y++) {
                for (int x = 0; x < img2->width; x++) {
                    img2->imageData[y * img2->widthStep + x * img2->nChannels + ch] = (unsigned char)((tmp[y * (img2->width * img2->nChannels) + x * img2->nChannels + ch] - minV[ch]) / (maxV[ch] - minV[ch]) * 255);
                }
            }
        }

        // 変換前の最大値、最小値をchごとに表示
        for (int ch = 0; ch < img2->nChannels; ch++)
            printf("\tMinV[%d] = %f", ch, minV[ch]);
        printf("\n");
        for (int ch = 0; ch < img2->nChannels; ch++)
            printf("\tMaxV[%d] = %f", ch, maxV[ch]);
    }
    else { // フィルタ係数の合計が 1.0 の場合などはこれだけで良い
        for (int y = ope.size; y < img1->height - ope.size - 1; y++) {
            for (int x = ope.size; x < img1->width - ope.size - 1; x++) {
                for (int ch = 0; ch < img2->nChannels; ch++) {    // グレイスケール／カラー両対応のためのループ
                    img2->imageData[y * img2->widthStep + x * img2->nChannels + ch] = clip(linearFilter(subImage(buff, img1, x - ope.size, y - ope.size, ope.size * 2 + 1, ope.size * 2 + 1), ch, &ope));
                }
            }
        }
    }
}



// x方向の一次微分フィルタの係数を作る (これを参考に・・・!!)
void makeXDiffOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = 0;    ope->weight[1] = 0;    ope->weight[2] = 0;
    ope->weight[3] = 0;    ope->weight[4] = -1;    ope->weight[5] = 1;
    ope->weight[6] = 0;    ope->weight[7] = 0;    ope->weight[8] = 0;
}


// y方向の一次微分フィルタの係数を作る
void makeYDiffOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = 0;    ope->weight[1] = 0;    ope->weight[2] = 0;
    ope->weight[3] = 0;    ope->weight[4] = -1;    ope->weight[5] = 0;
    ope->weight[6] = 0;    ope->weight[7] = 1;    ope->weight[8] = 0;
}

// x方向に一次微分する Prewitt フィルタの係数を作る
void makeXPrewittOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = -1;    ope->weight[1] = 0;    ope->weight[2] = 1;
    ope->weight[3] = -1;    ope->weight[4] = 0;    ope->weight[5] = 1;
    ope->weight[6] = -1;    ope->weight[7] = 0;    ope->weight[8] = 1;
}

// y方向に一次微分する Prewitt フィルタの係数を作る
void makeYPrewittOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = -1;    ope->weight[1] = -1;    ope->weight[2] = -1;
    ope->weight[3] = 0;    ope->weight[4] = 0;    ope->weight[5] = 0;
    ope->weight[6] = 1;    ope->weight[7] = 1;    ope->weight[8] = 1;
}

// x方向に一次微分する Sobel フィルタの係数を作る
void makeXSobelOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = -1;    ope->weight[1] = 0;    ope->weight[2] = 1;
    ope->weight[3] = -2;    ope->weight[4] = 0;    ope->weight[5] = 2;
    ope->weight[6] = -1;    ope->weight[7] = 0;    ope->weight[8] = 1;
}

// y方向に一次微分する Sobel フィルタの係数を作る
void makeYSobelOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = -1;    ope->weight[1] = -2;    ope->weight[2] = -1;
    ope->weight[3] = 0;    ope->weight[4] = 0;    ope->weight[5] = 0;
    ope->weight[6] = 1;    ope->weight[7] = 2;    ope->weight[8] = 1;
}


// 4近傍のラプラシアンフィルタの係数を作る
void makeLaplacianOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = 0;    ope->weight[1] = 1;    ope->weight[2] = 0;
    ope->weight[3] = 1;    ope->weight[4] = -4;    ope->weight[5] = 1;
    ope->weight[6] = 0;    ope->weight[7] = 1;    ope->weight[8] = 0;
}


// 先鋭化フィルタの係数を作る
void makeSharpenOpe(Operator* ope) {
    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 0; // フィルタ係数の合計が 1.0 になるフィルタの場合は、変換不要

    ope->size = 1;     // 今回は　ope->sie = 1 固定としてよい
    ope->widthStep = ope->size * 2 + 1;
    free(ope->weight);
    ope->weight = (double*)malloc(sizeof(double) * ope->widthStep * ope->widthStep);
    ope->stretch = 1;  // 変換が必要

    // ----
    ope->weight[0] = 0;    ope->weight[1] = -1;    ope->weight[2] = 0;
    ope->weight[3] = -1;    ope->weight[4] = 5;    ope->weight[5] = -1;
    ope->weight[6] = 0;    ope->weight[7] = -1;    ope->weight[8] = 0;
}



// フィルタ係数(weight)を表示する
void showWeight(Operator* ope) {
    printf("<<Operator>>\n");
    for (int y = 0; y < ope->widthStep; y++) {
        for (int x = 0; x < ope->widthStep; x++) {
            printf("[%01.4f] ", ope->weight[y * ope->widthStep + x]);
        }
        printf("\n");
    }
}


void main(int argc, char* argv[])
{
    IplImage* img1;
    IplImage* img2;
    Operator ope;
    ope.weight = NULL;  // NULLで初期化 (make????Ope() 内で free() されるので)

    // ファイルを Drag&Dropで処理できるようにするには、①この下の一連のコメントアウトを外し、
    char fn[256];

    // 起動オプションのチェック
    printf("argc = %d\n", argc);
    for (int k = 0; k < argc; k++) {
        printf("argv[%d] = %s\n", k, argv[k]);
    }
    printf("\n\n");

    if (argc < 2) {
        printf("ファイル名を指定してください。\n");
        return;
    }
    strcpy(fn, argv[1]);

    // char fn[] = "LENNA.BMP"; // Drag&Dropで処理する場合は　②ここをコメントアウトする



    // 画像データの読み込み
    if ((img1 = cvLoadImage(fn, CV_LOAD_IMAGE_UNCHANGED)) == NULL) {   // 読み込んだ画像はカラーの場合も、グレイスケール画像の場合もある
        printf("画像ファイルの読み込みに失敗しました。\n");
        return;
    }
    // 読み込んだ画像の表示
    cvNamedWindow("Original", 1);
    cvShowImage("Original", img1);
    printf("File Name = %s\n", fn);

    img2 = cvCreateImage(cvSize(img1->width, img1->height), img1->depth, img1->nChannels);   // 読み込んだ画像と同じ大きさの画像を生成;

    // -------------------------------------------------------------
    cvSetZero(img2); // 0(黒)で初期化しておく

    makeXDiffOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("XDiff", 1);
    cvShowImage("XDiff", img2);
    writeFile(img2, fn, "XDiff");
    printf("\n");


    // -------------------------------------------------------------
    // -------------------------------------------------------------
    // -------------------------------------------------------------
    // -------------------------------------------------------------
    // ここに make???Ope() を呼び出して、結果を表示するコード書く
    cvSetZero(img2); // 0(黒)で初期化しておく

    makeYDiffOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("YDiff", 1);
    cvShowImage("YDiff", img2);
    writeFile(img2, fn, "YDiff");
    printf("\n");

    cvSetZero(img2); // 0(黒)で初期化しておく

    makeXPrewittOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("XPrewitt", 1);
    cvShowImage("XPrewitt", img2);
    writeFile(img2, fn, "XPrewitt");
    printf("\n");

    cvSetZero(img2); // 0(黒)で初期化しておく

    makeYPrewittOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("YPrewitt", 1);
    cvShowImage("YPrewitt", img2);
    writeFile(img2, fn, "YPrewitt");
    printf("\n");

    cvSetZero(img2); // 0(黒)で初期化しておく

    makeXSobelOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("XSobel", 1);
    cvShowImage("XSobel", img2);
    writeFile(img2, fn, "XSobel");
    printf("\n");

    cvSetZero(img2); // 0(黒)で初期化しておく

    makeYSobelOpe(&ope); 
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("YSobel", 1);
    cvShowImage("YSobel", img2);
    writeFile(img2, fn, "YSobel");
    printf("\n");
    // -------------------------------------------------------------
    // -------------------------------------------------------------
    // -------------------------------------------------------------


    // -------------------------------------------------------------
    cvSetZero(img2);   // 0(黒)で初期化しておく

    makeLaplacianOpe(&ope);       // ここでラプラシアンフィルタ用の係数等のパラメータを作成。
    showWeight(&ope);             // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("Laplacian", 1);
    cvShowImage("Laplacian", img2);
    writeFile(img2, fn, "Laplacian");
    printf("\n");

    // -------------------------------------------------------------
    cvSetZero(img2);   // 一旦消す

    makeSharpenOpe(&ope);        // ここで鮮鋭化フィルタ用の係数等のパラメータを作成。
    showWeight(&ope);            // 作成したフィルタ係数を表示

    filterImageDbl(img1, img2, ope);
    cvNamedWindow("Sharpen", 1);
    cvShowImage("Sharpen", img2);
    writeFile(img2, fn, "Sharpen");
    printf("\n");
    

    // -------------------------------------------------------------
    free(ope.weight);
    cvWaitKey(0);
    cvDestroyAllWindows();
}