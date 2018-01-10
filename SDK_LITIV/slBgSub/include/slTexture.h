/******************************************************************************
	File:	slSimpleGauss.h
	Description:	This is the Gaussian Mixture algorithm class.
	Created:	November 2006 (Pier-Luc St-Onge, Atousa Torabi, Parisa Darvish Zadeh Varcheie)
					Some comments (Pier-Luc St-Onge, Atousa Torabi, Parisa Darvish Zadeh Varcheie)
******************************************************************************/

#ifndef _SLTEXTURE_H_
#define _SLTEXTURE_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"


class slTexture: public slBgSub
{
public:
	virtual ~slTexture();

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
	slTexture(const std::string &name = "");
	slTexture(const slParameters& parameters);

private:
	//This class contains histogram of pixeles and updaintg their informtion
	class slHistogram
	{
	public:
		slHistogram(): mW(0)
		{
			mBins.resize(1 << mP);

			for (int ind = 0; ind < (1 << mP); ind++) {
				mBins[ind] = 0;
			}
		}
        //Fill histogram bins
		const slHistogram& operator=(const slHistogram& histogram)
		{
			if (&histogram != this) {
				mW = histogram.mW;

				for (int ind = 0; ind < (1 << mP); ind++) {
					mBins[ind] = histogram.mBins[ind];
				}
			}

			return *this;
		}
        //Update histogram
		const slHistogram& operator+=(const slHistogram& histogram)
		{
			for (int ind = 0; ind < (1 << mP); ind++) {
				mBins[ind] = (short)(mAb * histogram.mBins[ind] + (1 - mAb) * mBins[ind]);
			}

			return *this;
		}

		inline void addLBP(const unsigned char lbp) { mBins[lbp]++; }

        //Update weights
		inline void addW(const double Mk) { mW = mAw * Mk + (1 - mAw) * mW; }

		inline bool operator<(const slHistogram& histogram) { return mW > histogram.mW; }

		double intersection(const slHistogram& histogram);

		void setW(const double W) { mW = W; }
		inline double getW() const { return mW; }

	public:
		static void setP(int P)
		{
			mP = P;
			
			if (mP < 3 || 8 < mP) {
				throw "Neighborhood too small or too large.";
			}
		}

		static int getP() { return mP; }

		static void setAb(double Ab) { mAb = Ab; }
		static double getAb() { return mAb; }

		static void setAw(double Aw) { mAw = Aw; }
		static double getAw() { return mAw; }

	private:
		double mW;
		std::vector<short> mBins;

		static int mP;
		static double mAb;
		static double mAw;

	};

private:
	class slTextureComp
	{
	public:
		slTextureComp()
		{
			mHistograms.resize(mK);
			mB = mK;
		}

		void setInitHistogram(const slHistogram &histogram)
		{
			mHistograms[0] = histogram;
			mHistograms[0].setW(1);

			for (int ind = 1; ind < mK; ind++)
			{
				mHistograms[ind].setW(0);
			}

			mB = 1;
		}

		bool isPixBackground(const slHistogram &histogram);

	public:
		static void setK(int K) { mK = K; }
		static int getK() { return mK; }

		static void setTp(double Tp) { mTp = Tp; }
		static double getTp() { return mTp; }

		static void setTb(double Tb) { mTb = Tb; }
		static double getTb() { return mTb; }

	private:
		std::vector<slHistogram> mHistograms;
		int mB;

		static int mK;
		static double mTp;
		static double mTb;
	};

private:
	void computeTheHistograms(const slImage &image);

private:
	int mA;
    double mR;
    double mRr;

	double* offsets[2];

	// For all pixels, keep K histograms of LBPs
	slTextureComp* mTexturePix;

	// For all pixels of the current frame, keep an histogram of LPBs
	slHistogram* mCurrentHist;	

private:
	static slTexture Creator_;

};




#endif	// _SLTEXTURE_H_


