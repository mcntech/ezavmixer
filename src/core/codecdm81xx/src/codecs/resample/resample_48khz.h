#ifndef __RESAMPLE_48_KZ__
#define __RESAMPLE_48_KZ__
#include "typedefs.h"
typedef struct {
  int32_t S_8_16[8];
  int32_t S_16_12[8];
  int32_t S_12_24[8];
  int32_t S_24_48[8];
} WebRtcSpl_State8khzTo48khz;

void WebRtcSpl_Resample8khzTo48khz(const int16_t* in, int16_t* out,
                                   WebRtcSpl_State8khzTo48khz* state, int32_t* tmpmem);

void WebRtcSpl_ResetResample8khzTo48khz(WebRtcSpl_State8khzTo48khz* state);
#endif