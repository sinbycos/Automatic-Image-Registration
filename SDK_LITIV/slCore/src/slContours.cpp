#include "slContours.h"


using namespace cv;


enum Direction {NEXT, PREVIOUS, CHILD, PARENT};


slContours::slContours()
{
}

slContours::slContours(const slContour &contour)
{
	contours_.push_back(contour);
	hierarchy_.push_back(Vec4i(-1, -1, -1, -1));
}


void slContours::clear()
{
	contours_.clear();
	hierarchy_.clear();
}


void slContours::findAll(slImage1ch &image)
{
	clear();
	cv::findContours(image, contours_, hierarchy_, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
}


slContours::iterator slContours::begin()
{
	return slContours_iterator(this, hierarchy_.size() > 0 ? 0 : -1);
}

slContours::const_iterator slContours::begin() const
{
	return slContours_const_iterator(this, hierarchy_.size() > 0 ? 0 : -1);
}


slContours_iterator::slContours_iterator(slContours *ref, int index)
: ref_(ref), index_(index)
{
}

slContours_const_iterator::slContours_const_iterator(const slContours *ref, int index)
: ref_(ref), index_(index)
{
}

slContours_const_iterator::slContours_const_iterator(const slContours_iterator &it)
: ref_(it.ref_), index_(it.index_)
{
}


slContours_iterator slContours_iterator::next() const
{
	return slContours_iterator(ref_, ref_->hierarchy_[index_][NEXT]);
}

slContours_const_iterator slContours_const_iterator::next() const
{
	return slContours_const_iterator(ref_, ref_->hierarchy_[index_][NEXT]);
}


slContours_iterator slContours_iterator::previous() const
{
	return slContours_iterator(ref_, ref_->hierarchy_[index_][PREVIOUS]);
}

slContours_const_iterator slContours_const_iterator::previous() const
{
	return slContours_const_iterator(ref_, ref_->hierarchy_[index_][PREVIOUS]);
}


slContours_iterator slContours_iterator::child() const
{
	return slContours_iterator(ref_, ref_->hierarchy_[index_][CHILD]);
}

slContours_const_iterator slContours_const_iterator::child() const
{
	return slContours_const_iterator(ref_, ref_->hierarchy_[index_][CHILD]);
}


slContours_iterator slContours_iterator::parent() const
{
	return slContours_iterator(ref_, ref_->hierarchy_[index_][PARENT]);
}

slContours_const_iterator slContours_const_iterator::parent() const
{
	return slContours_const_iterator(ref_, ref_->hierarchy_[index_][PARENT]);
}


bool slContours_iterator::isNull() const
{
	return (index_ < 0 || ref_ == NULL);
}

bool slContours_const_iterator::isNull() const
{
	return (index_ < 0 || ref_ == NULL);
}


cv::Mat slContours_iterator::mat()
{
	return Mat(ref_->contours_[index_]);
}

const cv::Mat slContours_const_iterator::mat() const
{
	return Mat(ref_->contours_[index_]);
}


slContour& slContours_iterator::operator*()
{
	return ref_->contours_[index_];
}

const slContour& slContours_const_iterator::operator*() const
{
	return ref_->contours_[index_];
}


slContour* slContours_iterator::operator->()
{
	return &ref_->contours_[index_];
}

const slContour* slContours_const_iterator::operator->() const
{
	return &ref_->contours_[index_];
}


bool slContours_iterator::operator<(const slContours_iterator &it) const
{
	return (index_ < it.index_ || (index_ == it.index_ && ref_ < it.ref_));
}

bool slContours_const_iterator::operator<(const slContours_const_iterator &it) const
{
	return (index_ < it.index_ || (index_ == it.index_ && ref_ < it.ref_));
}


