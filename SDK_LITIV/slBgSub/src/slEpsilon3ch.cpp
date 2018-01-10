#include "slEpsilon3ch.h"


using namespace std;


slEpsilon3ch::slEpsilon3ch(slPixel1ch eps0, slPixel1ch eps1, slPixel1ch eps2)
{
	setEpsilon(eps0, eps1, eps2);
}


void slEpsilon3ch::setEpsilon(slPixel1ch eps0, slPixel1ch eps1, slPixel1ch eps2)
{
	eps0_ = eps0;
	eps1_ = eps1;
	eps2_ = eps2;
}


bool slEpsilon3ch::pixIsBackground(const slPixel3ch &newPix, const slPixel3ch &bgPix,
								 typeColorSys csys, bool considerLightChanges) const
{
	bool disableVtest = (csys == SL_HSV && considerLightChanges == false);

	return	(		abs(newPix.val[0] - bgPix.val[0]) <= eps0_ &&
					abs(newPix.val[1] - bgPix.val[1]) <= eps1_ &&
				(	abs(newPix.val[2] - bgPix.val[2]) <= eps2_ || disableVtest	)
			);
}


ostream& operator<<(ostream &ostr, const slEpsilon3ch &eps)
{
	ostr << (int)eps.eps0_ << " " << (int)eps.eps1_ << " " << (int)eps.eps2_;

	return ostr;
}


