/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifdef WIN32
#include <winsock2.h>
#endif
#include <string.h>
#include "g711.h"
#include "g711_interface.h"
#include "typedefs.h"
#include "resample_48khz.h"

int16_t WebRtcG711_EncodeA(void* state,
                           int16_t* speechIn,
                           int16_t len,
                           int16_t* encoded) {
  int n;
  uint16_t tempVal, tempVal2;

  // Set and discard to avoid getting warnings
  (void)(state = NULL);

  // Sanity check of input length
  if (len < 0) {
    return (-1);
  }

  // Loop over all samples
  for (n = 0; n < len; n++) {
    tempVal = (uint16_t) linear_to_alaw(speechIn[n]);

#ifdef WEBRTC_ARCH_BIG_ENDIAN
    if ((n & 0x1) == 1) {
      encoded[n >> 1] |= ((uint16_t) tempVal);
    } else {
      encoded[n >> 1] = ((uint16_t) tempVal) << 8;
    }
#else
    if ((n & 0x1) == 1) {
      tempVal2 |= ((uint16_t) tempVal) << 8;
      encoded[n >> 1] |= ((uint16_t) tempVal) << 8;
    } else {
      tempVal2 = ((uint16_t) tempVal);
      encoded[n >> 1] = ((uint16_t) tempVal);
    }
#endif
  }
  return (len);
}

int16_t WebRtcG711_EncodeU(void* state,
                           int16_t* speechIn,
                           int16_t len,
                           int16_t* encoded) {
  int n;
  uint16_t tempVal;

  // Set and discard to avoid getting warnings
  (void)(state = NULL);

  // Sanity check of input length
  if (len < 0) {
    return (-1);
  }

  // Loop over all samples
  for (n = 0; n < len; n++) {
    tempVal = (uint16_t) linear_to_ulaw(speechIn[n]);

#ifdef WEBRTC_ARCH_BIG_ENDIAN
    if ((n & 0x1) == 1) {
      encoded[n >> 1] |= ((uint16_t) tempVal);
    } else {
      encoded[n >> 1] = ((uint16_t) tempVal) << 8;
    }
#else
    if ((n & 0x1) == 1) {
      encoded[n >> 1] |= ((uint16_t) tempVal) << 8;
    } else {
      encoded[n >> 1] = ((uint16_t) tempVal);
    }
#endif
  }
  return (len);
}

int16_t WebRtcG711_DecodeA(void* state,
                           int16_t* encoded,
                           int16_t len,
                           int16_t* decoded,
                           int16_t* speechType) {
  int n;
  uint16_t tempVal;

  // Set and discard to avoid getting warnings
  (void)(state = NULL);

  // Sanity check of input length
  if (len < 0) {
    return (-1);
  }

  for (n = 0; n < len; n++) {
#ifdef WEBRTC_ARCH_BIG_ENDIAN
    if ((n & 0x1) == 1) {
      tempVal = ((uint16_t) encoded[n >> 1] & 0xFF);
    } else {
      tempVal = ((uint16_t) encoded[n >> 1] >> 8);
    }
#else
    if ((n & 0x1) == 1) {
      tempVal = (encoded[n >> 1] >> 8);
    } else {
      tempVal = (encoded[n >> 1] & 0xFF);
    }
#endif
    decoded[n] = (int16_t) alaw_to_linear(tempVal);
  }

  *speechType = 1;
  return (len);
}

int16_t WebRtcG711_DecodeU(void* state,
                           int16_t* encoded,
                           int16_t len,
                           int16_t* decoded,
                           int16_t* speechType) {
  int n;
  uint16_t tempVal;

  // Set and discard to avoid getting warnings
  (void)(state = NULL);

  // Sanity check of input length
  if (len < 0) {
    return (-1);
  }

  for (n = 0; n < len; n++) {
#ifdef WEBRTC_ARCH_BIG_ENDIAN
    if ((n & 0x1) == 1) {
      tempVal = ((uint16_t) encoded[n >> 1] & 0xFF);
    } else {
      tempVal = ((uint16_t) encoded[n >> 1] >> 8);
    }
#else
    if ((n & 0x1) == 1) {
      tempVal = (encoded[n >> 1] >> 8);
    } else {
      tempVal = (encoded[n >> 1] & 0xFF);
    }
#endif
    decoded[n] = (int16_t) ulaw_to_linear(tempVal);
  }

  *speechType = 1;
  return (len);
}

