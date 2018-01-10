// Adaptive background mixture models for real-time tracking
// Chris Stauffer, W.E.L Grimson
// http://www.ai.mit.edu/projects/vsam/Publications/stauffer_cvpr98_track.pdf

#include "slSphericalGaussian.h"

#define _USE_MATH_DEFINES
#include <math.h>


using namespace cv;


slSphericalGaussian::slSphericalGaussian(const cv::Vec3f &mean, float variance)
{
	setMean(mean);
	setVariance(variance);
}


void slSphericalGaussian::setMean(const cv::Vec3f &mean)
{
	mean_ = mean;
}


const cv::Vec3f& slSphericalGaussian::getMean() const
{
	return mean_;
}


void slSphericalGaussian::setVariance(float variance)
{
	variance_ = variance;
}


float slSphericalGaussian::getVariance() const
{
	return variance_;
}


float slSphericalGaussian::prob(const cv::Vec3f &X_t) const
{
	// (X_t - Mu_t)
	Vec3f xt_mu = X_t - mean_;

	// -1/2 * (X_t - Mu_t)' * Covar^-1 * (X_t - Mu_t)
	// where Covar^-1 = (variance * I(3))^-1 =  I(3) / variance
	// -1/2 * (X_t - Mu_t)'(X_t - Mu_t) / variance
	double exponent = -0.5 * (xt_mu.t() * xt_mu).val[0] / variance_;

	// (2*Pi)^(3/2) * |Covar|^1/2
	// where |Covar| = variance * variance * variance = variance ^ 3
	// (2*Pi)^(3/2) * (variance^3)^1/2 = (2*Pi*variance)^(3/2)
	double den = pow(2 * M_PI * variance_, 1.5);

	return (float)(exp(exponent) / den);
}


void slSphericalGaussian::insertInlier(const cv::Vec3f &X_t, float alpha)
{
	// X_t - Mu_{t-1}
	Vec3f xt_mu = X_t - mean_;

	// rho = alpha * prob(X_t) / prob(Mu_{t-1})
	// rho = alpha * e^(-1/2 * (X_t - Mu_{t-1})' * Covar^-1 * (X_t - Mu_{t-1}))
	// where Covar^-1 = (variance * I(3))^-1 =  I(3) / variance
	// rho = alpha * e^(-1/2 * (X_t - Mu_{t-1})'(X_t - Mu_{t-1}) / variance)
	float rho = alpha * (float)exp(-0.5 * (xt_mu.t() * xt_mu).val[0] / variance_);

	// Update Mu_t
	// Mu_t = (1 - rho) * Mu_{t-1} + rho * X_t
	// mean_ = (1.0f - rho) * mean_ + rho * X_t
	// mean_ = mean_ - rho * mean_ + rho * X_t
	mean_ += rho * (X_t - mean_);

	// X_t - Mu_t
	xt_mu = X_t - mean_;

	// Update variance
	// variance_t = (1 - rho) * variance_t-1 + rho * (X_t - Mu_t)'(X_t - Mu_t)
	// variance_ = (1.0f - rho) * variance_ + rho * (xt_mu.t() * xt_mu).val[0];
	// variance_ = variance_ - rho * variance_ + rho * (xt_mu.t() * xt_mu).val[0];
	variance_ += rho * ((xt_mu.t() * xt_mu).val[0] - variance_);
}


bool slSphericalGaussian::testInlier(const cv::Vec3f &X_t, float nbStdDev) const
{
	// X_t - Mu_t
	Vec3f xt_mu = X_t - mean_;

	// ||X_t - Mu_t|| < nbStdDev * StdDev
	// (X_t - Mu_t)'(X_t - Mu_t) < nbStdDev^2 * variance
	return ((xt_mu.t() * xt_mu).val[0]) < (nbStdDev * nbStdDev * variance_);
}


