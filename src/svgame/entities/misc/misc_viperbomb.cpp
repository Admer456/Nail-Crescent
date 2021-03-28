// LICENSE HERE.

//
// svgame/entities/misc_viperbomb.c
//
//
// misc_viperbomb entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"   how much boom should the bomb make?
*/
void misc_viper_bomb_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    G_UseTargets(self, self->activator);

    self->s.origin.z = self->absmin.z + 1;
    T_RadiusDamage(self, self, self->dmg, NULL, self->dmg + 40, MOD_BOMB);
    BecomeExplosion2(self);
}

void misc_viper_bomb_prethink(edict_t* self)
{
    vec3_t  v;
    float   diff;

    self->groundentity = NULL;

    diff = self->timestamp - level.time;
    if (diff < -1.0)
        diff = -1.0;

    Vec3_Scale_(self->moveinfo.dir, 1.0 + diff, v);
    v.z = diff;

    diff = self->s.angles.z;
    vectoangles(v, self->s.angles);
    self->s.angles.z = diff + 10;
}

void misc_viper_bomb_use(edict_t* self, edict_t* other, edict_t* activator)
{
    edict_t* viper;

    self->solid = SOLID_BBOX;
    self->svflags &= ~SVF_NOCLIENT;
    self->s.effects |= EF_ROCKET;
    self->use = NULL;
    self->movetype = MOVETYPE_TOSS;
    self->prethink = misc_viper_bomb_prethink;
    self->touch = misc_viper_bomb_touch;
    self->activator = activator;

    viper = G_Find(NULL, FOFS(classname), "misc_viper");
    Vec3_Scale_(viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

    self->timestamp = level.time;
    Vec3_Copy_(viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb(edict_t* self)
{
    self->movetype = MOVETYPE_NONE;
    self->solid = SOLID_NOT;
    Vec3_Set_(self->mins, -8, -8, -8);
    Vec3_Set_(self->maxs, 8, 8, 8);

    self->s.modelindex = gi.modelindex("models/objects/bomb/tris.md2");

    if (!self->dmg)
        self->dmg = 1000;

    self->use = misc_viper_bomb_use;
    self->svflags |= SVF_NOCLIENT;

    gi.linkentity(self);
}