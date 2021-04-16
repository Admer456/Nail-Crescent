
// LICENSE HERE.

//
// clg_input.c
//
//
// Handles the game specific related input areas.
//
#include "clg_local.h"
#include "clg_input.h"

static cvar_t* m_filter;
cvar_t* m_accel;
cvar_t* m_autosens;

static cvar_t* cl_upspeed;
static cvar_t* cl_forwardspeed;
static cvar_t* cl_sidespeed;
static cvar_t* cl_yawspeed;
static cvar_t* cl_pitchspeed;
static cvar_t* cl_run;
static cvar_t* cl_anglespeedkey;

static cvar_t* cl_instantpacket;

static cvar_t* freelook;
static cvar_t* lookspring;
static cvar_t* lookstrafe;
cvar_t* sensitivity;

cvar_t* m_pitch;
cvar_t* m_invert;
cvar_t* m_yaw;
static cvar_t* m_forward;
static cvar_t* m_side;

typedef struct {
    int         old_dx;
    int         old_dy;
} in_state_t;
static in_state_t inputState;

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition


Key_Event (int key, qboolean down, unsigned time);

  +mlook src time

===============================================================================
*/


static KeyBinding    in_klook;
static KeyBinding in_left, in_right, in_forward, in_back;
static KeyBinding in_lookup, in_lookdown, in_moveleft, in_moveright;
static KeyBinding in_strafe, in_speed, in_use, in_attack;
static KeyBinding in_up, in_down;

static int          in_impulse;
static qboolean     in_mlooking;

void KeyDown(KeyBinding* b)
{
    int k;
    unsigned com_eventTime = clgi.Com_GetEventTime();
    const char* c; // C++20: STRING: Added const to char*

    c = clgi.Cmd_Argv(1);
    if (c[0])
        k = atoi(c);
    else
        k = -1;        // typed manually at the console for continuous down

    if (k == b->keys[0] || k == b->keys[1])
        return;        // repeating key

    if (!b->keys[0])
        b->keys[0] = k;
    else if (!b->keys[1])
        b->keys[1] = k;
    else {
        Com_WPrint("Three keys down for a button!\n");
        return;
    }

    if (b->state & BUTTON_STATE_HELD)
        return;        // still down

    // save timestamp
    c = clgi.Cmd_Argv(2);
    b->downtime = atoi(c);
    if (!b->downtime) {
        b->downtime = com_eventTime - 100;
    }

    b->state |= BUTTON_STATE_HELD + BUTTON_STATE_DOWN;    // down + impulse down
}

void KeyUp(KeyBinding* b)
{
    int k;
    const char* c; // C++20: STRING: Added const to char*
    unsigned uptime;

    c = clgi.Cmd_Argv(1);
    if (c[0])
        k = atoi(c);
    else {
        // typed manually at the console, assume for unsticking, so clear all
        b->keys[0] = b->keys[1] = 0;
        b->state = 0;    // impulse up
        return;
    }

    if (b->keys[0] == k)
        b->keys[0] = 0;
    else if (b->keys[1] == k)
        b->keys[1] = 0;
    else
        return;        // key up without coresponding down (menu pass through)
    if (b->keys[0] || b->keys[1])
        return;        // some other key is still holding it down

    if (!(b->state & BUTTON_STATE_HELD))
        return;        // still up (this should not happen)

    // save timestamp
    c = clgi.Cmd_Argv(2);
    uptime = atoi(c);
    if (!uptime) {
        b->msec += 10;
    }
    else if (uptime > b->downtime) {
        b->msec += uptime - b->downtime;
    }

    b->state &= ~BUTTON_STATE_HELD;        // now up
}

static void KeyClear(KeyBinding* b)
{
    unsigned com_eventTime = clgi.Com_GetEventTime();
    b->msec = 0;
    b->state &= ~BUTTON_STATE_DOWN;        // clear impulses
    if (b->state & BUTTON_STATE_HELD) {
        b->downtime = com_eventTime; // still down
    }
}

