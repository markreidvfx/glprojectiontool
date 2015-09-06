#include "imagereader.h"
#include <iostream>

#include <thread>
#include <future>

#include <Magick++.h>
//using namespace Magick;

#include <chrono>

ImageReader::ImageReader()
{
    Magick::InitializeMagick(NULL);
    m_loading = false;
    m_requested_time = 0;
}


static bool load(ImageReader *reader)
{
    //std::cerr << "loading image bg in thread\n";
    bool result = reader->loadData();
    //std::cerr << "LOADDDED FULL image in bg thread\n";
    return result;

}

void read_image(const std::string &path,
                std::vector<float> &data,
                int &width, int &height)
{
    Magick::Image image;
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();

    try {
        //image.read("logo:");
        image.read( path);
    }
    catch (...) {
        std::cerr << "error reading" << path << "\n";
        image.read("logo:");
        //return false;
    }
    image.resize("1024x1024!");
    width = image.size().width();
    height = image.size().height();

    data.resize(width * height * 4 );
    image.write(0, 0, width, height, "RGBA",  Magick::FloatPixel, &data[0]);

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "image read in " << elapsed_seconds.count() << " secs\n";

}

void ImageReader::read(const std::string &path, double time)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    setPath(path, time);

    if(!m_loading) {
        m_loading = true;
        m_loading_time = time;
        m_future = std::async(std::launch::async, load, this);
    }

}
bool ImageReader::loadData()
{
    Magick::Image image;
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();

    try {
        image.read( path());
    }
    catch (...) {
        std::cerr << "error reading" << path() << "\n";
        //return false;
        image.read("logo:");
    }
    image.resize("1024x1024!");
    int image_width =image.size().width();
    int image_height = image.size().height();
    // std::cerr << image_width << "x" << image_height << "\n";
    std::lock_guard<std::recursive_mutex> lock(data_lock);
    setWidth(image_width);
    setHeight(image_height);
    image_data.resize(image_width * image_height * 4 );
    image.write(0, 0, image_width, image_height, "RGBA",  Magick::FloatPixel, &image_data[0]);

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "image read in " << elapsed_seconds.count() << " secs\n";

    return true;
}
void ImageReader::requeue()
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    read(m_path, m_requested_time);
}

bool ImageReader::dataReady(double &time)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (!m_loading)
        return false;

    std::future_status status;
    status = m_future.wait_for(std::chrono::microseconds(1));

    if (status ==  std::future_status::ready){
        m_loading = false;
        time = m_loading_time;
        return m_future.get();
    }

    return false;
}
