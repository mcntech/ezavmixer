#ifndef __JD_RTCP_FROM_SPEC_H__
#define __JD_RTCP_FROM_SPEC_H__

#include <stdlib.h>
#include <time.h>
#include <map>

class RTCPMemberDatabase; // forward

// RTCP packet types:
const unsigned char RTCP_PT_SR = 200;
const unsigned char RTCP_PT_RR = 201;
const unsigned char RTCP_PT_SDES = 202;
const unsigned char RTCP_PT_BYE = 203;
const unsigned char RTCP_PT_APP = 204;
const unsigned char RTCP_PT_RTPFB = 205; // Generic RTP Feedback [RFC4585]
const unsigned char RTCP_PT_PSFB = 206; // Payload-specific [RFC4585]
const unsigned char RTCP_PT_XR = 207; // extended report [RFC3611]
const unsigned char RTCP_PT_AVB = 208; // AVB RTCP packet ["Standard for Layer 3 Transport Protocol for Time Sensitive Applications in Local Area Networks." Work in progress.]
const unsigned char RTCP_PT_RSI = 209; // Receiver Summary Information [RFC5760]
const unsigned char RTCP_PT_TOKEN = 210; // Port Mapping [RFC6284]
const unsigned char RTCP_PT_IDMS = 211; // IDMS Settings [RFC7272]

// SDES tags:
const unsigned char RTCP_SDES_END = 0;
const unsigned char RTCP_SDES_CNAME = 1;
const unsigned char RTCP_SDES_NAME = 2;
const unsigned char RTCP_SDES_EMAIL = 3;
const unsigned char RTCP_SDES_PHONE = 4;
const unsigned char RTCP_SDES_LOC = 5;
const unsigned char RTCP_SDES_TOOL = 6;
const unsigned char RTCP_SDES_NOTE = 7;
const unsigned char RTCP_SDES_PRIV = 8;

/* The code from the spec assumes a type "event"; make this a void*: */

#define EVENT_UNKNOWN 0
#define EVENT_REPORT 1
#define EVENT_BYE 2


#define PACKET_UNKNOWN_TYPE 0
#define PACKET_RTP 1
#define PACKET_RTCP_REPORT 2
#define PACKET_BYE 3
#define PACKET_RTCP_APP 4

/* The code from the spec calls drand48(), but we have drand30() instead */
#define drand48 drand30

/* The code calls "exit()", but we don't want to exit, so make it a noop: */
#define exit(n) do {} while (0)

class OutPacketBuffer {
public:
	OutPacketBuffer(unsigned preferredPacketSize, unsigned maxPacketSize,
			unsigned maxBufferSize = 0);
	// if "maxBufferSize" is >0, use it - instead of "maxSize" to compute the buffer size
	~OutPacketBuffer();

	static unsigned maxSize;
	static void increaseMaxSizeTo(unsigned newMaxSize) {
		if (newMaxSize > OutPacketBuffer::maxSize)
			OutPacketBuffer::maxSize = newMaxSize;
	}

	unsigned char* curPtr() const {
		return &mBuf[mPacketStart + mCurOffset];
	}
	unsigned totalBytesAvailable() const {
		return mLimit - (mPacketStart + mCurOffset);
	}
	unsigned totalBufferSize() const {
		return mLimit;
	}
	unsigned char* packet() const {
		return &mBuf[mPacketStart];
	}
	unsigned curPacketSize() const {
		return mCurOffset;
	}

	void increment(unsigned numBytes) {
		mCurOffset += numBytes;
	}

	void enqueue(unsigned char const* from, unsigned numBytes);
	void enqueueWord(unsigned long word);
	void insert(unsigned char const* from, unsigned numBytes,
			unsigned toPosition);
	void insertWord(unsigned long word, unsigned toPosition);
	void extract(unsigned char* to, unsigned numBytes, unsigned fromPosition);
	unsigned long extractWord(unsigned fromPosition);

	void skipBytes(unsigned numBytes);

