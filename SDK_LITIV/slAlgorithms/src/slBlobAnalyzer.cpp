#include "slBlobAnalyzer.h"
#include "slContourEngine.h"

#include <iostream>


using namespace cv;
using namespace std;
using namespace slAH;


///////////////////////////////////////////////////////////////////////////////
//	slBlobAnalyzer
///////////////////////////////////////////////////////////////////////////////


#define ARG_MINAREA "-a"


slBlobAnalyzer::slBlobAnalyzer(bool analyzeHoles)
: analyzeHoles_(analyzeHoles), minArea_(1)
{
}


slBlobAnalyzer::~slBlobAnalyzer()
{
}


void slBlobAnalyzer::fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	slBlobAnalyzerFactory::fillAllParamSpecs(paramSpecMap);
}


void slBlobAnalyzer::fillGlobalParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_MINAREA, "Minimum area") << slSyntax("1..n", "1"));
}


void slBlobAnalyzer::setParameters(const slAH::slParameters& parameters)
{
	// Set global parameters
	setMinArea(atof(parameters.getValue(ARG_MINAREA).c_str()));

	// Other parameters
	setSubParameters(parameters);
}


void slBlobAnalyzer::setMinArea(double minArea)
{
	minArea_ = (minArea >= 1 ? minArea : 1);
}


void slBlobAnalyzer::showParameters() const
{
	cout << "--- slBlobAnalyzer ---" << endl;

	cout << "Minimum area : " << minArea_ << endl;

	cout << endl;

	showSubParameters();
}


void slBlobAnalyzer::analyzeAllBlobs(const slContours &contours)
{
	points_.clear();

	// For each external contour
	for (slContours::const_iterator contour = contours.begin();
		!contour.isNull(); contour = contour.next())
	{
		points_[contour] = (contourArea(contour.mat()) >= minArea_ ?
			analyzeBlob(contour) : slKeyPoints());

		if (analyzeHoles_) {
			// For each hole or internal contour
			for (slContours::const_iterator child = contour.child();
				!child.isNull(); child = child.next())
			{
				points_[child] = (contourArea(child.mat()) >= minArea_ ?
					analyzeBlob(child) : slKeyPoints());
			}
		}
	}
}


bool slBlobAnalyzer::hasKeyPoints(const slContours::const_iterator &contour) const
{
	return (points_.find(contour) != points_.end());
}


const slKeyPoints& slBlobAnalyzer::getKeyPoints(const slContours::const_iterator &contour) const
{
	map<slContours::const_iterator, slKeyPoints>::const_iterator it = points_.find(contour);

	if (it == points_.end()) {
		throw slException("slBlobAnalyzer::getNeighborhood(): contour does not exist.");
	}

	return it->second;
}


///////////////////////////////////////////////////////////////////////////////
//	slDce
///////////////////////////////////////////////////////////////////////////////


#define ARG_MAXPT "-n"


slDce::slDce()
: slBlobAnalyzer(true), maxPt_(12)
{
}


slDce::~slDce()
{
}


void slDce::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_MAXPT, "Maximum number of points") << slSyntax("3..n", "12"));
}


void slDce::setSubParameters(const slParameters& parameters)
{
	setMaxNumOfPoints(atoi(parameters.getValue(ARG_MAXPT).c_str()));
}


void slDce::setMaxNumOfPoints(int maxPt)
{
	maxPt_ = (maxPt >= 3 ? maxPt : 3);
}


void slDce::showSubParameters() const
{
	cout << "--- slDce ---" << endl;
	cout << "Max. Num. of points : " << maxPt_ << endl;
}


