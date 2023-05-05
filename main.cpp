
#include "iostream"
#include "memory_resource"
#include "VideoAudioCapture.h"
//launch gstreamer
void launch_gs(){


}

int main(){



    VideoAudioCapture capture;
    capture.buildLaunchStr();
    capture.initPipeline();
    std::cout<<"hello";
}
