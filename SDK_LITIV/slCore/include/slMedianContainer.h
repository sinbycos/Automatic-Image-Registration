#ifndef _SLMEDIANCONTAINER_H_
#define _SLMEDIANCONTAINER_H_


#include <vector>


using namespace std;


#define MEDIAN_MIN_SIZE 1


template <class T>
class slMedianContainer
{
public:
	void resize(int size = MEDIAN_MIN_SIZE)
	{
		if (size >= MEDIAN_MIN_SIZE)
		{
			// Set the new size
			mSize = size;

			// Default mNbElements and next position
			mNbElements = 0;
			mNextInsPos = 0;

			// Resize vectors
			mIndex.resize(size);
			mValues.resize(size);
		}
	}

	inline const T& getMedian() const
	{
		return mValues[mIndex[mNbElements >> 1]];
	}

	const slMedianContainer<T>& operator+=(const T &value)
	{
		// Find the position of mNextInsPos in mIndex at the current state
		int indPos = getIndexPosition();

		// Insert the new value at position mNextInsPos
		mValues[mNextInsPos] = value;

		// Count the new value if needed
		if (mNbElements < mSize) mNbElements++;

		// Bubble sort : going to 0
		while (indPos > 0 && value < mValues[mIndex[indPos - 1]])
		{
			mIndex[indPos] = mIndex[indPos - 1];
			indPos--;
		}

		// Bubble sort : going to mNbElements - 1
		while (indPos + 1 < mNbElements && value > mValues[mIndex[indPos + 1]])
		{
			mIndex[indPos] = mIndex[indPos + 1];
			indPos++;
		}

		// Update mIndex with mNextInsPos
		mIndex[indPos] = mNextInsPos;

		// Update mNextInsPos and loop to 0 if needed
		if (++mNextInsPos == mSize) mNextInsPos = 0;

		return *this;
	}

private:
	int getIndexPosition()
	{
		if (mNbElements < mSize)
		{
			return mNbElements;
		}
		else
		{
			int indLower = 0;
			int indUpper = mSize;

			// Find the lower bound of the equivalent class
			while (indLower != indUpper)
			{
				int indPosit = (indLower + indUpper) >> 1;

				if (mValues[mIndex[indPosit]] < mValues[mNextInsPos])
				{
					indLower = indPosit + 1;
				}
				else
				{
					indUpper = indPosit;
				}
			}

			// Find the position of mNextInsPos in mIndex
			while (mIndex[indLower] != mNextInsPos)
			{
				indLower++;
			}

			return indLower;
		}
	}

private:
	int mSize;

	int mNbElements;
	int mNextInsPos;

	vector<int> mIndex;	// Sorted index pointing to values
	vector<T> mValues;	// Values

};


#endif	// _SLMEDIANCONTAINER_H_


