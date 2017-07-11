#include <android/bitmap.h>
#include "common.h"

uint64_t *mIntegralMatrix = NULL;
uint64_t *mIntegralMatrixSqr = NULL;
uint8_t *mSkinMatrix = NULL;
uint32_t *mImageData_rgb = NULL;
uint8_t *mImageData_yuv = NULL;

void initBeautiMatrix(uint32_t *pix, int width, int height);

void initSkinMatrix(uint32_t *pix, int width, int height);

void initIntegralMatrix(int width, int height);

void setSmooth(uint32_t *pix, float smoothValue, int width, int height);

void setWhiteSkin(uint32_t *pix, float whiteVal, int width, int height);

void freeMatrix();

extern "C"
JNIEXPORT void JNICALL
Java_com_xinlan_meitu_Tools_freeBeautifyMatrix(JNIEnv *env, jobject obj) {
    freeMatrix();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xinlan_meitu_Tools_handleSmoothAndWhiteSkin(JNIEnv *env, jobject obj, jobject bitmap,
                                         jfloat smoothValue,jfloat whiteValue) {
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if (bitmap == NULL)
        return;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    LOGE("Bitmap smooth and whiteskin handle");
    initBeautiMatrix((uint32_t *) pixels, info.width, info.height);

    LOGE("Bitmap smooth = %f and whiteSkin = %f", smoothValue,whiteValue);

    setSmooth((uint32_t *) pixels, smoothValue, info.width, info.height);
    setWhiteSkin((uint32_t *) pixels, whiteValue, info.width, info.height);

    AndroidBitmap_unlockPixels(env, bitmap);

    //free memory code
    freeMatrix();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_xinlan_meitu_Tools_handleSmooth(JNIEnv *env, jobject obj, jobject bitmap,
                                         jfloat smoothValue) {
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if (bitmap == NULL)
        return;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    LOGE("AndroidBitmap_smooth handle");

    initBeautiMatrix((uint32_t *) pixels, info.width, info.height);
    setSmooth((uint32_t *) pixels, smoothValue, info.width, info.height);

    AndroidBitmap_unlockPixels(env, bitmap);

    //free memory code
    freeMatrix();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xinlan_meitu_Tools_handleWhiteSkin(JNIEnv *env, jobject obj, jobject bitmap,
                                            jfloat whiteValue) {
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if (bitmap == NULL)
        return;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    LOGE("AndroidBitmap_whiteSkin handle");
    initBeautiMatrix((uint32_t *) pixels, info.width, info.height);

    LOGE("AndroidBitmap_whiteSkin whiteValue = %f", whiteValue);
    setWhiteSkin((uint32_t *) pixels, whiteValue, info.width, info.height);

    AndroidBitmap_unlockPixels(env, bitmap);

    //free memory
    freeMatrix();
}

void setWhiteSkin(uint32_t *pix, float whiteVal, int width, int height) {
    if (whiteVal >= 1.0 && whiteVal <= 10.0) { //1.0~10.0
        float a = log(whiteVal);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int offset = i * width + j;
                ARGB RGB;
                convertIntToArgb(mImageData_rgb[offset], &RGB);
                if (a != 0) {
                    RGB.red = 255 * (log(div255(RGB.red) * (whiteVal - 1) + 1) / a);
                    RGB.green = 255 * (log(div255(RGB.green) * (whiteVal - 1) + 1) / a);
                    RGB.blue = 255 * (log(div255(RGB.blue) * (whiteVal - 1) + 1) / a);
                }
                pix[offset] = convertArgbToInt(RGB);
            }
        }
    }//end if
}

void setSmooth(uint32_t *pix, float smoothValue, int width, int height) {//磨皮操作
    if (mIntegralMatrix == NULL || mIntegralMatrixSqr == NULL || mSkinMatrix == NULL) {//预操作辅助未准备好
        LOGE("not init correctly");
        return;
    }

    LOGE("AndroidBitmap_smooth setSmooth start---- smoothValue = %f", smoothValue);

    RGBToYCbCr((uint8_t *) mImageData_rgb, mImageData_yuv, width * height);

    int radius = width > height ? width * 0.02 : height * 0.02;

    for (int i = 1; i < height; i++) {
        for (int j = 1; j < width; j++) {
            int offset = i * width + j;
            if (mSkinMatrix[offset] == 255) {
                int iMax = i + radius >= height - 1 ? height - 1 : i + radius;
                int jMax = j + radius >= width - 1 ? width - 1 : j + radius;
                int iMin = i - radius <= 1 ? 1 : i - radius;
                int jMin = j - radius <= 1 ? 1 : j - radius;

                int squar = (iMax - iMin + 1) * (jMax - jMin + 1);
                int i4 = iMax * width + jMax;
                int i3 = (iMin - 1) * width + (jMin - 1);
                int i2 = iMax * width + (jMin - 1);
                int i1 = (iMin - 1) * width + jMax;

                float m = (mIntegralMatrix[i4]
                           + mIntegralMatrix[i3]
                           - mIntegralMatrix[i2]
                           - mIntegralMatrix[i1]) / squar;

                float v = (mIntegralMatrixSqr[i4]
                           + mIntegralMatrixSqr[i3]
                           - mIntegralMatrixSqr[i2]
                           - mIntegralMatrixSqr[i1]) / squar - m * m;
                float k = v / (v + smoothValue);

                mImageData_yuv[offset * 3] = ceil(m - k * m + k * mImageData_yuv[offset * 3]);
            }
        }
    }
    YCbCrToRGB(mImageData_yuv, (uint8_t *) pix, width * height);

    LOGE("AndroidBitmap_smooth setSmooth END!----");
}

void freeMatrix() {
    if (mIntegralMatrix != NULL) {
        delete[] mIntegralMatrix;
        mIntegralMatrix = NULL;
    }

    if (mIntegralMatrixSqr != NULL) {
        delete[] mIntegralMatrixSqr;
        mIntegralMatrixSqr = NULL;
    }

    if (mSkinMatrix != NULL) {
        delete[] mSkinMatrix;
        mSkinMatrix = NULL;
    }

    if (mImageData_rgb != NULL) {
        delete[] mImageData_rgb;
        mImageData_rgb = NULL;
    }

    if (mImageData_yuv != NULL) {
        delete[] mImageData_yuv;
        mImageData_yuv = NULL;
    }
}

void initBeautiMatrix(uint32_t *pix, int width, int height) {
    if (mImageData_rgb == NULL)
        mImageData_rgb = new uint32_t[width * height];

    memcpy(mImageData_rgb, pix, sizeof(uint32_t) * width * height);

    if (mImageData_yuv == NULL)
        mImageData_yuv = new uint8_t[width * height * 3];
    RGBToYCbCr((uint8_t *) mImageData_rgb, mImageData_yuv, width * height);

    initSkinMatrix(pix, width, height);
    initIntegralMatrix(width, height);
}

void initSkinMatrix(uint32_t *pix, int w, int h) {
    LOGE("start - initSkinMatrix");
    if (mSkinMatrix == NULL)
        mSkinMatrix = new uint8_t[w * h];

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int offset = i * w + j;
            ARGB RGB;
            convertIntToArgb(pix[offset], &RGB);
            if ((RGB.blue > 95 && RGB.green > 40 && RGB.red > 20 &&
                 RGB.blue - RGB.red > 15 && RGB.blue - RGB.green > 15) ||//uniform illumination
                (RGB.blue > 200 && RGB.green > 210 && RGB.red > 170 &&
                 abs(RGB.blue - RGB.red) <= 15 && RGB.blue > RGB.red &&
                 RGB.green > RGB.red))//lateral illumination
                mSkinMatrix[offset] = 255;
            else
                mSkinMatrix[offset] = 0;
        }
    }
    LOGE("end - initSkinMatrix");
}

