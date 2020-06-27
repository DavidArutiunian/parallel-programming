#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <vector>
#include <utility>
#include <tuple>

#include "EasyBMP.h"

constexpr int MIN_SECTION_HEIGHT = 2;
constexpr int BLUR_RADIUS = 5;

struct ThreadData
{
    BMP* bmp;
    int index;
    int top;
    int left;
    int width;
    int height;
    int blur_radius;
};

template <typename T>
T safe_div(T a, T b) noexcept
{
    return a / max(b, 1);
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

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
        static_cast<ebmpBYTE>(safe_div(b, count)),
        static_cast<ebmpBYTE>(safe_div(g, count)),
        static_cast<ebmpBYTE>(safe_div(r, count)),
        main_pixel.Alpha
    };
}

DWORD WINAPI ThreadFunc(CONST LPVOID lp_param)
{
    const ThreadData* data = static_cast<ThreadData*>(lp_param);
    BMP* bmp = data->bmp;
    const int thread_index = data->index;
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

auto get_arg_factory(int argc, char* argv[]) {
    return [=](int index, const char* fallback = nullptr) -> const char* {
        if (argc <= index) {
            return fallback;
        }
        return argv[index];
    };
}

int main(int argc, char* argv[])
{

    const auto get_arg = get_arg_factory(argc, argv);

    const auto raw_input_path = get_arg(1);
    if (raw_input_path == nullptr)
    {
        std::cerr << "Path to input directory must be specified" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string input_path = get_arg(1);

    const auto raw_output_path = get_arg(2);
    if (raw_output_path == nullptr)
    {
        std::cerr << "Path to output directory must be specified" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string output_path = get_arg(2);

    constexpr char default_mode[] = "thread"; // default mode
    const std::string mode(get_arg(3, default_mode));

    constexpr char default_count[] = "4"; // default count for threads and blocks
    const int blocks_count = std::strtol(get_arg(4, default_count), nullptr, 10);
    const int threads_count = std::strtol(get_arg(5, default_count), nullptr, 10);

    std::vector<std::string> input_file_paths;
    std::vector<std::string> output_file_paths;

    /**
    * @see https://docs.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
    */
    WIN32_FIND_DATAA ffd;
    HANDLE const h_find = FindFirstFileA((input_path + "\\*").data(), &ffd);

    if (h_find != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                const std::string file_name = &ffd.cFileName[0];
                std::string file_ext = file_name.substr(file_name.find('.'));
                std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), std::tolower);
                if (file_ext == ".bmp" || file_ext == ".jpg" || file_ext == ".png") { // only bmp, jpg, png supported
                    input_file_paths.push_back(input_path + "\\" + file_name);
                    output_file_paths.push_back(output_path + "\\" + file_name);
                }
            }
        } while (FindNextFileA(h_find, &ffd) != 0);
        FindClose(h_find);
    }

    std::vector<std::pair<std::string, std::string>> file_paths;
    for (std::size_t i = 0; i < input_file_paths.size(); i++)
    {
        auto zipped = std::make_pair(input_file_paths[i], output_file_paths[i]);
        file_paths.push_back(zipped);
    }

    if (mode == "thread")
    {
        for (auto&& zip : file_paths)
        {
            std::string input_path, output_path;
            std::tie(input_path, output_path) = zip;

            BMP bmp;
            bmp.ReadFromFile(input_path.c_str());

            std::cout << "Successfully read file \"" << input_path.c_str() << "\"" << std::endl;

            const int w = bmp.TellWidth();
            const int h = bmp.TellHeight();


            const int section_height = max(h / threads_count, MIN_SECTION_HEIGHT);
            const int total_threads_count = min(threads_count, h / section_height);
            const int rest = max(h - (threads_count * section_height), 0);

            if (threads_count > h / section_height)
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
                << "Blurring "
                << input_path
                << "with radius "
                << BLUR_RADIUS << " in "
                << total_threads_count
                << " threads"
                << std::endl;

            ThreadData* params = new ThreadData[total_threads_count];
            HANDLE* handles = new HANDLE[total_threads_count];
            for (int i = 0; i < total_threads_count; i++)
            {
                const int top = i * section_height;
                const bool is_last = i == total_threads_count - 1;
                params[i] = ThreadData{
                    &bmp,
                    i,
                    top,
                    0,
                    w,
                    section_height + (is_last ? rest : 0),
                    BLUR_RADIUS
                };
                handles[i] = CreateThread(nullptr, 0, &ThreadFunc, &params[i], CREATE_SUSPENDED, nullptr);
            }

            for (int i = 0; i < total_threads_count; i++)
            {
                ResumeThread(handles[i]);
            }

            WaitForMultipleObjects(total_threads_count, handles, true, INFINITE);

            delete[] params;
            delete[] handles;

            bmp.WriteToFile(output_path.c_str());
        }
    }


    return EXIT_SUCCESS;
}
