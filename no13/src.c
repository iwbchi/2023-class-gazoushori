// No.13 : メディアンフィルタ (OPE_SIZE = 1 ==> 3x3 サイズ, OPE_SIZE = 2 ==> 5x5 サイズ)
//
#include <stdio.h>
#include <opencv/highgui.h>

#define OPE_SIZE 1

// プロトタイプ宣言
IplImage *subImage(IplImage *subImg, IplImage *img, int x, int y, int w, int h);
unsigned char medianFilter(IplImage *subImg, int ch);
void filterImage(IplImage *img1, IplImage *img2);

int compare_int(const void *a, const void *b)
{
    unsigned char a_num = *(unsigned char *)a;
    unsigned char b_num = *(unsigned char *)b;

    if (a_num < b_num)
    {
        return -1;
    }
    else if (a_num > b_num)
    {
        return 1;
    }
    return 0;
}

// 画像の一部分(画像ブロック)を切り出す
IplImage *subImage(IplImage *subImg, IplImage *img, int x, int y, int w, int h)
{

    // いったん0で初期化(imgの幅や高さを超えた場合の対策)
    cvSetZero(subImg);

    for (int i = y; i < y + h && i < img->height; i++)
    {
        for (int j = x; j < x + w && j < img->width; j++)
        {
            for (int ch = 0; ch < img->nChannels; ch++)
            {
                subImg->imageData[(i - y) * subImg->widthStep + (j - x) * img->nChannels + ch] = img->imageData[img->widthStep * i + j * img->nChannels + ch];
            }
        }
    }
    return subImg;
}

// メディアン値(中央値)を求める (カラー／グレイスケール両対応のため、ch を指定。(BGRのチャネル毎に処理する))
// ※グレイスケール画像(nChannel=1) の場合は、ch は常に0
unsigned char medianFilter(IplImage *subImg, int ch)
{
    unsigned char median = 0;
    int n = (OPE_SIZE * 2 + 1) * (OPE_SIZE * 2 + 1);
    unsigned char neighbor[n];
    int i = 0;

    // 近傍のピクセル値を neighbor[] に収集
    for (int y = 0; y < subImg->height; y++)
    {
        for (int x = 0; x < subImg->width; x++)
        {
            neighbor[i++] = (unsigned char)subImg->imageData[y * subImg->widthStep + subImg->nChannels * x + ch];
        }
    }
    qsort(neighbor, sizeof(neighbor) / sizeof(neighbor[0]), sizeof(unsigned char), compare_int);
    // for (int j = 0; j < n; j++)
    // {
    //     printf("%d\n", neighbor[j]);
    // }
    // getc(stdin);

    median = neighbor[n / 2];

    return median;
}

void filterImage(IplImage *img1, IplImage *img2)
{
    IplImage *buff = cvCreateImage(cvSize(OPE_SIZE * 2 + 1, OPE_SIZE * 2 + 1), img1->depth, img1->nChannels);

    for (int y = OPE_SIZE; y < img1->height - OPE_SIZE - 1; y++)
    {
        for (int x = OPE_SIZE; x < img1->width - OPE_SIZE - 1; x++)
        {
            for (int ch = 0; ch < img2->nChannels; ch++)
            { // グレイスケール／カラー両対応のためのループ
                img2->imageData[y * img2->widthStep + x * img2->nChannels + ch] = medianFilter(subImage(buff, img1, x - OPE_SIZE, y - OPE_SIZE, OPE_SIZE * 2 + 1, OPE_SIZE * 2 + 1), ch);
            }
        }
    }
}

// フィルタ結果の画像書き出し(ファイル名も自動生成)
void writeFile(IplImage *img, char *imgFilename, const char *addName)
{
    char filename[256];
    char *ext_p;

    strcpy(filename, imgFilename); // 読み込んだ画像のファイル名をコピー
    ext_p = strrchr(filename, '.');
    *ext_p = '\0';                                                                                // ファイル名から拡張子を消す
    sprintf(filename, "%s+%s(%dx%d).bmp", filename, addName, OPE_SIZE * 2 + 1, OPE_SIZE * 2 + 1); // ノイズ名を付加したファイル名を生成

    cvSaveImage(filename, img, 0);
}

void main(int argc, char *argv[])
{
    IplImage *img1;
    IplImage *img2;

    // ファイルを Drag&Dropで処理できるようにするには、①この下の一連のコメントアウトを外し、
    char fn[256];

    // 起動オプションのチェック
    printf("argc = %d\n", argc);
    for (int k = 0; k < argc; k++)
    {
        printf("argv[%d] = %s\n", k, argv[k]);
    }
    printf("\n\n");

    if (argc < 2)
    {
        printf("ファイル名を指定してください。\n");
        return;
    }
    strcpy(fn, argv[1]);

    // char fn[] = "LENNA+Salt&PepperNoise.BMP"; // Drag&Dropで処理する場合は　②ここをコメントアウトする

    // 画像データの読み込み
    if ((img1 = cvLoadImage(fn, CV_LOAD_IMAGE_UNCHANGED)) == NULL)
    { // 読み込んだ画像はカラーの場合も、グレイスケール画像の場合もある
        printf("画像ファイルの読み込みに失敗しました。\n");
        return;
    }
    // 読み込んだ画像の表示
    cvNamedWindow("Original", 1);
    cvShowImage("Original", img1);
    printf("File Name = %s\n", fn);
    printf("OPE_SIZE = %d (%d x %d)\n", OPE_SIZE, OPE_SIZE * 2 + 1, OPE_SIZE * 2 + 1);

    img2 = cvCreateImage(cvSize(img1->width, img1->height), img1->depth, img1->nChannels); // 読み込んだ画像と同じ大きさの画像を生成;

    // -------------------------------------------------------------
    cvSetZero(img2); // 0(黒)で初期化しておく

    filterImage(img1, img2);
    cvNamedWindow("Median Filter", 1);
    cvShowImage("Median Filter", img2);

    writeFile(img2, fn, "_Median"); // フィルタ結果を画像ファイルとして出力

    // -------------------------------------------------------------
    cvWaitKey(0);
    cvDestroyAllWindows();
}
