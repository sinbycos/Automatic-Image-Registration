#ifndef _SLOPENCV_H_
#define _SLOPENCV_H_


#include "slBgSub.h"


struct CvBGStatModel;


class slOpenCv: public slBgSub
{
public:
	virtual ~slOpenCv();

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
	slOpenCv(const std::string &name = "");
	slOpenCv(const slParameters& parameters);

private:
	CvBGStatModel *mStatModel;

private:
	static slOpenCv Creator_;

};


#endif	// _SLOPENCV_H_


