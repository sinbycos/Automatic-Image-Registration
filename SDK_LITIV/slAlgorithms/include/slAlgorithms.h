/*!	\file	slAlgorithms.h
 *	\brief	This file contains DLL stuff for slAlgorithms
 *
 *	\author		Pier-Luc St-Onge
 *	\date		November 2011
 */

#ifndef SLALGORITHMS_H
#define SLALGORITHMS_H


#ifndef SLALGORITHMS_DLL_EXPORT
	#ifdef WIN32
		#define SLALGORITHMS_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLALGORITHMS_DLL_EXPORT
	#endif
#endif


#endif	// SLALGORITHMS_H


