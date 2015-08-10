#ifndef __G711U__
#define __G711U__
void *g711uCreate(int nResampleFreq, int fStereo);
void g711uDelete(void *_pState);
int g711uDecode(void *_pState, unsigned char *pBuff, int nLen, unsigned short *pBuffOut);
#endif