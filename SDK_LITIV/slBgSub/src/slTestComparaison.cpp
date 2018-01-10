/*
 *	  file	   slTestComparaison.cpp
 *	  PURPOSE: Comparer des images segmenter a la main avec celles faite par un soustracteur.
 *
 *	  author  Michael Sills Lavoie, Pier-Luc St-Onge
 *       
 *    date    20.06.2007 - February 2012
 */

#include "slTestComparaison.h"


using namespace cv;
using namespace std;
using namespace slAH;


/*
*    Destructeur de la classe.
*	 Cette methode finalise l analyse du fichier_ en entran le total des informations traite
*	 et en fermant le fichier_.
*/
slTestComparaison::~slTestComparaison()
{
	close();
}


/*
*    Initialisation de l objet avant le debut du traitement.
*	 On creer ici le fichier_ texte qui sera utiliser pour le traitement en faisant bien sur
*	 qu il n existe pas deja. Si c est le cas le nom utiliser sera [nom du video][(version)][.txt]
*/
void slTestComparaison::open(const std::string &filename, const slAH::slParameters& parameters)
{
	// Set default output filename prefix
	filenamePrefix_ = filename.substr(0, filename.size() - 4);

	//Test to see if the file already exist (if it exist we change the name of the file
	bool openCorrectly = false;
	int nbOfFile = 0;
	string resultsFilename = filenamePrefix_;

	do {
		ifstream exist;
		exist.open((resultsFilename + ".txt").c_str(), ios::in);

		if (!exist.fail()) {
			exist.close();

			char temp[10];
			_itoa_s(++nbOfFile, temp, 10, 10);
			resultsFilename = filenamePrefix_ + "(" + string(temp) + ")";
		}
		else {
			exist.clear();
			openCorrectly = true;
		}
	} while(!openCorrectly);

	// Open file and write header
	results_.open((resultsFilename + ".txt").c_str(), ios::out);
	results_ << "TEST DE QUALITE DE LA SOUSTRACTION D'ARRIERE-PLAN"<< endl << endl;
	results_ << "Parametres utilises avec leurs valeurs respectives: " << endl << endl;
	parameters.printParameters(results_);
	results_ << endl << endl;

	// Reset all statistics
	truePositives_ = 0;
	trueNegatives_ = 0;
	falsePositives_ = 0;
	falseNegatives_ = 0;
}


/*
*    Methode executer apres le traitement de chaque frame.
*	 Cette methode cherche dans le dossier ou se trouve le video pour voir si il 
*	 y a une image traite a la main, si oui elle execute la comparaison, si non,
*	 elle ne fait rien. (voir la declaration de la classe pour plus de detail.
*/
void slTestComparaison::compare(int index, const slImage1ch &binaryFG, const slImage1ch &groundTruth)
{
	if (!results_.is_open()) return;
	if (groundTruth.empty()) return;

	const Size imSize = binaryFG.size();

	unsigned int truePos = 0;
	unsigned int trueNeg = 0;
	unsigned int falsePos = 0;
	unsigned int falseNeg = 0;

	for (int i = 0; i < imSize.height; i++) {
		for (int j = 0; j < imSize.width; j++) {
			if (groundTruth[i][j] < 128) {
				if (binaryFG[i][j] < 128) {
					trueNeg++;
				}
				else {	// binaryFG[i][j] >= 128
					falsePos++;
				}
			}
			else {	// groundTruth[i][j] >= 128
				if (binaryFG[i][j] < 128) {
					falseNeg++;
				}
				else {	// binaryFG[i][j] >= 128
					truePos++;
				}
			}
		}
	}

	unsigned int nbCorrect = truePos + trueNeg;
	unsigned int nbTotal = truePos + trueNeg + falsePos + falseNeg;

	results_ << "Frame : " << index << endl;
	results_ << "\tTrue positives :                             " << truePos << endl;
	results_ << "\tTrue negatives :                             " << trueNeg << endl;
	results_ << "\tCorrectly identified pixels :                " << nbCorrect << endl;
	results_ << "\tPercentage of correctly identified pixels :  "
		<< (100.0 * nbCorrect / nbTotal) << "%" << endl;
	results_ << "\tFalse positives :                            " << falsePos << endl;
	results_ << "\tFalse negatives :                            " << falseNeg << endl;
	results_ << "\tPercentage of incorrectly identifid pixels : "
		<< (100.0 * (falsePos + falseNeg) / nbTotal) << "%" << endl << endl;

	truePositives_ += truePos;
	trueNegatives_ += trueNeg;
	falsePositives_ += falsePos;
	falseNegatives_ += falseNeg;
}


void slTestComparaison::close()
{
	if (results_.is_open()) {
		unsigned long long int nbCorrect = truePositives_ + trueNegatives_;
		unsigned long long int nbTotal =
			truePositives_ + trueNegatives_ + falsePositives_ + falseNegatives_;

		results_ << "Total : " << endl;
		results_ << "\tTrue positives :                             " << truePositives_ << endl;
		results_ << "\tTrue negatives :                             " << trueNegatives_ << endl;
		results_ << "\tCorrectly identified pixels :                " << nbCorrect << endl;
		results_ << "\tPercentage of correctly identified pixels :  "
			<< (100.0 * nbCorrect / nbTotal) << "%" << endl;
		results_ << "\tFalse positives :                            " << falsePositives_ << endl;
		results_ << "\tFalse negatives :                            " << falseNegatives_ << endl;
		results_ << "\tPercentage of incorrectly identifid pixels : "
			<< (100.0 * (falsePositives_ + falseNegatives_) / nbTotal) << "%" << endl << endl;

		results_.close();
	}
}


std::string slTestComparaison::getFilenamePrefix() const
{
	return filenamePrefix_;
}


