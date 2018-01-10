#ifndef _SLEPSILON3CH_H_
#define _SLEPSILON3CH_H_


#include "slQuantParams.h"


//Treshold structure
class SLCORE_DLL_EXPORT slEpsilon3ch
{
public:
	slEpsilon3ch(slPixel1ch eps0 = 0, slPixel1ch eps1 = 0, slPixel1ch eps2 = 0);

	void setEpsilon(slPixel1ch eps0, slPixel1ch eps1, slPixel1ch eps2);

	bool pixIsBackground(const slPixel3ch &newPix, const slPixel3ch &bgPix,
		typeColorSys csys = SL_BGR, bool considerLightChanges = true) const;

	SLCORE_DLL_EXPORT friend std::ostream& operator<<(std::ostream &ostr, const slEpsilon3ch &eps);

private:
	slPixel1ch eps0_;
	slPixel1ch eps1_;
	slPixel1ch eps2_;

};


#endif	// _SLEPSILON3CH_H_


