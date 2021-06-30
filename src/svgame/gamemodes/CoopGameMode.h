/*
// LICENSE HERE.

//
// CoopGameMode.h
//
// Default game mode to run, allows for all sorts of stuff.
//
*/
#ifndef __SVGAME_GAMEMODES_COOPGAMEMODE_H__
#define __SVGAME_GAMEMODES_COOPGAMEMODE_H__

#include "IGameMode.h"
#include "DefaultGameMode.h"

class CoopGameMode : public DefaultGameMode {
public:
    // Constructor/Deconstructor.
    CoopGameMode();
    virtual ~CoopGameMode() override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;

private:

};

#endif // __SVGAME_GAMEMODES_COOPGAMEMODE_H__