#include "stdafx.h"
#include "slUnibrainCam.h"


using namespace slAH;


unsigned long slUnibrainCam::nbUnibrainObj_ = 0;
std::set<long long> slUnibrainCam::usedGUID_;


slUnibrainCam::slUnibrainCam(): slCamIn()
{
	iidcInit();

	guid_ = invalidGUID();
	handle_ = NULL;
	settings_ = NULL;
	engine_ = NULL;

	mono_ = false;
	doBayer_ = false;
}


slUnibrainCam::~slUnibrainCam()
{
	stop();
	hideParameters();
	close();
	iidcUninit();
}


//! Initialisation du gestionnaire du IIDC-Module
/*!
 *	À faire une seule fois pour une capture de plusieurs caméras
 */
void slUnibrainCam::iidcInit()
{
	if (nbUnibrainObj_ == 0) {
		if (FiInitialize() != FIREi_STATUS_SUCCESS) {
			throw slExceptionCamIn("UnibrainCamera::iidcInit(): can not initiate IIDC module");
		}

		if (FiInitializeDisplay() != FIREi_STATUS_SUCCESS) {
			throw slExceptionCamIn("UnibrainCamera::iidcInit(): FiInitializeDisplay() failed");
		}
	}

	nbUnibrainObj_++;
}


slUnibrainCam& slUnibrainCam::open(const std::string &guid)
{
	// Nettoyer la connexion
	close();

	// Sélection de la caméra (GUID)
	unsigned long adapterIndex;

	if (guid.length() < sizeof(guid_)) {
		if (!FiSelectCamera(&guid_, &adapterIndex)) {
			guid_ = invalidGUID();	// On ne connaît pas le contenu de guid_ après FiSelectCamera()
			throw slExceptionCamIn("Aucune caméra sélectionnée");
		}
	}
	else {
		guid_ = invalidGUID(); // *(FIREi_CAMERA_GUID*)guid.c_str();
	}

	// Vérifier que la caméra n'est pas déjà sélectionnée
	if (usedGUID_.find(guidToLL(guid_)) != usedGUID_.end()) {
		guid_ = invalidGUID();	// Au prochain connect(), le disconnect() ne doit pas éliminer le GUID utilisé
		throw slExceptionCamIn("Caméra déjà sélectionnée");
	}

	// Ouvrir la connexion de la caméra
	if (FiOpenCameraHandle(&handle_, &guid_) != FIREi_STATUS_SUCCESS) {
		throw slExceptionCamIn("Erreur de la fonction FiOpenCameraHandle()");
	}

	// Choisir le format voulu
	if (!FiSelectVideoFormat(handle_, &startupInfo_)) {
		// Commun dénominateur
		startupInfo_.Tag = FIREi_CAMERA_STARTUP_INFO_TAG;
		startupInfo_.FrameRate     = fps_7_5;
		startupInfo_.VideoMode     = Mode_5;
		startupInfo_.VideoFormat   = Format_0;
		startupInfo_.TransmitSpeed = S400;
		startupInfo_.IsochSyCode   = 1;
		startupInfo_.ChannelNumber = (UCHAR)0;
	}

	// Obtenir les informations de la caméra
	obtainSpecifications();

	// Démarrer la caméra
	if (FiStartCamera(handle_, &startupInfo_) != FIREi_STATUS_SUCCESS) {
		throw slExceptionCamIn("Erreur de la fonction FiStartCamera()");
	}

	// Fin de la connexion réussie
	usedGUID_.insert(guidToLL(guid_));
	connected_ = true;

	return *this;
}


