// No.14 : 動画像処理のサンプルプログラム
//

#include <stdio.h>
#include <opencv/highgui.h>

#define N_FRAME 5

IplImage *historyFrame[N_FRAME]; // モーションブラー用の画像

// 画像フレームへの図形描画
void drawShape(IplImage *frame, IplImage *outputFrame)
{
    cvCopy(frame, outputFrame, NULL);
    cvLine(outputFrame, cvPoint(10, 10), cvPoint(100, 100), cvScalar(0, 0, 255), 5, 1, 1);             // 直線
    cvRectangle(outputFrame, cvPoint(110, 110), cvPoint(200, 200), cvScalar(255, 0, 255), 5, 1);       // 四角形
    cvRectangle(outputFrame, cvPoint(210, 110), cvPoint(300, 220), cvScalar(0, 255, 0), -1, 1);        // 四角形(内部塗りつぶし)
    cvCircle(outputFrame, cvPoint(160, 120), 30, cvScalar(0, 255, 255), 2, 1);                         // 円
    cvEllipse(outputFrame, cvPoint(120, 80), cvSize(60, 30), 15, 0, 270, cvScalar(255, 255, 0), 2, 1); // 楕円
}

// 画素値の入れ替え
void swap(char *p1, char *p2)
{
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}

// 画像フレームの左右入れ替え(鏡像)
void mirror(IplImage *frame, IplImage *outputFrame)
{
    cvCopy(frame, outputFrame, NULL);
    for (int y = 0; y < outputFrame->height; y++)
    {
        for (int x = 0; x < outputFrame->width / 2; x++)
        {
            for (int ch = 0; ch < outputFrame->nChannels; ch++)
            {
                swap(&outputFrame->imageData[y * outputFrame->widthStep + (outputFrame->width - x) * outputFrame->nChannels + ch], &outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch]);
            }
        }
    }
}

// 画像フレームの左右対象画像の生成
void mirror2(IplImage *frame, IplImage *outputFrame)
{
    cvCopy(frame, outputFrame, NULL);
    for (int y = 0; y < outputFrame->height; y++)
    {
        for (int x = 0; x < outputFrame->width / 2; x++)
        {
            for (int ch = 0; ch < outputFrame->nChannels; ch++)
            {
                outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] = outputFrame->imageData[y * outputFrame->widthStep + (outputFrame->width - x) * outputFrame->nChannels + ch];
            }
        }
    }
}

// 1ラインおきに左右反転
void scrumble(IplImage *frame, IplImage *outputFrame)
{
    cvCopy(frame, outputFrame, NULL);
    for (int y = 0; y < outputFrame->height - 1; y += 2)
    {
        for (int x = 0; x < outputFrame->width / 2; x++)
        {
            for (int ch = 0; ch < outputFrame->nChannels; ch++)
            {
                swap(&outputFrame->imageData[y * outputFrame->widthStep + (outputFrame->width - x) * outputFrame->nChannels + ch], &outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch]);
            }
        }
    }
}

// Rチャネルのみ抽出
void chR(IplImage *frame, IplImage *outputFrame)
{
    cvCopy(frame, outputFrame, NULL);
    for (int y = 0; y < outputFrame->height; y++)
    {
        for (int x = 0; x < outputFrame->width; x++)
        {
            for (int ch = 0; ch < 2; ch++)
            {
                outputFrame->imageData[y * outputFrame->widthStep + (outputFrame->width - x) * outputFrame->nChannels + ch] = 0;
            }
        }
    }
}

// カラーチャネルの入れ替え
void chShift(IplImage *frame, IplImage *outputFrame)
{
    for (int y = 0; y < frame->height; y++)
    {
        for (int x = 0; x < frame->width; x++)
        {
            for (int ch = 0; ch < frame->nChannels; ch++)
            {
                outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] = frame->imageData[y * frame->widthStep + x * frame->nChannels + ((ch + 1) % 3)];
            }
        }
    }
}

// 0-255 の値にクリップ
unsigned char clip(int v)
{
    return (v < 0) ? 0 : (v > 255) ? 255
                                   : v;
}

// モーションブラー
void motionBlur(IplImage *frame, double w, IplImage *outputFrame)
{
    IplImage *p = historyFrame[N_FRAME - 1];

    for (int k = N_FRAME - 1; k > 0; k--)
    { // ポインタをひとつずらす
        historyFrame[k] = historyFrame[k - 1];
    }
    historyFrame[0] = p;                  // 一番後ろを一番前に持ってくる(ポインタのつなぎ変え)
    cvCopy(frame, historyFrame[0], NULL); // 一番前の historyFrame を現在のフレームで上書き

    cvSetZero(outputFrame);

    for (int k = 0; k < N_FRAME; k++)
    {
        for (int y = 0; y < outputFrame->height; y++)
        {
            for (int x = 0; x < outputFrame->width; x++)
            {
                for (int ch = 0; ch < outputFrame->nChannels; ch++)
                {
                    outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] = clip((unsigned char)outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] + (unsigned char)historyFrame[k]->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] / N_FRAME);
                }
            }
        }
    }
}

