/*!	\file	slCore.h
 *	\brief	This file contains some definitions like pixels and images
 *
 *	The standard SDK_LITIV pixels and images types are defined here.
 *	Example:
 *	\code
 *	slImage3ch source;
 *	slImage1ch target;
 *
 *	source.create(Size(width, height));
 *	source = HSV_WHITE;					// Fill with white HSV
 *	target = grayClone(source, SL_HSV);	// Get the Value components
 *	\endcode
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */

#ifndef SLCORE_H
#define SLCORE_H


#ifndef SLCORE_DLL_EXPORT
	#ifdef WIN32
		#define SLCORE_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLCORE_DLL_EXPORT
	#endif
#endif


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


//!	Constants for colors system identification
/*!
 *	- SL_BGR: the RGB color space is saved in this order: BGRBGR...
 *	- SL_HSV: the HSV color space
 *	- SL_GRAYSCALE: for grayscale values (uchar or float)
 *	- COLOR_SYS_COUNT: number of supported color spaces
 */
typedef enum typeColorSys {SL_BGR, SL_HSV, SL_GRAYSCALE, COLOR_SYS_COUNT};


// Names and syntax for argument handling
#define BGR_NAME "rgb"
#define HSV_NAME "hsv"
#define CSYS_SYNTAX (string(BGR_NAME) + "|" + HSV_NAME)
#define CSYS_DEFAULT BGR_NAME


typedef cv::Vec3b	slPixel3ch;				//!< Regular BGR or HSV pixel
typedef uchar		slPixel1ch;				//!< Regular grayscale pixel (0..255)
typedef float		slPixel1fl;				//!< Special grayscale pixel (0..1.0)

typedef cv::Mat              slMat;			//!< Generic OpenCV matrix
typedef cv::Mat_<slPixel3ch> slImage3ch;	//!< Regular BGR or HSV image
typedef cv::Mat_<slPixel1ch> slImage1ch;	//!< Regular grayscale image (0..255)
typedef cv::Mat_<slPixel1fl> slImage1fl;	//!< Special grayscale image (0..1.0)

#define PIXEL_1CH_BLACK ((slPixel1ch)0x00)		//!< Black pixel, 1 channel
#define PIXEL_1CH_WHITE ((slPixel1ch)0xFF)		//!< White pixel, 1 channel
#define BGR_WHITE slPixel3ch(0xff, 0xff, 0xff)	//!< White pixel, 3 channels Blue-Green-Red
#define HSV_WHITE slPixel3ch(0x00, 0x00, 0xff)	//!< White pixel, 3 channels Hue-Saturation-Value (red white)

#define slRGB(r, g, b) slPixel3ch((b), (g), (r))	//!< Convenient RGB constructor

//! Converts a 3-channels image into a grayscale image
/*!
 *	This function creates a new slImage1ch (new buffer).
 *	Then, it converts the 3-channels image into a grayscale image
 *	according to its color space.
 *	- For BGR images, it uses <tt>cv::cvtColor(..., ..., CV_BGR2GRAY)</tt>
 *	- For HSV images, it returns the V component only
 */
SLCORE_DLL_EXPORT slImage1ch grayClone(const slImage3ch &image, typeColorSys csys = SL_BGR);


#endif	// SLCORE_H


