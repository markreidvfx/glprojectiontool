#include "imagereader.h"
#include <iostream>

#include <thread>
#include <future>

#include <Magick++.h>
//using namespace Magick;

#include <list>
#include <chrono>


void setup_imagemagick()
{
    Magick::InitializeMagick(NULL);
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

void read_image(const std::string &path, FloatImageData &image)
{
    read_image(path, image.data, image.width, image.height);
}

static void prep_image(Magick::Image &image)
{
    image.modifyImage();
    Magick::PixelPacket *pixels = image.getPixels(0, 0, image.size().width(), 1);

    for (int i = 0; i < image.size().width(); i++ ) {
        float alpha = .01 * 1.0 / (i+1);
        Magick::Color c = pixels[i];
        c.alpha(1.0 - alpha + c.alpha());
        pixels[i] = c;
    }
    image.syncPixels();

}


void write_template_psd(const std::string &imageplane,
                        const std::string &dest,
                        const std::string &data_dir,
                        FloatImageData &color_data,
                        FloatImageData &alpha_data,
                        FloatImageData &contour_data,
                        Progress &p)
{
    Magick::Image plane;
    Magick::Image color;
    Magick::Image alpha;
    Magick::Image contour;

    Magick::Geometry geo("1920x1080!");

    color.read(color_data.width, color_data.height, "RGBA", Magick::FloatPixel, &color_data.data[0]);
    alpha.read(alpha_data.width, alpha_data.height, "RGBA", Magick::FloatPixel, &alpha_data.data[0]);
    contour.read(contour_data.width, contour_data.height, "RGBA", Magick::FloatPixel, &contour_data.data[0]);

    p.set_value(50);

    alpha.channel(Magick::RedChannel);
    color.composite(alpha, 0, 0, Magick::CopyOpacityCompositeOp);
    color.attribute("label", "guides");
    color.resize(geo);
    color.flip();

    contour.flip();
    contour.attribute("label", "contour");
    contour.resize(geo);

    Magick::Image empty(geo,"rgba(0,0,0,0.0)" );
    empty.type(Magick::TrueColorMatteType);
    empty.attribute("label", "clones");

    prep_image(empty);
    prep_image(color);
    prep_image(contour);

    p.set_value(60);

    plane.read(imageplane);

    plane.write(data_dir + "/plane.png");
    alpha.write(data_dir + "/alpha.png");
    color.write(data_dir + "/color.png");
    contour.write(data_dir + "/contour.png");
    empty.write(data_dir + "/empty.png");

    p.set_value(80);

    plane.attribute("colorspace", "rgb");
    plane.magick("PSD");
    plane.depth(16);
    plane.verbose(true);
    plane.compressType(Magick::RLECompression);
    plane.attribute("label", "background");

    Magick::Image liquify = plane;
    liquify.attribute("label", "liquify");

    std::list<Magick::Image> images;
    images.push_back(plane);
    images.push_back(plane);
    images.push_back(liquify);
    images.push_back(empty);
    images.push_back(color);
    images.push_back(contour);
    p.set_value(90);
    Magick::writeImages(images.begin(), images.end(), dest);
}
