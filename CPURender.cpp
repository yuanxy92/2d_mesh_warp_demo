#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/stitching.hpp>

#include "CPURender.hpp"

/**
* Mesh Warp
*/

MeshWarp::MeshWarp(){}
MeshWarp::~MeshWarp(){}

int MeshWarp::setMesh(std::string filename) {
    std::fstream fs(filename, std::ios::in);
    // read mesh information
    fs >> meshRows;
    fs >> meshCols;
    fs >> quadHeight;
    fs >> quadWidth;
    fs >> imgHeight;
    fs >> imgWidth;
    // init mesh
    sourceMesh = cv::Mat::zeros(meshRows, meshCols, CV_32FC2);
    targetMesh = cv::Mat::zeros(meshRows, meshCols, CV_32FC2);
    //
    for (int i = 0; i < sourceMesh.rows; i ++) {
        for (int j = 0; j < sourceMesh.cols; j ++) {
            float srcX, srcY, tarX, tarY;
            fs >> srcX >> srcY >> tarX >> tarY;
            sourceMesh.at<cv::Point2f>(i, j) = cv::Point2f(srcX, srcY);
            targetMesh.at<cv::Point2f>(i, j) = cv::Point2f(tarX, tarY);
        }
    }
    fs.close();
    return 0;
}

int MeshWarp::computeWarpingField(cv::Mat & warpingField, cv::Mat & mask) {
    warpingField = cv::Mat::zeros(this->imgHeight, this->imgWidth, CV_32FC2);
    mask.create(this->imgHeight, this->imgWidth, CV_8U);
    mask = cv::Scalar(255);
    for (int i = 0; i < meshRows - 1; i ++) {
		for (int j = 0; j < meshCols - 1; j ++) {
#ifdef MY_DEBUG
			printf("Calculate flowfield for quad (%d, %d) ...\n", i, j);
#endif
			// get 4 source points
			std::vector<cv::Point2f> vs;
			vs.resize(4);
			vs[0] = sourceMesh.at<cv::Point2f>(i, j);
			vs[1] = sourceMesh.at<cv::Point2f>(i, j + 1);
			vs[2] = sourceMesh.at<cv::Point2f>(i + 1, j);
			vs[3] = sourceMesh.at<cv::Point2f>(i + 1, j + 1);
			// get 4 target points
			std::vector<cv::Point2f> vt;
			vt.resize(4);
			vt[0] = targetMesh.at<cv::Point2f>(i, j);
			vt[1] = targetMesh.at<cv::Point2f>(i, j + 1);
			vt[2] = targetMesh.at<cv::Point2f>(i + 1, j);
			vt[3] = targetMesh.at<cv::Point2f>(i + 1, j + 1);
			// get homography matrix
			cv::Mat H = getPerspectiveTransform(vs, vt);
			H.convertTo(H, CV_32F);
			// calculate target points of all the pixels in this quad
			cv::Mat pts = cv::Mat::zeros(3, (static_cast<int>(vs[3].y) - static_cast<int>(vs[0].y) + 1)
             * (static_cast<int>(vs[3].x) - static_cast<int>(vs[0].x) + 1), CV_32F);
			int index = 0;
			for (int pi = static_cast<int>(vs[0].y); pi <= static_cast<int>(vs[3].y); pi++) {
				for (int pj = static_cast<int>(vs[0].x); pj <= static_cast<int>(vs[3].x); pj++) {
					pts.at<float>(0, index) = pj;
					pts.at<float>(1, index) = pi;
					pts.at<float>(2, index) = 1.0;
					index++;
				}
			}
			pts = H * pts;
			index = 0;
			for (int pi = static_cast<int>(vs[0].y); pi <= static_cast<int>(vs[3].y); pi++) {
				for (int pj = static_cast<int>(vs[0].x); pj <= (vs[3].x); pj++) {
					cv::Point2f p;
					p.x = pts.at<float>(0, index) / pts.at<float>(2, index);
					p.y = pts.at<float>(1, index) / pts.at<float>(2, index);
					warpingField.at<cv::Point2f>(pi, pj) = p;
                    if ((p.x < 0) || (p.y < 0) || (p.x >= imgWidth) || (p.y >= imgHeight)) {
                        mask.at<uchar>(pi, pj) = 0;
                    }
					index++;
				}
			}
		}
	}
    return 0;
}


