#ifdef WIN32
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	return TRUE;
}
#endif	// WIN32


#include <iostream>

#include "slBgSub.h"


using namespace cv;
using namespace std;
using namespace slAH;


/****************************************************************************
 * Description    :  Constructor
 * Parameters     :
 * Return value	:  No
 ***************************************************************************/
slBgSub::slBgSub()
{
	colorSystem_ = SL_BGR;
	doSmooth_ = false;
	doQuantification_ = false;
	doConsiderLightChanges_ = false;
	doShadowFilter_ = false;
	doSizeFilter_ = false;

	nbFrames_ = 0;
}


/****************************************************************************
 * Description    :  Destructor
 * Parameters     :  No
 * Return value	:  No
 ***************************************************************************/
slBgSub::~slBgSub()
{
	// Delete all windows
	for (windowsList_t::iterator itW = windowsList_.begin();
		itW != windowsList_.end(); itW++)
	{
		delete itW->second;
	}
}


void slBgSub::fillAllParamSpecs(slParamSpecMap& paramSpecMap)
{
	slBgSubFactory::fillAllParamSpecs(paramSpecMap);
}


/****************************************************************************
 * Description    :  fillGlobalParamSpecs()
                     Fills a map with parameter specifications.
 * Parameters     :  - pParamSpecMap (pParamSpecMap*): Pointer towards the map of
                                                      parameter specifications
                                                      used by the arg handler.
 * Return value   :  No
 ***************************************************************************/
void slBgSub::fillGlobalParamSpecs(slParamSpecMap& paramSpecMap)
{
	// About the colors
	paramSpecMap
		<< (slParamSpec(ARG_COLOR_S, "Color system") << slSyntax(CSYS_SYNTAX, CSYS_DEFAULT))
		<< slQuant3ch::fillSyntax(slParamSpec(ARG_QUANT, "Quantification, max(H)==180"))
		<< slParamSpec(ARG_LIGHT_C, "Consider light changes");

	// About the filters
	paramSpecMap
		<< (slParamSpec(ARG_SMOOTH, "Smooth level (gaussian)") << slSyntax("1..63"))
		<< (slParamSpec(ARG_SHADOW_FILTER, "Shadow filter")
			<< slSyntax("Th", "0.3") << slSyntax("Ts", "0.4") << slSyntax("Tv", "0.2"))
		<< (slParamSpec(ARG_BLOB_FILTER, "Blob size filter") << slSyntax("1..16384", "0"))
		<< (slParamSpec(ARG_HOLE_FILTER, "Hole size filter") << slSyntax("1..16384", "0"))
		;

	// To show specified windows
	paramSpecMap
		<< slParamSpec(ARG_BG, "Background")
		<< slParamSpec(ARG_FG, "Foreground")

		<< slParamSpec(ARG_QBG, "Quantified background")
		<< slParamSpec(ARG_BFG, "Binary foreground")

		<< slParamSpec(ARG_C_FR, "Current frame")
		<< slParamSpec(ARG_QC_FR, "Quantified current frame");
}


