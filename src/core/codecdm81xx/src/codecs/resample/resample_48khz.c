/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


/*
 * This file contains resampling functions between 48 kHz and nb/wb.
 * The description header can be found in signal_processing_library.h
 *
 */

#include <string.h>
//#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#include "resample_48khz.h"
#include "resample_by_2_internal.h"


//   Resampling ratio: 3/4
// input:  int32_t (normalized, not saturated) :: size 4 * K
// output: int32_t (shifted 15 positions to the left, + offset 16384) :: size 3 * K
//      K: number of blocks

static const int16_t kCoefficients32To24[3][8] = {
        {767, -2362, 2434, 24406, 10620, -3838, 721, 90},
        {386, -381, -2646, 19062, 19062, -2646, -381, 386},
        {90, 721, -3838, 10620, 24406, 2434, -2362, 767}
};

void WebRtcSpl_Resample32khzTo24khz(const int32_t *In, int32_t *Out,
                                    int32_t K)
{
    /////////////////////////////////////////////////////////////
    // Filter operation:
    //
    // Perform resampling (4 input samples -> 3 output samples);
    // process in sub blocks of size 4 samples.
    int32_t m;
    int32_t tmp;

    for (m = 0; m < K; m++)
    {
        tmp = 1 << 14;
        tmp += kCoefficients32To24[0][0] * In[0];
        tmp += kCoefficients32To24[0][1] * In[1];
        tmp += kCoefficients32To24[0][2] * In[2];
        tmp += kCoefficients32To24[0][3] * In[3];
        tmp += kCoefficients32To24[0][4] * In[4];
        tmp += kCoefficients32To24[0][5] * In[5];
        tmp += kCoefficients32To24[0][6] * In[6];
        tmp += kCoefficients32To24[0][7] * In[7];
        Out[0] = tmp;

        tmp = 1 << 14;
        tmp += kCoefficients32To24[1][0] * In[1];
        tmp += kCoefficients32To24[1][1] * In[2];
        tmp += kCoefficients32To24[1][2] * In[3];
        tmp += kCoefficients32To24[1][3] * In[4];
        tmp += kCoefficients32To24[1][4] * In[5];
        tmp += kCoefficients32To24[1][5] * In[6];
        tmp += kCoefficients32To24[1][6] * In[7];
        tmp += kCoefficients32To24[1][7] * In[8];
        Out[1] = tmp;

        tmp = 1 << 14;
        tmp += kCoefficients32To24[2][0] * In[2];
        tmp += kCoefficients32To24[2][1] * In[3];
        tmp += kCoefficients32To24[2][2] * In[4];
        tmp += kCoefficients32To24[2][3] * In[5];
        tmp += kCoefficients32To24[2][4] * In[6];
        tmp += kCoefficients32To24[2][5] * In[7];
        tmp += kCoefficients32To24[2][6] * In[8];
        tmp += kCoefficients32To24[2][7] * In[9];
        Out[2] = tmp;

        // update pointers
        In += 4;
        Out += 3;
    }
}


// 8 -> 48 resampler
void WebRtcSpl_Resample8khzTo48khz(const int16_t* in, int16_t* out,
                                   WebRtcSpl_State8khzTo48khz* state, int32_t* tmpmem)
{
    ///// 8 --> 16 /////
    // int16_t  in[80]
    // int32_t out[160]
    /////
    WebRtcSpl_UpBy2ShortToInt(in, 80, tmpmem + 264, state->S_8_16);

    ///// 16 --> 12 /////
    // int32_t  in[160]
    // int32_t out[120]
    /////
    // copy state to and from input array
    memcpy(tmpmem + 256, state->S_16_12, 8 * sizeof(int32_t));
    memcpy(state->S_16_12, tmpmem + 416, 8 * sizeof(int32_t));
    WebRtcSpl_Resample32khzTo24khz(tmpmem + 256, tmpmem + 240, 40);

    ///// 12 --> 24 /////
    // int32_t  in[120]
    // int16_t out[240]
    /////
    WebRtcSpl_UpBy2IntToInt(tmpmem + 240, 120, tmpmem, state->S_12_24);

    ///// 24 --> 48 /////
    // int32_t  in[240]
    // int16_t out[480]
    /////
    WebRtcSpl_UpBy2IntToShort(tmpmem, 240, out, state->S_24_48);
}

// initialize state of 8 -> 48 resampler
void WebRtcSpl_ResetResample8khzTo48khz(WebRtcSpl_State8khzTo48khz* state)
{
    memset(state->S_8_16, 0, 8 * sizeof(int32_t));
    memset(state->S_16_12, 0, 8 * sizeof(int32_t));
    memset(state->S_12_24, 0, 8 * sizeof(int32_t));
    memset(state->S_24_48, 0, 8 * sizeof(int32_t));
}
