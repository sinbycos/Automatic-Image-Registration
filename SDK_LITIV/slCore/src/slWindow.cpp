/*!	\file	slWindow.cpp
 *	\brief	Contains a class managing an OpenCV window
 *
 *	This file contains the implementation of the class slWindow.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		14/06/2010
 */

#include <iostream>
#include <sstream>

#include "slWindow.h"


using namespace cv;
using namespace std;


map<string, unsigned int> *slWindow::indexes_ = NULL;


/*!
 *	This function calls \c cv::namedWindow() to create the window.
 *	If another window has already the same input text, a unique id
 *	is appended to \c text: text (id).
 *
 *	Finally, all mouse events are set to be received.
 *
 *	\param	text	is the title of the window.
 *		Examples: Output, Foreground, etc.
 */
slWindow::slWindow(const char *text)
: text_(text), caption_(generateCaption(text_))
{
	namedWindow(caption_);
	setMouseCallback(caption_, mouseCallback, this);
}


slWindow::slWindow(const slWindow& c)
: text_(c.text_), caption_(generateCaption(text_))
{
	namedWindow(caption_);
	setMouseCallback(caption_, mouseCallback, this);
}


slWindow::~slWindow()
{
	destroyWindow(caption_);
}


slWindow& slWindow::operator=(const slWindow& c)
{
	if (this != &c) {
		destroyWindow(caption_);
		text_ = c.text_;
		caption_ = generateCaption(text_);
		namedWindow(caption_);
	}

	return *this;
}


void slWindow::show(const slMat& image, typeColorSys csys)
{
	if (!image.empty()) {
		if (csys != SL_HSV) {
			imshow(caption_, image);
		}
		else {
			slMat tmp;

			cvtColor(image, tmp, CV_HSV2BGR);
			imshow(caption_, tmp);
		}
	}
}


void slWindow::mouseCallback(int cvEvent, int x, int y, int eventFlags, void* window)
{
	if (window != NULL) {
		((slWindow*)window)->mouseEvent(cvEvent, x, y, eventFlags);
	}
}


void slWindow::mouseEvent(int cvEvent, int x, int y, int eventFlags)
{
	// Do nothing by default
}


string slWindow::generateCaption(const std::string& text) const
{
	if (indexes_ == NULL) {
		indexes_ = new map<string, unsigned int>;
	}

	if (indexes_->find(text) == indexes_->end()) {
		(*indexes_)[text] = 0;
	}

	unsigned int id = ++((*indexes_)[text]);

	ostringstream oss;

	oss << text;

	if (id > 1) {
		oss << " (" << id << ")";
	}

	return oss.str();
}


