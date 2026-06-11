#ifndef SAVEGAME_H
#define SAVEGAME_H

#include <string>

void savegame(std::string);

// Returns false if the file is missing, corrupted, or fails the CRC integrity check
// (in that case nothing is loaded into the world).
bool loadgame(std::string);


#endif // SAVEGAME_H
