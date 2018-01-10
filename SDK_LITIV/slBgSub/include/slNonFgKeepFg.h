/*
 *	  file	   slNonFgKeepFg.h
 *    
 *	  author  Michael Sills Lavoie 
 *      
 *    date    24.05.2007
 */

#ifndef _SLNONFGKEEPFG_H_
#define _SLNONFGKEEPFG_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"


class slNonFgKeepFg: public slBgSub
{
public:
	virtual ~slNonFgKeepFg();

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
	slNonFgKeepFg(const std::string &name = "");
	slNonFgKeepFg(const slParameters& parameters);

private:
	slEpsilon3ch mEpsilon;
	static slNonFgKeepFg Creator_;

};


#endif	// _SLNONFGKEEPFG_H_


