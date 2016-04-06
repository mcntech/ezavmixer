#ifndef __JD_RTCP_FROM_SPEC_H__
#define __JD_RTCP_FROM_SPEC_H__

#include <stdlib.h>

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
		return &fBuf[fPacketStart + fCurOffset];
	}
	unsigned totalBytesAvailable() const {
		return fLimit - (fPacketStart + fCurOffset);
	}
	unsigned totalBufferSize() const {
		return fLimit;
	}
	unsigned char* packet() const {
		return &fBuf[fPacketStart];
	}
	unsigned curPacketSize() const {
		return fCurOffset;
	}

	void increment(unsigned numBytes) {
		fCurOffset += numBytes;
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
		return fCurOffset >= fPreferred;
	}
	bool wouldOverflow(unsigned numBytes) const {
		return (fCurOffset + numBytes) > fMax;
	}
	unsigned numOverflowBytes(unsigned numBytes) const {
		return (fCurOffset + numBytes) - fMax;
	}
	bool isTooBigForAPacket(unsigned numBytes) const {
		return numBytes > fMax;
	}

	void setOverflowData(unsigned overflowDataOffset, unsigned overflowDataSize,
			struct timeval const& presentationTime,
			unsigned durationInMicroseconds);
	unsigned overflowDataSize() const {
		return fOverflowDataSize;
	}
	struct timeval overflowPresentationTime() const {
		return fOverflowPresentationTime;
	}
	unsigned overflowDurationInMicroseconds() const {
		return fOverflowDurationInMicroseconds;
	}
	bool haveOverflowData() const {
		return fOverflowDataSize > 0;
	}
	void useOverflowData();

	void adjustPacketStart(unsigned numBytes);
	void resetPacketStart();
	void resetOffset() {
		fCurOffset = 0;
	}
	void resetOverflowData() {
		fOverflowDataOffset = fOverflowDataSize = 0;
	}

private:
	unsigned fPacketStart, fCurOffset, fPreferred, fMax, fLimit;
	unsigned char* fBuf;

	unsigned fOverflowDataOffset, fOverflowDataSize;
	struct timeval fOverflowPresentationTime;
	unsigned fOverflowDurationInMicroseconds;
};



class RTPReceptionStatsDB; // forward

class RTPSource/*: public FramedSource*/ {
public:
 /* static bool lookupByName(UsageEnvironment& env, char const* sourceName,
			      RTPSource*& resultSource);
*/
  bool curPacketMarkerBit() const { return fCurPacketMarkerBit; }

  unsigned char rtpPayloadFormat() const { return fRTPPayloadFormat; }

  virtual bool hasBeenSynchronizedUsingRTCP();

  Groupsock* RTPgs() const { return fRTPInterface.gs(); }

  virtual void setPacketReorderingThresholdTime(unsigned uSeconds) = 0;

  // used by RTCP:
  unsigned long SSRC() const { return mSSRC; }
      // Note: This is *our* SSRC, not the SSRC in incoming RTP packets.
     // later need a means of changing the SSRC if there's a collision #####
  void registerForMultiplexedRTCPPackets(class RTCPInstance* rtcpInstance) {
    fRTCPInstanceForMultiplexedRTCPPackets = rtcpInstance;
  }
  void deregisterForMultiplexedRTCPPackets() { registerForMultiplexedRTCPPackets(NULL); }

  unsigned timestampFrequency() const {return fTimestampFrequency;}

  RTPReceptionStatsDB& receptionStatsDB() const {
    return *fReceptionStatsDB;
  }

  unsigned long lastReceivedSSRC() const { return fLastReceivedSSRC; }
  // Note: This is the SSRC in the most recently received RTP packet; not *our* SSRC

  bool& enableRTCPReports() { return fEnableRTCPReports; }
  bool const& enableRTCPReports() const { return fEnableRTCPReports; }

  void setStreamSocket(int sockNum, unsigned char streamChannelId) {
    // hack to allow sending RTP over TCP (RFC 2236, section 10.12)
    fRTPInterface.setStreamSocket(sockNum, streamChannelId);
  }

  void setAuxilliaryReadHandler(AuxHandlerFunc* handlerFunc,
                                void* handlerClientData) {
    fRTPInterface.setAuxilliaryReadHandler(handlerFunc,
					   handlerClientData);
  }

  // Note that RTP receivers will usually not need to call either of the following two functions, because
  // RTP sequence numbers and timestamps are usually not useful to receivers.
  // (Our implementation of RTP reception already does all needed handling of RTP sequence numbers and timestamps.)
  unsigned short curPacketRTPSeqNum() const { return fCurPacketRTPSeqNum; }
private: friend class MediaSubsession; // "MediaSubsession" is the only outside class that ever needs to see RTP timestamps!
  unsigned long curPacketRTPTimestamp() const { return fCurPacketRTPTimestamp; }

protected:
  RTPSource(UsageEnvironment& env, Groupsock* RTPgs,
	    unsigned char rtpPayloadFormat, unsigned long rtpTimestampFrequency);
      // abstract base class
  virtual ~RTPSource();

protected:
  RTPInterface fRTPInterface;
  unsigned short fCurPacketRTPSeqNum;
  unsigned long fCurPacketRTPTimestamp;
  bool fCurPacketMarkerBit;
  bool fCurPacketHasBeenSynchronizedUsingRTCP;
  unsigned long fLastReceivedSSRC;
  class RTCPInstance* fRTCPInstanceForMultiplexedRTCPPackets;

private:
  // redefined virtual functions:
  virtual bool isRTPSource() const;
  virtual void getAttributes() const;

private:
  unsigned char fRTPPayloadFormat;
  unsigned fTimestampFrequency;
  unsigned long mSSRC;
  bool fEnableRTCPReports; // whether RTCP "RR" reports should be sent for this source (default: True)

