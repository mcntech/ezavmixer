#ifndef __VDEC_OMX_CHAIN_H__
#define __VDEC_OMX_CHAIN_H__

#include "strmcomp.h"

/******************************************************************************/

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

#define MAX_CODEC_NAME_SIZE     16
#define MAX_FILE_NAME_SIZE      256

typedef struct VidArgs
{
  int nInstanceId;
  char input_file[MAX_FILE_NAME_SIZE];
  char codec_name[MAX_CODEC_NAME_SIZE];

  int dec_width;
  int dec_height;
  int disp_width;
  int disp_height;

  int frame_rate;
  int gfx;
  int display_id;
  int use_demux;
  int eventlong;
  int statuslog;
  int dbglevel;
  int sync;
  int latency;
  int deinterlace;
  int buffer;
  void (*strmCallback) (void *, int);
  void *pAppCtx;
} IL_VID_ARGS;

/**
 ** API
 */
#define DEC_CMD_SET_PARAMS		1

StrmCompIf *vidchainCreate();


#ifdef __cplusplus              /* matches __cplusplus construct above */
}
#endif

#endif
