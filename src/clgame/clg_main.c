// LICENSE HERE.

//
// clg_main.c
//
//
// Handles the main initialisation of the client game dll.
// 
// Initialises the appropriate cvars, precaches data, and handles
// shutdowns.
//
#include "clg_local.h"

// Contains the function pointers being passed in from the engine.
clg_import_t clgi;

//
//=============================================================================
//
//	CGAME API
//
//=============================================================================
//
#ifdef __cplusplus
extern "C" {
#endif

void CLG_StartServerMessage(void) {

}
qboolean CLG_ParseServerMessage(int serverCommand) {

}
void CLG_EndServerMessage(int realTime) {

}

clg_export_t *GetClientGameAPI (clg_import_t *clgimp)
{
    // Static export variable, lives as long as the client game dll lives.
    static clg_export_t	clge;

    // Store a copy of the engine imported function pointer struct.
    clgi = *clgimp;

    // Setup the API version.
    clge.apiversion = CGAME_API_VERSION;

    // Setup the game export function pointers.
    // Core.
    clge.Init                       = CLG_Init;
    clge.Shutdown                   = CLG_Shutdown;

    // ServerMessage.
    clge.StartServerMessage         = CLG_StartServerMessage;
    clge.ParseServerMessage         = CLG_ParseServerMessage;
    clge.EndServerMessage           = CLG_EndServerMessage;

    // Return cgame function pointer struct.
    return &clge;
}

#ifdef __cplusplus
}; // Extern "C"
#endif


//
//=============================================================================
//
//	CGAME INIT AND SHUTDOWN
//
//=============================================================================
//
//
//===============
// CLG_Init
// 
// Handles the initialisation of the client game dll.
//===============
//
void CLG_Init(void) {
    // Begin init log.
    Com_Print("\n%s\n", "==== InitCGame ====");

    // Execute tests.
    CLG_ExecuteTests();
}

//
//===============
// CLG_Shutdown
// 
// Handles the shutdown of the client game dll.
// ===============
//
void CLG_Shutdown(void) {
    // Begin shutdown log.
    Com_Print("\n%s\n", "==== ShutdownCGame ====");
}


//
//=============================================================================
//
//	COMMON
//
//=============================================================================
// Prints a message of type PRINT_ALL. Using variable arg formatting.
void Com_Print(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ALL, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_DEVELOPER. Using variable arg formatting.
// Only prints when developer cvar is set to 1.
void Com_DPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_DEVELOPER, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_WARNING. Using variable arg formatting.
void Com_WPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_WARNING, "%s", buffer);
    va_end (args);
}
// Prints a message of type PRINT_ERROR. Using variable arg formatting.
void Com_EPrint(char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(PRINT_ERROR, "%s", buffer);
    va_end (args);
}

// Triggers an error code of type. Using variable arg formatting.
void Com_Error (error_type_t code, char *fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start (args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_Error(code, "%s", buffer);
    va_end (args);
}

// This is mainly here to support shared/shared.c, do not use it yourself
// unless you know what you are doing.
// Prints a message of a type of your own liking. Using variable arg formatting
void Com_LPrintf(print_type_t type, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    clgi.Com_LPrintf(type, "%s", buffer);
    va_end(args);
}