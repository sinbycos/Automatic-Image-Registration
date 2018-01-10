/*
 *	  file	   slTestComparaison.h
 *	  PURPOSE: Comparer des images segmenter a la main avec celles faite par un soustracteur.
 *
 *	  author  Michael Sills Lavoie, Pier-Luc St-Onge
 *       
 *    date    20.06.2007 - February 2012
 */
#ifndef _SLTESTCOMPARAISON_H_
#define _SLTESTCOMPARAISON_H_


#include "slBgSub.h"

#include <string>
#include <fstream>


/*
*	Utilise pour comparer un image "binaire" creer a la main avec un creer par le bgs.
*
*	Lorsque l image est comparer le programme creer l image segmenter par l ordinateur
*	et la nomme [nom du video][numero du frame].bmp, il creer aussi un fichier texte du meme
*	nom contenant les statistiques du video traite.
*
*	attention Dans l etat actuel, cet analyseur regarde si a chaque frame, il y a une image
*	ayant le format suivant dans le meme dossier que le video traite "[nom du video][HS][numero du frame][.bmp]
*	par exemple, pour le video Visible4.avi pour comparer le frame 15 on devra l image de celui-ci 
*	Visible4HS15.bmp.
*
*	author Michael Sills Lavoie
*/
class SLBGSUB_DLL_EXPORT slTestComparaison/* : public slBaseAnalyzer*/
{
public:
	slTestComparaison() {}
	virtual ~slTestComparaison();

	virtual void open(const std::string &filename, const slAH::slParameters& parameters = slAH::slParameters());
	virtual void compare(int index, const slImage1ch &binaryFG, const slImage1ch &groundTruth);
	virtual void close();

	std::string getFilenamePrefix() const;

private:
	std::string filenamePrefix_;
	std::ofstream results_;

	unsigned long long int truePositives_;	//!< Nombre de vrais positifs détectés en tout dans le video.
	unsigned long long int trueNegatives_;	//!< Nombre de vrais négatifs détectés en tout dans le video.
	unsigned long long int falsePositives_;	//!< Nombre de faux positifs détectés en tout dans le video.
	unsigned long long int falseNegatives_;	//!< Nombre de faux negatifs détectés en tout dans le video.
};


#endif	// _SLTESTCOMPARAISON_H_