slKeyPoints slDce::analyzeBlob(const slContours::const_iterator &contour)
{
	struct PtDCE {
		cv::Point pt;
		float K;
		PtDCE *left;
		PtDCE *right;
	};

	if (contour->empty()) return slKeyPoints();

	// Copy all points into a vector
	int totalPt = (int)contour->size();
	vector<PtDCE> points(totalPt);

	for (int ind = 0; ind < totalPt; ind++) {
		points[ind].pt = (*contour)[ind];
	}

	// Create a circular double-linked list
	PtDCE *head = &points.front();

	for (int ind = 1; ind < totalPt; ind++)
	{
		head[ind - 1].right = &head[ind];
		head[ind].left = &head[ind - 1];
	}
	points.back().right = head;
	head->left = &points.back();

	// Compute K for each point
	for (int ind = 0; ind < totalPt; ind++) {
		head[ind].K = slDceK::compute(head[ind].left->pt, head[ind].pt, head[ind].right->pt);
	}

	// Loop while the number of remaining points is too high
	while (totalPt > maxPt_) {
		// Find the least significant point
		PtDCE *ptrMin = head;

		for (PtDCE *ptr = head->right; ptr != head; ptr = ptr->right) {
			if (ptr->K < ptrMin->K) ptrMin = ptr;
		}

		// Delete the least significant point
		totalPt--;
		if (head == ptrMin) head = head->right;

		// Join the neighbors together
		ptrMin->left->right = ptrMin->right;
		ptrMin->right->left = ptrMin->left;

		// Update neighbors' K value
		ptrMin->left->K = slDceK::compute(ptrMin->left->left->pt, ptrMin->left->pt, ptrMin->left->right->pt);
		ptrMin->right->K = slDceK::compute(ptrMin->right->left->pt, ptrMin->right->pt, ptrMin->right->right->pt);
	}

	// Return remaining points
	slKeyPoints keyPoints;

	for (int ind = 0; ind < totalPt; ind++) {
		CvPoint2D32f position = cvPointTo32f(head->pt);
		slKeyPoint &keyPoint = keyPoints[position];

		// Save its descriptors
		keyPoint.moveTo(position);
		keyPoint.insert(new slDceK(head->K));
		keyPoint.insert(head->left->pt);
		keyPoint.insert(head->right->pt);

		head = head->right;
	}

	return keyPoints;
}


float slDce::compareKeyPoints(const slKeyPoint &kPt1, const slKeyPoint &kPt2,
							  float diagLength, int offsetX, int offsetY) const
{
	float score = 0;

	score += kPt1.scoreEuclidean(kPt2, diagLength, offsetX, offsetY);
	score += kPt1.scoreOrientation(kPt2);

	const slDceK *k1 = dynamic_cast<const slDceK*>(kPt1.find(DESC_K));
	const slDceK *k2 = dynamic_cast<const slDceK*>(kPt2.find(DESC_K));

	if (k1 != NULL && k2 != NULL) score += k1->scoreK(*k2);

	return score;
}


///////////////////////////////////////////////////////////////////////////////
//	slSkel
///////////////////////////////////////////////////////////////////////////////


#define ARG_SIZE "-s"
#define ARG_MINHOLEAREA "-ha"
#define ARG_APPROX "-ap"
#define ARG_PEAKTH "-pt"


slSkel::slSkel(): slBlobAnalyzer(),
size_(320, 240), minHoleArea_(512), doApprox_(false), extDist_(3.5), intDist_(3.0),
peekThreshold_(4), kernel181_(Size(3, 3))/*, winSkel_("winSkel_")*/
{
	// [-1 -1 -1 ; -1  8 -1 ; -1 -1 -1]
	kernel181_ = -1;
	kernel181_[1][1] = 8;
}


slSkel::~slSkel()
{
	kernel181_.release();
}


void slSkel::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_SIZE, "Size of image buffer")
		<< slSyntax("width", "320") << slSyntax("height", "240"));

	paramSpecMap << (slParamSpec(ARG_MINHOLEAREA, "Minimum hole area") << slSyntax("area", "512"));

	paramSpecMap << (slParamSpec(ARG_APPROX, "Approx. cont. (distance)")
		<< slSyntax("external", "3.5") << slSyntax("internal", "3.0"));

	paramSpecMap << (slParamSpec(ARG_PEAKTH, "Peak Threshold") << slSyntax("minThres", "4.0"));
}


void slSkel::setSubParameters(const slParameters& parameters)
{
	setSize(Size(
		atoi(parameters.getValue(ARG_SIZE, 0).c_str()),
		atoi(parameters.getValue(ARG_SIZE, 1).c_str())));

	setMinHoleArea(atoi(parameters.getValue(ARG_MINHOLEAREA).c_str()));

	if (parameters.isParsed(ARG_APPROX)) {
		setApprox(true,
			atof(parameters.getValue(ARG_APPROX, 0).c_str()),
			atof(parameters.getValue(ARG_APPROX, 1).c_str()));
	}
	else {
		setApprox(false);
	}

	setPeakThreshold((float)atof(parameters.getValue(ARG_PEAKTH).c_str()));
}


