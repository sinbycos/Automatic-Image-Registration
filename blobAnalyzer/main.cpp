#include <iostream>
#include <slArgHandler.h>
#include <slBgSub.h>
#include <slClock.h>
#include <slVideoIn.h>
#include <slImageOut.h>

#include "slBlobAnalyzer.h"
#include "slContourEngine.h"


using namespace cv;
using namespace slAH;
using namespace std;

void paint(const slImage1ch &fg, const slContours &contours, const slBlobAnalyzer *ba, slImage3ch &output);
void paintKeyPoints(const slKeyPoints &kPt, cv::Scalar color, slImage3ch &output);


// Usage: -i ..\..\..\mediaFiles\stereo1_thermal.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba dce -n 16
// Usage: -i ..\..\..\mediaFiles\stereo1_thermal.avi bgSub: -a tempAVG -bf 8 -bfg contour: -c 3 3 blobAn: -a 5 -ba skel

int main(int argc, char **argv)
{
	slArgProcess argProcess;
	slArgHandler argHbgSub("bgSub"), argHContour("contour"), argHBlobAn("blobAn");

	// Get all possible parameters
	argProcess
		.addGlobal(slParamSpec("-i", "Video source", MANDATORY) << slSyntax("video.avi"))
		.addGlobal(slParamSpec("-h", "Help"));
	slBgSub::fillAllParamSpecs(argHbgSub);
	slContourEngine::fillParamSpecs(argHContour);
	slBlobAnalyzer::fillAllParamSpecs(argHBlobAn);

	argProcess << argHbgSub << argHContour << argHBlobAn;

	slVideoIn videoIn;
	slBgSub *bgSub = NULL;
	slContourEngine *contourEngine = new slContourEngine;
	slBlobAnalyzer *ba = NULL;
	slWindow winGraph("Graph from contours");

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
		bgSub = slBgSubFactory::createInstance(argProcess.getParameters("bgSub"));
		contourEngine->setParameters(argProcess.getParameters("contour"));
		ba = slBlobAnalyzerFactory::createInstance(argProcess.getParameters("blobAn"));

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
	try {
		slClock horloge;
		slImage3ch imSource;
		slImage1ch bForeground, fgTemp;

		/*slImageOut imOut1, imOut2;
		imOut1.open("contour.png");
		imOut2.open("foreground.png");*/

		horloge.setFPS(videoIn.getFPS());
		horloge.start();

		for (unsigned int ind = 0; ind < videoIn.getNbImages(); ind++) {
			videoIn.read(imSource);
			bgSub->compute(imSource, bForeground);

			//fgTemp = bForeground.clone();

			contourEngine->findContours(bForeground);
			ba->analyzeAllBlobs(contourEngine->getContours());

			paint(bForeground, contourEngine->getContours(), ba, imSource);
			winGraph.show(imSource);
			waitKey(horloge.nextDelay());

			/*if (ind == 153)
			{
				imOut1.write(imSource);
				imOut2.write(fgTemp);

				cout << "Index : " << ind << endl;
				waitKey();
			}*/
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


void paint(const slImage1ch &fg, const slContours &contours, const slBlobAnalyzer *ba, slImage3ch &output)
{
	// Empty output image
	output.create(fg.size());
	output = 0;

	// For each external contour
	for (slContours::const_iterator contour = contours.begin();
		!contour.isNull(); contour = contour.next())
	{
		if (ba->hasKeyPoints(contour)) {
			paintKeyPoints(ba->getKeyPoints(contour), CV_RGB(0, 255, 0), output);

			// For each hole or internal contour
			/*for (slContours::const_iterator child = contour.child();
				!child.isNull(); child = child.next())
			{
				if (ba->hasKeyPoints(child)) {
					paintKeyPoints(ba->getKeyPoints(child), CV_RGB(255, 0, 0), output);
				}
			}*/
		}
	}
}


void paintKeyPoints(const slKeyPoints &kPt, cv::Scalar color, slImage3ch &output)
{
	for (slKeyPoints::const_iterator itPoint = kPt.begin();
		itPoint != kPt.end(); itPoint++)
	{
		circle(output, Point(itPoint->first), 2, CV_RGB(0, 0, 255), 2, 8, 0);

		for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors();
			itNb != itPoint->second.endNeighbors(); itNb++)
		{
			if (CvPoint2fLessThan()(itPoint->first, *itNb)) {
				line(output, Point(itPoint->first), Point(*itNb), color);
			}
		}
	}
}


