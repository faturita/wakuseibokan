
#include "FractalNoise.h"

#include <stdlib.h>		// rand() etc.
#include <assert.h>


#define FRAND(a) ((rand()/float(RAND_MAX))*a)


/**
@param aDest - FracVal array to put noise into. Pre-seed to vals or FRACVAL_UNINIT.
@param aSideLen - Side length of aDest array.  Accepts aSideLen = 2^n+1, for (4 <= n <= 10).
                  I.e. only interprets aDest buffer as a square.
*/
CFractalNoise::CFractalNoise(TFracVal* aDest, int aSideLen)
 : mDest(aDest), mSideLength(aSideLen)
{
	assert(mDest);
	assert(aSideLen == 17 || aSideLen == 33 || aSideLen == 65 || aSideLen == 129 || aSideLen == 257 || aSideLen == 513 || aSideLen == 1025);

	mMaxVal = 0;
	mMinVal = 0;
}


/*
Public interface to generate the fractal noise into the constructor-provided buffer.

@param aSeed - Seed to use.
@param aAdjust - The level to adjust heights by with each recursive step.
@param aFrequency - 'Frequency' of noise (roughness: suggest 1.2 to 3.0).
*/
void CFractalNoise::Generate(int aSeed, float aAdjust, float aFrequency)
{
	mMaxVal = MIN_FRACVAL;
	mMinVal = MAX_FRACVAL;

	mFrequency = aFrequency;
	srand(aSeed);

	// Set the four initial corners to random values within aAdjust.
	float adj = aAdjust / 2.0f;
	if (GetVal(0,0) == FRACVAL_UNINIT)
		SetVal(0, 0, TFracVal(FRAND(aAdjust)-adj));
	if (GetVal(mSideLength-1, 0) == FRACVAL_UNINIT)
		SetVal(mSideLength-1, 0, TFracVal(FRAND(aAdjust)-adj));
	if (GetVal(0, mSideLength-1) == FRACVAL_UNINIT)
		SetVal(0, mSideLength-1, TFracVal(FRAND(aAdjust)-adj));
	if (GetVal(mSideLength-1, mSideLength-1) == FRACVAL_UNINIT)
		SetVal(mSideLength-1, mSideLength-1, TFracVal(FRAND(aAdjust)-adj));

	// Initiate the recursive subdivision.
	Subdivide(0, 0, mSideLength-1, mSideLength-1, aAdjust/aFrequency);
}


// Recursive subdivision.
void CFractalNoise::Subdivide(int aLeft, int aTop, int aRight, int aBot, float aAdjust)
{
	// Assumes corners are set already.
	TFracVal tl = GetVal(aLeft, aTop);
	TFracVal tr = GetVal(aRight, aTop);
	TFracVal bl = GetVal(aLeft, aBot);
	TFracVal br = GetVal(aRight, aBot);

	int rnd;
	float adj = aAdjust/2.0f;	// For adjusting to +/-.

	int midX = aLeft + ((aRight - aLeft) >> 1);
	int midY = aTop  + ((aBot - aTop) >> 1);

	// Set vals for side mid points.
	TFracVal tm = GetVal(midX, aTop);
	if (tm == FRACVAL_UNINIT)
	{
		rnd = int((FRAND(aAdjust))-adj);
		tm = ((tl + tr)/2) + rnd;
		SetVal(midX, aTop, tm);		// top
	}

	TFracVal rm = GetVal(aRight, midY);
	if (rm == FRACVAL_UNINIT)
	{
		rnd = int((FRAND(aAdjust))-adj);
		rm = ((tr + br)/2) + rnd;
		SetVal(aRight, midY, rm);	// right
	}

	TFracVal bm = GetVal(midX, aBot);
	if (bm == FRACVAL_UNINIT)
	{
		rnd = int((FRAND(aAdjust))-adj);
		bm = ((bl + br)/2) + rnd;
		SetVal(midX, aBot, bm);		// bottom
	}

	TFracVal lm = GetVal(aLeft, midY);
	if (lm == FRACVAL_UNINIT)
	{
		rnd = int((FRAND(aAdjust))-adj);
		lm = ((tl + bl)/2) + rnd;
		SetVal(aLeft, midY, lm);	// left
	}

	// Adjust mid point.
	if (GetVal(midX, midY) == FRACVAL_UNINIT)
	{
		int avg = int((tm + rm + bm + lm) / 4.0);
		rnd = int((FRAND(aAdjust))-adj);
		SetVal(midX, midY, avg + rnd);
	}

	// Terminate recursion (assumes width==height=2^n+1 i.e. divisions are square).
	if (((aRight - aLeft) >> 1) == 1)
		return;

	// Recurse with four child 'pieces'.
	Subdivide(aLeft, aTop, midX, midY, aAdjust/mFrequency);		// Top-left.
	Subdivide(midX, aTop, aRight, midY, aAdjust/mFrequency);	// Top-right.
	Subdivide(aLeft, midY, midX, aBot, aAdjust/mFrequency);		// Bot-left.
	Subdivide(midX, midY, aRight, aBot, aAdjust/mFrequency);	// Bot-right.
}


// Sets the 'height' at (aX, aY) to the given value.
void CFractalNoise::SetVal(int aX, int aY, TFracVal aVal)
{
	assert(aX <= mSideLength  &&  aY <= mSideLength);
	mDest[aY*mSideLength+aX] = aVal;

	// Update max/min values.
	if (aVal > mMaxVal)
		mMaxVal = aVal;
	if (aVal < mMinVal)
		mMinVal = aVal;
}


// Returns the 'height' value at (aX, aY).
TFracVal CFractalNoise::GetVal(int aX, int aY) const
{
	assert(aX <= mSideLength  &&  aY <= mSideLength);
	return mDest[aY*mSideLength+aX];
}
