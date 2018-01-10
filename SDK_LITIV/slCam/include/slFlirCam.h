#ifndef FLIRCAMERA_H
#define FLIRCAMERA_H


#include "slCamIn.h"


class CFlirProperties;
class CWnd;


class SLCAMIN_DLL_EXPORT slFlirCam: public slCamIn
{
public:
	slFlirCam(CWnd *parentWnd = NULL);
	virtual ~slFlirCam();

	slFlirCam& open(const std::string &name = "");
	void close();

	bool inColors() const;

	bool hasParameters() const;
	bool hasDynamicParameters() const;
	void showParameters();
	void hideParameters();

	void read(slMat &image);
	cv::Scalar getPixel(const cv::Point &pos) const;

protected:
	void startAfter();

private:
	CFlirProperties* dialogFlir_;	//!< Pointeur sur une boite de dialogue de la caméra FLIR.

#pragma warning(disable: 4251)
	slMat imageHeader_;				//!< Header pour l'image temporaire de la caméra FLIR
#pragma warning(default: 4251)

	CWnd *refParentWnd_;

	static unsigned int nbFlir_;
};


#endif	// FLIRCAMERA_H


