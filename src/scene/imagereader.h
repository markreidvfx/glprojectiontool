#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <string>
#include <vector>
#include <mutex>
#include <future>


void read_image(const std::string &path,
                std::vector<float> &data,
                int &width, int &height);


class ImageReader
{
public:
    ImageReader();

    void read(const std::string &path, double time);
    bool dataReady(double &time);
    std::vector<float> image_data;
    int width()
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_width;
    }
    void setWidth(int width)
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        m_width = width;
    }

    int height()
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_height;
    }

    void setHeight(int height)
    {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        m_height = height;
    }

    void setPath(const std::string &path, double time) {
         std::lock_guard<std::recursive_mutex> lock(m_lock);
         m_requested_time = time;
         m_path = path;
    }
    void requeue();
    std::string path() {
        std::lock_guard<std::recursive_mutex> lock(m_lock);
        return m_path;
    }
    void clear()
    {
        std::lock_guard<std::recursive_mutex> lock(data_lock);
        image_data.clear();
    }

    bool loadData();
    std::recursive_mutex data_lock;

private:

    bool m_loading;
    double m_loading_time;
    double m_requested_time;

    int m_width;
    int m_height;

    std::string m_path;
    std::future<bool> m_future;
    std::recursive_mutex m_lock;

};


#endif // IMAGEREADER_H