void slBgSub::setParameters(const slParameters& parameters)
{
	// Color system or color space
	if (parameters.getValue(ARG_COLOR_S) == HSV_NAME) {
		setColorSystem(SL_HSV);
	}
	else {
		setColorSystem(SL_BGR);
	}

	// Smoothing
	if (parameters.isParsed(ARG_SMOOTH)) {
		setSmooth(true, atoi(parameters.getValue(ARG_SMOOTH).c_str()));
	}
	else {
		setSmooth(false);
	}

	// Quantification
	slQuant3ch quant;

	if (parameters.isParsed(ARG_QUANT)) {
		quant.setParams(atoi(parameters.getValue(ARG_QUANT, 0).c_str()),
						atoi(parameters.getValue(ARG_QUANT, 1).c_str()),
						atoi(parameters.getValue(ARG_QUANT, 2).c_str()));

		setQuantification(true, quant);
	}
	else {	// Need a default quantification for Epsilon
		setQuantification(false, quant);
	}

	// Considering light changes
	setConsiderLightChanges(parameters.isParsed(ARG_LIGHT_C));

	// Shadow detector and remover
	setShadowFilter(parameters.isParsed(ARG_SHADOW_FILTER),
		atof(parameters.getValue(ARG_SHADOW_FILTER, 0).c_str()),
		atof(parameters.getValue(ARG_SHADOW_FILTER, 1).c_str()),
		atof(parameters.getValue(ARG_SHADOW_FILTER, 2).c_str()));

	// Blob size filter
	if (parameters.isParsed(ARG_BLOB_FILTER) || parameters.isParsed(ARG_HOLE_FILTER)) {
		setSizeFilter(true,
			atoi(parameters.getValue(ARG_BLOB_FILTER).c_str()),
			atoi(parameters.getValue(ARG_HOLE_FILTER).c_str()));
	}
	else {
		setSizeFilter(false);
	}

	// Windows
	setWindowEnabled(ARG_C_FR, parameters.isParsed(ARG_C_FR));
	setWindowEnabled(ARG_QC_FR, parameters.isParsed(ARG_QC_FR));

	setWindowEnabled(ARG_BG, parameters.isParsed(ARG_BG));
	setWindowEnabled(ARG_QBG, parameters.isParsed(ARG_QBG));

	setWindowEnabled(ARG_FG, parameters.isParsed(ARG_FG));
	setWindowEnabled(ARG_BFG, parameters.isParsed(ARG_BFG));

	// Other parameters
	setSubParameters(parameters);
}


void slBgSub::setColorSystem(typeColorSys colorSystem)
{
	colorSystem_ = colorSystem;
}


void slBgSub::setConsiderLightChanges(bool enabled)
{
	doConsiderLightChanges_ = enabled;
}


void slBgSub::setSmooth(bool enabled, int level)
{
	doSmooth_ = enabled;
	smoothLevel_ = level;
}


void slBgSub::setQuantification(bool enabled, const slQuant3ch& quant)
{
	doQuantification_ = enabled;
	quantParams_ = quant;
}


void slBgSub::setShadowFilter(bool enabled, double th, double ts, double tv)
{
	doShadowFilter_ = enabled;

	th_ = th;
	ts_ = ts;
	tv_ = tv;
}


void slBgSub::setSizeFilter(bool enabled, int minBlobSize, int minHoleSize)
{
	doSizeFilter_ = enabled;
	minBlobSize_ = minBlobSize;
	minHoleSize_ = minHoleSize;
}


void slBgSub::setWindowEnabled(const char *arg, bool enable)
{
	windowsList_t::iterator itW = windowsList_.find(arg);

	if (enable == true) {	// Create window
		// If not found
		if (itW == windowsList_.end()) {
			if (strcmp(arg, ARG_C_FR) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Current");
			}
			if (strcmp(arg, ARG_QC_FR) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Current (quantified)");
			}
			if (strcmp(arg, ARG_BG) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Background");
			}
			if (strcmp(arg, ARG_QBG) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Background (quantified)");
			}
			if (strcmp(arg, ARG_FG) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Foreground");
			}
			if (strcmp(arg, ARG_BFG) == 0) {
				windowsList_[arg] = new slWindow("BgSub - Foreground (binary)");
			}
		}
	}
	else {	// Delete window
		// If found
		if (itW != windowsList_.end()) {
			delete itW->second;
			windowsList_.erase(itW);
		}
	}
}