void slSkel::setSize(cv::Size size)
{
	size_ = size;

	imBlob_.create(size);
	imDist_.create(size);
	imSkel_.create(size);
}


void slSkel::setMinHoleArea(unsigned int minHoleArea)
{
	minHoleArea_ = minHoleArea;
}


void slSkel::setApprox(bool doApprox, double extDist, double intDist)
{
	doApprox_ = doApprox;
	extDist_ = extDist;
	intDist_ = intDist;
}


void slSkel::setPeakThreshold(float peakTh)
{
	peekThreshold_ = peakTh;
}


void slSkel::showSubParameters() const
{
	cout << "--- slSkel ---" << endl;
	cout << "Min. Hole Area : " << minHoleArea_ << endl;

	if (doApprox_) {
		cout << "Approximate cont.: ext(" << extDist_ << "), int(" << intDist_ << ")" << endl;
	}
	else {
		cout << "Approximate cont.: no" << endl;
	}

	cout << "Min. Peak Threshold : " << peekThreshold_ << endl;
}


slKeyPoints slSkel::analyzeBlob(const slContours::const_iterator &contour)
{
	if (contour->empty()) return slKeyPoints();

	// Prepare the images (buffers)
	Rect rect = boundingRect(*contour);
	rect.x -= 1; rect.width += 2;	// Encadrer d'un pixel pour
	rect.y -= 1; rect.height += 2;	// la transformée distance

	int width = size_.width, height = size_.height;
	if (width < rect.x + rect.width) width = rect.x + rect.width;
	if (height < rect.y + rect.height) height = rect.y + rect.height;
	if (width > size_.width || height > size_.height) setSize(cvSize(width, height));

	// Fill with black
	imBlob_ = PIXEL_1CH_BLACK;
	imDist_ = 0;

	// Compute the distance-transformed image, highligt the peaks with kernel 181
	distanceTransform(contour, rect);

	// Extract raw points (peaks)
	CvPtVector_t rawPts = getRawPoints(contour, rect);

	// Create complete graph from all points
	const unsigned int N = rawPts.size(), distInf = width * width + height * height + 1;
	unsigned int *dists = createDistMat(rawPts);
	UIntVector_t parents(N, N);

	// Apply Prim's algorithm (as seen on Wikipedia)
	computeParents(parents, dists, distInf);

	// No need to keep dists at this point
	delete [] dists;
	// Search for keypoints
	return computeKeyPoints(rawPts, parents);
}


void slSkel::distanceTransform(const slContours::const_iterator &contour, const cv::Rect &rect)
{
	const Scalar BLACK(0), WHITE(255);

	// Compute approximated external contour
	slContours approxExt(doApprox_ ? slContourEngine::approximate(*contour, extDist_) : *contour);

	// Fill with approximated contour in white
	drawContours(imBlob_, approxExt.all(), 0, WHITE, CV_FILLED, 8, approxExt.hierarchy(), 0);

	// For each hole
	for (slContours::const_iterator hole = contour.child(); !hole.isNull(); hole = hole.next()) {
		// If the hole is large enough
		if (fabs(contourArea(hole.mat())) >= minHoleArea_) {
			// Compute approximated internal contour
			slContours approxInt(doApprox_ ? slContourEngine::approximate(*hole, intDist_) : *hole);

			// Paint the hole in black
			drawContours(imBlob_, approxInt.all(), 0, BLACK, CV_FILLED, 8, approxInt.hierarchy(), 0);
		}
	}

	// Optimize computation by using ROI
	Mat imBlob(imBlob_, rect);
	Mat imDist(imDist_, rect);
	Mat imSkel(imSkel_, rect);

	// Apply distance transformation on imBlob and highlight peaks in imSkel
	cv::distanceTransform(imBlob, imDist, CV_DIST_L2, 3);
	filter2D(imDist, imSkel, -1, kernel181_);

	// Normalize imDist for the feature points' values
	normalize(imDist, imDist, 0, 1, NORM_MINMAX);

	//winSkel_.show(imSkel_);	waitKey();
}


