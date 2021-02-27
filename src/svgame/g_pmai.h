/*
// LICENSE HERE.

//
// g_pmai.h
//
//
// Contains the definitions of the import, and export API structs.
//
*/
#ifndef __G_PMAI_H__
#define __G_PMAI_H__

//
//=============================================================================
//
//	AI Configuration.
//
//=============================================================================
//-------------------
// Target flags, whether the target was found by sight, or hearing, or
// other possible methods.
//-------------------
enum {
	FL_AI_TARGET_HEARD,
	FL_AI_TARGET_SEEN,

	// MODDERS: You can add your own here if you wish to tweak the AI code.

	// This one is used in unknown cases, currently none.
	FL_AI_TARGET_OTHER
};


//
//=============================================================================
//
//	Player Movement AI structures.
//
//=============================================================================
//
//-------------------
// Configuration struct. Use this to configure the AI monster's settings.
//-------------------
typedef struct pmai_settings_s {
	// View related settings.
	struct {
		// The AI its viewheight from origin.
		int height;
	} view;

	// Range related settings.
	struct {
		// The maximum range the AI will accept for being triggered by audio.
		float max_hearing;

		// The maximum range the AI will accept for being triggered by
		// seeing a possible enemy.
		float max_sight;

		// The maximum range for melee combat.
		float melee;

		// Range for checking hostility enemies that are visible and in front.
		float hostility;

		// The minimal dot product required for visibility fov of a monster.
		float min_dot;
	} ranges;
} pmai_settings_t;

//-------------------
// The 'targets' struct stores the target states, such as which goal to move
// forward to, or which entity we last attacked etc.
//
// The 'target' struct stores the target, and whether we've heard it, or seen
// it.
//-------------------
typedef struct pmai_target_s {
	// An actual reference to the entity.
	edict_t* entity;

	// Whether we've heard the target, or seen it, or whatever else may
	// have triggered this target to be enlisted.
	int flags;
} pmai_target_t;

typedef struct pmai_targets_s {
	// The actual goal that the AI will try to head forward to.
	pmai_target_t goal;

	// This can be any enemy that hurt it, even a platform etc.
	pmai_target_t enemy;
} pmai_targets_t;

//-------------------
// Additional transform data state. To live aside of pmove which contains
// the actual physical transform.
//-------------------
//typedef struct pmai_addtransform_s {
//	float ideal_yaw;
//} pmai_addtransform_t;

//-------------------
// Current frame movement state.
//-------------------
typedef struct pmai_movement_s{
	// The actual user input being sent to the movement code.
	usercmd_t cmd;
} pmai_movement_t;

//-------------------
// Stores the actual animation data, start and end frame, and also
// the current frame where it is at.
//-------------------
typedef struct pmai_animation_s {
	// The first frame of the animation.
	int startframe;
	// The last frame of the animation.
	int endframe;
	// The current frame being played of this animation.
	int currentframe;
	// Should the animation loop?
	qboolean loop;

} pmai_animation_t;

//-------------------
// Stores a list of animations, these can be customized.
//-------------------
typedef struct pmai_animations_s {
	// The list of animations, 20 max for now..
	pmai_animation_t list[20];

	// The current animation which we are playing.
	int current;
} pmai_animations_t;


//-------------------
// This is the main Player Move AI structure.
// It contains all relevant states to work with for the PMAI system.
//-------------------
typedef struct {
	// The entity AI movement state.
	pmai_animations_t animations;

	// The entity AI movement state.
	pmai_movement_t movement;

	// The entity AI settings.
	pmai_settings_t settings;

	// The entity AI target state.
	pmai_targets_t targets;

	// The pmove params for this AI entity.
	pmoveParams_t pmp;

	// The pmove_t state for this AI entity.
	pmove_t pmove;
} pmai_t;


//
//=============================================================================
//
//	Functions.
//
//=============================================================================
//

//-------------------
// Utility.
//-------------------
void		PMAI_SetSightClient(void);

float		PMAI_EntityRange(edict_t* self, edict_t* other);

qboolean	PMAI_EntityIsVisible(edict_t* self, edict_t* other);
qboolean	PMAI_EntityIsInFront(edict_t* self, edict_t* other, float min_dot);

int			PMAI_BrushInFront(edict_t* self, float viewheight);
qboolean	PMAI_CheckEyes(edict_t* self, usercmd_t* ucmd);

//-------------------
// Animations.
//-------------------
void		PMAI_FillAnimation(edict_t* self, int animationID, int startFrame, int endFrame, qboolean loop);

//-------------------
// Movement.
//-------------------
void		PMAI_ProcessMovement(edict_t *self);

//-------------------
// Settings. 
//-------------------
void		PMAI_Initialize(edict_t* self);

//-------------------
// Target searching.
//-------------------
qboolean	PMAI_FindEnemyTarget(edict_t* self);

#endif // __G_PMAI_H__