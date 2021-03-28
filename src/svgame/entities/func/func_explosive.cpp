// LICENSE HERE.

//
// svgame/entities/v.c
//
//
// func_explosive entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
    vec3_t  origin;
    vec3_t  chunkorigin;
    vec3_t  size;
    int     count;
    int     mass;

    // bmodel origins are (0 0 0), we need to adjust that here
    Vec3_Scale(self->size, 0.5, size);
    Vec3_Add(self->absmin, size, origin);
    Vec3_Copy_(origin, self->s.origin);

    self->takedamage = DAMAGE_NO;

    if (self->dmg)
        T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);

    Vec3_Subtract(self->s.origin, inflictor->s.origin, self->velocity);
    VectorNormalize(self->velocity);
    Vec3_Scale(self->velocity, 150, self->velocity);

    // start chunks towards the center
    Vec3_Scale(size, 0.5, size);

    mass = self->mass;
    if (!mass)
        mass = 75;

    // big chunks
    if (mass >= 100) {
        count = mass / 100;
        if (count > 8)
            count = 8;
        while (count--) {
            chunkorigin[0] = origin[0] + crandom() * size[0];
            chunkorigin[1] = origin[1] + crandom() * size[1];
            chunkorigin[2] = origin[2] + crandom() * size[2];
            ThrowDebris(self, "models/objects/debris1/tris.md2", 1, chunkorigin);
        }
    }

    // small chunks
    count = mass / 25;
    if (count > 16)
        count = 16;
    while (count--) {
        chunkorigin[0] = origin[0] + crandom() * size[0];
        chunkorigin[1] = origin[1] + crandom() * size[1];
        chunkorigin[2] = origin[2] + crandom() * size[2];
        ThrowDebris(self, "models/objects/debris2/tris.md2", 2, chunkorigin);
    }

    G_UseTargets(self, attacker);

    if (self->dmg)
        BecomeExplosion1(self);
    else
        G_FreeEdict(self);
}

void func_explosive_use(edict_t* self, edict_t* other, edict_t* activator)
{
    func_explosive_explode(self, other, activator, self->health, vec3_origin);
}

void func_explosive_spawn(edict_t* self, edict_t* other, edict_t* activator)
{
    self->solid = SOLID_BSP;
    self->svflags &= ~SVF_NOCLIENT;
    self->use = NULL;
    KillBox(self);
    gi.linkentity(self);
}

void SP_func_explosive(edict_t* self)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(self);
        return;
    }

    self->movetype = MOVETYPE_PUSH;

    gi.modelindex("models/objects/debris1/tris.md2");
    gi.modelindex("models/objects/debris2/tris.md2");

    gi.setmodel(self, self->model);

    if (self->spawnflags & 1) {
        self->svflags |= SVF_NOCLIENT;
        self->solid = SOLID_NOT;
        self->use = func_explosive_spawn;
    }
    else {
        self->solid = SOLID_BSP;
        if (self->targetname)
            self->use = func_explosive_use;
    }

    if (self->spawnflags & 2)
        self->s.effects |= EF_ANIM_ALL;
    if (self->spawnflags & 4)
        self->s.effects |= EF_ANIM_ALLFAST;

    if (self->use != func_explosive_use) {
        if (!self->health)
            self->health = 100;
        self->die = func_explosive_explode;
        self->takedamage = DAMAGE_YES;
    }

    gi.linkentity(self);
}