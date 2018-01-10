#include <math.h>
#include <sstream>

#include "slQuantParams.h"


using namespace cv;
using namespace std;
using namespace slAH;


////////////////
// slQuant1ch //
////////////////


slQuant1ch::slQuant1ch(int nbLevels, int limitValue)
{
	setParams(nbLevels, limitValue);
}


void slQuant1ch::setParams(int nbLevels, int limitValue)
{
	limitValue_ = limitValue;
	nbLevels_ = nbLevels;

	// Q_MIN_LEVELS <= limitValue_ <= Q_CHAN_LIMIT
	if (limitValue_ < Q_MIN_LEVELS) limitValue_ = Q_MIN_LEVELS;
	if (limitValue_ > Q_CHAN_LIMIT) limitValue_ = Q_CHAN_LIMIT;

	// Q_MIN_LEVELS <= nbLevels_ <= limitValue_
	if (nbLevels_ < Q_MIN_LEVELS) nbLevels_ = Q_MIN_LEVELS;
	if (nbLevels_ > limitValue_) nbLevels_ = limitValue_;

	double step = (double)limitValue_ / nbLevels_;
	slPixel1ch offset = (slPixel1ch)(int)floor(step / 2);

	// Compute all values of lut_
	for (int ind = 0; ind < Q_CHAN_LIMIT; ind++) {
		lut_[ind] = (slPixel1ch)(int)( step * floor((ind % limitValue_) / step) ) + offset;
	}
}


void slQuant1ch::quantify(const slImage1ch &source, slImage1ch &target) const
{
	if (source.empty() || target.empty()) return;

	if (source.rows != target.rows || source.cols != target.cols) {
		target.create(source.size());
	}

	for (int i = 0; i < source.rows; i++) {
		const slPixel1ch *src = source[i];
		slPixel1ch *tgt = target[i];

		for (int j = 0; j < source.cols; j++) {
			tgt[j] = lut_[src[j]];
		}
	}
}


////////////////
// slQuant3ch //
////////////////


slQuant3ch::slQuant3ch(const slQuant1ch &q0, const slQuant1ch &q1, const slQuant1ch &q2):
q0_(q0),
q1_(q1),
q2_(q2)
{
}


void slQuant3ch::setParams(const slQuant1ch &q0, const slQuant1ch &q1, const slQuant1ch &q2)
{
	q0_ = q0;
	q1_ = q1;
	q2_ = q2;
}


void slQuant3ch::quantify(const slImage3ch &source, slImage3ch &target) const
{
	if (source.empty() || target.empty()) return;

	if (source.rows != target.rows || source.cols != target.cols) {
		target.create(source.size());
	}

	const slPixel1ch *q0 = q0_.getLUT();
	const slPixel1ch *q1 = q1_.getLUT();
	const slPixel1ch *q2 = q2_.getLUT();

	for (int i = 0; i < source.rows; i++) {
		const slPixel3ch *src = source[i];
		slPixel3ch *tgt = target[i];

		for (int j = 0; j < source.cols; j++) {
			tgt[j] = slPixel3ch( q0[src[j].val[0]], q1[src[j].val[1]], q2[src[j].val[2]] );
		}
	}
}


slPixel3ch slQuant3ch::operator[](const slPixel3ch &pixel) const
{
	return slPixel3ch(q0_[pixel.val[0]], q1_[pixel.val[1]], q2_[pixel.val[2]]);
}


string slQuant3ch::getStr(typeColorSys csys) const
{
	ostringstream ostr;

	ostr << q0_.getNbLevels() << " " << q1_.getNbLevels() << " " << q2_.getNbLevels();

	return ostr.str();
}


slParamSpec& slQuant3ch::fillSyntax(slParamSpec& paramSpec)
{
	{
		ostringstream ostr;
		ostr << Q_MIN_LEVELS << ".." << "(" << Q_CHAN_LIMIT << "|" << Q_H_LIMIT << ")";
		paramSpec << slSyntax(ostr.str());
	}

	{
		ostringstream ostr;
		ostr << Q_MIN_LEVELS << ".." << Q_CHAN_LIMIT;
		paramSpec << slSyntax(ostr.str());
	}

	{
		ostringstream ostr;
		ostr << Q_MIN_LEVELS << ".." << Q_CHAN_LIMIT;
		paramSpec << slSyntax(ostr.str());
	}

	return paramSpec;
}


