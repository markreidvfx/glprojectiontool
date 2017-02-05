#include "imagereader.h"
#include <iostream>

#include <Magick++.h>

#include <list>
#include <chrono>


void setup_imagemagick()
{
    Magick::InitializeMagick(NULL);
}

static Magick::Image empty_image()
{
    return Magick::Image("1024x1024","rgba(0,0,0,0.0)");
}

bool read_image_blob(const char *blob_data, int blob_size, std::vector<float> &data, int &width, int &height)
{

    Magick::Blob blob(blob_data, blob_size);
    Magick::Image image;
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();
    std::cerr << "reading " << "blob" << "\n";

    bool result = false;
    try {
        image.read(blob);
    } catch (...) {
        std::cerr << "error reading" << "blob" << "\n";
        image = empty_image();
        //return false;
    }

    image.resize("1024x1024!");
    width = image.size().width();
    height = image.size().height();

    data.resize(width * height * 4 );
    image.write(0, 0, width, height, "RGBA",  Magick::FloatPixel, &data[0]);

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "image read in " << elapsed_seconds.count() << " secs\n";

    return true;
}

bool write_image_blob(const char *blob_data, int blob_size, const std::string &path)
{
    Magick::Blob blob(blob_data, blob_size);
    Magick::Image image;

    try {
        image.read(blob);
    } catch (...) {
        std::cerr << "error reading" << "blob" << "\n";
        return false;
    }

    image.depth(8);
    image.write(path);
}

bool read_image(const std::string &path,
                std::vector<float> &data,
                int &width, int &height)
{
    Magick::Image image;
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();
    std::cerr << "reading " << path << "\n";

    bool result = false;
    try {
        if (path.empty()) {
            image = empty_image();
        } else {
            image.read( path);
            result = true;
        }
    }
    catch (...) {
        std::cerr << "error reading" << path << "\n";
        image = empty_image();
        //return false;
    }
    image.resize("1024x1024!");
    width = image.size().width();
    height = image.size().height();

    data.resize(width * height * 4 );
    image.write(0, 0, width, height, "RGBA",  Magick::FloatPixel, &data[0]);

    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-start;
    std::cerr << "image read in " << elapsed_seconds.count() << " secs\n";

    return result;

}

bool read_image(const std::string &path, FloatImageData &image)
{
    return read_image(path, image.data, image.width, image.height);
}

static Magick::Image montage_tiles(const std::vector<FloatImageData> &data, int tiles)
{
    Magick::Image image;

    std::vector<Magick::Image> images;

    images.resize(data.size());
    for (int i = 0; i < data.size(); i++) {

        const FloatImageData &d = data[i];

        images[i].read(d.width, d.height, "RGBA", Magick::FloatPixel, &d.data[0]);
        char buff[100];
        snprintf(buff, sizeof(buff), "%04d", i);
        std::string pad = buff;
        std::string path = "tile" +  pad + ".jpg";
        //images[i].write(path);
    }

    Magick::Montage opts;
    Magick::Geometry t;
    t.width(tiles);
    t.height(tiles);

    opts.tile(t);

    opts.geometry(images[0].size());
    opts.backgroundColor("rgba(0, 0, 0, 0.0)");

    std::vector<Magick::Image> result;
    Magick::montageImages(&result, images.begin(), images.end(), opts);

    result[0].flip();

    //result[0].write("montage.png");
    return result[0];
}

static void prep_image(Magick::Image &image, const std::string &label)
{
    /*
    image.modifyImage();

    Magick::PixelPacket *pixels = image.getPixels(0, 0, image.size().width(), 1);
    for (int i = 0; i < image.size().width(); i++ ) {
        float alpha = .01 * 1.0 / (i+1);
        Magick::Color c = pixels[i];
        c.alpha(1.0 - alpha + c.alpha());
        pixels[i] = c;
    }
    image.syncPixels(); */

    image.strip();
    image.attribute("colorspace", "srgb");
    image.attribute("label", label);
    image.filterType(Magick::TriangleFilter);
    image.depth(16);
    image.magick("PSD");
    image.backgroundColor("rgba(0, 0, 0, 0.0)");
    image.compressType(Magick::RLECompression);
    image.type(Magick::TrueColorMatteType);
    image.verbose(true);

}


void write_template_psd(const std::string &imageplane,
                        const std::string &dest,
                        const std::string &data_dir,
                        const std::vector<FloatImageData> &color_tiles,
                        const std::vector<FloatImageData> &alpha_tiles,
                        const std::vector<FloatImageData> &contour_tiles,
                        int tiles,
                        Progress &p)
{
    Magick::Image plane;

    p.set_value(50);
    Magick::Image color = montage_tiles(color_tiles, tiles);
    p.set_value(52);
    Magick::Image alpha = montage_tiles(alpha_tiles, tiles);
    p.set_value(54);
    Magick::Image contour = montage_tiles(contour_tiles, tiles);
    p.set_value(58);

    Magick::Geometry geo("1920x1080!");



    alpha.channel(Magick::RedChannel);
    color.composite(alpha, 0, 0, Magick::CopyOpacityCompositeOp);
    color.resize(geo);
    //color.flip();
    p.set_value(60);
    //contour.flip();
    contour.resize(geo);
    p.set_value(65);
    Magick::Image empty(geo,"rgba(0,0,0,0.0)" );

    prep_image(empty, "clones");
    p.set_value(68);
    prep_image(color, "guides");
    p.set_value(68);
    prep_image(contour, "contour");

    p.set_value(77);

    if (imageplane.empty())
        plane = empty;
    else
        plane.read(imageplane);

    prep_image(plane, "background");
    p.set_value(80);
    //plane.write(data_dir + "/plane.png");
    //alpha.write(data_dir + "/alpha.png");
    //color.write(data_dir + "/color.png");
    //contour.write(data_dir + "/contour.png");
    //empty.write(data_dir + "/empty.png");

    p.set_value(85);

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
