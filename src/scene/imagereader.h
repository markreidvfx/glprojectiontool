#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <string>
#include <vector>
#include <mutex>

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

bool read_image(const std::string &path,
                std::vector<float> &data,
                int &width, int &height);

bool read_image_blob(const char *blob_data, int blob_size, std::vector<float> &data, int &width, int &height);
bool write_image_blob(const char *blob_data, int blob_size, const std::string &path);

void read_image(const std::string &path, FloatImageData &image);


void write_template_psd(const std::string &imageplane,
                        const std::string &dest,
                        const std::string &data_dir,
                        const std::vector<FloatImageData> &color_tiles,
                        const std::vector<FloatImageData> &alpha_tiles,
                        const std::vector<FloatImageData> &contour_tiles,
                        int tiles,
                        Progress &p);

//void montage_tiles(const std::vector<FloatImageData> &data, int tiles);

#endif // IMAGEREADER_H
