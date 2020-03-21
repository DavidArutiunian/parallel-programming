#define DEBUG false
#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <windows.h>

#include "EasyBMP.h"

constexpr int THREADS = 8;
constexpr int BLUR_RADIUS = 5;

struct ThreadData
{
    BMP* bmp;
    int top;
    int left;
    int width;
    int height;
};

RGBApixel ApplyBlurForPixel(int i, int j, BMP& bmp)
{
    int r = 0;
    int g = 0;
    int b = 0;
    int count = 0;
    const int width = bmp.TellWidth();
    const int height = bmp.TellHeight();
    for (int ix = -BLUR_RADIUS; ix < BLUR_RADIUS; ++ix)
    {
        for (int iy = -BLUR_RADIUS; iy < BLUR_RADIUS; ++iy)
        {
            const int x = i + iy;
            const int y = j + ix;
            const bool is_positive = x >= 0 && y >= 0;
            const bool no_overflow = x < width && y < height;
            if (is_positive && no_overflow)
            {
                const RGBApixel pixel = bmp.GetPixel(x, y);
                r += pixel.Red;
                g += pixel.Green;
                b += pixel.Blue;
                count++;
            }
        }
    }
    return RGBApixel{
        static_cast<ebmpBYTE>(b / count),
        static_cast<ebmpBYTE>(g / count),
        static_cast<ebmpBYTE>(r / count)
    };
}

DWORD WINAPI ThreadFunc(CONST LPVOID lp_param)
{
    const ThreadData* data = static_cast<ThreadData*>(lp_param);
    BMP* bmp = data->bmp;
    const int top = data->top;
    const int left = data->left;
    const int width = data->width;
    const int height = data->height;
    for (int i = left; i < left + width; i++)
    {
        for (int j = top; j < top + height; j++)
        {
            const RGBApixel blurred_pixel = ApplyBlurForPixel(i, j, *bmp);
            bmp->SetPixel(i, j, blurred_pixel);
        }
    }
    ExitThread(0);
}

int main(int argc, char* argv[])
{
    const std::string input_path(argv[1]);
    const std::string output_path(argv[2]);

    BMP bmp;
    bmp.ReadFromFile(input_path.c_str());

    const int w = bmp.TellWidth();
    const int h = bmp.TellHeight();

#if DEBUG

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            const auto blurred_pixel = ApplyBlurForPixel(i, j, bmp);
            bmp.SetPixel(i, j, blurred_pixel);
        }
    }

#else

    ThreadData* params = new ThreadData[THREADS];
    HANDLE* handles = new HANDLE[THREADS];
    for (int i = 0; i < THREADS; i++)
    {
        const int top = i * (h / THREADS);
        const int left = 0;
        const int width = w;
        const int height = h / THREADS;
        params[i] = ThreadData{&bmp, top, left, width, height};
        handles[i] = CreateThread(NULL, 0, &ThreadFunc, &params[i], CREATE_SUSPENDED, NULL);
    }

    for (int i = 0; i < THREADS; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(THREADS, handles, true, INFINITE);

    delete[] params;
    delete[] handles;

#endif

    bmp.WriteToFile(output_path.c_str());
}
