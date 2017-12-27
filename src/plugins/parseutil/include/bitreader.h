//
// Created by ramp on 12/21/17.
//

#ifndef UDPPLAYER_BITREADER_H
#define UDPPLAYER_BITREADER_H

// --- cBitReader ----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#define uchar    unsigned char
#define uint8_t  unsigned char
#define uint32_t unsigned int
#define int32_t  int
void Error(const char *pErr);

class cBitReader {
public:
    class cBookMark {
    private:
        uint8_t *data;
        int count;
        uint32_t bits;
        uint32_t bitsAvail;
        int countZeros;
        cBookMark(void) {}
        friend class cBitReader;
    };
private:
    cBookMark bm;
    uint8_t NextByte(void);
    uint32_t ReadBits(uint32_t n);
public:
    cBitReader(uint8_t *Data, int Count);
    uint32_t u(uint32_t n) { return ReadBits(n); } // read n bits as unsigned number
    uint32_t ue(void); // read Exp-Golomb coded unsigned number
    int32_t se(void); // read Exp-Golomb coded signed number
    uint32_t GetBitsAvail(void) { return (bm.bitsAvail & 0x07); }
    bool GetBytesAvail(void) { return (bm.count > 0); }
    const cBookMark BookMark(void) const { return bm; }
    void BookMark(const cBookMark &b) { bm = b; }
};

inline cBitReader::cBitReader(unsigned char *Data, int Count)
{
    bm.data = Data;
    bm.count = Count;
    bm.bitsAvail = 0;
    bm.countZeros = 0;
}

inline uint8_t cBitReader::NextByte(void)
{
    if (bm.count < 1) // there is no more data left in this NAL unit
        Error("ERROR: H264::cBitReader::NextByte(): premature end of data");
    // detect 00 00 00, 00 00 01 and 00 00 03 and handle them
    if (*bm.data == 0x00) {
        if (bm.countZeros >= 3) // 00 00 00: the current NAL unit should have been terminated already before this sequence
            Error("ERROR: H264::cBitReader::NextByte(): premature end of data");
        // increase the zero counter as we have a zero byte
        bm.countZeros++;
    }
    else {
        if (bm.countZeros >= 2) {
            if (*bm.data == 0x01) // 00 00 01: the current NAL unit should have been terminated already before this sequence
                Error("ERROR: H264::cBitReader::NextByte(): premature end of data");
            if (*bm.data == 0x03) {
                // 00 00 03 xx: the emulation prevention byte 03 needs to be removed and xx must be returned
                if (bm.count < 2)
                    Error("ERROR: H264::cBitReader::NextByte(): premature end of data");
                // drop 03 and xx will be returned below
                bm.count--;
                bm.data++;
            }
        }
        // reset the zero counter as we had a non zero byte
        bm.countZeros = 0;
    }
    bm.count--;
    return *bm.data++;
}

inline uint32_t cBitReader::ReadBits(uint32_t n)
{
    // fill the "shift register" bits with sufficient data
    while (n > bm.bitsAvail) {
        bm.bits <<= 8;
        bm.bits |= NextByte();
        bm.bitsAvail += 8;
        if (bm.bitsAvail > 24) { // a further turn will overflow bitbuffer
            if (n <= bm.bitsAvail)
                break; // service non overflowing request
            if (n <= 32) // split overflowing reads into concatenated reads
                return (ReadBits(16) << 16) | ReadBits(n - 16);
            // cannot read more than 32 bits at once
            Error("ERROR: H264::cBitReader::ReadBits(): bitbuffer overflow");
        }
    }
    // return n most significant bits
    bm.bitsAvail -= n;
    return (bm.bits >> bm.bitsAvail) & (((uint32_t)1 << n) - 1);
}

inline uint32_t cBitReader::ue(void)
{
    // read and decode an Exp-Golomb coded unsigned number
    //
    // bitstring             resulting number
    //       1               0
    //     0 1 x             1 ... 2
    //   0 0 1 x y           3 ... 6
    // 0 0 0 1 x y z         7 ... 14
    // ...
    int LeadingZeroBits = 0;
    while (ReadBits(1) == 0)
        LeadingZeroBits++;
    if (LeadingZeroBits == 0)
        return 0;
    if (LeadingZeroBits >= 32)
        Error("ERROR: H264::cBitReader::ue(): overflow");
    return ((uint32_t)1 << LeadingZeroBits) - 1 + ReadBits(LeadingZeroBits);
}

inline int32_t cBitReader::se(void)
{
    // read and decode an Exp-Golomb coded signed number
    //
    // unsigned value       resulting signed value
    // 0                     0
    // 1                    +1
    // 2                    -1
    // 3                    +2
    // 4                    -2
    // ...
    uint32_t r = ue();
    if (r > 0xFFFFFFFE)
        Error("ERROR: MPEG2::cBitReader::se(): overflow");
    //int n = (1 - 2 * (r & 1)) * ((r + 1) / 2);
    int n = (r % 2) ? (r / 2) : -(r / 2);
    return n;
}

#endif //UDPPLAYER_BITREADER_H