// 指定したx,y が画像内であればその輝度値を返し、画像外であれば0を返す
int chk(IplImage *frame, int x, int y)
{

    // 画像外の場合は 0 を返す
    if (x > frame->width - 1 || x < 0 || y > frame->height - 1 || y < 0)
        return 0;

    // 輝度値を返す
    return 0.298912 * (unsigned char)frame->imageData[y * frame->widthStep + x * frame->nChannels + 2] + 0.586611 * (unsigned char)frame->imageData[y * frame->widthStep + x * frame->nChannels + 1] + 0.114478 * (unsigned char)frame->imageData[y * frame->widthStep + x * frame->nChannels + 0];
}

// 画像内に動くボールを合成
void ball(IplImage *frame, IplImage *outputFrame)
{
    // 初期値を適当に指定
    // (※static 修飾子があるものは、関数を抜けても値が保持される。また最初に一度だけ初期化される)
    static float x = frame->width / 2;
    static float y = 30;
    static float xx = 8; // x方向の加速度
    static float yy = 0; // y方向の加速度
    const float g = 9.8; // 重力加速度
    const float m = 0.1; // 質量
    const int r = 10;    // ボールの半径

    int next_x = x + xx; // 次のx座標
    int next_y = y + yy; // 次のy座標

    mirror(frame, outputFrame); // 操作しやすいように、画像を鏡像にしておく
    // cvCopy(frame, outputFrame, NULL); // 鏡像にしない場合はこちらに置き換える

    // 画面をはみ出す場合の処理
    if (next_x > outputFrame->width)
        next_x = 0;
    if (next_x < 0)
        next_x = outputFrame->width - 1;
    if (next_y > outputFrame->height)
        next_y = outputFrame->height - 1;
    if (next_y < 0)
        next_y = 0;

    // 各方向跳ね返る場合の処理
    if (chk(outputFrame, next_x, next_y + r) < 128)
    {
        yy = -abs(yy) * 0.98;
    }

    if (chk(outputFrame, next_x + r, next_y) < 128)
    {
        xx = -abs(xx);
    }
    else if (chk(outputFrame, next_x - r, next_y) < 128)
    {
        xx = abs(xx);
    }

    x = next_x;  // x座標の更新
    y = next_y;  // y座標の更新
    yy += m * g; // y加速度の更新

    cvCircle(outputFrame, cvPoint(x, y), r, cvScalar(0, 255, 0), -1);
}

// 0-255にクリップする。ただし、負の値は絶対値とする
unsigned char clipAbs(int v)
{
    return (v < 0) ? -v : (v > 255) ? 255
                                    : v;
}

// 差分画像の生成
void diffim(IplImage *frame, IplImage *oldframe, IplImage *outputFrame)
{
    for (int y = 0; y < frame->height; y++)
    {
        for (int x = 0; x < frame->width; x++)
        {
            for (int ch = 0; ch < frame->nChannels; ch++)
            {
                outputFrame->imageData[y * outputFrame->widthStep + x * outputFrame->nChannels + ch] = clipAbs((unsigned char)frame->imageData[y * frame->widthStep + x * frame->nChannels + ch] - (unsigned char)oldframe->imageData[y * oldframe->widthStep + x * oldframe->nChannels + ch]);
            }
        }
    }
}

