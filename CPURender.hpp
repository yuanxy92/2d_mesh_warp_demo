#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/opencv.hpp>
#if CV_MAJOR_VERSION == 2
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#else
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#endif

class MyUtility {
public:
	static int mkdir(char* foldername) {
        #ifdef WIN32
		char command[256];
		sprintf(command, "mkdir %s", replaceSlash(foldername).c_str());
		system(command);
        #else
            char command[256];
            sprintf(command, "mkdir -p %s", foldername);
            system(command);
        #endif
            return 0;
    }

	static int mkdir(std::string foldername) {
        mkdir((char *)foldername.c_str());
	    return 0;
    }

	static std::string replaceSlash(std::string input) {
        std::string::size_type startpos = 0;
        while (startpos != std::string::npos)
        {
            startpos = input.find('/'); 
            if (startpos != std::string::npos) {
                input.replace(startpos, 1, "\\");
            }
        }
        return input;
    }
};

class MeshWarp {
private:
    int meshRows;
    int meshCols;
    int imgWidth;
    int imgHeight;
    int quadWidth;
    int quadHeight;

    cv::Mat sourceMesh;
    cv::Mat targetMesh;
public:

private:

public:
    MeshWarp();
    ~MeshWarp();

    int setMesh(std::string filename);
    int computeWarpingField(cv::Mat & warpingField, cv::Mat & mask);
};


class CPURender {
private:
    float scaleFactor;
    int camNum;
    std::vector<cv::Point2f> refpos;
    std::string datapath;
    std::string outputpath;

    cv::Mat refImg;
    std::vector<cv::Mat> detailImgs;
    std::vector<cv::Mat> masks;
public:

private:
    cv::Mat imfillholes(cv::Mat src);
    cv::Mat_<cv::Vec3f> applyLinearBlendMask(cv::Mat input, cv::Mat_<float> mask);
    int genGraphCutMask(std::vector<cv::Mat> smallImgs, std::vector<cv::Point2i> graphcutCorners,
         std::vector<cv::Mat> & masks);
public:
    CPURender();
    ~CPURender();

    int setInput(std::string datapath, std::string outputpath);
    int warp();
    int render();

};
