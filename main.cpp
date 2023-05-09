
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
    while (capture.Capture(imageFormat::IMAGE_RGBA8,1000,nullptr)!=nullptr){
        std::cout<<"wait for capture"<<std::endl;
    }
    //auto data=capture.Capture(imageFormat::IMAGE_RGBA8,1000,nullptr);
    //auto mat=cv::Mat(544,960,CV_8UC4,data->data());
    //cv::cvtColor(mat,mat,cv::COLOR_RGBA2BGR);
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    //cv::imwrite("/home/topeet/test.jpg",mat);
    std::cout<<"hello"<<std::endl;
    return 0;
}
