/*
 *	  file	   slNonFgMergeFg.h
 *    
 *	  author  Michael Sills Lavoie 
 *      
 *    date    24.05.2007
 */

#ifndef _SLNONFGMERGEFG_H_
#define _SLNONFGMERGEFG_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"


class slNonFgMergeFg: public slBgSub
{
public:
	virtual ~slNonFgMergeFg();

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
	slNonFgMergeFg(const std::string &name = "");
	slNonFgMergeFg(const slParameters& parameters);

private:
	unsigned long int mNbPixel;
	unsigned long int mNbPixelCount;

	slEpsilon3ch mEpsilon;
	static slNonFgMergeFg Creator_;

};


#endif	// _SLNONFGMERGEFG_H_


