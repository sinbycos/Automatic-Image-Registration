#ifndef _SLMINMAX_H_
#define _SLMINMAX_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"
#include <highgui.h>
#include <cv.h>
#include <map>

struct slBG_Model //Structure representant le modele du BG
{
	vector<int> max;
	vector<int> min;
	vector<int> ecart;
};

class slMinMax: public slBgSub
{
public:
	virtual ~slMinMax();

protected:
	virtual void showSpecificParameters() const;
	virtual void fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const;

	virtual slBgSub* createSpecificInstance(const slParameters& parameters) const;

	virtual int specificInit();
	virtual void computeFrame();
	virtual void setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
		slPixel1ch *b_fg_data, const int& w, const int& i, const int& j);
	virtual int specificLoadNextFrame();

private:
	slMinMax(const string &name = "");
	slMinMax(const slArgHandler* pArgHandlerBgSub);

private:
	static slMinMax Creator_;
	slBG_Model mBG;	//Modele du BG general
	slBG_Model mBG_FG;//Modele du BG pour les pixels du FG
	slBG_Model mBG_BG;//Modele du BG pour les pixels du BG
	IplImage* Prec;	//image precedente
	IplImage* Prec2;//image precedente-1
	map<int, vector<int> > InfoInit; //Map pour l'apprentissage du 1er BG
	vector<int> GS;//nombre de fois qu'un pixel a ete dans le BG
	vector<int> MS;//nombre de fois qu'un pixel a ete dans le FG
	vector<int> InitMap; //Vecteur qui sert a reinitiliser GS et MS
	int K;			//constante pour la séparation du FG
	double k;		//constante pour l'update du BG (avec GS)
	double r;		//constante pour l'update du BG (avec MS)
	int nbInit;		//nombre de frame de construction du BG initiale
	int	RR;			//refresh rate pour l'update du BG
	double sigma;	//ecart-type
	int Reset;
	double change;
	int FO;			//indique si l'on veut le Flux Optique


};


#endif	// _SLMINMAX_H_