/**
* CPU Render
*/

CPURender::CPURender(){}
CPURender::~CPURender(){}

int CPURender::setInput(std::string datapath, std::string outputpath) {
    // read basic information
    this->datapath = datapath;
    this->outputpath = outputpath;
    std::fstream fs(cv::format("%s/info.txt", datapath.c_str()), std::ios::in);
    fs >> scaleFactor;
    fs >> camNum;
    refpos.clear();
    for (int i = 0; i < camNum; i ++) {
        int x, y;
        fs >> x;
        fs >> y;
        refpos.push_back(cv::Point2f(x, y));
    }
    fs.close();
    return 0;
}

int CPURender::warp() {
    // read image
    refImg = cv::imread(cv::format("%s/ref.jpg", datapath.c_str()));
    for (int i = 0; i < camNum; i ++) {
        detailImgs.push_back(cv::imread(cv::format("%s/%d.jpg", datapath.c_str(), i)));
    }
    // warping
    for (int i = 0; i < camNum; i ++) {
        cv::Mat warpImg;
        std::string meshfile = cv::format("%s/%d_mesh_after.txt", datapath.c_str(), i);
        MeshWarp meshwarp;
        meshwarp.setMesh(meshfile);
        cv::Mat mask, warpingField;
        meshwarp.computeWarpingField(warpingField, mask);
        cv::remap(detailImgs[i], warpImg, warpingField, cv::Mat(), cv::INTER_LINEAR);
        cv::imwrite(cv::format("%s/%d_warped.jpg", outputpath.c_str(), i), warpImg);
        cv::imwrite(cv::format("%s/%d_mask.png", outputpath.c_str(), i), mask);
    }
    return 0;
}

cv::Mat CPURender::imfillholes(cv::Mat src) {
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	if (!contours.empty() && !hierarchy.empty()) {
		for (int idx = 0; idx < contours.size(); idx++) {
			drawContours(src, contours, idx, cv::Scalar::all(255), CV_FILLED, 8);
		}
	}
	return src;
}

cv::Mat_<cv::Vec3f> CPURender::applyLinearBlendMask(cv::Mat input, cv::Mat_<float> mask) {
	cv::Mat_<cv::Vec3f> output = cv::Mat_<cv::Vec3f>(input.rows, input.cols);
	for (int i = 0; i < input.rows; i ++) {
		for (int j = 0; j < input.cols; j ++) {
			cv::Vec3f valf;
			cv::Vec3b val = input.at<cv::Vec3b>(i, j);
			valf.val[0] = static_cast<float>(val.val[0]) * mask(i, j);
			valf.val[1] = static_cast<float>(val.val[1]) * mask(i, j);
			valf.val[2] = static_cast<float>(val.val[2]) * mask(i, j);
			output(i, j) = valf;
		}
	}
	return output;
}

int CPURender::genGraphCutMask(std::vector<cv::Mat> smallImgs, std::vector<cv::Point2i> graphcutCorners,
         std::vector<cv::Mat> & masks) {
    // init GPU array
	std::vector<cv::UMat> smallImgsd;
	std::vector<cv::UMat> masksd;
	smallImgsd.resize(smallImgs.size());
	masksd.resize(smallImgs.size());
	for (int i = 0; i < smallImgs.size(); i++) {
		smallImgs[i].convertTo(smallImgs[i], CV_32FC3);
		smallImgs[i].copyTo(smallImgsd[i]);
		masks[i].copyTo(masksd[i]);
	}
	// perform graph cut
	cv::Ptr<cv::detail::SeamFinder> seamfinder = cv::makePtr<cv::detail::GraphCutSeamFinder>();
	seamfinder->find(smallImgsd, graphcutCorners, masksd);
	// bring gpu array back
	for (int i = 0; i < smallImgs.size(); i++) {
		masksd[i].copyTo(masks[i]);
		masksd[i].release();
		smallImgsd[i].release();
		masks[i] = this->imfillholes(masks[i]);
	}
    return 0;
}

