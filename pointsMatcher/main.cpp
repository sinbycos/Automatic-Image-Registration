#include "Outils.h";
#include "KeyPt.h";

using namespace Outils;

//**** Usage avant ****// -> Traitement d'une seule vidéo
// Usage: -i ..\SDK_LITIV\mediaFiles\vid3_rectified_IR.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba dce -n 16
// Usage: -i ..\SDK_LITIV\mediaFiles\vid3_rectified_IR.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba skel

//**** Nouvel usage ****// -> Traitement de deux vidéos ("-i" pour visible et "-k" pour infrarouge)
// Usage: -i ..\SDK_LITIV\mediaFiles\vid3_rectified_IR.avi -k ..\SDK_LITIV\mediaFiles\vid3_rectified_Vis.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba dce -n 16
// Usage: -i ..\SDK_LITIV\mediaFiles\vid3_rectified_IR.avi -k ..\SDK_LITIV\mediaFiles\vid3_rectified_Vis.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba skel

int main(int argc, char **argv)
{
	slArgProcess argProcess;
	slArgHandler argHbgSub("bgSub"), argHContour("contour"), argHBlobAn("blobAn");

	// Get all possible parameters
	argProcess
		.addGlobal(slParamSpec("-i", "Video source", MANDATORY) << slSyntax("video.avi"))
		.addGlobal(slParamSpec("-k", "Video source", MANDATORY) << slSyntax("video.avi"))
		.addGlobal(slParamSpec("-h", "Help"));
	slBgSub::fillAllParamSpecs(argHbgSub);
	slContourEngine::fillParamSpecs(argHContour);
	slBlobAnalyzer::fillAllParamSpecs(argHBlobAn);

	argProcess << argHbgSub << argHContour << argHBlobAn;

	slVideoIn videoIn, videoIn2;
	slImageOut imOut1, imOut2, imOut3;
	slImageIn imIn1, imIn2;

	slImage3ch groundTruthImTemp;
	slImage3ch groundTruthIm;
	Mat map_x, map_y;

	std::vector<CvMat*> transformMatrix;
	CvMat* LSSTransformMatrix;

	imOut1.open("..\\Results\\output\\1Person\\forgroundIR_On_GroundTruth.png");
	imOut2.open("..\\Results\\output\\1Person\\forgroundIR_On_foregroundVis.png");
	imOut3.open("..\\Results\\output\\1Person\\contoursIR_On_contoursVis.png");

	slBgSub *bgSub = NULL;
	slBgSub *bgSub2 = NULL;
	slContourEngine *contourEngine = new slContourEngine;
	slContourEngine *contourEngine2 = new slContourEngine;
	slBlobAnalyzer *ba = NULL;
	slBlobAnalyzer *ba2 = NULL;
	slWindow winGraph1("Graph from Vis contours"), winGraph2("Graph from IR contours"), winGraph3("Test"), winGraph4("Test2");

	try {
		// Load parameters from command line
		argProcess.parse(argc, argv);
		const slParameters &globalParams = argProcess.getParameters("");

		if (globalParams.isParsed("-h")) {	// Help wanted
			argProcess.printUsage();
			return 0;
		}

		// Configure all compute nodes and others
		videoIn.open(globalParams.getValue("-i").c_str());
		videoIn2.open(globalParams.getValue("-k").c_str());
		bgSub = slBgSubFactory::createInstance(argProcess.getParameters("bgSub"));
		bgSub2 = slBgSubFactory::createInstance(argProcess.getParameters("bgSub"));
		contourEngine->setParameters(argProcess.getParameters("contour"));
		contourEngine2->setParameters(argProcess.getParameters("contour"));
		ba = slBlobAnalyzerFactory::createInstance(argProcess.getParameters("blobAn"));
		ba2 = slBlobAnalyzerFactory::createInstance(argProcess.getParameters("blobAn"));

		// Show configuration
		bgSub->showParameters();
		contourEngine->showParameters();
		ba->showParameters();
	}
	catch (const slExceptionArgHandler &err)
	{
		argProcess.printUsage();
		fprintf(stderr, "Error: %s\n", err.getMessage());
		return -1;
	}
	catch (const slException &err)
	{
		fprintf(stderr, "Error: %s\n", err.getMessage());
		return -1;
	}

	// Start main process
	try
	{
		slClock horloge;
		slImage3ch imSource, imSource2, imDest, imDest2, imDest3, imDest4, imDest5, imDest6, fg3ch1, fg3ch2, fg1, fg2;
		slImage3ch imContour, imContour2;
		slImage1ch bForeground, bForeground2;

		horloge.setFPS(videoIn.getFPS());
		horloge.start();
		std::vector<std::vector<slKeyPoint>> allBlobsKeyPoints;
		std::vector<slKeyPoint> vecSlKeyPoint;
		std::vector<slKeyPoints> keys;
		std::vector<cv::Point> points1, points2;
		std::vector<std::vector<KeyPt>> modifiedPoints;
		std::vector<KeyPoint> keypointsA, keypointsB;

		std::vector<std::vector<KeyPt>> vecNewKeyPts, vecNewKeyPts2, vecNewKeyPtsTemp;
		std::vector<KeyPt> newKeyPts;
		std::vector<KeyPt> vTemp1, vTemp2;
		std::vector<Point2f> translateVec;

		std::vector<std::vector<KeyPt>> matchedPoints1, matchedPoints2, matchedPointsTemp1, matchedPointsTemp2;
		std::vector<std::vector<KeyPt>> vecMatchedPoints1, vecMatchedPoints2;
		std::vector< std::pair< int, std::vector<int> > > matchedBlobs, matchedBlobs2;

		Mat FREAKdescriptors1, FREAKdescriptors2;
		std::vector<DMatch> matches;
		unsigned int nbMatch;

		// Variables pour le calcul de centre de masse des blobs
		int thresh = 100;
		Mat src_gray, src_gray2;
		Point2f centr;

		int valTemp = 0;

		float euclideanError;
		float d, d2, d3, d4, d5, mean;

		Point2f pt1(127.831, 104.145);
		Point2f pt2(126.725, 103.632);

		Point2f pt3(202.136, 74.16715);
		Point2f pt4(203.2685, 76.33705);

		Point2f pt5(165.59, 75.9793);
		Point2f pt6(166.012, 73.9983);

		Point2f pt7(253.53, 55.1214);
		Point2f pt8(256.655, 53.8391);



		Point2f pt9(335.498, 211.137);
		Point2f pt10(333.145, 212.589);

		cv::Point2f vecTemp = pt1 - pt2;
		cv::Point2f vecTemp2 = pt3 - pt4;
		cv::Point2f vecTemp3 = pt5 - pt6;
		cv::Point2f vecTemp4 = pt7 - pt8;
		cv::Point2f vecTemp5 = pt9 - pt10;

		d = sqrt((vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y));
		d2 = sqrt((vecTemp2.x*vecTemp2.x) + (vecTemp2.y*vecTemp2.y));
		d3 = sqrt((vecTemp3.x*vecTemp3.x) + (vecTemp3.y*vecTemp3.y));
		d4 = sqrt((vecTemp4.x*vecTemp4.x) + (vecTemp4.y*vecTemp4.y));
		d5 = sqrt((vecTemp5.x*vecTemp5.x) + (vecTemp5.y*vecTemp5.y));

		mean = (d + d2) / 2;

		std::cout << "Distance1 = " << d << std::endl;
		std::cout << "Distance2 = " << d2 << std::endl;
		//std::cout << "Distance3 = " << d3 << std::endl;
		//std::cout << "Distance4 = " << d4 << std::endl;
		//std::cout << "Distance5 = " << d5 << std::endl;

		std::cout << "Mean = " << mean << std::endl;
		Size S = Size((int)videoIn.getWidth(),    //Acquire input size
			(int)videoIn.getHeight());





		VideoWriter record;
		record.open("..\\Results\\output2\\1Person\\forgroundIR_On_foregroundVis.avi", -1, 30, S, true);
		if (!record.isOpened())
		{
			cout << "Could not open the output video for write: " << endl;
			return -1;
		}
		for (unsigned int ind = 0; ind < videoIn.getNbImages(); ind++)
		{
			nbMatch = 0;

			videoIn.read(imSource);
			videoIn2.read(imSource2);

			bgSub->compute(imSource, bForeground);
			bgSub2->compute(imSource2, bForeground2);

			fg3ch1.zeros(imSource.size().width, imSource.size().height);
			fg3ch2.zeros(imSource.size().width, imSource.size().height);
			fg1.zeros(imSource.size().width, imSource.size().height);
			fg2.zeros(imSource.size().width, imSource.size().height);

			cvtColor(bForeground, fg3ch1, CV_GRAY2RGB, 0);
			cvtColor(bForeground2, fg3ch2, CV_GRAY2RGB, 0);
			cvtColor(bForeground, fg1, CV_GRAY2RGB, 0);
			cvtColor(bForeground2, fg2, CV_GRAY2RGB, 0);

			min(fg3ch1, bgSub->getCurrent(), fg3ch1);
			min(fg3ch2, bgSub2->getCurrent(), fg3ch2);

			vecNewKeyPts.clear();
			vecNewKeyPts2.clear();


			if (ind >= BEGIN_FRAME)
			{
				++valTemp;
				contourEngine->findContours(bForeground);
				contourEngine2->findContours(bForeground2);

				ba->analyzeAllBlobs(contourEngine->getContours());
				ba2->analyzeAllBlobs(contourEngine2->getContours());

				keys = extractKeysPoints(contourEngine->getContours(), ba);

				if (keys.size() > 0)
				{
					convertAndSortKeyPoints(keys, vecNewKeyPts);

					paintKeyPoints(bForeground, vecNewKeyPts, CV_RGB(0, 255, 0), CV_RGB(0, 0, 255), imContour);
					winGraph1.show(imContour);

					keys.clear();

					keys = extractKeysPoints(contourEngine2->getContours(), ba2);
					convertAndSortKeyPoints(keys, vecNewKeyPts2);

					paintKeyPoints(bForeground2, vecNewKeyPts2, CV_RGB(0, 0, 255), CV_RGB(255, 0, 0), imContour2);
					winGraph2.show(imContour2);

					matchBlobs(vecNewKeyPts, vecNewKeyPts2, matchedPoints1, matchedPoints2);
					matchedPointsTemp1 = matchedPoints1;
					matchedPointsTemp2 = matchedPoints2;

					// Transformation de plusieurs vecteurs en un seul
					for (unsigned int it = 0; it < matchedPoints1.size(); ++it)
					{
						vTemp1.insert(vTemp1.end(), matchedPoints1.at(it).begin(), matchedPoints1.at(it).end());
						vTemp2.insert(vTemp2.end(), matchedPoints2.at(it).begin(), matchedPoints2.at(it).end());
					}

					vecMatchedPoints1.push_back(vTemp1);
					vecMatchedPoints2.push_back(vTemp2);

					// Copie de tous les points des autres frames dans un seul vecteur
					for (unsigned int i = 0; i < vecMatchedPoints1.size() - 1; ++i)
					{
						vTemp1.insert(vTemp1.end(), vecMatchedPoints1.at(i).begin(), vecMatchedPoints1.at(i).end());
						vTemp2.insert(vTemp2.end(), vecMatchedPoints2.at(i).begin(), vecMatchedPoints2.at(i).end());
					}

					if (vecMatchedPoints1.size() == NB_FRAME_MEMORY)
					{
						vecMatchedPoints1.erase(vecMatchedPoints1.begin());
						vecMatchedPoints2.erase(vecMatchedPoints2.begin());
					}

					if (vTemp1.size() >= 4)
					{
						if (ind == 649)
						{

							CvMat* trans = Ransac(vTemp1, vTemp2);

							cv::Mat transMat = (cv::Mat)trans;
							transMat.convertTo(transMat, CV_32FC1, 1, 0);	// Convertir la matrice en float

							cv::warpPerspective(fg1, imDest, transMat, cv::Size(imSource.size().width, imSource.size().height), 1, 0);
							cv::warpPerspective(fg3ch1, imDest2, transMat, cv::Size(imSource.size().width, imSource.size().height), 1, 0);
							cv::warpPerspective(imContour, imDest5, transMat, cv::Size(imSource.size().width, imSource.size().height), 1, 0);


							string pathFile = "..\\SDK_LITIV\\mediaFiles\\GroundTruth\\vid3\\1Person\\vid3_1Person.txt";
							string pathVisForegroundFile = "vid3\\\\1Person\\\\Foreground\\\\VisForeground236.jpg";

							groundTruthIm = fg1.clone();
							transformGroundTruth(fg1, groundTruthIm, pathFile, pathVisForegroundFile);

							cvtColor(groundTruthIm, src_gray2, CV_BGR2GRAY);
							centr = findBlobCentroid(src_gray2);

							// Ground truth foreground
							winGraph1.show(src_gray2);

							cvtColor(imDest, src_gray, CV_BGR2GRAY);
							centr = findBlobCentroid(src_gray);

							// Transformed IR foreground
							winGraph2.show(src_gray);

							//forgroundIR_On_GroundTruth
							subtract(groundTruthIm, imDest2, imDest3, noArray(), 1);
							imOut1.write(imDest3);

							// forgroundIR_On_foregroundVis
							add(imDest2, fg3ch2, imDest4, noArray(), 1);
							winGraph3.show(imDest4);
							imOut2.write(imDest4);
							//record.write(imDest4);
							// contoursIR_On_contoursVis
							add(imDest5, imContour2, imDest6, noArray(), 1);
							winGraph4.show(imDest6);
							imOut3.write(imDest6);

							add(imDest, groundTruthIm, imDest6, noArray(), 1);


							int count1 = countWhitePixel(groundTruthIm);
							int count2 = countWhitePixel(imDest6);

							float blobRatio = (float)count2 / (float)count1;

							std::cout << std::endl << "Blob Ratio = " << count2 << "/" << count1 << " = " << blobRatio << std::endl;

							//waitKey();
						}

					}


					vTemp1.clear();
					vTemp2.clear();

					//waitKey();

					matchedPoints1.clear();
					matchedPoints2.clear();
					modifiedPoints.clear();

				}
			}

			waitKey(horloge.nextDelay());
		}

		horloge.stop();
	}
	catch (const slException &err)
	{
		fprintf(stderr, "Error: %s\n", err.getMessage());
		return -1;
	}

	videoIn.close();
	delete ba;
	delete contourEngine;
	delete bgSub;
	return 0;
}