static void IN_KLookDown(void) { KeyDown(&in_klook); }
static void IN_KLookUp(void) { KeyUp(&in_klook); }
static void IN_UpDown(void) { KeyDown(&in_up); }
static void IN_UpUp(void) { KeyUp(&in_up); }
static void IN_DownDown(void) { KeyDown(&in_down); }
static void IN_DownUp(void) { KeyUp(&in_down); }
static void IN_LeftDown(void) { KeyDown(&in_left); }
static void IN_LeftUp(void) { KeyUp(&in_left); }
static void IN_RightDown(void) { KeyDown(&in_right); }
static void IN_RightUp(void) { KeyUp(&in_right); }
static void IN_ForwardDown(void) { KeyDown(&in_forward); }
static void IN_ForwardUp(void) { KeyUp(&in_forward); }
static void IN_BackDown(void) { KeyDown(&in_back); }
static void IN_BackUp(void) { KeyUp(&in_back); }
static void IN_LookupDown(void) { KeyDown(&in_lookup); }
static void IN_LookupUp(void) { KeyUp(&in_lookup); }
static void IN_LookdownDown(void) { KeyDown(&in_lookdown); }
static void IN_LookdownUp(void) { KeyUp(&in_lookdown); }
static void IN_MoveleftDown(void) { KeyDown(&in_moveleft); }
static void IN_MoveleftUp(void) { KeyUp(&in_moveleft); }
static void IN_MoverightDown(void) { KeyDown(&in_moveright); }
static void IN_MoverightUp(void) { KeyUp(&in_moveright); }
static void IN_SpeedDown(void) { KeyDown(&in_speed); }
static void IN_SpeedUp(void) { KeyUp(&in_speed); }
static void IN_StrafeDown(void) { KeyDown(&in_strafe); }
static void IN_StrafeUp(void) { KeyUp(&in_strafe); }

static void IN_AttackDown(void)
{
    KeyDown(&in_attack);

    if (cl_instantpacket->integer && clgi.GetClienState() == ca_active) {// && cls->netchan) {
        cl->sendPacketNow = true;
    }
}

static void IN_AttackUp(void)
{
    KeyUp(&in_attack);
}

static void IN_UseDown(void)
{
    KeyDown(&in_use);

    if (cl_instantpacket->integer && clgi.GetClienState() == ca_active) {// && cls.netchan) {
        cl->sendPacketNow = true;
    }
}

static void IN_UseUp(void)
{
    KeyUp(&in_use);
}

static void IN_Impulse(void)
{
    in_impulse = atoi(clgi.Cmd_Argv(1));
}

static void IN_CenterView(void)
{
    cl->viewAngles.x = -SHORT2ANGLE(cl->frame.playerState.pmove.delta_angles[0]);
}

static void IN_MLookDown(void)
{
    in_mlooking = true;
}

static void IN_MLookUp(void)
{
    in_mlooking = false;

    if (!freelook->integer && lookspring->integer)
        IN_CenterView();
}

/*
===============
CL_KeyState

Returns the fraction of the frame that the key was down
===============
*/
static float CL_KeyState(KeyBinding* key)
{
    unsigned com_eventTime = clgi.Com_GetEventTime();
    unsigned msec = key->msec;
    float val;

    if (key->state & BUTTON_STATE_HELD) {
        // still down
        if (com_eventTime > key->downtime) {
            msec += com_eventTime - key->downtime;
        }
    }

    // special case for instant packet
    if (!cl->cmd.msec) {
        return (float)(key->state & BUTTON_STATE_HELD);
    }

    val = (float)msec / cl->cmd.msec;

    return Clampf(val, 0, 1);
}



//==========================================================================
/*
================
CL_MouseMove
================
*/
static void CLG_MouseMove() {
    int deltaX, deltaY;
    float motionX, motionY;
    float speed;

    if (!clgi.Mouse_GetMotion(&deltaX, &deltaY)) {
        return;
    }
    if (clgi.Key_GetDest() & (KEY_MENU | KEY_CONSOLE)) {
        return;
    }

    if (m_filter->integer) {
        motionX = (deltaX + inputState.old_dx) * 0.5f;
        motionY = (deltaY + inputState.old_dy) * 0.5f;
    }
    else {
        motionX = deltaX;
        motionY = deltaY;
    }

    inputState.old_dx = deltaX;
    inputState.old_dy = deltaY;

    if (!motionX && !motionY) {
        return;
    }

    clgi.Cvar_ClampValue(m_accel, 0, 1);

    speed = std::sqrtf(motionX * motionX + motionY * motionY);
    speed = sensitivity->value + speed * m_accel->value;

    motionX *= speed;
    motionY *= speed;

    if (m_autosens->integer) {
        motionX *= cl->fov_x * cl->autosens_x;
        motionY *= cl->fov_y * cl->autosens_y;
    }

    // add mouse X/Y movement
    if ((in_strafe.state & 1) || (lookstrafe->integer && !in_mlooking)) {
        cl->mousemove[1] += m_side->value * motionX;
    }
    else {
        cl->viewAngles[vec3_t::Yaw] -= m_yaw->value * motionX;
    }

    if ((in_mlooking || freelook->integer) && !(in_strafe.state & 1)) {
        cl->viewAngles[vec3_t::Pitch] += m_pitch->value * motionY * (m_invert->integer ? -1.f : 1.f);
    }
    else {
        cl->mousemove[0] -= m_forward->value * motionY;
    }
}


