#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "CPURender.hpp"

int main(int argc, char * argv[]) {
    std::string datapath = std::string(argv[1]);
    std::string outputpath = std::string(argv[2]);
    std::cout << "Hello world!" << std::endl;
    MyUtility::mkdir(outputpath);
    CPURender render;
    render.setInput(datapath, outputpath);
	render.warp();
    render.render();
    return 0;
}