/****************************************************************************
 * Description    :  showParameters()
                     Shows the parameters used, i.e. the arguments specified
 *							for the new background subtractor. Used for debugging.
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::showParameters() const
{
	cout << "--- slBgSub ---" << endl;

	if (colorSystem_ == SL_BGR)
		cout << "Color system : " << BGR_NAME << endl;
	else
		cout << "Color system : " << HSV_NAME << endl;

	if (doSmooth_)
		cout << "Smoothing : yes -> " << smoothLevel_ << endl;
	else
		cout << "Smoothing : no" << endl;

	if (doQuantification_)
		cout << "Quantification : yes -> " << quantParams_.getStr(colorSystem_) << endl;
	else
		cout << "Quantification : no" << endl;

	if (doConsiderLightChanges_)
		cout << "Consider light changes : yes" << endl;
	else
		cout << "Consider light changes : no" << endl;

	cout << endl;

	if (doShadowFilter_)
		cout << "Shadow filter : yes -> " << th_ << " " << ts_ << " " << tv_ << endl;
	else
		cout << "Shadow filter : no" << endl;

	if (doSizeFilter_)
		cout << "Size filter : yes -> blobs: " << minBlobSize_ << ", holes: " << minHoleSize_ << endl;
	else
		cout << "Size filter : no" << endl;

	cout << endl;

	// By polymorphism, call the right function
	showSubParameters();
	cout << endl;
}


void slBgSub::compute(const slImage3ch &image, slImage1ch &bForeground)
{
	// Prepare current image
	if (nbFrames_ == 0) {
		current_ = image.clone();
	}
	else {
		image.copyTo(current_);
	}

	// Convert colors to colorSystem_ if needed
	if (colorSystem_ == SL_HSV) {
		cvtColor(current_, current_, CV_BGR2HSV);
	}

	// Clear noises
	if (doSmooth_) {
		GaussianBlur(current_, current_, Size(smoothLevel_, smoothLevel_), 0);
	}

	// If first frame
	if (nbFrames_ == 0) {
		// Keep a copy of the size
		imageSize_ = current_.size();

		// Create a quantified version of current image
		qCurrent_ = current_.clone();

		if (doQuantification_) {
			quantParams_.quantify(qCurrent_, qCurrent_);
		}

		// Clone the first image to the background
		background_ = current_.clone();
		qBackground_ = qCurrent_.clone();

		// Create foreground images
		foreground_.create(imageSize_);
		bForeground.create(imageSize_);

		// Do a specific inits if needed
		init();
	}
	else {
		// Quantification if needed
		if (doQuantification_) {
			quantParams_.quantify(background_, qBackground_);
			quantParams_.quantify(current_, qCurrent_);
		}
		else {
			background_.copyTo(qBackground_);
			current_.copyTo(qCurrent_);
		}

		prepareNextSubtraction();
	}

	nbFrames_++;

	// Main action
	doSubtraction(bForeground);

	if (doShadowFilter_) {
		shadowFilter(bForeground);
	}

	// Find blobs and apply size filter
	findBlobs(bForeground);

	// Update images and their corresponding window
	updateWindows(bForeground);
	updateSubWindows();
}


/****************************************************************************
 * Description    :  getContour()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
const slContours& slBgSub::getContours() const 
{ 
	return contours_; 
}


slImage3ch& slBgSub::getCurrent()
{
	return current_;
}


slImage3ch& slBgSub::getCurrentQuant()
{
	return qCurrent_;
}


const slImage3ch& slBgSub::getCurrent() const
{
	return current_;
}


const slImage3ch& slBgSub::getCurrentQuant() const
{
	return qCurrent_;
}


slImage3ch& slBgSub::getBackground()
{
	return background_;
}


slImage3ch& slBgSub::getBackgroundQuant()
{
	return qBackground_;
}


const slImage3ch& slBgSub::getBackground() const
{
	return background_;
}


const slImage3ch& slBgSub::getBackgroundQuant() const
{
	return qBackground_;
}


slImage3ch& slBgSub::getForeground()
{
	return foreground_;
}


const slImage3ch& slBgSub::getForeground() const
{
	return foreground_;
}


/****************************************************************************
 * Description    :  getSize()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
//CvSize slBgSub::getSize() const 
//{ 
//	return Size_; 
//}


/****************************************************************************
 * Description    :  getNbFrames()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
//int slBgSub::getNbFrames() const 
//{ 
//	return nbFrames_; 
//}


/****************************************************************************
 * Description    :  shadowFilter()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::shadowFilter(slImage1ch &bForeground)
{
	// Get picture size and thresholds
	const int w = imageSize_.width, h = imageSize_.height;

	slImage3ch bgHSV = background_.clone();
	slImage3ch curHSV = current_.clone();

	if (colorSystem_ == SL_BGR) {
		cvtColor(bgHSV, bgHSV, CV_BGR2HSV);		// Convert background image to HSV
		cvtColor(curHSV, curHSV, CV_BGR2HSV);	// Convert current image to HSV
	}

	// Apply shadow filter
#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		// Get buffer pointers
		const slPixel3ch *pBgHSV = bgHSV[i];
		const slPixel3ch *pCurHSV = curHSV[i];
		const slPixel3ch *cur_row = current_[i];
		slPixel3ch *bg_row = background_[i];
		slPixel1ch *b_fg_row = bForeground[i];

		for (int j = 0; j < w; j++) {
			if (b_fg_row[j] != 0) {
				if (fabs((double)pBgHSV[j].val[2] - pCurHSV[j].val[2]) /
						(0.001 + max(pBgHSV[j].val[2], pCurHSV[j].val[2])) <= tv_ &&
					fabs((double)pBgHSV[j].val[1] - pCurHSV[j].val[1]) /
						(0.001 + max(pBgHSV[j].val[1], pCurHSV[j].val[1])) <= ts_ &&
					fabs((double)pBgHSV[j].val[0] - pCurHSV[j].val[0]) /
						(0.001 + max(pBgHSV[j].val[0], pCurHSV[j].val[0])) <= th_)
				{
					setBgPixel(cur_row, bg_row, b_fg_row, w, i, j);
				}
			}
		}
	}
}


/****************************************************************************
 * Description    :  findBlobs()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::findBlobs(slImage1ch &bForeground)
{
	// Clean image edges
	rectangle(bForeground, Point(0, 0),
		Point(imageSize_.width - 1, imageSize_.height - 1), Scalar(PIXEL_1CH_BLACK));

	// Get a clone of the binary foreground image
	slImage1ch src = bForeground.clone();

	// If we must filter small blobs
	if (doSizeFilter_)
	{
		// Find all contours
		contours_.findAll(src);

		for (slContours::const_iterator itContour = contours_.begin();
			!itContour.isNull(); itContour = itContour.next())
		{
			// If the region is too small to be in the foreground
			if (getAreaSurface(itContour) < minBlobSize_) {
				// Set itContour as bg object
				copyBlobIntoBG(itContour, bForeground);
			}
			else if (minHoleSize_ > 0) {
				// For each hole in foreground blob
				for (slContours::const_iterator child = itContour.child();
					!child.isNull(); child = child.next())
				{
					// If the hole is too small to be in the background
					if (getAreaSurface(child) < minHoleSize_) {
						// Set child as part of the foreground object
						copyHoleIntoFG(child, bForeground);
					}
				}
			}
		}

		// Copy the final binary foreground image in src
		bForeground.copyTo(src);
	}

	// Find all contours
	contours_.findAll(src);
}


/****************************************************************************
 * Description    :  getAreaSurface()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
double slBgSub::getAreaSurface(const slContours::const_iterator &contour)
{
	double area = fabs(contourArea(contour.mat()));
	
	// Substract holes' area
	for (slContours::const_iterator child = contour.child();
		!child.isNull(); child = child.next())
	{
		area -= fabs(contourArea(child.mat()));
	}

	return (area >= 0 ? area : 0);
}


/****************************************************************************
 * Description    :  copyBlobIntoBG()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::copyBlobIntoBG(const slContours::const_iterator &contour, slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const Mat cont(contour.mat());
	const Rect rect = boundingRect(cont);

	// x-coordinates
	const int x1 = rect.x;
	const int x2 = rect.x + rect.width;

	// y-coordinates
	const int y1 = rect.y;
	const int y2 = rect.y + rect.height;

	// For each row
	for (int i = y1; i < y2; i++) {
		const slPixel3ch *cur_row = current_[i];
		slPixel3ch *bg_row = background_[i];
		slPixel1ch *b_fg_row = bForeground[i];

		// For each column
		for (int j = x1; j < x2; j++) {
			// Test inside or outside
			double inside = pointPolygonTest(cont, Point(j, i), false);

			// Point to be copied into bg
			if (inside >= 0) {
				setBgPixel(cur_row, bg_row, b_fg_row, w, i, j);
			}
		}
	}
}


/****************************************************************************
 * Description    :  copyHoleIntoFG()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::copyHoleIntoFG(const slContours::const_iterator &contour, slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const Mat cont(contour.mat());
	const Rect rect = boundingRect(cont);

	// x-coordinates
	const int x1 = rect.x;
	const int x2 = rect.x + rect.width;

	// y-coordinates
	const int y1 = rect.y;
	const int y2 = rect.y + rect.height;

	// For each row
	for (int i = y1; i < y2; i++) {
		slPixel1ch *b_fg_row = bForeground[i];

		// For each column
		for (int j = x1; j < x2; j++) {
			// Test inside or outside
			double inside = pointPolygonTest(cont, Point(j, i), false);

			// Point to be copied into foreground
			if (inside > 0) {
				b_fg_row[j] = PIXEL_1CH_WHITE;
			}
		}
	}
}


/****************************************************************************
 * Description    :  updateWindows()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slBgSub::updateWindows(const slImage1ch &bForeground)
{
	windowsList_t::iterator it;

	if ((it = windowsList_.find(ARG_C_FR)) != windowsList_.end()) {
		it->second->show(current_, colorSystem_);	// current frame
	}
	if ((it = windowsList_.find(ARG_QC_FR)) != windowsList_.end()) {
		it->second->show(qCurrent_, colorSystem_);	// quantified current frame
	}

	if ((it = windowsList_.find(ARG_BG)) != windowsList_.end()) {
		it->second->show(background_, colorSystem_);	// background
	}
	if ((it = windowsList_.find(ARG_QBG)) != windowsList_.end()) {
		it->second->show(qBackground_, colorSystem_);	// quantified background
	}

	if ((it = windowsList_.find(ARG_FG)) != windowsList_.end()) {
		foreground_ = (colorSystem_ == SL_BGR ? BGR_WHITE : HSV_WHITE);
		current_.copyTo(foreground_, bForeground);
		it->second->show(foreground_, colorSystem_);	// foreground
	}
	if ((it = windowsList_.find(ARG_BFG)) != windowsList_.end()) {
		it->second->show(bForeground);	// binary foreground
	}
}


///////////////////////////////////////////////////////////////////////////////
//	slBgSubCreator
///////////////////////////////////////////////////////////////////////////////


// Initializing factories list
slBgSubFactory::factories_t* slBgSubFactory::factories_ = NULL;


slBgSubFactory::slBgSubFactory(const std::string& name)
: name_(name)
{
	if (factories_ == NULL) {
		factories_ = new factories_t;
	}

	(*factories_)[name_] = this;
}


slBgSubFactory::~slBgSubFactory()
{
}


void slBgSubFactory::fillAllParamSpecs(slParamSpecMap& paramSpecMap)
{
	slBgSub::fillGlobalParamSpecs(paramSpecMap);

	// Algorithm
	slParamSpec specAlgo(ARG_ALGO, "Algorithm name", MANDATORY);
	specAlgo << slSyntax("ALGORITHM");

	if (factories_ != NULL) {
		// Add all parameters for all algorithms
		for (factories_t::const_iterator factory = factories_->begin();
			factory != factories_->end(); factory++)
		{
			slParamSpecMap algoParams;

			factory->second->fillParamSpecs(algoParams);
			specAlgo.setSubParamSpec(slValVect() << factory->first, algoParams);
		}
	}

	paramSpecMap << specAlgo;
}


slBgSub* slBgSubFactory::createInstance(const char *bgSubName)
{
	if (factories_ == NULL) {
		throw slExceptionBgSub("slBgSubFactory::createInstance(): no factory available");
	}

	// Find the factory corresponding to bgSubName
	string algoName(bgSubName);
	factories_t::const_iterator itFactory = factories_->find(algoName);

	// If the factory does not exist
	if (itFactory == factories_->end()) {
		throw slExceptionBgSub("Bad algorithm name : \"" + algoName + "\"");
	}

	// Create a new instance of the specified algorithm
	return itFactory->second->createInstance();
}


slBgSub* slBgSubFactory::createInstance(const slParameters& parameters)
{
	// Create a new instance of the specified algorithm
	slBgSub *bgSub = createInstance(parameters.getValue(ARG_ALGO).c_str());

	try {
		// Configure the bgSub
		bgSub->setParameters(parameters);
	}
	catch (...) {
		delete bgSub;
		throw;
	}

	return bgSub;
}


