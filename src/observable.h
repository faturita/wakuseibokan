/*
 * observable.h
 *
 *  Created on: Feb 7, 2012
 *      Author: faturita
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "math/yamathutil.h"

class Observable
{
	void virtual getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward) = 0;
};

#endif /* OBSERVABLE_H_ */
