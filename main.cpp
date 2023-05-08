
#include "iostream"
#include "memory_resource"
#include "VideoAudioCapture.h"
#include "opencv2/opencv.hpp"
//launch gstreamer
void launch_gs(){


}

int main(){

    VideoAudioOption option;

    auto xx=std::make_unique<VideoAudioOption>();
    VideoAudioCapture capture(std::move(xx));

    spdlog::set_level(spdlog::level::debug);
    capture.initPipeline();
    capture.Open();
    auto data=capture.Capture(imageFormat::IMAGE_RGBA8,1000,nullptr);


    cv::imwrite("/home/topeet/test.jpg",cv::Mat(960,544,CV_8UC4,data->data()));
    std::cout<<"hello";
    return 0;
}
