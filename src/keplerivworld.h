//
//  keplerivworld.h
//  Wakuseibokan
//
//  Created by Rodrigo Ramele on 24/05/14.
//

#ifndef __mycarrier__keplerivworld__
#define __mycarrier__keplerivworld__

#include <iostream>



// *********************** ODE Controlss ********************************
/* some constants */

#define NUM 10			/* number of boxes */
#define SIDE (0.2)		/* side length of a box */
#define MASS (1.0)		/* mass of a box */
#define RADIdUS (0.1732f)	/* sphere radius */
#define RADIUS (1.0f)

#define kmf             *1000.0f
#define CYCLES_IN_SOL   10000

void nearCallback (void *data, dGeomID o1, dGeomID o2);
void _nearCallback (void *data, dGeomID o1, dGeomID o2);

void setupWorldModelling();
void initWorldModelling();
void initWorldModelling(int testcase);
void worldStep(int value);
void endWorldModelling();

void savegame();
void loadgame();

// **********************************************************************


enum gamemodes { ACTIONGAME, STRATEGYGAME };

enum tracemodes { NOTRACE, RECORD, REPLAY };

enum peermodes { CLIENT, SERVER };

#include "messages.h"


#endif /* defined(__mycarrier__keplerivworld__) */
