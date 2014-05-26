//
// This Fractal Noise class written by Martin G Bell, 2003.
//
// Description:
//
//  Simple generator for fractal subdivision noise.  Especially useful for e.g. terrain
//  heightmap generation.
//
// Usage:
//
//  1. Create a fractal noise class, initialising it with a buffer to put the noise in,
//     and the square size of the buffer:
//
//         int sideLength = 129;	// Accepts 2^n+1, for (4 <= n <= 10).
//         TFracVal* buff = new TFracVal[sideLength*sideLength];
//         CFractalNoise myNoise(buff, sideLength);
//
//  2. Pre-seed buffer as required (no pre-seeding here - see note below):
//
//         for (int i=0 ; i<sideLength*sideLength ; i++)
//		     buff[i] = FRACVAL_UNINIT;
//
//  3. Make some noise with the Generate() interface:
//
//         int seed = 13;
//         float adjust = 32000.0f;
//         float frequency = 2.5f;
//         myNoise.Generate(seed, adjust, frequency);
//
//  4. Remember to delete the buffer after use.
//
//         delete [] buff;
//         buff = 0;
//
//	Note: To create an island approximation, pre-seed the buffer 'edges' with zero and
//	the centre point with some positive value.
//

#ifndef _FRACTALNOISE_H
#define _FRACTALNOISE_H

#include <limits.h>

typedef int TFracVal;
const TFracVal FRACVAL_UNINIT = INT_MIN;	// Uninitialised height value.
const TFracVal MAX_FRACVAL = INT_MAX;
const TFracVal MIN_FRACVAL = FRACVAL_UNINIT+1;


class CFractalNoise
{

public:		// Construction / destruction.

	// Public constructor:
	// All parts of aDest set to FRACVAL_UNINIT will be affected by the fractal noise.
	// Currently accepts aSideLen = 2^n+1, for (4 <= n <= 10)
	CFractalNoise(TFracVal* aDest, int aSideLen);

public:		// Interface

	// Main generation interface. (Average settings: aAdjust:32000, aFrequency: 2.0)
	void Generate(int aSeed, float aAdjust, float aFrequency);

	void SetVal(int aX, int aY, TFracVal aVal);
	TFracVal GetVal(int aX, int aY) const;
	int GetMinVal() const {return mMinVal;}
	int GetMaxVal() const {return mMaxVal;}

private:	// Implementation

	CFractalNoise();		// Private to stop class misuse.
	void Subdivide(int aLeft, int aTop, int aRight, int aBot, float aAdjust);

private:	// Data

	TFracVal* const mDest;	// Store of data destination pointer.
	const int mSideLength;	// Side length of mDest buffer (i.e. the square size).
	float mFrequency;		// Fractal 'roughness' 1.2 to 3.0 ish.

	TFracVal mMaxVal;
	TFracVal mMinVal;
};


#endif
