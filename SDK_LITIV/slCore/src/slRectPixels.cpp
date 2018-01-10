/*!	\file	slRectPixels.cpp
 *	\brief	Rectangle used in rectangle background subtractor
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		23.05.2007 - February 2012
 */

#include "slRectPixels.h"


using namespace std;


bool slRectPixels::useTexture_ = false;


/*!
 *	Constructor for the RectanglePixels
 *	\param	coordX (int) - The true x co-ordinate of the top left corner of the slRectPixels
 *	\param	coordY (int) - The true y co-ordinate of the top left corner of the slRectPixels
 *	\param	isLowestLevel (bool) - Tell if the slRectPixels is at the lowest level (do not contain any slRectPixels)
 *	\param	dimX (int) - The number of pixel in the x dimension in this slRectPixels
 *	\param	dimY (int) - The number of pixel in the x dimension in this slRectPixels
 */
slRectPixels::slRectPixels(int coordX, int coordY, bool isLowestLevel,int dimX,int dimY)
	:coordX_(coordX), coordY_(coordY), isLowestLevel_(isLowestLevel), dimX_(dimX), dimY_(dimY)
{
	isBackground_ = true;
	isChecked_ = false;
	texture_.mean_ = 0;
	textureB_.mean_ = 0;
	texture_.var_ = 0;
	textureB_.var_ = 0;
}


slRectPixels::~slRectPixels(){}


/*!
 *	Add links to the small slRectPixels that are contained into this slRectPixels
 *	\param	rect1 (slRectPixels*) -Top left slRectPixels 
 *	\param	rect2 (slRectPixels*) -Top right slRectPixels  
 *	\param	rect3 (slRectPixels*) -bottom left slRectPixels  
 *	\param	rect4 (slRectPixels*) -bottom right slRectPixels  
 */
void slRectPixels::addLink(slRectPixels* rect1, slRectPixels* rect2, slRectPixels* rect3, slRectPixels* rect4)
{
	contain_.push_back(rect1);
	contain_.push_back(rect2);
	contain_.push_back(rect3);
	contain_.push_back(rect4);
}


/*!
 *	Add links to the small slRectPixels that bord this one
 *	\param	rect (slRectPixels*) -One of the slRectPixels  
 */
void slRectPixels::addExternalLink(slRectPixels* rect)
{
	external_.push_back(rect);
}			


/*!
 *	Update the histograms and the textures (only if useTexture is set to true) of this slRectPixels
 *	By doing that, this slRectPixels update all the slRectPixels that are contain in it
 *	\param	data (slImage3ch) -Data to be put in the histogram of the foreground (used by the lower level slRectPixels)
 *	\param	dataB (slImage3ch) -Data to be put in the histogram of the background (used by the lower level slRectPixels)
 */