void initIntegralMatrix(int width, int height) {
    LOGE("initIntegral");
    LOGE("width = %d height = %d", width, height);

    if (mIntegralMatrix == NULL)
        mIntegralMatrix = new uint64_t[width * height];
    if (mIntegralMatrixSqr == NULL)
        mIntegralMatrixSqr = new uint64_t[width * height];

    LOGE("malloc complete");

    uint64_t *columnSum = new uint64_t[width];
    uint64_t *columnSumSqr = new uint64_t[width];

    columnSum[0] = mImageData_yuv[0];
    columnSumSqr[0] = mImageData_yuv[0] * mImageData_yuv[0];

    mIntegralMatrix[0] = columnSum[0];
    mIntegralMatrixSqr[0] = columnSumSqr[0];

    for (int i = 1; i < width; i++) {

        columnSum[i] = mImageData_yuv[3 * i];
        columnSumSqr[i] = mImageData_yuv[3 * i] * mImageData_yuv[3 * i];

        mIntegralMatrix[i] = columnSum[i];
        mIntegralMatrix[i] += mIntegralMatrix[i - 1];
        mIntegralMatrixSqr[i] = columnSumSqr[i];
        mIntegralMatrixSqr[i] += mIntegralMatrixSqr[i - 1];
    }

    for (int i = 1; i < height; i++) {
        int offset = i * width;

        columnSum[0] += mImageData_yuv[3 * offset];
        columnSumSqr[0] += mImageData_yuv[3 * offset] * mImageData_yuv[3 * offset];

        mIntegralMatrix[offset] = columnSum[0];
        mIntegralMatrixSqr[offset] = columnSumSqr[0];

        for (int j = 1; j < width; j++) {
            columnSum[j] += mImageData_yuv[3 * (offset + j)];
            columnSumSqr[j] += mImageData_yuv[3 * (offset + j)] * mImageData_yuv[3 * (offset + j)];

            mIntegralMatrix[offset + j] = mIntegralMatrix[offset + j - 1] + columnSum[j];
            mIntegralMatrixSqr[offset + j] = mIntegralMatrixSqr[offset + j - 1] + columnSumSqr[j];
        }
    }
    delete[] columnSum;
    delete[] columnSumSqr;
    LOGE("initIntegral~end");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_xinlan_meitu_Tools_processImage(JNIEnv *env, jobject obj, jobject bitmap) {
    //LOGE("process Imagess!");
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        //LOGE(");
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    blur_ARGB_8888((int *) pixels, info.width, info.height, 100);//调用模糊方法进行模糊

    AndroidBitmap_unlockPixels(env, bitmap);
}

int *blur_ARGB_8888(int *pix, int w, int h, int radius) {
    int wm = w - 1;
    int hm = h - 1;
    int wh = w * h;
    int div = radius + radius + 1;

    short *r = (short *) malloc(wh * sizeof(short));
    short *g = (short *) malloc(wh * sizeof(short));
    short *b = (short *) malloc(wh * sizeof(short));
    int rsum, gsum, bsum, x, y, i, p, yp, yi, yw;

    int *vmin = (int *) malloc(MAX(w, h) * sizeof(int));

    int divsum = (div + 1) >> 1;
    divsum *= divsum;
    short *dv = (short *) malloc(256 * divsum * sizeof(short));
    for (i = 0; i < 256 * divsum; i++) {
        dv[i] = (short) (i / divsum);
    }

    yw = yi = 0;

    int(*stack)[3] = (int (*)[3]) malloc(div * 3 * sizeof(int));
    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    int r1 = radius + 1;
    int routsum, goutsum, boutsum;
    int rinsum, ginsum, binsum;

    for (y = 0; y < h; y++) {
        rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
        for (i = -radius; i <= radius; i++) {
            p = pix[yi + (MIN(wm, MAX(i, 0)))];
            sir = stack[i + radius];
            sir[0] = (p & 0xff0000) >> 16;
            sir[1] = (p & 0x00ff00) >> 8;
            sir[2] = (p & 0x0000ff);

            rbs = r1 - ABS(i);
            rsum += sir[0] * rbs;
            gsum += sir[1] * rbs;
            bsum += sir[2] * rbs;
            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }
        }
        stackpointer = radius;

        for (x = 0; x < w; x++) {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (y == 0) {
                vmin[x] = MIN(x + radius + 1, wm);
            }
            p = pix[yw + vmin[x]];

            sir[0] = (p & 0xff0000) >> 16;
            sir[1] = (p & 0x00ff00) >> 8;
            sir[2] = (p & 0x0000ff);

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[(stackpointer) % div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];

            yi++;
        }
        yw += w;
    }
    for (x = 0; x < w; x++) {
        rinsum = ginsum = binsum = routsum = goutsum = boutsum = rsum = gsum = bsum = 0;
        yp = -radius * w;
        for (i = -radius; i <= radius; i++) {
            yi = MAX(0, yp) + x;

            sir = stack[i + radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];

            rbs = r1 - ABS(i);

            rsum += r[yi] * rbs;
            gsum += g[yi] * rbs;
            bsum += b[yi] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }

            if (i < hm) {
                yp += w;
            }
        }
        yi = x;
        stackpointer = radius;
        for (y = 0; y < h; y++) {
            // Preserve alpha channel: ( 0xff000000 & pix[yi] )
            pix[yi] = (0xff000000 & pix[yi]) | (dv[rsum] << 16) | (dv[gsum] << 8) | dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (x == 0) {
                vmin[y] = MIN(y + r1, hm) * w;
            }
            p = x + vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];

            yi += w;
        }
    }

    free(r);
    free(g);
    free(b);
    free(vmin);
    free(dv);
    free(stack);
    return (pix);
}









