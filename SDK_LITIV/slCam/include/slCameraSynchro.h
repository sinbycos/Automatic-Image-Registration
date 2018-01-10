/*!	\file	slCameraSynchro.h
 *	\brief	This file contains classes slCameraSynchro,
 *			slSynchroSem and slSynchroSemPtr
 *
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */

#ifndef SLCAMERASYNCHRO_H
#define SLCAMERASYNCHRO_H


#ifndef SLSYNCHRO_DLL_EXPORT
	#ifdef WIN32
		#define SLSYNCHRO_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLSYNCHRO_DLL_EXPORT
	#endif
#endif


#include <set>
#include <time.h>
#include <WinBase.h>


//!	This is a dual-semaphore (go, wait) for camera synchronisation
/*!
 *	While capturing with threads, the main loop must be able to unlock
 *	all capturing threads and, then, wait for all threads to complete
 *	their capture of one image.
 *	One instance of slSynchroSem is already a member of the slCamIn class.
 *
 *	Here is how it works:
 *	- slCameraSynchro::goAndWait() fixes a common base time for all slSynchroSem objects.
 *	- This base time is the parameter of go(), which will update the internal base time
 *	of the current slSynchroSem instance.  The semaphore "go" is then unlocked.
 *	- The function waitForGo() waits for the "go" semaphore.
 *	Finally, waitForGo() will loop as long as the current time is less than
 *	the sum of the base time and the private delay.
 *
 *	The private delay is almost always updated by using function reset(), which
 *	also resets both internal semaphores.
 *
 *	\see		slCamIn, slCameraSynchro, slSynchroSemPtr
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */
class SLSYNCHRO_DLL_EXPORT slSynchroSem
{
public:
	slSynchroSem(clock_t delay = 0);	//!< Constructor which initiates both semaphores
	~slSynchroSem();

	void reset(clock_t delay);			//!< Closes both semaphores, and recreate them both

	void go(clock_t baseTime = 0);		//!< Unlocks "go" semaphore
	void done();						//!< Unlocks "done" semaphore

	void waitForGo();		//!< Waits for "go" semaphore and applies the delay (loop for a few milliseconds)
	void waitForDone();		//!< Waits for "done" semaphore

	bool loop() const;		//!< Returns true if not stopping
	void stopLoop();		//!< Modifies "stop" boolean and unlocks "go" semaphore

	bool operator<(const slSynchroSem &ss) const;	//!< Less-than operator for sorting purposes

protected:
	slSynchroSem operator=(const slSynchroSem &ss);

private:
	void init(clock_t delay);
	void clear();

private:
	int id_;			// Make sure each instance is not equal (< or >) to another instance

	clock_t baseTime_;	// Common basetime for each synchronized capture
	clock_t delay_;		// Private delay

	HANDLE go_;			// "go" semaphore
	HANDLE done_;		// "done" semaphore

	bool stop_;			// True if must stop loop

	static int count_;	// Count for a unique id
};


//!	This is an automatic pointer for slSynchroSem instances
/*!
 *	It implements the arrow operator.
 *	It also implements the less-than operator, since slSynchroSemPtr objects
 *	are sorted in a slCameraSynchro set.
 *
 *	\see		slSynchroSem, slCameraSynchro, slCamIn
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */
class SLSYNCHRO_DLL_EXPORT slSynchroSemPtr
{
public:
	slSynchroSemPtr(slSynchroSem *ref = NULL);

	slSynchroSem* operator->() const;	//!< Arrow operator, like a regular pointer

	bool operator<(const slSynchroSemPtr &ssptr) const;		//!< For sorting purposes

private:
	slSynchroSem *ref_;

};


//!	This is a set of dual-semaphores for camera synchronisation
/*!
 *	<tt>class slCameraSynchro: public std::set<slSynchroSemPtr> {...};</tt>
 *
 *	The current class only stores slSynchroSemPtr objects, which are
 *	automatic pointers to slSynchroSem instances:
 *	\code
 *	slCameraSynchro synchro;
 *
 *	synchro.insert(cam1->synchro());
 *	synchro.insert(cam2->synchro());
 *	\endcode
 *
 *	The real instances of slSynchroSem are owned by the slCamIn objects.
 *	So, each camera is controlled by its own dual-semaphore.
 *	Furthermore, each camera could be started with an individual delay.
 *
 *	The role of slCameraSynchro is to unlock all threads, and
 *	wait for all captures to be done:
 *	\code
 *	while (capture) {
 *		// This will trigger both captures at the same time according to the individual delay
 *		// This function returns when both captures are done
 *		synchro.goAndWait();
 *	}
 *	\endcode
 *
 *	\see		slCamIn for an example, slSynchroSemPtr, slSynchroSem
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */
class SLSYNCHRO_DLL_EXPORT slCameraSynchro: public std::set<slSynchroSemPtr>
{
public:
	void goAndWait();	//!< Releases all "go" semaphores and waits for all "wait" semaphores

};


#endif	// SLCAMERASYNCHRO_H