  RTPReceptionStatsDB* fReceptionStatsDB;
};


class RTPReceptionStats; // forward

class RTPReceptionStatsDB {
public:
  unsigned totNumPacketsReceived() const { return fTotNumPacketsReceived; }
  unsigned numActiveSourcesSinceLastReset() const {
    return fNumActiveSourcesSinceLastReset;
 }

  void reset();
      // resets periodic stats (called each time they're used to
      // generate a reception report)

  class Iterator {
  public:
    Iterator(RTPReceptionStatsDB& receptionStatsDB);
    virtual ~Iterator();

    RTPReceptionStats* next(bool includeInactiveSources = False);
        // NULL if none

  private:
    HashTable::Iterator* fIter;
  };

  // The following is called whenever a RTP packet is received:
  void noteIncomingPacket(unsigned long SSRC, unsigned short seqNum,
			  unsigned long rtpTimestamp,
			  unsigned timestampFrequency,
			  bool useForJitterCalculation,
			  struct timeval& resultPresentationTime,
			  bool& resultHasBeenSyncedUsingRTCP,
			  unsigned packetSize /* payload only */);

  // The following is called whenever a RTCP SR packet is received:
  void noteIncomingSR(unsigned long SSRC,
		      unsigned long ntpTimestampMSW, unsigned long ntpTimestampLSW,
		      unsigned long rtpTimestamp);

  // The following is called when a RTCP BYE packet is received:
  void removeRecord(unsigned long SSRC);

  RTPReceptionStats* lookup(unsigned long SSRC) const;

protected: // constructor and destructor, called only by RTPSource:
  friend class RTPSource;
  RTPReceptionStatsDB();
  virtual ~RTPReceptionStatsDB();

protected:
  void add(unsigned long SSRC, RTPReceptionStats* stats);

protected:
  friend class Iterator;
  unsigned fNumActiveSourcesSinceLastReset;

private:
  HashTable* fTable;
  unsigned fTotNumPacketsReceived; // for all SSRCs
};

class RTPReceptionStats {
public:
  unsigned long SSRC() const { return mSSRC; }
  unsigned numPacketsReceivedSinceLastReset() const {
    return fNumPacketsReceivedSinceLastReset;
  }
  unsigned totNumPacketsReceived() const { return fTotNumPacketsReceived; }
  double totNumKBytesReceived() const;

  unsigned totNumPacketsExpected() const {
    return (fHighestExtSeqNumReceived - fBaseExtSeqNumReceived) + 1;
  }

  unsigned baseExtSeqNumReceived() const { return fBaseExtSeqNumReceived; }
  unsigned lastResetExtSeqNumReceived() const {
    return fLastResetExtSeqNumReceived;
  }
  unsigned highestExtSeqNumReceived() const {
    return fHighestExtSeqNumReceived;
  }

  unsigned jitter() const;

  unsigned lastReceivedSR_NTPmsw() const { return fLastReceivedSR_NTPmsw; }
  unsigned lastReceivedSR_NTPlsw() const { return fLastReceivedSR_NTPlsw; }
  struct timeval const& lastReceivedSR_time() const {
    return fLastReceivedSR_time;
  }

  unsigned minInterPacketGapUS() const { return fMinInterPacketGapUS; }
  unsigned maxInterPacketGapUS() const { return fMaxInterPacketGapUS; }
  struct timeval const& totalInterPacketGaps() const {
    return fTotalInterPacketGaps;
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
  unsigned fNumPacketsReceivedSinceLastReset;
  unsigned fTotNumPacketsReceived;
  unsigned long fTotBytesReceived_hi, fTotBytesReceived_lo;
  bool fHaveSeenInitialSequenceNumber;
  unsigned fBaseExtSeqNumReceived;
  unsigned fLastResetExtSeqNumReceived;
  unsigned fHighestExtSeqNumReceived;
  int fLastTransit; // used in the jitter calculation
  unsigned long fPreviousPacketRTPTimestamp;
  double fJitter;
  // The following are recorded whenever we receive a RTCP SR for this SSRC:
  unsigned fLastReceivedSR_NTPmsw; // NTP timestamp (from SR), most-signif
  unsigned fLastReceivedSR_NTPlsw; // NTP timestamp (from SR), least-signif
  struct timeval fLastReceivedSR_time;
  struct timeval fLastPacketReceptionTime;
  unsigned fMinInterPacketGapUS, fMaxInterPacketGapUS;
  struct timeval fTotalInterPacketGaps;

private:
  // Used to convert from RTP timestamp to 'wall clock' time:
  bool fHasBeenSynchronized;
  unsigned long fSyncTimestamp;
  struct timeval fSyncTime;
};

class CJdRtcp
{
public:
	CJdRtcp::CJdRtcp();
	void OnExpire(void *event, int, int, double, int, double*, int*, double , double *, int*);

	void OnReceive(void *packet, void *event, int*, int*, int*, double*, double*, double, double);

	void Process(JdRtp *pRtp);
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
	double mPrevReportTime;
	double mNextReportTime;

	  int mLastSentSize;
	  int mLastReceivedSize;
	  unsigned long mLastReceivedSSRC;
	  int mTypeOfEvent;
	  int mTypeOfPacket;

	  OutPacketBuffer* mOutBuf;

	  void *mSource;
};

#endif
