#define DEBUG false
#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <windows.h>

#include "EasyBMP.h"

struct ThreadData
{
    BMP* bmp;
    int top;
    int left;
    int width;
    int height;
    int blur_radius;
};

RGBApixel ApplyBlurForPixel(int i, int j, BMP& bmp, int blur_radius)
{
    int r = 0;
    int g = 0;
    int b = 0;
    int count = 0;
    const int width = bmp.TellWidth();
    const int height = bmp.TellHeight();
    for (int ix = -blur_radius; ix < blur_radius; ++ix)
    {
        for (int iy = -blur_radius; iy < blur_radius; ++iy)
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
    const int blur_radius = data->blur_radius;
    for (int i = left; i < left + width; i++)
    {
        for (int j = top; j < top + height; j++)
        {
            const RGBApixel blurred_pixel = ApplyBlurForPixel(i, j, *bmp, blur_radius);
            bmp->SetPixel(i, j, blurred_pixel);
        }
    }
    ExitThread(0);
}

int main(int argc, char* argv[])
{
    if (argv[1] == nullptr)
    {
        std::cerr << "You must specify input path" << std::endl;
        return EXIT_FAILURE;
    }
    if (argv[2] == nullptr)
    {
        std::cerr << "You must specify output path" << std::endl;
        return EXIT_FAILURE;
    }
    if (argv[3] == nullptr)
    {
        std::cerr << "You must specify threads count" << std::endl;
        return EXIT_FAILURE;
    }
    if (argv[4] == nullptr)
    {
        std::cerr << "You must specify cores count" << std::endl;
        return EXIT_FAILURE;
    }
    if (argv[5] == nullptr)
    {
        std::cerr << "You must specify blur radius" << std::endl;
        return EXIT_FAILURE;
    }


    const std::string input_path(argv[1]);
    const std::string output_path(argv[2]);
    const int threads = std::strtol(argv[3], nullptr, 10);
    const int cores = std::strtol(argv[4], nullptr, 10);
    const int blur_radius = std::strtol(argv[5], nullptr, 10);

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

    ThreadData* params = new ThreadData[threads];
    HANDLE* handles = new HANDLE[threads];
    for (int i = 0; i < threads; i++)
    {
        const int top = i * (h / threads);
        const int left = 0;
        const int width = w;
        const int height = h / threads;
        params[i] = ThreadData{&bmp, top, left, width, height, blur_radius};
        handles[i] = CreateThread(nullptr, 0, &ThreadFunc, &params[i], CREATE_SUSPENDED, nullptr);
    }

    for (int i = 0; i < threads; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(threads, handles, true, INFINITE);

    delete[] params;
    delete[] handles;

#endif

    bmp.WriteToFile(output_path.c_str());

    return EXIT_SUCCESS;
}
