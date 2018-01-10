/*
 *	  @file	   slRectMedian.h
 *	  Remake of rafik background subtractor updated with
 *	  the median bgs
 *    
 *	  @author  Michael Sills Lavoie
 *	  @author (Median) ???
 *      
 *    @date    30.05.2007
 */

#ifndef _SLRECTMEDIAN_H_
#define _SLRECTMEDIAN_H_

#include "slRectPixels.h"
#include "slArgHandler.h"
#include "slHistogram3ch.h"
#include "slPixel3ch.h"
#include "slBgSub.h"
#include "slMedianContainer.h"
#include <vector>


using namespace std;

/*
*	slRectSimple based on rafik bg subtractor.
*	@author Michael Sills Lavoie
*/
class slRectMedian: public slBgSub
{
public:
	virtual ~slRectMedian();

protected:
	virtual void showSpecificParameters() const;
	virtual void fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const;

	virtual slBgSub* createSpecificInstance(const slParameters& parameters) const;

	virtual int specificInit();
	virtual void computeFrame();
	virtual void setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
		slPixel1ch *b_fg_data, const int& w, const int& i, const int& j);
	virtual int specificLoadNextFrame();

private:
	slRectMedian(const string &name);
	slRectMedian(const slParameters& parameters);

private:

	/*
	*	This class contain the midian pixel.
	*	@author ???
	*/
	class slMedianPixel3ch
	{
	public:
		void resize(int size)
		{
			mB.resize(size);
			mG.resize(size);
			mR.resize(size);
		}

		const slMedianPixel3ch& operator+=(const slPixel3ch &pixel)
		{
			mB += pixel.b();
			mG += pixel.g();
			mR += pixel.r();

			return *this;
		}

		slPixel3ch getMeanPixel() const
		{
			return slPixel3ch(	(BGRComp)mB.getMedian(),
								(BGRComp)mG.getMedian(),
								(BGRComp)mR.getMedian());
		}

	private:
		typedef slMedianContainer<slPixel1ch> slMedianPix;

	private:
		slMedianPix mB;
		slMedianPix mG;
		slMedianPix mR;
	};

private:
	/**
	*	Container for the statistic of each level
	*/
	vector<slLevelStat> statistic_;

	/**
	*	Container of the level of slRectPixels for the current frame
	*	Each level contain all the slRectPixels of this level
	*/
	vector<vector<slRectPixels*>> level_;

	/**
	*	Th for the upper level slRectPixels when we compare their histograms
	*/
	float th_;
	/**
	*	The value that we add to the th_ for each lower level
	*/
	float deltath_;
	/**
	*	Minimal number of slRectPixels that are at the upper level
	*/
	int minRect_;
	/**
	*	Bool to know the color space
	*/
	bool isHSV_;

	/**
	*	Bool to if we use the MDPA distance for the histograms
	*/
	bool MPDA_;

	void createLevel (vector<vector<slRectPixels*>> &level);

	int mMValue;

	/**
	*	Treshold for the texture to know if a rectangle is textured or not
	*/
	float Tth_;

	/**
	*	The value that we add to the Tth_ for each lower level
	*/
	float Tdeltath_;

	/**
	*	Bool to know if we use texture
	*/
	bool useTexture_;

	// Medians
	vector<slMedianPixel3ch> mMedians;

private:
	static slRectMedian Creator_;

};


#endif	// _SLRECTMEDIAN_H_


