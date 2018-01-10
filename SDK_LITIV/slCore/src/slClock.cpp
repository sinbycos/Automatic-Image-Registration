#include "slClock.h"


slClock::slClock(double period)
{
	setPeriod(period);
	start();
}


slClock::~slClock()
{
}


void slClock::setFPS(double fps)
{
	setPeriod(1.0 / fps);
}


void slClock::setPeriod(double period)
{
	periodInClocks_ = period * CLOCKS_PER_SEC;
}


void slClock::start()
{
	stop();
	nbFrames_ = 0;

	startTime_ = clock();
}


clock_t slClock::nextDelay()
{
	stop();
	nbFrames_++;

	clock_t delay = (clock_t)floor(periodInClocks_ * nbFrames_) - elapsed();
	return (delay > 0 ? delay : 1);
}


void slClock::stop()
{
	stopTime_ = clock();
}


clock_t slClock::elapsed() const
{
	return (stopTime_ - startTime_ + 1);
}


unsigned int slClock::nbFrames() const
{
	return nbFrames_;
}


double slClock::getMeasuredFPS() const
{
	double nb = (nbFrames_ > 0 ? nbFrames_ : 1);
	clock_t temps = elapsed();
	temps = (temps > 0 ? temps : 1);

	return nb * CLOCKS_PER_SEC / temps;
}