int main(int argc, char *argv[])
{

    CvCapture *capture = NULL; // ビデオキャプチャ構造体

    int c;
    int n = 0;
    IplImage *grab = NULL;        // cvQueryFrame() の返り値を一時的に保存するための変数
    IplImage *frame = NULL;       // 現在のフレーム(※この変数の内容を書き換えてはいけない)
    IplImage *outputFrame = NULL; // 表示する画像(画像処理結果を入れる)
    IplImage *oldframe = NULL;    // 一つ前のフレーム

    int mode = 0;    // 起動時のモード
    int width = 640; // デフォルトの画像サイズ
    int height = 480;

    // 起動オプションの数が 0 (argc==1)の場合　… キャプチャサイズ: 640x480、カメラ番号:0
    // 起動オプションの数が 1 (argc==2)の場合　… 指定した動画ファイルを読み込む
    // 起動オプションの数が 2 (argc==3)の場合　… キャプチャサイズ: arg[1]×arg[2]、カメラ番号:0
    // 起動オプションの数が 3 (argc==4)の場合　… キャプチャサイズ: arg[1]×arg[2]、カメラ番号:arg[3]
    switch (argc)
    {
    case 1:
        capture = cvCreateCameraCapture(0);
        printf("Capture mode. (Default)\n");
        break;

    case 2:
        capture = cvCaptureFromFile(argv[1]);
        printf("File mode : [%s]\n", argv[1]);
        break;

    case 3:
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        capture = cvCreateCameraCapture(0);
        printf("Capture mode. [%d x %d]\n", width, height);
        break;

    case 4:
        width = atoi(argv[1]);
        height = atoi(argv[2]);
        capture = cvCreateCameraCapture(atoi(argv[3]));
        printf("Capture mode. [%d x %d], Camera No = %d\n", width, height, atoi(argv[3]));
        break;
    }

    // 取込サイズの設定
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, width);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, height);

    // ウィンドウの表示
    //    cvNamedWindow("Capture");
    cvNamedWindow("Output");

    // カメラの準備ができるまで待つ
    while (cvQueryFrame(capture) == NULL)
    {
        c = cvWaitKey(2); // esc を押した場合は終了(存在しないカメラ番号を指定した場合などのため)
        if (c == '\x1b')
        {
            return -1;
        }
    }

    // 現在のフレームをキャプチャし、frameにコピー(同じフォーマットの画像を生成して初期化)
    frame = cvCloneImage(cvQueryFrame(capture));
    outputFrame = cvCloneImage(frame); // キャプチャ画像と同じフォーマットの画像を生成して初期化
    oldframe = cvCloneImage(frame);    // キャプチャ画像と同じフォーマットの画像を生成して初期化

    // モーションブラー用の画像群の初期化
    for (int k = 0; k < N_FRAME; k++)
    {
        historyFrame[k] = cvCloneImage(frame);
    }

    char modeStr[10][13] = {"Original", "drawShape()", "mirror()", "mirror2()", "scrumble()", "chR()", "chShift", "diffim", "motionBlur()", "ball"};

    printf("Hit '0'-'9' to select the mode.\n[space] : save the image as a png file\n[esc] : Exit.\n-----------------------------------------\n\n");
    // 動画像処理のループ ========================================
    while ((grab = cvQueryFrame(capture)) != NULL)
    {                              // NULLが返った場合(動画ファイルの場合は末尾に達した場合)は終了
                                   // カメラから画像フレームを取込み(grab)、作業用の変数(frame)にコピー
        cvCopy(grab, frame, NULL); // ※grab の中身は書き換えてはいけません!!

        switch (mode)
        {
        case 1:
            drawShape(frame, outputFrame); // 図形描画のデモ
            break;

        case 2:
            mirror(frame, outputFrame); // 鏡像
            break;

        case 3:
            mirror2(frame, outputFrame); // 鏡像2
            break;

        case 4:
            scrumble(frame, outputFrame); // 1ライン置きに鏡像に
            break;

        case 5:
            chR(frame, outputFrame); // Rチャネルのみ抽出
            break;

        case 6:
            chShift(frame, outputFrame); // カラーチャネルの入れ替え
            break;

        case 7:
            diffim(frame, oldframe, outputFrame); // 差分画像(絶対値)
            break;

        case 8:
            motionBlur(frame, 0.5, outputFrame); // モーションブラー
            break;

        case 9:
            ball(frame, outputFrame); // 動くボール画像の合成
            break;

        case 0:
        default:
            cvCopy(frame, outputFrame, NULL); // キャプチャ画像をそのまま表示
            break;
        }

        // 処理結果の表示
        //        cvShowImage("Capture", frame);
        cvShowImage("Output", outputFrame);

        // キー入力待ち
        c = cvWaitKey(2);
        if (c == '\x1b')
        { // escキーで終了
            break;
        }
        else if (c >= '0' && c <= '9')
        {                   // 0～9 を押した場合に、mode = 0～9 にセット
            mode = c - '0'; // ※ASCIIコードの並びを参照
            printf("mode = %d ( %s )\n", mode, modeStr[mode]);
        }
        else if (c == ' ')
        { // 画像ファイルとして保存
            n++;
            char buf[10];
            sprintf_s(buf, 10, "%d.png", n);
            cvSaveImage(buf, outputFrame);
            puts(buf);
        }

        cvCopy(frame, oldframe, NULL); // 一つ前のフレームとして保存
    }

    // 後始末
    cvReleaseCapture(&capture);
    //   cvDestroyWindow("Capture");
    cvDestroyWindow("Output");
}