CvPtVector_t slSkel::getRawPoints(const slContours::const_iterator &contour, const cv::Rect &rect) const
{
	CvPtVector_t rawPts;

	// For each pixel of the ROI
	for (int row = rect.y; row < rect.y + rect.height; row++) {
		const float *rowPtr = imSkel_[row];

		for (int col = rect.x; col < rect.x + rect.width; col++) {
			// If it is a significant peak
			if (rowPtr[col] >= peekThreshold_) {
				rawPts.push_back(cvPoint(col, row));
			}
		}
	}

	return rawPts;
}


unsigned int* slSkel::createDistMat(const CvPtVector_t &rawPts) const
{
	const unsigned int N = rawPts.size();
	unsigned int *dists = new unsigned int[N * N];

	// Square of distance between all points
	for (unsigned int row = 0; row < N; row++) {
		for (unsigned int col = 0; col < N; col++) {
			int diffX = rawPts[row].x - rawPts[col].x;
			int diffY = rawPts[row].y - rawPts[col].y;

			dists[row * N + col] = (unsigned int)(diffX * diffX + diffY * diffY);
		}
	}

	return dists;
}


void slSkel::computeParents(UIntVector_t &parents, const unsigned int *dists, unsigned int distInf) const
{
	const unsigned int N = parents.size();
	UIntVector_t distTree(N + 1, distInf);
	vector<bool> inTree(N, false);
	unsigned int parent = 0;

	// While not end of parents
	while (parent < N) {
		// Add current parent in tree
		inTree[parent] = true;

		// Set nextParent NULL
		int nextParent = N;

		// For each child (we know 0 is always a parent)
		for (unsigned int child = 1; child < N; child++) {
			// If the child is not in tree
			if (!inTree[child]) {
				// If distance between the parent and the child is less than
				// the known distance between the tree and the child
				if (distTree[child] > dists[parent * N + child]) {
					distTree[child] = dists[parent * N + child];
					parents[child] = parent;	// Save potential parent for child
				}

				// Keep closest child to the tree
				if (distTree[nextParent] > distTree[child]) {
					nextParent = child;
				}
			}
		}

		parent = nextParent;
	}
}


slKeyPoints slSkel::computeKeyPoints(const CvPtVector_t &rawPts, UIntVector_t &parents) const
{
	const unsigned int N = rawPts.size();
	if (N < 2) return slKeyPoints();

	// Compute the number of neighbors for all points
	UIntVector_t nbNeighbors(N, 0);

	for (unsigned int ind = 0; ind < N; ind++) {
		// If the current point has a valid parent
		if (parents[ind] < N) {
			nbNeighbors[ind]++;				// Current point has one more neighbor
			nbNeighbors[parents[ind]]++;	// The parent has one more neighbor
		}
	}

	// Find a keypoint
	unsigned int keyPtInd = 0;

	while (nbNeighbors[keyPtInd] == 2) {
		keyPtInd++;
	}

	// From the keypoint, find the greatest parent and previous keypoint
	unsigned int greatestParent = keyPtInd;

	while (parents[greatestParent] < N) {
		greatestParent = parents[greatestParent];
		if (nbNeighbors[greatestParent] != 2) keyPtInd = greatestParent;
	}

	// Make sure the greatest parent is a keypoint
	parents[greatestParent] = keyPtInd;
	parents[keyPtInd] = N;
	greatestParent = keyPtInd;

	// Save keypoints and their neighbors
	slKeyPoints keyPoints;

	for (unsigned int ind = 0; ind < N; ind++) {
		// If it is a keypoint
		if (nbNeighbors[ind] != 2) {
			Point pt = rawPts[ind];
			Point2f position(pt);
			slKeyPoint &keyPoint = keyPoints[position];

			// Save its descriptors
			keyPoint.moveTo(position);
			keyPoint.insert(new slSkelRelDist(imDist_[pt.y][pt.x]));
		}
	}

	for (unsigned int ind = 0; ind < N; ind++) {
		// If it is a keypoint, but not the greatest parent
		if (nbNeighbors[ind] != 2 && ind != greatestParent) {
			Point2f pt(rawPts[ind]), neighborPt;
			unsigned int neighborInd = parents[ind];

			while (nbNeighbors[neighborInd] == 2) {
				neighborInd = parents[neighborInd];
			}

			neighborPt = rawPts[neighborInd];

			// Link the keypoint to its parent (neighbors)
			keyPoints[pt].insert(neighborPt);
			keyPoints[neighborPt].insert(pt);
		}
	}

	return keyPoints;
}


