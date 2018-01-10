/*!	\file	slWindow.h
 *	\brief	Contains a class managing an OpenCV window
 *
 *	This file contains the definition of the class slWindow.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		14/06/2010
 */

#ifndef SLWINDOW_H
#define SLWINDOW_H


#include "slCore.h"


//!	This is the class used for showing slImage objects
/*!
 *	As soon as a slWindow is created with a provided text, this text is
 *	formatted in order to create a unique window name (caption text).
 *	In fact, if several slWindow instances are created with the same input
 *	text, the resulting caption text will be "text (ID)".
 *	So, the window is created with the appropriate OpenCV function and with
 *	the caption text.
 *	Finally, the window will be destroyed when the corresponding slWindow
 *	will be destroyed too.
 *
 *	To show an image, simply call function show().
 *
 *	Mouse events could be managed by creating a class that derivates from slWindow.
 *	By reimplementing mouseEvent(), it is then possible to catch all mouse
 *	events detected by OpenCV.  Example:
 *	\code
 *	class ClickMe: public slWindow
 *	{
 *	public:
 *		ClickMe(const char *text): slWindow(text), image_(imread("test.png"))
 *		{
 *			show(image_);		// Show image test.png
 *		}
 *	
 *	protected:
 *		virtual void mouseEvent(int cvEvent, int x, int y, int eventFlags)
 *		{
 *			if (cvEvent == CV_EVENT_LBUTTONUP) {		// Left click
 *				// Print orange circle at mouse position
 *				circle(image_, cvPoint(x, y), 16, CV_RGB(255, 128, 0), -1);
 *				show(image_);	// Show modified image
 *			}
 *		}
 *	
 *	private:
 *		slImage3ch image_;
 *	
 *	};
 *	\endcode
 *
 *	\see		slImage3ch, slImage1ch
 *	\author		Pier-Luc St-Onge
 *	\date		15/04/2011
 */
class SLCORE_DLL_EXPORT slWindow
{
public:
	slWindow(const char *text);	//!< Constructor with the unformatted caption text
	slWindow(const slWindow& c);

	virtual ~slWindow();		//!< Virtual destructor

	slWindow& operator=(const slWindow& c);

	void show(const slMat& image, typeColorSys csys = SL_BGR);	//!< Shows an image, manual call

protected:
	//! Receives the mouse events and sends a signal if needed
	virtual void mouseEvent(int cvEvent, int x, int y, int eventFlags);

	static void mouseCallback(int cvEvent, int x, int y, int eventFlags, void* window);

private:
	slWindow();

	std::string generateCaption(const std::string& text) const;

private:
	std::string text_;
	std::string caption_;

	static std::map<std::string, unsigned int> *indexes_;

};


#endif	// SLWINDOW_H


