/*
 * h264parser.h: a minimalistic H.264 video stream parser
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 */

#ifndef __MPEG2PARSER_H
#define __MPEG2PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


  class CMpeg2Parser {
  public:
	int ParseSequenceHeader(uint8_t *Data, int Count, long *plWidth, long *plHeight);
  };

#endif // __MPEG2PARSER_H

