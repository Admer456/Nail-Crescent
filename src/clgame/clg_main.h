/*
// LICENSE HERE.

//
// clgame/clg_main.h
//
*/

#ifndef __CLGAME_MAIN_H__
#define __CLGAME_MAIN_H__

void CLG_Init();
void CLG_Shutdown(void);

void CLG_ClientDeltaFrame(void);
void CLG_ClientFrame(void);
void CLG_ClientBegin(void);
void CLG_ClientDisconnect(void);
void CLG_ClearState(void);
void CLG_DemoSeek(void);

void CLG_UpdateUserInfo(cvar_t* var, from_t from);

void Com_Print(const char* fmt, ...);
void Com_DPrint(const char* fmt, ...);
void Com_WPrint(const char* fmt, ...);
void Com_EPrint(const char* fmt, ...);
void Com_Error(error_type_t code, const char* fmt, ...);
void Com_LPrintf(print_type_t type, const char* fmt, ...);
#endif // __CLGAME_INPUT_H__