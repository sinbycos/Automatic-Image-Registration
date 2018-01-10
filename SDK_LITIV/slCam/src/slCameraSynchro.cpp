#include "stdafx.h"
#include "slCameraSynchro.h"


//////////////////
// slSynchroSem //
//////////////////


int slSynchroSem::count_ = 0;


slSynchroSem::slSynchroSem(clock_t delay): id_(count_++), baseTime_(clock())
{
	init(delay);
}


void slSynchroSem::reset(clock_t delay)
{
	clear();
	init(delay);
}


slSynchroSem::~slSynchroSem()
{
	clear();
}


void slSynchroSem::init(clock_t delay)
{
	delay_ = delay;
	go_ = CreateSemaphore(NULL, 0, 1, NULL);
	done_ = CreateSemaphore(NULL, 0, 1, NULL);
	stop_ = false;
}


void slSynchroSem::clear()
{
	CloseHandle(go_);
	CloseHandle(done_);
}


void slSynchroSem::go(clock_t baseTime)
{
	baseTime_ = baseTime;
	ReleaseSemaphore(go_, 1, NULL);
}


void slSynchroSem::done()
{
	ReleaseSemaphore(done_, 1, NULL);
}


void slSynchroSem::waitForGo()
{
	WaitForSingleObject(go_, INFINITE);

	while (clock() < baseTime_ + delay_) {
#if 1
		Sleep(0);
#endif
	}
}


void slSynchroSem::waitForDone()
{
	WaitForSingleObject(done_, INFINITE);
}


bool slSynchroSem::loop() const
{
	return !stop_;
}


void slSynchroSem::stopLoop()
{
	stop_ = true;
	go();
}


bool slSynchroSem::operator<(const slSynchroSem &ss) const
{
	return (delay_ < ss.delay_) || (delay_ == ss.delay_) && (id_ < ss.id_);
}


/////////////////////
// slSynchroSemPtr //
/////////////////////


slSynchroSemPtr::slSynchroSemPtr(slSynchroSem *ref): ref_(ref)
{
}


slSynchroSem* slSynchroSemPtr::operator->() const
{
	return ref_;
}


bool slSynchroSemPtr::operator<(const slSynchroSemPtr &ssptr) const
{
	return (*ref_ < *ssptr.ref_);
}


/////////////////////
// slCameraSynchro //
/////////////////////


void slCameraSynchro::goAndWait()
{
	clock_t baseTime = clock();

	for (iterator it = begin(); it != end(); it++) {
		(*it)->go(baseTime);
	}

	for (iterator it = begin(); it != end(); it++) {
		(*it)->waitForDone();
	}
}