int WebRtcG711_DurationEst(void* state,
                           const uint8_t* payload,
                           int payload_length_bytes) {
  (void) state;
  (void) payload;
  /* G.711 is one byte per sample, so we can just return the number of bytes. */
  return payload_length_bytes;
}

int16_t WebRtcG711_Version(char* version, int16_t lenBytes) {
  strncpy(version, "2.0.0", lenBytes);
  return 0;
}

#define MAX_G711_FRAME_SIZE	(512)
#define MAX_PCM_MONO_FRAME_SIZE	(MAX_G711_FRAME_SIZE * 6 * 2) //8 to 48, 1chan, 16bit

typedef struct _G711U_STATE_T
{
	signed short *pPcmBuffMono8khz; 
	signed short *pPcmBuffMono48khz; 
	int nResampleFreq;
	int fStereoOut;
	int32_t      *topmem;
	WebRtcSpl_State8khzTo48khz resample_state;

} G711U_STATE_T;

int g711uDecode(void *_pState, unsigned char *pBuff, int nLen, signed short *pBuffOut)
{
	int ret = 0;
	G711U_STATE_T *pState = (G711U_STATE_T *)_pState;
	int nSpeechType;
	signed short *pPcmBuffMono8khz = pState->pPcmBuffMono8khz;

	int i;
	WebRtcG711_DecodeU(NULL, (signed short *)pBuff, (signed short)nLen, pPcmBuffMono8khz, (signed short *)&nSpeechType);
#ifdef WIN32
	//myWaveWrite(pBuff, nLen);
	//myWaveWrite(pPcmBuffMono8khz, nLen * 2);
#endif
	if(pState->nResampleFreq == 48000) {
		signed short *pPcmBuffMono48khz = pState->pPcmBuffMono48khz;
		signed short *pPcmBuffStereo = pBuffOut;
		WebRtcSpl_Resample8khzTo48khz(pPcmBuffMono8khz, pPcmBuffMono48khz, &pState->resample_state, pState->topmem);
		for (i = 0; i < nLen * 6; i++) {
			if(pState->fStereoOut) {
				*pPcmBuffStereo++ = *pPcmBuffMono48khz;
			}
			*pPcmBuffStereo++ = *pPcmBuffMono48khz++;
		}
		if(pState->fStereoOut) {
			ret = nLen * 6 * 2 * 2;
		} else {
			ret = nLen * 6 * 2;
		}
	} else {
		memcpy(pBuffOut, pPcmBuffMono8khz, nLen * 2);
		ret = nLen * 2;
	}
	return ret;
}

void *g711uCreate(int nResampleFreq, int fStereo)
{
	G711U_STATE_T *pState = (G711U_STATE_T *)malloc(sizeof(G711U_STATE_T));
	pState->pPcmBuffMono8khz = (signed short *) malloc(MAX_G711_FRAME_SIZE);
	pState->pPcmBuffMono48khz = (signed short *) malloc(MAX_PCM_MONO_FRAME_SIZE);
	pState->topmem = (signed short *) malloc(MAX_PCM_MONO_FRAME_SIZE);
	pState->nResampleFreq = nResampleFreq;
	pState->fStereoOut = fStereo;
	return pState;
}

void g711uDelete(void *_pState)
{
	G711U_STATE_T *pState = (G711U_STATE_T *)_pState;
	if(pState) {
		free(pState->pPcmBuffMono8khz);
		free(pState->pPcmBuffMono48khz);
		free(pState->topmem);
		free(pState);
	}
}