/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
static void CL_AdjustAngles(int msec)
{
    float speed;

    if (in_speed.state & BUTTON_STATE_HELD)
        speed = msec * cl_anglespeedkey->value * 0.001f;
    else
        speed = msec * 0.001f;

    if (!(in_strafe.state & 1)) {
        cl->viewAngles[vec3_t::Yaw] -= speed * cl_yawspeed->value * CL_KeyState(&in_right);
        cl->viewAngles[vec3_t::Yaw] += speed * cl_yawspeed->value * CL_KeyState(&in_left);
    }
    if (in_klook.state & 1) {
        cl->viewAngles[vec3_t::Pitch] -= speed * cl_pitchspeed->value * CL_KeyState(&in_forward);
        cl->viewAngles[vec3_t::Pitch] += speed * cl_pitchspeed->value * CL_KeyState(&in_back);
    }

    cl->viewAngles[vec3_t::Pitch] -= speed * cl_pitchspeed->value * CL_KeyState(&in_lookup);
    cl->viewAngles[vec3_t::Pitch] += speed * cl_pitchspeed->value * CL_KeyState(&in_lookdown);
}

/*
================
CL_BaseMove

Build and return the intended movement vector
================
*/
static vec3_t CL_BaseMove(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    if (in_strafe.state & 1) {
        outMove[1] += cl_sidespeed->value * CL_KeyState(&in_right);
        outMove[1] -= cl_sidespeed->value * CL_KeyState(&in_left);
    }

    outMove[1] += cl_sidespeed->value * CL_KeyState(&in_moveright);
    outMove[1] -= cl_sidespeed->value * CL_KeyState(&in_moveleft);

    outMove[2] += cl_upspeed->value * CL_KeyState(&in_up);
    outMove[2] -= cl_upspeed->value * CL_KeyState(&in_down);

    if (!(in_klook.state & 1)) {
        outMove[0] += cl_forwardspeed->value * CL_KeyState(&in_forward);
        outMove[0] -= cl_forwardspeed->value * CL_KeyState(&in_back);
    }

    // Adjust for speed key / running
    //if ((in_speed.state & 1) ^ cl_run->integer) {
    //    cl->cmd.buttons |= BUTTON_WALK;
    //    //VectorScale(outMove, 2, outMove);
    //    //Com_Print("HELLO IN_SPEED");
    //}
    //else {

    //}

    return outMove;
}

/*
================
CL_ClampSpeed

Returns the clamped movement speeds.
================
*/
static vec3_t CL_ClampSpeed(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    //float speed = cl_forwardspeed->value; // TODO: FIX PM_ //pmoveParams->maxspeed;

    //outMove[0] = Clampf(outMove[0], -speed, speed);
    //outMove[1] = Clampf(outMove[1], -speed, speed);
    //outMove[2] = Clampf(outMove[2], -speed, speed);

    return outMove;
}

static void CL_ClampPitch(void)
{
    float pitch;

    pitch = SHORT2ANGLE(cl->frame.playerState.pmove.delta_angles[vec3_t::Pitch]);
    if (pitch > 180)
        pitch -= 360;

    if (cl->viewAngles[vec3_t::Pitch] + pitch < -360)
        cl->viewAngles[vec3_t::Pitch] += 360; // wrapped
    if (cl->viewAngles[vec3_t::Pitch] + pitch > 360)
        cl->viewAngles[vec3_t::Pitch] -= 360; // wrapped

    if (cl->viewAngles[vec3_t::Pitch] + pitch > 89)
        cl->viewAngles[vec3_t::Pitch] = 89 - pitch;
    if (cl->viewAngles[vec3_t::Pitch] + pitch < -89)
        cl->viewAngles[vec3_t::Pitch] = -89 - pitch;
}

