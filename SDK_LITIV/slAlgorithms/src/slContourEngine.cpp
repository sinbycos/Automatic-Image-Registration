#include "slContourEngine.h"
#include "slKeyPoint.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>


using namespace cv;
using namespace std;
using namespace slAH;


#define ARG_CLOSURE "-c"


slContourEngine::slContourEngine()
:doClosure_(false)
{
}


slContourEngine::~slContourEngine()
{
	kernel_.release();
	contours_.clear();
}


void slContourEngine::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_CLOSURE, "Do closure with kernel size w*k")
		<< slSyntax("w", "3") << slSyntax("h", "3"));
}


void slContourEngine::setParameters(const slAH::slParameters& parameters)
{
	// Closure
	if (parameters.isParsed(ARG_CLOSURE)) {
		setClosure(true,
			atoi(parameters.getValue(ARG_CLOSURE, 0).c_str()), 
			atoi(parameters.getValue(ARG_CLOSURE, 1).c_str()));
	}
	else {
		setClosure(false);
	}
}


void slContourEngine::setClosure(bool enabled, int w, int h)
{
	kernel_.release();

	if (doClosure_ = enabled) {
		kernel_ = getStructuringElement(MORPH_ELLIPSE, Size(w, h));
	}
}


void slContourEngine::showParameters() const
{
	cout << "--- slContour ---" << endl;

	cout << "Closure : ";
	if (doClosure_) {
		cout << "Yes, " << kernel_.size().width << "x" << kernel_.size().height << endl;
	}
	else {
		cout << "No" << endl;
	}

	cout << endl;
}


void slContourEngine::findContours(slImage1ch &bForeground)
{
	if (doClosure_) {
		morphologyEx(bForeground, bForeground, MORPH_CLOSE, kernel_);
	}

	// Find all contours
	contours_.findAll(bForeground);
}


slContour slContourEngine::approximate(const slContour &contour, double distance)
{
	slContour approx;

	if (!contour.empty()) {
		CvPt2fVector_t approx2f;

		// Approximate contour
		approxPolyDP(contour, approx2f, distance, true);

		// Conversion Point2f->Point
		for (CvPt2fVector_t::const_iterator it = approx2f.begin(); it != approx2f.end(); it++) {
			approx.push_back(*it);
		}
	}

	return approx;
}