float slSkel::compareKeyPoints(const slKeyPoint &kPt1, const slKeyPoint &kPt2,
							   float diagLength, int offsetX, int offsetY) const
{
	float score = 0;

	score += kPt1.scoreEuclidean(kPt2, diagLength, offsetX, offsetY);
	score += kPt1.scoreOrientation(kPt2);
	score += kPt1.scoreNbNeighbors(kPt2);

	const slSkelRelDist *rd1 = dynamic_cast<const slSkelRelDist*>(kPt1.find(DESC_RELDIST));
	const slSkelRelDist *rd2 = dynamic_cast<const slSkelRelDist*>(kPt2.find(DESC_RELDIST));

	if (rd1 != NULL && rd2 != NULL) score += rd1->scoreRelDist(*rd2);

	return score;
}



///////////////////////////////////////////////////////////////////////////////
//	slBlobAnalyzerFactory
///////////////////////////////////////////////////////////////////////////////


// Initializing factories list
slBlobAnalyzerFactory::factories_t* slBlobAnalyzerFactory::factories_ = NULL;


slBlobAnalyzerFactory::slBlobAnalyzerFactory(const std::string& name)
: name_(name)
{
	if (factories_ == NULL) {
		factories_ = new factories_t;
	}

	(*factories_)[name_] = this;
}


slBlobAnalyzerFactory::~slBlobAnalyzerFactory()
{
}


void slBlobAnalyzerFactory::fillAllParamSpecs(slParamSpecMap& paramSpecMap)
{
	slBlobAnalyzer::fillGlobalParamSpecs(paramSpecMap);

	// Algorithm
	slParamSpec specAlgo(ARG_BA, "Blob analyzer name", MANDATORY);
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


slBlobAnalyzer* slBlobAnalyzerFactory::createInstance(const char *baName)
{
	if (factories_ == NULL) {
		throw slException("slBlobAnalyzerFactory::createInstance(): no factory available");
	}

	// Find the factory corresponding to baName
	string algoName(baName);
	factories_t::const_iterator itFactory = factories_->find(algoName);

	// If the factory does not exist
	if (itFactory == factories_->end()) {
		throw slException(string("Bad algorithm name : \"" + algoName + "\"").c_str());
	}

	// Create a new instance of the specified algorithm
	return itFactory->second->createInstance();
}


slBlobAnalyzer* slBlobAnalyzerFactory::createInstance(const slParameters& parameters)
{
	// Create a new instance of the specified algorithm
	slBlobAnalyzer *blobAnalyzer = createInstance(parameters.getValue(ARG_BA).c_str());

	// Configure the blobAnalyzer
	blobAnalyzer->setParameters(parameters);

	return blobAnalyzer;
}


///////////////////////////////////////////////////////////////////////////////
//	slDceFactory
///////////////////////////////////////////////////////////////////////////////


slDceFactory slDceFactory::factory_;


slDceFactory::slDceFactory()
: slBlobAnalyzerFactory("dce")
{
}


slDceFactory::~slDceFactory()
{
}


void slDceFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
	slDce::fillParamSpecs(paramSpecMap);
}


slDce* slDceFactory::createInstance() const
{
	return new slDce();
}


///////////////////////////////////////////////////////////////////////////////
//	slSkelFactory
///////////////////////////////////////////////////////////////////////////////


slSkelFactory slSkelFactory::factory_;


slSkelFactory::slSkelFactory()
: slBlobAnalyzerFactory("skel")
{
}


slSkelFactory::~slSkelFactory()
{
}


void slSkelFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
	slSkel::fillParamSpecs(paramSpecMap);
}


slSkel* slSkelFactory::createInstance() const
{
	return new slSkel();
}