void slRectPixels::update(const slImage3ch &data, const slImage3ch &dataB)
{
	//If we are not at the lowest level, we just update the histograms of the contained slRectPixels
	//and then update this one
	if(!isLowestLevel_)
	{
		//For each one of the rectangle that are contain within this one
		for(size_t i = 0; i < 4; ++i)
		{
			contain_[i]->update(data,dataB);

			//After updating the internal rectangle we can update our histograms
			histogram_ += contain_[i]->getHistogram();
			histogramB_ += contain_[i]->getHistogramB();

			//If we use texture, we also update the texture data
			if(useTexture_)
			{
				texture_.var_ += contain_[i]->texture_.var_;
				texture_.mean_ += contain_[i]->texture_.mean_;
				textureB_.var_ += contain_[i]->textureB_.var_;
				textureB_.mean_ += contain_[i]->textureB_.mean_;

				//The first time we reserve a certain size for the vector so it is not copied several time
				if (i == 0)
				{
					texture_.intensity_.reserve(4 * contain_[i]->texture_.intensity_.size());
					textureB_.intensity_.reserve(4 * contain_[i]->textureB_.intensity_.size());
				}

				for (size_t j = 0; j < contain_[i]->texture_.intensity_.size();++j)
				{
					texture_.intensity_.push_back(contain_[i]->texture_.intensity_[j]);
					textureB_.intensity_.push_back(contain_[i]->textureB_.intensity_[j]);
				}
			}

		}
		histogramB_.normalize();
		histogram_.normalize();

		//If we have updated the texture, we have to update this rectangle texture data
		//With the law of total variance...
		if(useTexture_)
		{
			float moyenneDeLaVariance = texture_.var_ / 4;
			float varianceDeLaMoyenne = 0;
			float dif = 0;
			float moyenneDeLaVarianceB = textureB_.var_ / 4;
			float varianceDeLaMoyenneB = 0;
			float difB = 0;

			texture_.mean_/=4;
			textureB_.mean_/=4;

			for(size_t i = 0; i < 4; ++i)
			{
				dif = texture_.mean_ - contain_[i]->texture_.mean_;
				varianceDeLaMoyenne += dif * dif;
				difB = textureB_.mean_ - contain_[i]->textureB_.mean_;
				varianceDeLaMoyenneB += difB * difB;
			}

			varianceDeLaMoyenne /= 4;
			varianceDeLaMoyenneB /= 4;

			texture_.var_ = varianceDeLaMoyenne + moyenneDeLaVariance;
			textureB_.var_ = varianceDeLaMoyenneB + moyenneDeLaVarianceB;
		}
	}
	//If we are at the lowest level, we create the histogram with the pixel contained within it
	else
	{
		size_t endX = coordX_ + dimX_ - 1;
		size_t endY = coordY_ + dimY_ - 1;

		for(size_t j = coordY_; j < endY; ++j)
		{
			for(size_t i = coordX_; i < endX; ++i)
			{
				histogram_+=data[j][i];
				histogramB_+=dataB[j][i];

				//Convertion of the BGR data to the intensity for each pixel if we use texture
				if(useTexture_)
				{
					texture_.intensity_.push_back((float)(0.299 * data[j][i][2] + 0.587 * data[j][i][1]
						+ 0.114 * data[j][i][0]));
					textureB_.intensity_.push_back((float)(0.299 * dataB[j][i][2] + 0.587 * dataB[j][i][1]
						+ 0.114 * dataB[j][i][0]));
					
					texture_.mean_ += texture_.intensity_.back();
					textureB_.mean_ += textureB_.intensity_.back();
				}
			}
		}
		histogram_.normalize();
		histogramB_.normalize();

		//Again we update this rectangle texture data
		if(useTexture_)
		{
			texture_.mean_ /= texture_.intensity_.size();
			textureB_.mean_ /= textureB_.intensity_.size();

			float dif = 0;
			float difB = 0;

			for (size_t i = 0; i < texture_.intensity_.size(); ++i)
			{
				dif = texture_.mean_ - texture_.intensity_[i];
				texture_.var_ += dif * dif;
				difB = textureB_.mean_ - textureB_.intensity_[i];
				textureB_.var_ += difB * difB;
			}

			texture_.var_ /= texture_.intensity_.size();
			textureB_.var_ /= textureB_.intensity_.size();
		}
	}
}


/*!
 *	Clear the data contained in this slRectPixels and in each contained slRectPixels
 */
void slRectPixels::clearData(void)
{
	histogram_.clear();
	histogramB_.clear();
	isBackground_ = true;
	isChecked_ = false;
	if(useTexture_)
	{
		texture_.intensity_.clear();
		texture_.mean_ = 0;
		texture_.var_ = 0;
		textureB_.intensity_.clear();
		textureB_.mean_ = 0;
		textureB_.var_ = 0;
	}

	if(!isLowestLevel_)
		for(size_t i = 0; i < 4; ++i)
			contain_[i]->clearData();
}


/*!
 *	Compare this histogram with the one in the argument.
 *	\param	th (float) -The theshold used to compare the histograms
 *	\param	deltath (float) -We increase the th for the next slRectPixels by this value
 *	\param	thTexture (float) -...
 *	\param	dthTexture (float) -...
 */