	bool isPreferredSize() const {
		return mCurOffset >= mPreferred;
	}
	bool wouldOverflow(unsigned numBytes) const {
		return (mCurOffset + numBytes) > mMax;
	}
	unsigned numOverflowBytes(unsigned numBytes) const {
		return (mCurOffset + numBytes) - mMax;
	}
	bool isTooBigForAPacket(unsigned numBytes) const {
		return numBytes > mMax;
	}

	void setOverflowData(unsigned overflowDataOffset, unsigned overflowDataSize,
			struct timeval const& presentationTime,
			unsigned durationInMicroseconds);
	unsigned overflowDataSize() const {
		return mOverflowDataSize;
	}
	struct timeval overflowPresentationTime() const {
		return mOverflowPresentationTime;
	}
	unsigned overflowDurationInMicroseconds() const {
		return mOverflowDurationInMicroseconds;
	}
	bool haveOverflowData() const {
		return mOverflowDataSize > 0;
	}
	void useOverflowData();

	void adjustPacketStart(unsigned numBytes);
	void resetPacketStart();
	void resetOffset() {
		mCurOffset = 0;
	}
	void resetOverflowData() {
		mOverflowDataOffset = mOverflowDataSize = 0;
	}

private:
	unsigned mPacketStart, mCurOffset, mPreferred, mMax, mLimit;
	unsigned char* mBuf;

	unsigned mOverflowDataOffset, mOverflowDataSize;
	struct timeval mOverflowPresentationTime;
	unsigned mOverflowDurationInMicroseconds;
};

class RTPReceptionStatsDB; // forward

class RTPSource
{
public:
	bool curPacketMarkerBit() const {
		return mCurPacketMarkerBit;
	}

	unsigned char rtpPayloadFormat() const {
		return mRTPPayloadFormat;
	}

	virtual bool hasBeenSynchronizedUsingRTCP();

	virtual void setPacketReorderingThresholdTime(unsigned uSeconds) = 0;

	// used by RTCP:
	unsigned long SSRC() const {
		return mSSRC;
	}

	unsigned timestampFrequency() const {
		return mTimestampFrequency;
	}

	RTPReceptionStatsDB& receptionStatsDB() const {
		return *mReceptionStatsDB;
	}

	unsigned long lastReceivedSSRC() const {
		return mLastReceivedSSRC;
	}
	// Note: This is the SSRC in the most recently received RTP packet; not *our* SSRC

	bool& enableRTCPReports() {
		return mEnableRTCPReports;
	}
	bool const& enableRTCPReports() const {
		return mEnableRTCPReports;
	}

	// Note that RTP receivers will usually not need to call either of the following two functions, because
	// RTP sequence numbers and timestamps are usually not useful to receivers.
	// (Our implementation of RTP reception already does all needed handling of RTP sequence numbers and timestamps.)
	unsigned short curPacketRTPSeqNum() const {
		return mCurPacketRTPSeqNum;
	}
	unsigned long curPacketRTPTimestamp() const {
		return mCurPacketRTPTimestamp;
	}

protected:
	RTPSource(unsigned char rtpPayloadFormat,
			unsigned long rtpTimestampFrequency);
	virtual ~RTPSource();

protected:
	unsigned short mCurPacketRTPSeqNum;
	unsigned long mCurPacketRTPTimestamp;
	bool mCurPacketMarkerBit;
	bool mCurPacketHasBeenSynchronizedUsingRTCP;
	unsigned long mLastReceivedSSRC;

private:
	// redefined virtual functions:
	virtual bool isRTPSource() const;
	virtual void getAttributes() const;

private:
	unsigned char mRTPPayloadFormat;
	unsigned mTimestampFrequency;
	unsigned long mSSRC;
	bool mEnableRTCPReports; // whether RTCP "RR" reports should be sent for this source (default: True)

	RTPReceptionStatsDB* mReceptionStatsDB;
};


class RTPReceptionStats; // forward

