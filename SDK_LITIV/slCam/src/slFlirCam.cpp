#include "stdafx.h"
#include "../slCam.h"
#include "slFlirCam.h"
#include "../FlirProperties.h"	      // Boite de dialogue pour la flir (grosse IR).


using namespace cv;


unsigned int slFlirCam::nbFlir_ = 0;


slFlirCam::slFlirCam(CWnd *parentWnd)
: refParentWnd_(parentWnd)
{
	// Création de la fenêtre de la caméra, celle-ci n'est pas encore montrée à l'utilisateur
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	dialogFlir_ = new CFlirProperties(refParentWnd_);
	dialogFlir_->Create(IDD_FLIR_PROP, refParentWnd_);

	imageHeader_ = NULL;
}


slFlirCam::~slFlirCam()
{
	stop();
	hideParameters();

	if (connected_) {
		// Ne pas appeler dialogFlir_->Disconnect(),
		// car ça plante si on ferme l'application en même temps
		nbFlir_--;
		connected_ = false;
	}

	dialogFlir_->DestroyWindow();
	delete dialogFlir_;
}


slFlirCam& slFlirCam::open(const std::string &name)
{
	name;

	// Nettoyer la connexion
	close();

	if (nbFlir_ > 0) {
		throw slExceptionCamIn("caméra Flir déjà connectée");
	}

	// Connexion
	connected_ = (dialogFlir_->flir_.Connect(5, 0, 5, 3, NULL) == 0);

	if (!connected_) {
		throw slExceptionCamIn("caméra Flir non connectée");
	}
	else {
		nbFlir_++;
		Sleep(2000);	// Pause pour attendre la connexion de la caméra
	}

	// Marque et modèle
	brand_ = "Flir";
	model_ = "A40";

	// Taille et FPS fournis par manufacturier
	width_ = 320;
	height_ = 240;
	fps_ = 30000.0f / 1001;		// 29.97 fps

	return *this;
}


void slFlirCam::close()
{
	if (connected_) {
		dialogFlir_->flir_.Disconnect();
		nbFlir_--;
		connected_ = false;
	}
}


bool slFlirCam::inColors() const
{
	return false;
}


bool slFlirCam::hasParameters() const
{
	return true;
}


bool slFlirCam::hasDynamicParameters() const
{
	return true;
}


void slFlirCam::showParameters()
{
	dialogFlir_->ShowWindow(SW_SHOW);
}


void slFlirCam::hideParameters()
{
	dialogFlir_->ShowWindow(SW_HIDE);
}


void slFlirCam::startAfter()
{
	// On place les paramètres de la caméra. Documentation pages 59 à 66
	dialogFlir_->flir_.SetCameraProperty(43, COleVariant((double)fps_));	// FPS
	dialogFlir_->flir_.SetCameraProperty(49, COleVariant((short)0));		// Overlay not visible

	imageHeader_ = Mat(height_, width_, CV_8UC1, NULL);
}


void slFlirCam::read(slMat &image)
{
	VARIANT donneesBrutes;
	COleSafeArray donneesConverties;

	// On va chercher l'image de la caméra.
	donneesBrutes = dialogFlir_->flir_.GetImage(4);
	if (donneesBrutes.vt == VT_I2) {
		throw slExceptionCamIn("Connect and read on two different threads.");
	}

	donneesConverties.Attach(donneesBrutes);
	donneesConverties.AccessData((void**)&imageHeader_.data);

	// Copier l'image
	imageHeader_.copyTo(image1ch_);

	// On va remettre l'image
	donneesConverties.UnaccessData();
	donneesConverties.Clear();

	image = getOutputImage();
}


cv::Scalar slFlirCam::getPixel(const cv::Point &pos) const
{
	Scalar pix(image1ch_[pos.y][pos.x]);

	// C'est relativement peu coûteux d'aller chercher la table à chaque fois
	// C'est aussi nécessaire, car la LUT peut changer pendant une capture
	VARIANT table = dialogFlir_->flir_.GetLUT(0);
	pix.val[2] = ((float*)table.parray->pvData)[(int)pix.val[0]] - 273.15;	// Kelvin -> Celsius
	SafeArrayDestroy(table.parray);

	return pix;
}


