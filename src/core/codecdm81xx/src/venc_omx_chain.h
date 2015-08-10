#ifndef __VENC_OMX_CHAIN_H__
#define __VENC_OMX_CHAIN_H__
#include "strmcomp.h"

#define MAX_FILE_NAME_SIZE      256
#define MAX_MODE_NAME_SIZE      16
typedef struct VENC_CHAIN_ARGS
{
  char output_file[MAX_FILE_NAME_SIZE];
  int frame_rate;
  int bit_rate;
  int num_frames;
  char mode[MAX_MODE_NAME_SIZE];
  int display_id;  
  int input_width;
  int input_height;
  int enc_width;
  int enc_height;
  int disp_width;
  int disp_height;
  int input_buffers;
  int output_buffers;
} VENC_CHAIN_ARGS;

#define VENC_CMD_SET_PARAMS		1


StrmCompIf *vencchainCreate();

#endif // __VENC_OMX_CHAIN_H__

/*--------------------------------- end of file -----------------------------*/
