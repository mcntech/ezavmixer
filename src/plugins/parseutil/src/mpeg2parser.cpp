/*
 * h264parser.c: a minimalistic H.264 video stream parser
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 * The code was originally written by Reinhard Nissl <rnissl@gmx.de>,
 * and adapted to the VDR coding style by Klaus.Schmidinger@cadsoft.de.
 */

//#include "tools.h"
#include "bitreader.h"
#include "mpeg2parser.h"

int CMpeg2Parser::ParseSequenceHeader(uint8_t *Data, int Count, long *plWidth, long *plHeight)
{
    cBitReader br((const char *)Data + 1, Count - 1);
    *plWidth = (long)br.u(12);
    *plHeight = (long)br.u(12);
    return 0;
}