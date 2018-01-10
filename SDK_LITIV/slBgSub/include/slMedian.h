#ifndef _SLMEDIAN_H_
#define _SLMEDIAN_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"
#include "slMedianContainer.h"


class slMedian: public slBgSub
{
public:
	virtual ~slMedian();

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
	slMedian(const string &name);
	slMedian(const slParameters& parameters);

private:

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
	// Parameters
	int mMValue;
	slEpsilon3ch mEpsilon;

	// Medians
	vector<slMedianPixel3ch> mMedians;

private:
	static slMedian Creator_;

};


#endif	// _SLMEDIAN_H_