void slUnibrainCam::obtainSpecifications()
{
	brand_ = getString(handle_, OID_VENDOR_NAME);
	model_ = getString(handle_, OID_MODEL_NAME);

	FIREi_VIDEO_FORMAT_INFO vidFormat;

	if (FiInitFormatInfo(&vidFormat, startupInfo_.VideoFormat) == FALSE) {
		throw slExceptionCamIn("FiInitFormatInfo failed");
	}

	// Vérifier le format des pixels
	if (vidFormat.Mode[startupInfo_.VideoMode].PixelFormat > YUV_444) {
		throw slExceptionCamIn("Pixel format not supported");
	}

	// Obtenir la taille des images
	width_ = vidFormat.Mode[startupInfo_.VideoMode].uWidth;
	height_ = vidFormat.Mode[startupInfo_.VideoMode].uHeight;

	// Traduire le FIREi_FPS en float
	int frameRate = startupInfo_.FrameRate;
	fps_ = 7.5;

	if (frameRate > fps_7_5) { fps_ *= (1 << (frameRate - fps_7_5)); }
	if (frameRate < fps_7_5) { fps_ /= (1 << (fps_7_5 - frameRate)); }

	// Gérer les images mono vs couleur
	mono_ = (vidFormat.Mode[startupInfo_.VideoMode].PixelFormat == Y_MONO);

	if (mono_) {
		doBayer_ = (AfxMessageBox(
			"Le format choisi est monochrome. Voulez-vous appliquer le filtre de Bayer?",
			MB_YESNO | MB_ICONQUESTION) == IDYES);
	}
	else {
		doBayer_ = false;
	}
}


std::string slUnibrainCam::description2() const
{
	std::stringstream sstr;

	sstr << (mono_ && !doBayer_ ? "monochrome" : "couleurs");
	sstr << (mono_ && doBayer_ ? " (Bayer)" : "");

	return sstr.str();
}


bool slUnibrainCam::inColors() const
{
	return (!mono_ || doBayer_);
}


bool slUnibrainCam::isStarted() const
{
	BOOLEAN isRunning = FALSE;

	if (handle_ != NULL && FiIsCameraRunning(handle_, &isRunning) != FIREi_STATUS_SUCCESS) {
		throw slExceptionCamIn("Erreur de la fonction FiIsCameraRunning()");
	}

	return (isRunning != FALSE);
}


bool slUnibrainCam::hasParameters() const
{
	return true;
}


bool slUnibrainCam::hasDynamicParameters() const
{
	return true;
}


void slUnibrainCam::showParameters()
{
	if (isStarted()) {
		if (!FiOpenCameraSettings(handle_, &settings_)) {
			throw slExceptionCamIn("Erreur de la fonction FiOpenCameraSettings()");
		}
	}
	else {
		AfxMessageBox("La caméra n'est pas correctement connectée.");
	}
}


void slUnibrainCam::hideParameters()
{
	if (settings_ != NULL) {
		FiCloseCameraSettings(settings_);
		settings_ = NULL;
	}
}


void slUnibrainCam::startAfter()
{
	// Create and start engine
	FIREi_STATUS status;

	status = FiCreateIsochReceiveEngine(&engine_);

	if (status != FIREi_STATUS_SUCCESS) {
		engine_ = NULL;
		throwLastError("FiCreateIsochReceiveEngine failed with error ", status);
	}

	status = FiStartIsochReceiveEngine(engine_, &startupInfo_, 1);

	if (status != FIREi_STATUS_SUCCESS) {
		FiDeleteIsochReceiveEngine(engine_);
		engine_ = NULL;
		throwLastError("FiStartIsochReceiveEngine failed with error ", status);
	}
}


void slUnibrainCam::read(slMat &image)
{
	FIREi_CAMERA_FRAME frame;

	if (handle_) {
		FIREi_STATUS status = FiGetNextCompleteFrame(&frame, engine_, 5000);

		if (status == FIREi_STATUS_SUCCESS) {
			if (mono_) {
				DWORD bufsize = image1ch_.total() * image1ch_.elemSize();
				memcpy(image1ch_.data, frame.pCameraFrameBuffer, bufsize);

				if (doBayer_) {
					cvtColor(image1ch_, image3ch_, CV_BayerGR2BGR);
				}
			}
			else {
				DWORD bufsize = image3ch_.total() * image3ch_.elemSize();
				status = FiYuv2Bgr(&frame, (BYTE*)image3ch_.data, &bufsize);

				if (status != FIREi_STATUS_SUCCESS) {
					throwLastError("FiYuv2Rgb failed with error ", status);
				}

				flip(image3ch_, image3ch_, 0);
			}

			image = getOutputImage();
		}
	}
}