/*
=================
CLG_BuildFrameMoveCommand

Updates msec, angles and builds interpolated movement vector for local prediction.
Doesn't touch command forward/side/upmove, these are filled by CLG_BuildMovementCommand.
=================
*/
void CLG_BuildFrameMoveCommand(int msec)
{
    cl->localmove = vec3_zero();

    if (sv_paused->integer) {
        return;
    }

    // Add to milliseconds of time to apply the move
    cl->cmd.msec += msec;

    // Adjust viewAngles
    CL_AdjustAngles(msec);

    // Get basic movement from keyboard
    cl->localmove = CL_BaseMove(cl->localmove);

    // Allow mice to add to the move
    CLG_MouseMove();

    // Add accumulated mouse forward/side movement
    cl->localmove[0] += cl->mousemove[0];
    cl->localmove[1] += cl->mousemove[1];

    // Clamp to server defined max speed
    cl->localmove = CL_ClampSpeed(cl->localmove);

    CL_ClampPitch();

    cl->cmd.angles[0] = ANGLE2SHORT(cl->viewAngles[0]);
    cl->cmd.angles[1] = ANGLE2SHORT(cl->viewAngles[1]);
    cl->cmd.angles[2] = ANGLE2SHORT(cl->viewAngles[2]);
}

//
//===============
// CLG_RegisterInput
// 
// Registered input messages and binds them to a callback function.
// Bindings are set in the config files, or the options menu.
// 
// For more information, it still works like in q2pro.
//===============
//
void CLG_RegisterInput(void)
{
    // Register Input Key bind Commands.
    clgi.Cmd_AddCommand("centerview", IN_CenterView);
    clgi.Cmd_AddCommand("+moveup", IN_UpDown);
    clgi.Cmd_AddCommand("-moveup", IN_UpUp);
    clgi.Cmd_AddCommand("+movedown", IN_DownDown);
    clgi.Cmd_AddCommand("-movedown", IN_DownUp);
    clgi.Cmd_AddCommand("+left", IN_LeftDown);
    clgi.Cmd_AddCommand("-left", IN_LeftUp);
    clgi.Cmd_AddCommand("+right", IN_RightDown);
    clgi.Cmd_AddCommand("-right", IN_RightUp);
    clgi.Cmd_AddCommand("+forward", IN_ForwardDown);
    clgi.Cmd_AddCommand("-forward", IN_ForwardUp);
    clgi.Cmd_AddCommand("+back", IN_BackDown);
    clgi.Cmd_AddCommand("-back", IN_BackUp);
    clgi.Cmd_AddCommand("+lookup", IN_LookupDown);
    clgi.Cmd_AddCommand("-lookup", IN_LookupUp);
    clgi.Cmd_AddCommand("+lookdown", IN_LookdownDown);
    clgi.Cmd_AddCommand("-lookdown", IN_LookdownUp);
    clgi.Cmd_AddCommand("+strafe", IN_StrafeDown);
    clgi.Cmd_AddCommand("-strafe", IN_StrafeUp);
    clgi.Cmd_AddCommand("+moveleft", IN_MoveleftDown);
    clgi.Cmd_AddCommand("-moveleft", IN_MoveleftUp);
    clgi.Cmd_AddCommand("+moveright", IN_MoverightDown);
    clgi.Cmd_AddCommand("-moveright", IN_MoverightUp);
    clgi.Cmd_AddCommand("+speed", IN_SpeedDown);
    clgi.Cmd_AddCommand("-speed", IN_SpeedUp);
    clgi.Cmd_AddCommand("+attack", IN_AttackDown);
    clgi.Cmd_AddCommand("-attack", IN_AttackUp);
    clgi.Cmd_AddCommand("+use", IN_UseDown);
    clgi.Cmd_AddCommand("-use", IN_UseUp);
    clgi.Cmd_AddCommand("impulse", IN_Impulse);
    clgi.Cmd_AddCommand("+klook", IN_KLookDown);
    clgi.Cmd_AddCommand("-klook", IN_KLookUp);
    clgi.Cmd_AddCommand("+mlook", IN_MLookDown);
    clgi.Cmd_AddCommand("-mlook", IN_MLookUp);

    // Create Cvars.
    cl_upspeed = clgi.Cvar_Get("cl_upspeed", "300", 0);
    cl_forwardspeed = clgi.Cvar_Get("cl_forwardspeed", "300", 0);
    cl_sidespeed = clgi.Cvar_Get("cl_sidespeed", "300", 0);
    cl_yawspeed = clgi.Cvar_Get("cl_yawspeed", "0.140", 0);
    cl_pitchspeed = clgi.Cvar_Get("cl_pitchspeed", "0.150", CVAR_CHEAT);
    cl_anglespeedkey = clgi.Cvar_Get("cl_anglespeedkey", "1.5", CVAR_CHEAT);
    cl_run = clgi.Cvar_Get("cl_run", "1", CVAR_ARCHIVE);

    freelook = clgi.Cvar_Get("freelook", "1", CVAR_ARCHIVE);
    lookspring = clgi.Cvar_Get("lookspring", "0", CVAR_ARCHIVE);
    lookstrafe = clgi.Cvar_Get("lookstrafe", "0", CVAR_ARCHIVE);

    // Fetch CVars.
    cl_instantpacket = clgi.Cvar_Get("cl_instantpacket", "0", 0);

    sensitivity = clgi.Cvar_Get("sensitivity", "0", 0);

    m_autosens = clgi.Cvar_Get("m_autosens", "0", 0);
    m_pitch = clgi.Cvar_Get("m_pitch", "", 0);
    m_invert = clgi.Cvar_Get("m_invert", "", 0);
    m_yaw = clgi.Cvar_Get("m_yaw", "", 0);
    m_forward = clgi.Cvar_Get("m_forward", "", 0);
    m_side = clgi.Cvar_Get("m_side", "", 0);
    m_filter = clgi.Cvar_Get("m_filter", "", 0);
    m_accel = clgi.Cvar_Get("m_accel", "", 0);
}