typedef std::map<unsigned long, RTPReceptionStats*>::iterator rec_tbl_iter;

class RTPReceptionStatsDB {
public:
	unsigned totNumPacketsReceived() const {
		return mTotNumPacketsReceived;
	}
	unsigned numActiveSourcesSinceLastReset() const {
		return mNumActiveSourcesSinceLastReset;
	}

  void reset();
      // resets periodic stats (called each time they're used to
      // generate a reception report)

  class Iterator {
  public:
    Iterator(RTPReceptionStatsDB& receptionStatsDB);
    virtual ~Iterator();

    RTPReceptionStats* next(bool includeInactiveSources = false);
        // NULL if none

  private:
    //HashTable::Iterator* mIter;
  };

	// The following is called whenever a RTP packet is received:
	void noteIncomingPacket(unsigned long SSRC,
			unsigned short seqNum,
			unsigned long rtpTimestamp,
			unsigned timestampFrequency,
			bool useForJitterCalculation,
			struct timeval& resultPresentationTime,
			bool& resultHasBeenSyncedUsingRTCP,
			unsigned packetSize /* payload only */);

  // The following is called whenever a RTCP SR packet is received:
	void noteIncomingSR(unsigned long SSRC, unsigned long ntpTimestampMSW,
			unsigned long ntpTimestampLSW, unsigned long rtpTimestamp);

  // The following is called when a RTCP BYE packet is received:
  void removeRecord(unsigned long SSRC);

  RTPReceptionStats* lookup(unsigned long SSRC);

public: // constructor and destructor, called only by RTPSource:
  //friend class RTPSource;
  RTPReceptionStatsDB();
  virtual ~RTPReceptionStatsDB();

protected:
  void add(unsigned long SSRC, RTPReceptionStats* stats);

protected:
  //friend class Iterator;
  unsigned mNumActiveSourcesSinceLastReset;

private:
  //HashTable* fTable;
  std::map<unsigned long, RTPReceptionStats*> mTable;
  unsigned  mTotNumPacketsReceived; // for all SSRCs
};


class RTPReceptionStats {
public:
  unsigned long SSRC() const { return mSSRC; }
  unsigned numPacketsReceivedSinceLastReset() const {
    return mNumPacketsReceivedSinceLastReset;
  }
  unsigned totNumPacketsReceived() const { return mTotNumPacketsReceived; }
  double totNumKBytesReceived() const;

  unsigned totNumPacketsExpected() const {
    return (mHighestExtSeqNumReceived - mBaseExtSeqNumReceived) + 1;
  }

  unsigned baseExtSeqNumReceived() const { return mBaseExtSeqNumReceived; }
  unsigned lastResetExtSeqNumReceived() const {
    return mLastResetExtSeqNumReceived;
  }
  unsigned highestExtSeqNumReceived() const {
    return mHighestExtSeqNumReceived;
  }

  unsigned jitter() const;

  unsigned lastReceivedSR_NTPmsw() const { return mLastReceivedSR_NTPmsw; }
  unsigned lastReceivedSR_NTPlsw() const { return mLastReceivedSR_NTPlsw; }
  struct timeval const& lastReceivedSR_time() const {
    return mLastReceivedSR_time;
  }

