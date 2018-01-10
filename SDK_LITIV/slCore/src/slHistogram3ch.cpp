/*!	\file	slHistogram3ch.cpp
 *	\brief	Histogram used in rectangle background subtractor
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		23.05.2007 - February 2012
 */

#include "slException.h"
#include "slHistogram3ch.h"

#include <algorithm>
#include <math.h>
#include <iostream>


using namespace std;


int slHistogram3ch::RANGE1 = 0;
int slHistogram3ch::RANGE2 = 0;
int slHistogram3ch::RANGE3 = 0;
bool slHistogram3ch::HSV_ = true;
bool slHistogram3ch::MPDA_ = false;


/*
 *    Default constructor for the slHistogram3ch.
 *    attention the range of the single histogram must be set before using
 */
slHistogram3ch::slHistogram3ch()
: isEmpty_(true), ch1_(RANGE1,0), ch2_(RANGE2,0), ch3_(RANGE3,0)
{
}


// *    Constructor for the slHistogram3ch
// *	attention the range of the single histogram must be set before using
// *    param	 pixel (slPixel3ch&) - Pixel used to create the histogram       
//slHistogram3ch::slHistogram3ch(const slPixel3ch& pixel)
//: isEmpty_(false), ch1_(RANGE1,0), ch2_(RANGE2,0), ch3_(RANGE3,0)
//{
//	//cout << "pixel h " << pixel.h() <<" s " << pixel.s() << " v " << pixel.v()<<endl;
//	++ch1_[pixel.h()];
//	++ch2_[pixel.s()];
//	++ch3_[pixel.v()];
//}


slHistogram3ch::~slHistogram3ch()
{}


/*!
 *	\param	 range1 (int) - upper limit of the first histogram  
 *	\param	 range2 (int) - upper limit of the second histogram
 *	\param	 range3 (int) - upper limit of the third histogram 
 */
void slHistogram3ch::setRange(int range1, int range2, int range3)
{
	if(HSV_)
	{
		if((180 % range1) != 0 || (256 % range2) != 0 || (256 % range3))
			throw slException("Quantification args must be divider of 180 for h and 256 for s and v");
	}
	else
		if((256 % range1) != 0 || (256 % range2) != 0 || (256 % range3))
			throw slException("Quantification args must be divider of 256 for r,g and b");

	RANGE1 = range1;
	RANGE2 = range2;
	RANGE3 = range3;
}


void slHistogram3ch::setColorSpace(bool HSV)
{
	HSV_ = HSV;
}


void slHistogram3ch::setDistanceMethod(bool MPDA)
{
	MPDA_ = MPDA;
}


/*
 *    Reset each histogram to 0
 */
void slHistogram3ch::clear(void)
{
	fill(ch1_.begin(),ch1_.end(),(float)0.0);
	fill(ch2_.begin(),ch2_.end(),(float)0.0);
	fill(ch3_.begin(),ch3_.end(),(float)0.0);

	isEmpty_ = true;
}


/*
 *    Normalize each histogram to lessen the negative effect of light change between frame
 */
void slHistogram3ch::normalize (void)
{
	float ch1Sum = 0;
	float ch2Sum = 0;
	float ch3Sum = 0;

	for (size_t i = 0; i < ch1_.size(); ++i)
		ch1Sum += ch1_[i];
	for (size_t i = 0; i < ch2_.size(); ++i)
		ch2Sum += ch2_[i];
	for (size_t i = 0; i < ch3_.size(); ++i)
		ch3Sum += ch3_[i];

	for (size_t i = 0; i < ch1_.size(); ++i)
		ch1_[i] = ch1_[i] / ch1Sum;
	for (size_t i = 0; i < ch2_.size(); ++i)
		ch2_[i] = ch2_[i] / ch1Sum;
	for (size_t i = 0; i < ch3_.size(); ++i)
		ch3_[i] = ch3_[i] / ch1Sum;
}


/*
 *    Tell if the histograms are empty     
 *    return	 bool (true if empty)
 */
bool slHistogram3ch::isEmpty (void) const
{
	return isEmpty_;
}


/*
 *    Used to add too histograms3ch
 *    param	 right (slHistogram3ch&) 
 */
slHistogram3ch& slHistogram3ch::operator += (const slHistogram3ch& right)
{
	if (!right.isEmpty())
	{
		for (size_t i = 0; i < ch1_.size(); ++i)
			ch1_[i] += right.ch1_[i];
		for (size_t i = 0; i < ch2_.size(); ++i)
			ch2_[i] += right.ch2_[i];
		for (size_t i = 0; i < ch3_.size(); ++i)
			ch3_[i] += right.ch3_[i];

		isEmpty_ = false;
	}
	return (*this);
}


/*
 *    Used to add a single pixel to the histogram
 *    param	 right (slPixel3ch&) 
 */
slHistogram3ch& slHistogram3ch::operator += (const slPixel3ch& right)
{
	//cout << "pixel h " << (int)right.h()/(180/RANGE1) <<" s " << (int)right.s()/(180/RANGE1) << " v " << (int)right.v()/(180/RANGE1)<<endl;
	if(HSV_)
	{
		++ch1_[right[0]/(180/RANGE1)];
		++ch2_[right[1]/(256/RANGE2)];
		++ch3_[right[2]/(256/RANGE3)];
	}
	else
	{
		++ch1_[right[0]/(256/RANGE1)];
		++ch2_[right[1]/(256/RANGE2)];
		++ch3_[right[2]/(256/RANGE3)];
	}

	isEmpty_ = false;
	return (*this);
}


/*
 *    Compare two histograms 3ch and return the distance between each histogram (3 distance)
 *    param	 right (slHistogram3ch&)       
 *    return	 vector<float> (contain 3 value one for each histogram)
 */
vector<float> slHistogram3ch::compare (const slHistogram3ch& right) const
{
	float distance1 = 0;
	float distance2 = 0;
	float distance3 = 0;
	vector<float> temp;

	if (!MPDA_)
	{
		for (size_t i = 0; i < ch1_.size(); ++i)
			distance1 += (fabs(ch1_[i] - right.ch1_[i])/(1 + ch1_[i] +right.ch1_[i]));

		for (size_t i = 0; i < ch2_.size(); ++i)
			distance2 += (fabs(ch2_[i] - right.ch2_[i])/(1 + ch2_[i] +right.ch2_[i]));

		for (size_t i = 0; i < ch3_.size(); ++i)
			distance3 += (fabs(ch3_[i] - right.ch3_[i])/(1 + ch3_[i] +right.ch3_[i]));
	}
	else
	{
		for (size_t i = 0; i < ch1_.size(); ++i)
		{
			float dint = 0;
			for (size_t j = 0; j <= i; ++j)
				dint = dint + right.ch1_[j] - ch1_[j];
			distance1 += fabs(dint);
		}
		distance1 /= ch1_.size();

		for (size_t i = 0; i < ch2_.size(); ++i)
		{
			float dint = 0;
			for (size_t j = 0; j <= i; ++j)
				dint = dint + right.ch2_[j] - ch2_[j];
			distance2 += fabs(dint);
		}
		distance2 /= ch2_.size();

		for (size_t i = 0; i < ch3_.size(); ++i)
		{
			float dint = 0;
			for (size_t j = 0; j <= i; ++j)
				dint = dint + right.ch3_[j] - ch3_[j];
			distance3 += fabs(dint);
		}
		distance3 /= ch3_.size();
	}
	temp.push_back(distance1);
	temp.push_back(distance2);
	temp.push_back(distance3);

	return temp;
}


