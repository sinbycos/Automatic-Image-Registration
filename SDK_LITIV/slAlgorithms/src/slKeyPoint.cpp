#include "slKeyPoint.h"


using namespace cv;
using namespace std;


///////////////////////////////////////////////////////////////////////////////
//	slKeyPoint
///////////////////////////////////////////////////////////////////////////////


slKeyPoint::slKeyPoint()
{
}


slKeyPoint::slKeyPoint(const slKeyPoint &kPt)
{
	*this = kPt;
}


slKeyPoint::~slKeyPoint()
{
	clearDescriptors();
}


void slKeyPoint::clearDescriptors()
{
	for (map<string, slDescriptor*>::iterator it = descriptors_.begin();
		it != descriptors_.end(); it++)
	{
		delete it->second;
	}

	descriptors_.clear();
}


const slKeyPoint& slKeyPoint::operator=(const slKeyPoint &kPt)
{
	if (&kPt != this) {
		position_ = kPt.position_;
		neighbors_ = kPt.neighbors_;
		clearDescriptors();

		for (map<string, slDescriptor*>::const_iterator it = kPt.descriptors_.begin();
			it != kPt.descriptors_.end(); it++)
		{
			descriptors_[it->first] = it->second->clone();
		}
	}

	return *this;
}


void slKeyPoint::moveTo(const cv::Point2f &position)
{
	position_ = position;
}


const cv::Point2f& slKeyPoint::position() const
{
	return position_;
}


void slKeyPoint::insert(const cv::Point &neighbor)
{
	insert(Point2f(neighbor));
}


void slKeyPoint::insert(const cv::Point2f &neighbor)
{
	neighbors_.push_back(neighbor);
}


CvPt2fVector_t::const_iterator slKeyPoint::beginNeighbors() const
{
	return neighbors_.begin();
}


CvPt2fVector_t::const_iterator slKeyPoint::endNeighbors() const
{
	return neighbors_.end();
}


const slDescriptor* slKeyPoint::find(const std::string &name) const
{
	map<string, slDescriptor*>::const_iterator it = descriptors_.find(name);

	return (it != descriptors_.end() ? it->second : NULL);
}


void slKeyPoint::insert(slDescriptor *desc)
{
	string name(desc->getName());

	remove(name);
	descriptors_[name] = desc;
}


void slKeyPoint::remove(const std::string &name)
{
	map<string, slDescriptor*>::iterator it = descriptors_.find(name);

	if (it != descriptors_.end()) {
		delete it->second;
		descriptors_.erase(it);
	}
}


float slKeyPoint::scoreEuclidean(const slKeyPoint &kPt2, float diagLength, int offsetX, int offsetY) const
{
	float diffx = position_.x + offsetX - kPt2.position_.x;
	float diffy = position_.y + offsetY - kPt2.position_.y;

	return (1 / (1 + exp(-3 + 6 * sqrt(diffx * diffx + diffy * diffy) / diagLength)));
}


float slKeyPoint::scoreOrientation(const slKeyPoint &kPt2) const
{
	const slKeyPoint &kPt1 = *this;
	const CvPoint2D32f pt1 = kPt1.position_;
	const CvPoint2D32f pt2 = kPt2.position_;
	const unsigned int nb1 = kPt1.neighbors_.size();
	const unsigned int nb2 = kPt2.neighbors_.size();
	float *cosTheta = new float[nb1 * nb2];

	// Compute all possible cos(Theta)
	for (unsigned int ind1 = 0; ind1 < nb1; ind1++) {
		CvPoint2D32f ne1 = kPt1.neighbors_[ind1];

		for (unsigned int ind2 = 0; ind2 < nb2; ind2++) {
			CvPoint2D32f ne2 = kPt2.neighbors_[ind2];

			float ax = ne1.x - pt1.x;
			float ay = ne1.y - pt1.y;
			float bx = ne2.x - pt2.x;
			float by = ne2.y - pt2.y;

			cosTheta[ind1 * nb2 + ind2] = (ax * bx + ay * by) /
				sqrt((ax * ax + ay * ay) * (bx * bx + by * by));
		}
	}

	float sumCosTheta = 0.0f;

	// For each neighbor of point 1
	for (unsigned int ind1 = 0; ind1 < nb1; ind1++) {
		float *row = &cosTheta[ind1 * nb2];
		int ind2Max = 0;

		// Find the neighbor of point 2 that gives a similar orientation
		for (unsigned int ind2 = 1; ind2 < nb2; ind2++) {
			if (row[ind2] > row[ind2Max]) ind2Max = ind2;
		}

		// Find the neighbor of point 1 that gives a similar orientation
		float *col = &cosTheta[ind2Max];
		int ind1Max = 0;

		for (unsigned int ind1_ = 1; ind1_ < nb1; ind1_++) {
			if (col[ind1_ * nb2] > col[ind1Max * nb2]) ind1Max = ind1_;
		}

		// Update the sum of cos(theta) if points 1 and 2 are mutually choosing each other
		if (ind1Max == ind1) sumCosTheta += row[ind2Max];
	}

	delete[] cosTheta;

	return (sumCosTheta / max(nb1, nb2));
}


float slKeyPoint::scoreNbNeighbors(const slKeyPoint &kPt2) const
{
	unsigned int nb1 = (     neighbors_.size() > 1) ? 3 : 1;
	unsigned int nb2 = (kPt2.neighbors_.size() > 1) ? 3 : 1;

	return ((nb1 == nb2) ? 2.0f : 0.0f);
}


