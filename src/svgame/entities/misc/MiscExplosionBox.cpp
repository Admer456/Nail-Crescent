/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "../../g_local.h"          // SVGame.
#include "../../effects.h"          // Effects.
#include "../../utils.h"            // Util funcs.
#include "../../physics/stepmove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../base/SVGBaseEntity.h"

// Misc Explosion Box Entity.
#include "MiscExplosionBox.h"



//
// Constructor/Deconstructor.
//
MiscExplosionBox::MiscExplosionBox(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
MiscExplosionBox::~MiscExplosionBox() {

}



//
// Interface functions. 
//
//
//===============
// MiscExplosionBox::Precache
//
//===============
//
void MiscExplosionBox::Precache() {
    // Precache actual barrel model.
    SVG_PrecacheModel("models/objects/barrels/tris.md2");

    // Precache the debris.
    SVG_PrecacheModel("models/objects/debris1/tris.md2");
    SVG_PrecacheModel("models/objects/debris2/tris.md2");
    SVG_PrecacheModel("models/objects/debris3/tris.md2");

    //gi.DPrintf("MiscExplosionBox::Precache();");
}

//
//===============
// MiscExplosionBox::Spawn
//
//===============
//
void MiscExplosionBox::Spawn() {
    // Ensure we call the base spawn function.
    SVGBaseEntity::Spawn();

    // Set solid.
    SetSolid(Solid::BoundingBox);

    // Set move type.
    SetMoveType(MoveType::Step);

    // Set the barrel model, and model index.
    SetModel("models/objects/barrels/tris.md2");

    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        {
            -16, -16, 0
        },
        // Maxs.
        {
            16, 16, 40
        }
    );

    // Set default values in case we have none.
    if (!GetMass()) {
        SetMass(400);
    }
    if (!GetHealth()) {
        SetHealth(10);
    }
    if (!GetDamage())
        SetDamage(150);

    // Set entity to allow taking damage (can't explode otherwise.)
    SetTakeDamage(TakeDamage::Yes);

    // Setup our MiscExplosionBox callbacks.
    SetThinkCallback(&MiscExplosionBox::MiscExplosionBoxThink);
    SetDieCallback(&MiscExplosionBox::MiscExplosionBoxDie);
    SetTouchCallback(&MiscExplosionBox::MiscExplosionBoxTouch);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2 * FRAMETIME);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscExplosionBox::PostSpawn
//
//===============
//
void MiscExplosionBox::PostSpawn() {
	//gi.DPrintf("MiscExplosionBox::PostSpawn();");
}

//
//===============
// MiscExplosionBox::Think
//
//===============
//
void MiscExplosionBox::Think() {
    SVGBaseEntity::Think();
	//gi.DPrintf("MiscExplosionBox::Think();");
}