/*
=================
CLG_FinalizeFrameMoveCommand

Builds the actual movement vector for sending to server. Assumes that msec
and angles are already set for this frame by CL_UpdateCmd.
=================
*/
void CLG_FinalizeFrameMoveCommand(void)
{
    if (clgi.GetClienState() != ca_active) {
        return; // not talking to a server
    }

    if (sv_paused->integer) {
        return;
    }

    //
    // figure button bits
    //
    if (in_attack.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN))
        cl->cmd.buttons |= BUTTON_ATTACK;

    if (in_use.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN))
        cl->cmd.buttons |= BUTTON_USE;

    if (cl_run->value) {
        if (in_speed.state & BUTTON_STATE_HELD) {
            cl->cmd.buttons |= BUTTON_WALK;
        }
    }
    else {
        if (!(in_speed.state & BUTTON_STATE_HELD)) {
            cl->cmd.buttons |= BUTTON_WALK;
        }
    }

    in_attack.state &= ~2;
    in_use.state &= ~2;

    if (clgi.Key_GetDest() == KEY_GAME && clgi.Key_AnyKeyDown()) {
        cl->cmd.buttons |= BUTTON_ANY;
    }

    if (cl->cmd.msec > 250) {
        cl->cmd.msec = 100;        // time was unreasonable
    }

    // rebuild the movement vector
    vec3_t move = vec3_zero();

    // get basic movement from keyboard
    move = CL_BaseMove(move);

    // add mouse forward/side movement
    move[0] += cl->mousemove[0];
    move[1] += cl->mousemove[1];

    // clamp to server defined max speed
    move = CL_ClampSpeed(move);

    // store the movement vector
    cl->cmd.forwardmove = move[0];
    cl->cmd.sidemove = move[1];
    cl->cmd.upmove = move[2];

    // clear all states
    cl->mousemove[0] = 0;
    cl->mousemove[1] = 0;

    KeyClear(&in_right);
    KeyClear(&in_left);

    KeyClear(&in_moveright);
    KeyClear(&in_moveleft);

    KeyClear(&in_up);
    KeyClear(&in_down);

    KeyClear(&in_forward);
    KeyClear(&in_back);

    KeyClear(&in_lookup);
    KeyClear(&in_lookdown);

    cl->cmd.impulse = in_impulse;
    in_impulse = 0;

    // save this command off for prediction
    cl->cmdNumber++;
    cl->cmds[cl->cmdNumber & CMD_MASK] = cl->cmd;

    // clear pending cmd
    memset(&cl->cmd, 0, sizeof(cl->cmd));
}