  unsigned minInterPacketGapUS() const { return mMinInterPacketGapUS; }
  unsigned maxInterPacketGapUS() const { return mMaxInterPacketGapUS; }
  struct timeval const& totalInterPacketGaps() const {
    return mTotalInterPacketGaps;
  }

protected:
  // called only by RTPReceptionStatsDB:
  friend class RTPReceptionStatsDB;
  RTPReceptionStats(unsigned long SSRC, unsigned short initialSeqNum);
  RTPReceptionStats(unsigned long SSRC);
  virtual ~RTPReceptionStats();

private:
  void noteIncomingPacket(unsigned short seqNum, unsigned long rtpTimestamp,
			  unsigned timestampFrequency,
			  bool useForJitterCalculation,
			  struct timeval& resultPresentationTime,
			  bool& resultHasBeenSyncedUsingRTCP,
			  unsigned packetSize /* payload only */);
  void noteIncomingSR(unsigned long ntpTimestampMSW, unsigned long ntpTimestampLSW,
		      unsigned long rtpTimestamp);
  void init(unsigned long SSRC);
  void initSeqNum(unsigned short initialSeqNum);
  void reset();
      // resets periodic stats (called each time they're used to
      // generate a reception report)

protected:
	unsigned long mSSRC;
	unsigned mNumPacketsReceivedSinceLastReset;
	unsigned mTotNumPacketsReceived;
	unsigned long mTotBytesReceived_hi, mTotBytesReceived_lo;
	bool mHaveSeenInitialSequenceNumber;
	unsigned mBaseExtSeqNumReceived;
	unsigned mLastResetExtSeqNumReceived;
	unsigned mHighestExtSeqNumReceived;
	int mLastTransit; // used in the jitter calculatmion
	unsigned long mPreviousPacketRTPTimestamp;
	double mJitter;
	// The following are recorded whenever we receive a RTCP SR for this SSRC:
	unsigned mLastReceivedSR_NTPmsw; // NTP timestamp (from SR), most-signif
	unsigned mLastReceivedSR_NTPlsw; // NTP timestamp (from SR), least-signif
	struct timeval mLastReceivedSR_time;
	struct timeval mLastPacketReceptionTime;
	unsigned mMinInterPacketGapUS, mMaxInterPacketGapUS;
	struct timeval mTotalInterPacketGaps;

private:
	// Used to convert from RTP timestamp to 'wall clock' time:
	bool mHasBeenSynchronized;
	unsigned long mSyncTimestamp;
	struct timeval mSyncTime;
};

class CJdRtcp
{
public:
	CJdRtcp();
	void OnExpire(void *event, int, int, double, int, double*, int*, double , double *, int*);

	void OnReceive(void *packet, void *event, int*, int*, int*, double*, double*, double, double);

	void Process(void *pRtp);
	/* IMPORTS: */

	void Schedule (double,void *event);
	void Reschedule(double,void *event);
	void SendRTCPReport (void *event);
	void SendBYEPacket(void *event);
	int TypeOfEvent(void *event);
	int SentPacketSize(void *event);
	int PacketType(void *packet);
	int ReceivedPacketSize(void *packet);
	int NewMember(void *packet);
	int NewSender(void *packet);
	void AddMember(void *packet);
	void AddSender(void *packet);
	void RemoveMember(void *packet);
	void RemoveSender(void *packet);
	double drand30(void);

	void removeSSRC(unsigned long oldSSRC, bool fremove);
private:
	void onReceive(int typeOfPacket, int totPacketSize, unsigned long ssrc);
	void processIncomingReport(unsigned packetSize, struct sockaddr_in const& fromAddressAndPort,
				int tcpSocketNum, unsigned char tcpStreamChannelId);

	bool addReport(bool alwaysAdd);

	void addRR();
	void enqueueCommonReportPrefix(unsigned char packetType, unsigned long SSRC,
					     unsigned numExtraWords = 0);
	void enqueueCommonReportSuffix();
	void enqueueReportBlock(RTPReceptionStats* stats);


private:
	char *m_pBuffer;
	int  m_maxBufferLen;

	double rtcp_interval(int members,
	                        int senders,
	                        double rtcp_bw,
	                        int we_sent,
	                        double avg_rtcp_size,
	                        int initial);
	double mPrevReportTime;
	double mNextReportTime;
	RTCPMemberDatabase* mKnownMembers;
	int mPrevNumMembers;
	double mAveRTCPSize;
	int mIsInitial;

	  int mLastSentSize;
	  int mLastReceivedSize;
	  unsigned long mLastReceivedSSRC;
	  int mTypeOfEvent;
	  int mTypeOfPacket;

	  OutPacketBuffer* mOutBuf;

	  RTPSource *mSource;
};
#endif
