/*!	\file	slQuantParams.h
 *	\brief	Contains classes that quantify integers
 *
 *	This file contains the definition of classes slQuant1ch and slQuant3ch
 *
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */

#ifndef _SLQUANTPARAMS_H_
#define _SLQUANTPARAMS_H_


#include <string>

#include "slArgHandler.h"
#include "slCore.h"


#define Q_MIN_LEVELS 2		//!< For binary images
#define Q_H_LIMIT 180		//!< H (<180)
#define Q_CHAN_LIMIT 256	//!< RGB and SV (<256)


//!	One channel integer quantifier
/*!
 *	This class can be used for 1-channel image quantification.
 *	All that is needed to set is the number of levels (steps) and the limit
 *	value (maximum allowed value plus 1).
 *	For example, the default constructor creates 256 levels, one for each
 *	possible value from 0 through 255.
 *
 *	Internally, a lookup table (LUT) is built in order to match any given
 *	unsigned 8-bit value to its corresponding output level.
 *	The output levels are pre-scaled to [0, limit value[ and they are centered
 *	in their level range.
 *	For example, a quantifier of 3 levels over [0, 256[ would have output
 *	values 42, 127 and 212.
 *
 *	Since OpenCV typically uses 180 values for the H component of HSV pixels, it
 *	is possible to create a quantifier of n levels, and set 180 as the limit value.
 *	Even in that case, the LUT would be built with 255 input values, but a modulo
 *	operator is applied: input value 180 has the same output value as 0, and so on.
 *
 *	\see		slQuant3ch, Q_CHAN_LIMIT, Q_H_LIMIT
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */
class SLCORE_DLL_EXPORT slQuant1ch
{
public:
	//! Default constructor (256 levels, <256)
	slQuant1ch(int nbLevels = Q_CHAN_LIMIT, int limitValue = Q_CHAN_LIMIT);

	//! Creates the lookup table
	void setParams(int nbLevels = Q_CHAN_LIMIT, int limitValue = Q_CHAN_LIMIT);

	// Compute methods
	void quantify(const slImage1ch &source, slImage1ch &target) const;					//!< Quantifies a 1-channel image
	inline slPixel1ch operator[](const slPixel1ch pixel) const { return lut_[pixel]; }	//!< Converts a value to its quantified level

	// Get methods
	inline int getNbLevels() const { return nbLevels_; }			//!< Returns the number of quantification levels
	inline int getLimitValue() const { return limitValue_; }		//!< Returns the limit value (255 or 180 typically)
	inline const slPixel1ch* getLUT() const { return &lut_[0]; }	//!< Returns a pointer to the LUT, if needed

private:
	int nbLevels_;
	int limitValue_;

	slPixel1ch lut_[Q_CHAN_LIMIT];

};


//!	One channel integer quantifier
/*!
 *	This class can quantify 3-channel pixels by using three slQuant1ch
 *	instances, one for each pixel component.
 *	In other words, it uses 3 lookup tables (LUT).
 *
 *	Here is an example of use for BGR images:
 *	\code
 *	slImage3ch bgrImage;
 *	// ...
 *	slQuant3ch quant(5, 9, 7);
 *	quant.quantify(bgrImage, bgrImage);
 *	\endcode
 *	
 *	Here is an example of use for HSV images:
 *	\code
 *	slImage3ch hsvImage;
 *	// ...
 *	quant.setParams(slQuant1ch(13, Q_H_LIMIT), 5, 9);
 *	quant.quantify(hsvImage, hsvImage);
 *	\endcode
 *
 *	\see		slQuant1ch, Q_CHAN_LIMIT, Q_H_LIMIT
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */
class SLCORE_DLL_EXPORT slQuant3ch
{
public:
	//! Default constructor (256, 256, 256)
	slQuant3ch(const slQuant1ch &q0 = slQuant1ch(), const slQuant1ch &q1 = slQuant1ch(), const slQuant1ch &q2 = slQuant1ch());

	//! Sets the three quantifiers, the first one must be defined
	void setParams(const slQuant1ch &q0, const slQuant1ch &q1 = slQuant1ch(), const slQuant1ch &q2 = slQuant1ch());

	// Compute methods
	void quantify(const slImage3ch &source, slImage3ch &target) const;	//!< Quantifies a 3-channels image
	slPixel3ch operator[](const slPixel3ch &pixel) const;				//!< Converts a pixel to its quantified levels

	// Get methods
	inline const slQuant1ch& getQ0() const { return q0_; }	//!< Returns quantifier of the first component of pixels
	inline const slQuant1ch& getQ1() const { return q1_; }	//!< Returns quantifier of the second component of pixels
	inline const slQuant1ch& getQ2() const { return q2_; }	//!< Returns quantifier of the third component of pixels

	std::string getStr(typeColorSys csys) const;			//!< Returns "nbLevels0 nbLevels1 nbLevels2"

public:
	static slAH::slParamSpec& fillSyntax(slAH::slParamSpec& paramSpec);	//!< Only fills the syntax of three arguments

private:
	slQuant1ch q0_;
	slQuant1ch q1_;
	slQuant1ch q2_;

};


#endif	// _SLQUANTPARAMS_H_


