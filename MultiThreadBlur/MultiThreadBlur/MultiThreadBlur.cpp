#define DEBUG false
#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <windows.h>
#include <chrono>

#include "EasyBMP.h"

constexpr int MIN_SECTION_HEIGHT = 2;

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
    const RGBApixel main_pixel = bmp.GetPixel(i, j);
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
        static_cast<ebmpBYTE>(r / count),
        main_pixel.Alpha
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

    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    BMP bmp;
    bmp.ReadFromFile(input_path.c_str());

    std::cout << "Successfully read file \"" << input_path.c_str() << "\"" << std::endl;

    const int w = bmp.TellWidth();
    const int h = bmp.TellHeight();

#if DEBUG

    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            const auto blurred_pixel = ApplyBlurForPixel(i, j, bmp, blur_radius);
            bmp.SetPixel(i, j, blurred_pixel);
        }
    }

#else

    const DWORD_PTR affinity_mask = (1 << cores) - 1;

    const int section_height = max(h / threads, MIN_SECTION_HEIGHT);
    const int total_threads_count = min(threads, h / section_height);
    const int rest = max(h - (threads * section_height), 0);

    if (threads > h / section_height)
    {
        std::cerr
            << "Warning! Cannot have more than "
            << total_threads_count
            << " threads for "
            << h
            << "px height"
            << std::endl;
        std::cerr << "Setting threads count to " << total_threads_count << std::endl;
    }

    std::cout
        << "Blurring with radius "
        << blur_radius << " in "
        << total_threads_count
        << " threads on "
        << cores
        << " cores"
        << std::endl;

    ThreadData* params = new ThreadData[total_threads_count];
    HANDLE* handles = new HANDLE[total_threads_count];
    for (int i = 0; i < total_threads_count; i++)
    {
        const int top = i * section_height;
        const bool is_last = i == total_threads_count - 1;
        params[i] = ThreadData{
            &bmp,
            top,
            0,
            w,
            section_height + (is_last ? rest : 0),
            blur_radius
        };
        handles[i] = CreateThread(nullptr, 0, &ThreadFunc, &params[i], CREATE_SUSPENDED, nullptr);
        if (!SetThreadAffinityMask(handles[i], affinity_mask))
        {
            std::cerr << "Error occured when setting affinity mask" << std::endl;
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < total_threads_count; i++)
    {
        ResumeThread(handles[i]);
    }

    WaitForMultipleObjects(total_threads_count, handles, true, INFINITE);

    delete[] params;
    delete[] handles;

#endif

    bmp.WriteToFile(output_path.c_str());

    std::cout << "Successfully saved file to \"" << output_path.c_str() << "\"" << std::endl;

    const std::chrono::steady_clock::time_point now_time = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_time - start_time).count();

    std::cout << "Total duration: " << duration << "ms" << std::endl;

    return EXIT_SUCCESS;
}