void slUnibrainCam::stopBefore()
{
	// Stop and delete engine
	if (engine_ != NULL) {
		FiStopIsochReceiveEngine(engine_);
		FiDeleteIsochReceiveEngine(engine_);
		engine_ = NULL;
	}
}


void slUnibrainCam::close()
{
	// La caméra ne sera plus utilisée
	connected_ = false;
	usedGUID_.erase(guidToLL(guid_));

	// Arrêter la caméra
	if (isStarted()) {
		FiStopCamera(handle_);
	}

	// Reset de la description
	resetDescription();
	mono_ = false;
	doBayer_ = false;

	// Fermer la connexion
	if (handle_) {
		FiCloseCameraHandle(handle_);
		handle_ = NULL;
	}

	// La sélection doit être nulle
	guid_ = invalidGUID();
}


void slUnibrainCam::iidcUninit()
{
	if (nbUnibrainObj_ == 1) {
		FiTerminate();
	}

	nbUnibrainObj_--;
}


FIREi_CAMERA_GUID slUnibrainCam::invalidGUID()
{
	return llToGUID(-1);
}


long long slUnibrainCam::compare(const FIREi_CAMERA_GUID &guid1, const FIREi_CAMERA_GUID &guid2)
{
	return guidToLL(guid1) - guidToLL(guid2);
}


std::string slUnibrainCam::guidToHex(const FIREi_CAMERA_GUID &guid)
{
	char buffer[24];

	sprintf_s(&buffer[0], 24, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
		(ULONG)guid.Bytes[0], (ULONG)guid.Bytes[1], (ULONG)guid.Bytes[2], (ULONG)guid.Bytes[3],
		(ULONG)guid.Bytes[4], (ULONG)guid.Bytes[5], (ULONG)guid.Bytes[6], (ULONG)guid.Bytes[7]);

	return &buffer[0];
}


long long slUnibrainCam::guidToLL(const FIREi_CAMERA_GUID &guid)
{
	return *(long long*)&guid;
}


FIREi_CAMERA_GUID slUnibrainCam::llToGUID(const long long llid)
{
	FIREi_CAMERA_GUID guid;

	*(long long*)&guid = llid;

	return guid;
}


std::string slUnibrainCam::getString(FIREi_CAMERA_HANDLE camHandle, FIREi_OID oidCode)
{
	const long nbChar = 64;
	char buffer[nbChar + 1];
	FIREi_STATUS status = FiQueryCameraRegister(camHandle, oidCode, buffer, nbChar);

	if (status != FIREi_STATUS_SUCCESS) {
		std::stringstream sstr;
		sstr << "UnibrainCamera::getString() with code" << oidCode << ": ";
		throwLastError(sstr.str(), status);
	}

	buffer[nbChar] = '\0';
	return &buffer[0];
}


void slUnibrainCam::throwLastError(const std::string &prefix, FIREi_STATUS status)
{
	throw slExceptionCamIn(prefix + FiStatusStringA(status));
}


///////////////////////////////////////////////////////////////////////////////
//	slUnibrainCamFactory
///////////////////////////////////////////////////////////////////////////////


slUnibrainCamFactory slUnibrainCamFactory::factory_;


slUnibrainCamFactory::slUnibrainCamFactory()
: slCamInFactory("unibrain")
{
}


slUnibrainCamFactory::~slUnibrainCamFactory()
{
}


void slUnibrainCamFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
}


slUnibrainCam* slUnibrainCamFactory::createInstance() const
{
	return new slUnibrainCam();
}


