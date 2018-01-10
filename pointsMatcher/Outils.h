#ifndef __OUTILS_H__
#define __OUTILS_H__

#include <iostream>
#include <fstream>

#include <slArgHandler.h>
#include <slBgSub.h>
#include <slClock.h>
#include <slVideoIn.h>
#include <slImageIn.h>
#include <slImageOut.h>

#include "slBlobAnalyzer.h"
#include "slContourEngine.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/flann/flann.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include "KeyPt.h";

using namespace cv;
using namespace slAH;

#define PI 3.141592653589793
#define RADIUS_MIN 65
#define ERREUR_ANGLE_MAX 40
#define BEGIN_FRAME 0
#define NB_FRAME_MEMORY 100

namespace Outils
{
	std::vector<slKeyPoints> extractKeysPoints(const slContours &contours, const slBlobAnalyzer *ba);
	std::vector<cv::Point> extractPoints(std::vector<slKeyPoints> keysPoints);
	std::vector<cv::Point> extractPoints(std::vector<KeyPoint> keysPoints);
	void drawImage (std::vector<cv::Point> points, slImage3ch &output);
	
	void paint(const slImage1ch fg, const slContours &contours, const slBlobAnalyzer *ba, slImage3ch &output);
	void paintKeyPoints(const slKeyPoints &kPt, cv::Scalar color, slImage3ch &output);
	void paintKeyPoints(const slImage1ch fg, std::vector<std::vector<KeyPt>> vecKeyPts, cv::Scalar vertexColor, cv::Scalar edgeColor, slImage3ch &output);
	void writeMatToFile(cv::Mat& m, const std::string& filename);


	CvMat* Ransac(std::vector<Point> &points1, std::vector<Point> &points2);
	CvMat* Ransac(std::vector<KeyPt> &points1, std::vector<KeyPt> &points2);

	std::vector<CvMat*> CV_Ransac_Simple(std::vector<Point> points1, std::vector<Point> points2);

	std::vector<KeyPoint> convertPoints2KeyPoints(std::vector<cv::Point> points);
	std::vector<KeyPt> convert2KeyPt(std::vector<std::vector<KeyPt>> &sortedKeysPoints);

	std::vector<KeyPoint> extractKeyPoints(std::vector<slKeyPoints> keysPoints);

	std::vector<Point> pointsTransform(std::vector<Point> points, cv::Mat transMat);
	std::vector<KeyPt> pointsTransform(std::vector<KeyPt> points, cv::Mat transMat);
	std::vector<std::vector<KeyPt>> pointsTransform(std::vector<std::vector<KeyPt>> points, cv::Mat transMat);

	float computeMeanEuclideanError(std::vector<Point> points1, std::vector<Point> points2, unsigned int frameNumber);
	float computeMeanEuclideanError(std::vector<KeyPt> points1, std::vector<KeyPt> points2, unsigned int frameNumber);

	void descriptorMatchLSS(slImage3ch im1, slImage3ch im2, std::vector<Point> &points1, std::vector<Point> &points2);
	float LSSDescriptorDistance(std::vector<float> descriptor1, std::vector<float> descriptor2);
	std::vector<KeyPt> convert2KeyPt(std::vector<slKeyPoint> &sortedKeysPoints);

	void computeBlobBoundingBox(cv::Point2f &bbMin, cv::Point2f &bbMax, std::vector<slKeyPoint> &sortedKeysPoints);
	void computeBlobBoundingBox(cv::Point2f &bbMin, cv::Point2f &bbMax, std::vector<KeyPt> &sortedKeysPoints);

	std::vector<slKeyPoint> sortKeyPoints(slKeyPoints &keysPoints);

	void convertAndSortKeyPoints(std::vector<slKeyPoints> &keys, std::vector<std::vector<KeyPt>> &vecNewKeyPts);

	float rad2Deg(float radian);
	float deg2Rad(float degree);

	std::vector<cv::Point2f> getCentroid(std::vector<std::vector<KeyPt>> keyPoints);
	cv::Point2f getCentroid(std::vector<KeyPt> keyPoints);
	cv::Point2f getCentroid(std::vector<slKeyPoint> &keyPoints);

	float determinerAngle(cv::Point2f vect1, cv::Point2f vect2, cv::Point2f vect3);
	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<Point> &matchedPoints1, std::vector<Point> &matchedPoints2);
	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<std::vector<KeyPt>> &matchedPoints1, std::vector<std::vector<KeyPt>> &matchedPoints2);
	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<KeyPt> &matchedPoints1, std::vector<KeyPt> &matchedPoints2);
	void matchPoints(std::vector<KeyPt> keyPoints1, std::vector<KeyPt> keyPoints2, std::vector<Point> &matchedPoints1, std::vector<Point> &matchedPoints2);
	void matchPoints(std::vector<KeyPt> keyPoints1, std::vector<KeyPt> keyPoints2, std::vector<KeyPt> &matchedPoints1, std::vector<KeyPt> &matchedPoints2);
	float computeDistance(cv::Point2f pt1, cv::Point2f pt2);

	Point2f findBlobCentroid(Mat &src_gray);
	void update_map( slImage3ch src, Mat &dest, Mat map_x, Mat map_y);

	void switchBlackAndWhite(Mat &src);
	void transformGroundTruth(Mat src, Mat &dest, string pathFile, string pathVisForegroundFile);
	int computeBlobsSurface(Mat src);

	std::vector<Point2f> computeTranslate(std::vector<std::vector<KeyPt>> modifiedPoints, std::vector<std::vector<KeyPt>> points2Compare);
	void substractRegion(Mat &src, Mat region2Substract);
	int countWhitePixel(Mat &src);
}

#endif