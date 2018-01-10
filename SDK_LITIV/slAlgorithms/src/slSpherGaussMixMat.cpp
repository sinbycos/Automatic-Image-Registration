// Adaptive background mixture models for real-time tracking
// Chris Stauffer, W.E.L Grimson
// http://www.ai.mit.edu/projects/vsam/Publications/stauffer_cvpr98_track.pdf

#include "slSpherGaussMixMat.h"

#include <iostream>


using namespace cv;
using namespace std;
using namespace slAH;


#define ARG_K			"-K"
#define ARG_DEF_VAR		"-dv"
#define ARG_DIST_WIDTH	"-dw"
#define ARG_ALPHA		"-A"
#define ARG_T			"-T"


void slSpherGaussMixMat::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap
		<< (slParamSpec(ARG_K,		"K: Nb of distributions")	<< slSyntax("3..12",	"3"))
		<< (slParamSpec(ARG_DEF_VAR,	"Default variance")		<< slSyntax("eps..inf",	"1.0"))
		<< (slParamSpec(ARG_DIST_WIDTH,	"Distribution width")	<< slSyntax("0.0..5.0",	"2.5"))
		<< (slParamSpec(ARG_ALPHA,	"Alpha: learning rate")		<< slSyntax("0.0..1.0",	"0.05"))
		<< (slParamSpec(ARG_T,		"T: Proportion for the Bg")	<< slSyntax("0.0..1.0",	"0.5"))
		;
}


slSpherGaussMixMat::slSpherGaussMixMat()
{
	setK(3);
	setDefVariance(1.0f);
	setDistWidth(2.5f);
	setAlpha(0.05f);
	setT(0.5f);
}


void slSpherGaussMixMat::setK(size_t K)
{
	K_ = K;
}


void slSpherGaussMixMat::setDefVariance(float defVar)
{
	defaultVariance_ = defVar;
}


void slSpherGaussMixMat::setDistWidth(float nbStdDev)
{
	distWidth_ = nbStdDev;
}


void slSpherGaussMixMat::setAlpha(float alpha)
{
	alpha_ = alpha;
}


void slSpherGaussMixMat::setT(float T)
{
	T_ = T;
}


void slSpherGaussMixMat::setParameters(const slAH::slParameters& parameters)
{
	// Number of distributions per pixel
	setK(atoi(parameters.getValue(ARG_K).c_str()));

	// Default variance
	setDefVariance((float)atof(parameters.getValue(ARG_DEF_VAR).c_str()));

	// Distribution width
	setDistWidth((float)atof(parameters.getValue(ARG_DIST_WIDTH).c_str()));

	// Alpha
	setAlpha((float)atof(parameters.getValue(ARG_ALPHA).c_str()));

	// T threshold
	setT((float)atof(parameters.getValue(ARG_T).c_str()));
}


void slSpherGaussMixMat::showParameters() const
{
	cout << "--- slSpherGaussMixMat ---" << endl;

	cout << "K (mixture size) :              " << K_ << endl;
	cout << "Default variance :              " << defaultVariance_ << endl;
	cout << "Distribution width (nbStdDev) : " << distWidth_ << endl;
	cout << "Alpha (learning rate) :         " << alpha_ << endl;
	cout << "T (B gaussians <= K) :          " << T_ << endl;
}


void slSpherGaussMixMat::reset(size_t nbMixtures)
{
	gaussMixtures_.resize(nbMixtures);

	for (vector<slSpherGaussMix>::iterator it = gaussMixtures_.begin();
		it != gaussMixtures_.end(); it++)
	{
		it->reset(K_);
	}
}


bool slSpherGaussMixMat::update(size_t index, const cv::Vec3f &X_t)
{
	return gaussMixtures_[index].update(X_t, defaultVariance_, distWidth_, alpha_, T_);
}


size_t slSpherGaussMixMat::getMixtureSize(size_t index) const
{
	return gaussMixtures_[index].getSize();
}


float slSpherGaussMixMat::getWeight(size_t index, size_t k) const
{
	return gaussMixtures_[index].getWeight(k);
}


const cv::Vec3f& slSpherGaussMixMat::getMean(size_t index, size_t k) const
{
	return gaussMixtures_[index][k].getMean();
}


float slSpherGaussMixMat::getVariance(size_t index, size_t k) const
{
	return gaussMixtures_[index][k].getVariance();
}


///////////////////////////////////////////////////////////////////////////////
//	slSpherGaussMixMat::slSpherGaussMix
///////////////////////////////////////////////////////////////////////////////