//
// Callback Functions.
//
//
//===============
// MiscExplosionBox::MiscExplosionBoxThink
//
// Think callback, to execute the needed physics for this pusher object.
//===============
//
void MiscExplosionBox::MiscExplosionBoxThink(void) {
    // Calculate trace end position.
    vec3_t end = GetOrigin() + vec3_t { 
        0.f, 
        0.f, 
        1.f 
    };

    // Set origin + 1 on the Z axis.
    SetOrigin(GetOrigin() + vec3_t{ 
        0.f, 
        0.f, 
        1.f }
    );
    
    // Calculate the end point for tracing.
    end = GetOrigin() + vec3_t { 
        0.f, 
        0.f, 
        256.f 
    };

    // Execute the trace.
    trace_t trace = gi.Trace(GetOrigin(), GetMins(), GetMaxs(), end, GetServerEntity(), CONTENTS_MASK_MONSTERSOLID);

    // Return in case of fraction 1 or allSolid.
    if (trace.fraction == 1 || trace.allSolid) {
        return;
    }

    // Set origin to the trace end position.
    SetOrigin(trace.endPosition);

    if (GetServerEntity()->state.number == 12) {
        gi.DPrintf("I think, therefor, as a misc_explobox I AM!\n");
    }
    
    // Link entity for collision testing.
    LinkEntity();

    // Do a check ground for the step move of this pusher.
    SVG_StepMove_CheckGround(this);
    //M_CatagorizePosition(ent);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxExplode
//
// 'Think' callback that is set when the explosion box is exploding.
// (Has died due to taking damage.)
//===============
//
void MiscExplosionBox::MiscExplosionBoxExplode(void)
{
    // Execute radius damage.
    SVG_RadiusDamage(this, GetActivator(), GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);

    // Retrieve origin.
    vec3_t save = GetOrigin();

    // Set the new origin.
    SetOrigin(vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize()));

    // Throw several "debris1/tris.md2" chunks.
    SpawnDebris1Chunk();
    SpawnDebris1Chunk();

    // Bottom corners
    vec3_t origin = GetAbsoluteMin();
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);

    // Spawn 8 "debris2/tris.md2" chunks.
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    
    // Reset origin to saved origin.
    SetOrigin(save);

    // Depending on whether we have a ground entity or not, we determine which explosion to use.
    if (GetServerEntity()->groundEntityPtr)
        BecomeExplosion2(GetServerEntity());
    else
        BecomeExplosion1(GetServerEntity());

    // Ensure we have no more think callback pointer set when this entity has "died"
    SetThinkCallback(nullptr);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscExplosionBox::MiscExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);
    
    // Attacker becomes this entity its "activator".
    if (attacker)
        SetActivator(attacker);

    // Setup the next think and think time.
    SetNextThinkTime(level.time + 2 * FRAMETIME);

    // Set think function.
    SetThinkCallback(&MiscExplosionBox::MiscExplosionBoxExplode);
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void MiscExplosionBox::MiscExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // Safety checks.
    if (!self)
        return;
    if (!other)
        return;

    // Ground entity checks.
    if ((!other->GetServerEntity()->groundEntityPtr) || (other->GetServerEntity()->groundEntityPtr == GetServerEntity()))
        return;

    // Calculate ratio to use.
    float ratio = (float)other->GetMass() / (float)GetMass();
    
    // Calculate direction.
    vec3_t dir = GetOrigin() - other->GetOrigin();

    // Calculate yaw to use based on direction.
    float yaw = vec3_to_yaw(dir);

    // Last but not least, move a step ahead.
    SVG_StepMove_Walk(this, yaw, 20 * ratio * FRAMETIME);
    gi.DPrintf("self: '%i' is TOUCHING other: '%i'\n", self->GetServerEntity()->state.number, other->GetServerEntity()->state.number);
}


//
//===============
// MiscExplosionBox::SpawnDebris1Chunk
// 
// Function to spawn "debris1/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris1Chunk() {
    // Speed to throw debris at.
    float speed = 1.5 * (float)GetDamage() / 200.0f;

    // Calculate random direction vector.
    vec3_t randomDirection = {
        crandom(),
        crandom(),
        crandom()
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Throw debris!
    SVG_ThrowDebris(this, "models/objects/debris1/tris.md2", speed, origin);
}


//
//===============
// MiscExplosionBox::SpawnDebris2Chunk
//
// Function to spawn "debris2/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris2Chunk() {
    // Speed to throw debris at.
    float speed = 2.f * GetDamage() / 200.f;
    
    // Calculate random direction vector.
    vec3_t randomDirection = { 
        crandom(), 
        crandom(), 
        crandom() 
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Last but not least, throw debris.
    SVG_ThrowDebris(this, "models/objects/debris2/tris.md2", speed, origin);
}

//
//===============
// MiscExplosionBox::SpawnDebris3Chunk
// 
// Function to spawn "debris3/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris3Chunk(const vec3_t &origin) {
    // Speed to throw debris at.
    float speed = 1.75 * (float)GetDamage() / 200.0f;

    // Throw debris!
    SVG_ThrowDebris(this, "models/objects/debris3/tris.md2", speed, origin);
}
