#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <string>
#include <vector>
#include <mutex>
#include <future>

struct FloatImageData {
    std::vector<float> data;
    int width;
    int height;
};


class Progress {
public:
    void set_value(int value)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_value = value;
    }

    int value()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_value;
    }

private:
    int m_value;
    std::mutex m_lock;

};

void setup_imagemagick();

void read_image(const std::string &path,
                std::vector<float> &data,
                int &width, int &height);

void read_image(const std::string &path, FloatImageData &image);


void write_template_psd(const std::string &imageplane,
                        const std::string &dest,
                        const std::string &data_dir,
                        FloatImageData &color_data,
                        FloatImageData &alpha_data,
                        FloatImageData &contour_data,
                        Progress &p);

#endif // IMAGEREADER_H
