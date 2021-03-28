// LICENSE HERE.

//
// svgame/entities/misc_blackhole.c
//
//
// misc_blackhole entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use(edict_t* ent, edict_t* other, edict_t* activator)
{
    /*
    gi.WriteByte (svg_temp_entity);
    gi.WriteByte (TE_BOSSTPORT);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PVS);
    */
    G_FreeEdict(ent);
}

void misc_blackhole_think(edict_t* self)
{
    if (++self->s.frame < 19)
        self->nextthink = level.time + FRAMETIME;
    else {
        self->s.frame = 0;
        self->nextthink = level.time + FRAMETIME;
    }
}

void SP_misc_blackhole(edict_t* ent)
{
    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_NOT;
    Vec3_Set_(ent->mins, -64, -64, 0);
    Vec3_Set_(ent->maxs, 64, 64, 8);
    ent->s.modelindex = gi.modelindex("models/objects/black/tris.md2");
    ent->s.renderfx = RF_TRANSLUCENT;
    ent->use = misc_blackhole_use;
    ent->think = misc_blackhole_think;
    ent->nextthink = level.time + 2 * FRAMETIME;
    gi.linkentity(ent);
}