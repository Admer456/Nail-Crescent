/*
// LICENSE HERE.

//
// clgame/clg_predict.h
//
*/

#ifndef __CLGAME_PREDICT_H__
#define __CLGAME_PREDICT_H__

void CLG_CheckPredictionError(cl_cmd_t* clientUserCommand);
void CLG_PredictAngles(void);
void CLG_PredictMovement(unsigned int ack, unsigned int current);

#endif // __CLGAME_PREDICT_H__