void slRectPixels::compareRectangle (float th, float deltath, float thTexture, float dthTexture)
{
	isChecked_ = true;
	vector<float> temp;
	float compDegree = 0;

	if(useTexture_)
	{
		float prodScal = 0;
		float NVc2 = 0;
		float NVb2 = 0;

		if (thTexture < 1.0)
		{
#ifdef _DEBUG
			cout << "You should enter a higher number for the texture threshold..." << endl;
#endif
			thTexture = 1.0;
		}
		//We check if those rectangle are textured or not
		//If the two of them are textured
		if(texture_.var_ >= thTexture && textureB_.var_ >= thTexture)
		{
			for (size_t i = 0; i < texture_.intensity_.size(); ++i)
			{
				//Norme de Vc au carre
				NVc2 += (texture_.intensity_[i]- texture_.mean_) * (texture_.intensity_[i]- texture_.mean_);
				NVb2 += (textureB_.intensity_[i]- textureB_.mean_) * (textureB_.intensity_[i]- textureB_.mean_);

				//Produit scalaire
				prodScal += ((texture_.intensity_[i] - texture_.mean_) * (textureB_.intensity_[i]
					- textureB_.mean_));
			}

			prodScal = fabs(prodScal);
			compDegree = prodScal / (sqrt(NVc2) * sqrt (NVb2));

			compDegree += (float)0.8;
		}
		else
		{
			//Si les deux rectangle ne sont pas texture on se dit qu'il n'y a rien
			if (texture_.var_ < thTexture && textureB_.var_ <= thTexture)
			{
				compDegree = 1.5;
			}
			else
				//Pour ces deux situations il y a de forte chance que ce soit un objet
				if(texture_.var_ < thTexture && textureB_.var_ > thTexture)
				{
					compDegree = (float)0.8;
				}
				else
				{
					compDegree = (float)0.8;
				}
		}
	}
	//If we do not use the texture, there are no dynamic ajustment of the th.
	else
		compDegree = 1;

	//If the rectangle have changed
	temp = histogram_.compare(histogramB_);
	if (temp[0] > (th * compDegree) || temp[1] > (th * compDegree) || temp[2] > (th * compDegree))
	{
		isBackground_ = false;
		//We must check if the lower level rectangle have changed or not...
		if(!isLowestLevel_)
		{
			for (size_t i = 0; i < external_.size(); ++i)
				//If the rectangle is not already checked
				if (!external_[i]->isChecked_)
					external_[i]->compareRectangle(th + deltath,
					deltath,thTexture + dthTexture,dthTexture);

			//we do the same thing for the internal rectangle
			for (size_t i = 0; i < contain_.size(); ++i)
				//If the rectangle is not already checked
				if (!contain_[i]->isChecked_)
					contain_[i]->compareRectangle(th + deltath,
					deltath,thTexture + dthTexture,dthTexture);
		}
	}
}


/*!
 *	Set the bool that tell if the slRectPixels is in the background
 *	\param isBack (bool) -Tell if it is background
 */
void slRectPixels::setBackground (bool isBack)
{
	isBackground_ = isBack;
}


/*!
 *	Return a bool that say if the slRectPixels is in the background
 *	\return (bool)	-Tell if the slRectPixels is background
 */
bool slRectPixels::getBackground (void) const
{
	return isBackground_;
}


/*!
 *	Return the histogram of the slRectPixels
 *	\return (slHistogram3ch)	-The histogram of the slRectPixels
 */
slHistogram3ch& slRectPixels::getHistogram (void)
{
	return histogram_;
}


/*!
 *	Return the histogram of the slRectPixels for the background data
 *	\return (slHistogram3ch)	-The histogram of the slRectPixels
 */
slHistogram3ch& slRectPixels::getHistogramB (void)
{
	return histogramB_;
}


/*!
 *	Return the position of the slRectPixels in the Image
 *	\return (int)	-The position of the slRectPixels
 */
int slRectPixels::getCoordX (void) const
{
	return coordX_;
}


/*!
 *	Return the position of the slRectPixels in the Image
 *	\return (int)	-The position of the slRectPixels
 */
int slRectPixels::getCoordY (void)const
{
	return coordY_;
}


/*!
 *	Return the number of pixel that the slRectPixels contain
 *	\return (int)	-The number of pixel contain by the slRectPixels(X)
 */
int slRectPixels::getDimX (void) const
{
	return dimX_;
}


/*!
 *	Return the number of pixel that the slRectPixels contain
 *	\return (int)	-The number of pixel contain by the slRectPixels(Y)
 */
int slRectPixels::getDimY (void)const
{
	return dimY_;
}


/*!
 *	Tell the Rectangle to use the texture or not
 */
void slRectPixels::setTexture (bool useTexture)
{
	useTexture_ = useTexture;
}


