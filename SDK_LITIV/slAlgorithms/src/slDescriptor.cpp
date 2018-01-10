#include "slDescriptor.h"


using namespace std;


///////////////////////////////////////////////////////////////////////////////
//	slDescriptor
///////////////////////////////////////////////////////////////////////////////


slDescriptor::slDescriptor()
{
}


slDescriptor::~slDescriptor()
{
}


///////////////////////////////////////////////////////////////////////////////
//	slDceK
///////////////////////////////////////////////////////////////////////////////


slDceK::slDceK(float k): slDescriptor(), k_(k)
{
}


slDceK* slDceK::clone() const
{
	return new slDceK(*this);
}


string slDceK::getName() const
{
	return DESC_K;
}


float slDceK::scoreK(const slDceK &k2) const
{
	float length = sqrt(k_ * k_ + k2.k_ * k2.k_);

	return (-fabs(k_ - k2.k_) / (length > 0 ? length : 1));
}


float slDceK::compute(const cv::Point &p0, const cv::Point &p1, const cv::Point &p2)
{
	CvPoint vect1 = cvPoint(p1.x - p0.x, p1.y - p0.y);
	CvPoint vect2 = cvPoint(p2.x - p1.x, p2.y - p1.y);
	float long1 = sqrt((float)vect1.x * vect1.x + vect1.y * vect1.y);
	float long2 = sqrt((float)vect2.x * vect2.x + vect2.y * vect2.y);
	float beta = acos(((float)vect1.x * vect2.x + vect1.y * vect2.y) / (long1 * long2));

	return (beta * long1 * long2 / (long1 + long2));
}


float slDceK::compute(const cv::Point2f &p0, const cv::Point2f &p1, const cv::Point2f &p2)
{
	CvPoint2D32f vect1 = cvPoint2D32f(p1.x - p0.x, p1.y - p0.y);
	CvPoint2D32f vect2 = cvPoint2D32f(p2.x - p1.x, p2.y - p1.y);
	float long1 = sqrt(vect1.x * vect1.x + vect1.y * vect1.y);
	float long2 = sqrt(vect2.x * vect2.x + vect2.y * vect2.y);
	float beta = acos((vect1.x * vect2.x + vect1.y * vect2.y) / (long1 * long2));

	return (beta * long1 * long2 / (long1 + long2));
}


///////////////////////////////////////////////////////////////////////////////
//	slSkelRelDist
///////////////////////////////////////////////////////////////////////////////


slSkelRelDist::slSkelRelDist(float relDist): slDescriptor(), relDist_(relDist)
{
}


slSkelRelDist* slSkelRelDist::clone() const
{
	return new slSkelRelDist(*this);
}


string slSkelRelDist::getName() const
{
	return DESC_RELDIST;
}


float slSkelRelDist::scoreRelDist(const slSkelRelDist &rd2) const
{
	return -fabs(relDist_ - rd2.relDist_);
}