int CPURender::render() {
	// read image
	refImg = cv::imread(cv::format("%s/ref.jpg", datapath.c_str()));
	detailImgs.clear();
	for (int i = 0; i < camNum; i++) {
		detailImgs.push_back(cv::imread(cv::format("%s/%d_warped.jpg", outputpath.c_str(), i)));
	}
    // read image
    for (int i = 0; i < camNum; i ++) {
        masks.push_back(cv::imread(cv::format("%s/%d_mask.png", outputpath.c_str(), i), CV_LOAD_IMAGE_GRAYSCALE));
    }
    // generate mask using graphcut
    float secondFactor = 2;
    float graphcutFactor = scaleFactor * secondFactor;
    std::vector<cv::Mat> smallImgs(camNum);
    std::vector<cv::Point2i> graphcutCorners(camNum);
    for (int i = 0; i < camNum; i ++) {
        cv::resize(detailImgs[i], smallImgs[i], cv::Size(detailImgs[i].cols * graphcutFactor,
         detailImgs[i].rows * graphcutFactor));
		cv::resize(masks[i], masks[i], cv::Size(detailImgs[i].cols * graphcutFactor,
			detailImgs[i].rows * graphcutFactor), cv::INTER_NEAREST);
		graphcutCorners[i] = refpos[i] / scaleFactor * graphcutFactor;
    }
    genGraphCutMask(smallImgs, graphcutCorners, masks);
    for (int i = 0; i < camNum; i ++) {
		cv::resize(masks[i], masks[i], cv::Size(detailImgs[i].cols, detailImgs[i].rows), cv::INTER_LINEAR);
        masks[i].convertTo(masks[i], CV_32F);
    }
    // generate final result
	int width = detailImgs[0].cols;
	int height = detailImgs[0].rows;
	int widthScale = static_cast<int>((refImg.cols + 1) / scaleFactor);
	int heightScale = static_cast<int>((refImg.rows + 1) / scaleFactor);
    // linear blending
	cv::Mat_<float> weight = cv::Mat_<float>::zeros(heightScale, widthScale);
	cv::Mat_<cv::Vec3f> imgf = cv::Mat_<cv::Vec3f>::zeros(heightScale, widthScale);
	for (int i = 0; i < detailImgs.size(); i ++) {
		cv::Rect rect;
		rect.width = detailImgs[i].cols;
		rect.height = detailImgs[i].rows;
		rect.x = round(refpos[i].x / scaleFactor);
		rect.y = round(refpos[i].y / scaleFactor);
		weight(rect) = weight(rect) + masks[i];
		imgf(rect) = imgf(rect) + applyLinearBlendMask(detailImgs[i], masks[i]);
	}
	// generate final result
    cv::Mat output;
	cv::resize(refImg, output, cv::Size(widthScale, heightScale));
	for (int i = 0; i < heightScale; i ++) {
		for (int j = 0; j < widthScale; j ++) {
			float weightVal = weight(i, j);
			if (weightVal > 0) {
				cv::Vec3b val;
				cv::Vec3f valf = imgf(i, j);
				val.val[0] = static_cast<uchar>(valf.val[0] / weightVal);
				val.val[1] = static_cast<uchar>(valf.val[1] / weightVal);
				val.val[2] = static_cast<uchar>(valf.val[2] / weightVal);
				output.at<cv::Vec3b>(i, j) = val;
			}
		}
	}
    cv::imwrite(cv::format("%s/render.jpg", outputpath.c_str()), output);
    return 0;
}