/*!	\file	slException.h
 *	\brief	Contains different exception classes
 *
 *	This file contains the definition of classes slException, slExceptionIO,
 *	slExceptionArgHandler, slExceptionBgSub and slExceptionCamIn.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		August 2008
 */

#ifndef _SLEXCEPTION_H_
#define	_SLEXCEPTION_H_


#include "slCore.h"

#include <string>


//!	Base exception class
/*!
 *	This is the base class used as an exception object containing an error
 *	message and, maybe, the name of the class from which the error was thrown.
 *	In fact, the class name may not be the actual class that raised the error.
 *	But, most of the time, all the details (class, function member) are in the message.
 *
 *	\see		slExceptionIO, slExceptionArgHandler, slExceptionBgSub, slExceptionCamIn
 *	\author		Michael Eilers-Smith, modified by Pier-Luc St-Onge
 *	\date		August 2008
 */
class SLCORE_DLL_EXPORT slException
{
public:
	slException(const char* error, const char* name = 0);	//!< throw slException(errorMessage, name);
	virtual ~slException();

	inline const char* getMessage() const { return err_; }	//!< Returns the error message
	inline const char* getName() const { return name_; }	//!< Returns the class name

protected:
	char* err_;		// 255 characters
	char* name_;	//  50 characters

};


//!	slArgHandler Exception class
/*!
 *	The class name would be "slArgHandler".
 *
 *	\see		slException
 *	\author		Michael Eilers-Smith, modified by Pier-Luc St-Onge
 *	\date		August 2008
 */
class SLCORE_DLL_EXPORT slExceptionArgHandler : public slException
{
public:
	slExceptionArgHandler(const std::string& error);	//!< Constructor

};


//!	slIO Exception class
/*!
 *	The class name would be "slIO".
 *
 *	\see		slException
 *	\author		Michael Eilers-Smith, modified by Pier-Luc St-Onge
 *	\date		August 2008
 */
class SLCORE_DLL_EXPORT slExceptionIO : public slException
{
public:
	slExceptionIO(const std::string& error);			//!< Constructor

};


//!	slCamIn Exception class
/*!
 *	The class name would be "slIO (Camera issue)".
 *
 *	\see		slException
 *	\author		Michael Eilers-Smith, modified by Pier-Luc St-Onge
 *	\date		August 2008
 */
class SLCORE_DLL_EXPORT slExceptionCamIn : public slExceptionIO
{
public:
	slExceptionCamIn(const std::string& error);			//!< Constructor

};


//!	slBgSub Exception class
/*!
 *	The class name would be "slBgSub".
 *
 *	\see		slException
 *	\author		Michael Eilers-Smith, modified by Pier-Luc St-Onge
 *	\date		August 2008
 */
class SLCORE_DLL_EXPORT slExceptionBgSub : public slException
{
public:
	slExceptionBgSub(const std::string& error);			//!< Constructor

};


#endif	// _SLEXCEPTION_H_