slSpherGaussMixMat::slSpherGaussMix::slSpherGaussMix(): K_(0), B_(0)
{
}


void slSpherGaussMixMat::slSpherGaussMix::reset(size_t K)
{
	// K spherical gaussians
	sphericalGaussians_.resize(K);

	// Weighted gaussian mixture
	gaussMixture_.resize(K);

	// No distribution is activated
	K_ = 0;
	B_ = 0;
}


bool slSpherGaussMixMat::slSpherGaussMix::update(const cv::Vec3f &X_t, float variance,
	float distW, float alpha, float T)
{
	size_t matchedK = 0;

	// From original paper:
	// "Every new pixel value, X_t, is checked against the existing
	// K Gaussian distributions, until a match is found."
	while ( (matchedK < K_) && !gaussMixture_[matchedK].gaussian->testInlier(X_t, distW) ) {
		matchedK++;
	}

	// Final test result
	bool isMatchedInFirstBDist = (matchedK < B_);

	// From original paper:
	// The prior weights of the K distributions at time t, w_{k,t}, are adjusted as follows:
	// w_{k,t} = (1.0f - alpha) * w_{k,t-1} + alpha * M_{k,t} (Eq. 5), where
	// M_{k,t} = "1 for the model which matched and 0 for the remaining models."
	// For every models: weight *= 1.0f - alpha
	// For matched model: weight += alpha

	// Lower all activated weights
	for (size_t k = 0; k < K_; k++) {
		gaussMixture_[k].weight *= 1.0f - alpha;
	}

	// Update special cases
	// If distribution matched
	if (matchedK < K_) {
		// Bonus weight: ... + alpha * M_{k,t}, where M_{k,t}=1 in this case
		gaussMixture_[matchedK].weight += alpha;

		// Update distribution
		gaussMixture_[matchedK].gaussian->insertInlier(X_t, alpha);

		// Sort distributions from indMatch to the best
		while (matchedK > 0 && gaussMixture_[matchedK] < gaussMixture_[matchedK - 1]) {
			swap(gaussMixture_[matchedK], gaussMixture_[matchedK - 1]);
			matchedK--;
		}
	}
	else {
		if (K_ < gaussMixture_.size()) {
			// Activate a new least significant distribution
			gaussMixture_[K_].gaussian = &sphericalGaussians_[K_];
			K_++;
		}

		// The new data needs to replace the least significant distribution
		slWeightedSpherGauss &wsg = gaussMixture_[K_ - 1];

		wsg.weight = (K_ == 1 ? 1.0f : 0.0f);
		wsg.gaussian->setMean(X_t);
		wsg.gaussian->setVariance(variance);
	}

	// Normalize weights
	float sumW = 0;
	for (size_t k = 0; k < K_; k++) sumW += gaussMixture_[k].weight;
	for (size_t k = 0; k < K_; k++) gaussMixture_[k].weight /= sumW;

	// From original paper:
	// "Then the first B distributions are chosen as the background model, where:"
	// B_ = argmin_{b}(sum_{k=1}^{b}(w_k) > T) (Eq. 9)
	// B_ = smallest b such that the sum of weights is greater than T
	// While the sum (sumW) is less than or equal to T, increment b (B_)

	sumW = 0;
	B_ = 0;

	while (sumW <= T && B_ < K_) {
		sumW += gaussMixture_[B_].weight;
		B_++;
	}

	return isMatchedInFirstBDist;
}


size_t slSpherGaussMixMat::slSpherGaussMix::getSize() const
{
	return K_;
}


float slSpherGaussMixMat::slSpherGaussMix::getWeight(size_t k) const
{
	return gaussMixture_[k].weight;
}


const slSphericalGaussian& slSpherGaussMixMat::slSpherGaussMix::operator[](size_t k) const
{
	return *gaussMixture_[k].gaussian;
}


///////////////////////////////////////////////////////////////////////////////
//	slSpherGaussMixMat::slSpherGaussMix::slWeightedSpherGauss
///////////////////////////////////////////////////////////////////////////////


bool slSpherGaussMixMat::slSpherGaussMix::slWeightedSpherGauss::operator<(
	const slWeightedSpherGauss& wsg) const
{
	// weight / stdDev > wsg.weight / wsg.stdDev
	// weight * wsg.stdDev > wsg.weight * stdDev
	// weight^2 * wsg.variance > wsg.weight^2 * variance
	return
		(weight * weight * wsg.gaussian->getVariance())
		>
		(wsg.weight * wsg.weight * gaussian->getVariance());
}


