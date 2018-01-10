#include "slException.h"


using namespace std;


#define SL_ERR_MESSAGE_SIZE 255
#define SL_CLASS_NAME_SIZE 50


slException::slException(const char* error, const char* name):
err_(new char[SL_ERR_MESSAGE_SIZE]), name_(new char[SL_CLASS_NAME_SIZE])
{
	*err_ = 0;
	strcat_s(err_, SL_ERR_MESSAGE_SIZE, error);

	*name_ = 0;
	if (name != NULL) strcat_s(name_, SL_CLASS_NAME_SIZE, name);
}


slException::~slException()
{
	delete [] err_;
	delete [] name_;
}


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - name (string): the name of the sample
 * Return value	:  No
 ***************************************************************************/
slExceptionArgHandler::slExceptionArgHandler(const std::string& error)
: slException(error.c_str(), "slArgHandler")
{
}


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - name (string): the name of the sample
 * Return value	:  No
 ***************************************************************************/
slExceptionIO::slExceptionIO(const std::string& error)
: slException(error.c_str(), "slIO")
{
};


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - name (string): the name of the sample
 * Return value	:  No
 ***************************************************************************/
slExceptionCamIn::slExceptionCamIn(const std::string& error)
: slExceptionIO(error)
{
	strcat_s(name_, SL_CLASS_NAME_SIZE, " (Camera issue)");
};


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - name (string): the name of the sample
 * Return value	:  No
 ***************************************************************************/
slExceptionBgSub::slExceptionBgSub(const std::string& error)
: slException(error.c_str(), "slBgSub")
{
}


