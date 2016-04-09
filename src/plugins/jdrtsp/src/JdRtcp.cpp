#include "JdRtcp.h"
#include "JdRtp.h"
#include <map>
#include <sys/endian.h>


////////// OutPacketBuffer //////////

// A data structure that a sink may use for an output packet:

static unsigned const IP_UDP_HDR_SIZE = 28;

unsigned OutPacketBuffer::maxSize = 60000; // by default

OutPacketBuffer::OutPacketBuffer(unsigned preferredPacketSize,
		unsigned maxPacketSize, unsigned maxBufferSize) :
		mPreferred(preferredPacketSize), mMax(maxPacketSize), mOverflowDataSize(
				0) {
	if (maxBufferSize == 0)
		maxBufferSize = maxSize;
	unsigned maxNumPackets = (maxBufferSize + (maxPacketSize - 1))
			/ maxPacketSize;
	mLimit = maxNumPackets * maxPacketSize;
	mBuf = new unsigned char[mLimit];
	resetPacketStart();
	resetOffset();
	resetOverflowData();
}

OutPacketBuffer::~OutPacketBuffer() {
	delete[] mBuf;
}

void OutPacketBuffer::enqueue(unsigned char const* from, unsigned numBytes) {
	if (numBytes > totalBytesAvailable()) {
#ifdef DEBUG
		fprintf(stderr, "OutPacketBuffer::enqueue() warning: %d > %d\n", numBytes, totalBytesAvailable());
#endif
		numBytes = totalBytesAvailable();
	}

	if (curPtr() != from)
		memmove(curPtr(), from, numBytes);
	increment(numBytes);
}

void OutPacketBuffer::enqueueWord(unsigned long word) {
	unsigned long nWord = htonl(word);
	enqueue((unsigned char*) &nWord, 4);
}

void OutPacketBuffer::insert(unsigned char const* from, unsigned numBytes,
		unsigned toPosition) {
	unsigned realToPosition = mPacketStart + toPosition;
	if (realToPosition + numBytes > mLimit) {
		if (realToPosition > mLimit)
			return; // we can't do this
		numBytes = mLimit - realToPosition;
	}

	memmove(&mBuf[realToPosition], from, numBytes);
	if (toPosition + numBytes > mCurOffset) {
		mCurOffset = toPosition + numBytes;
	}
}

void OutPacketBuffer::insertWord(unsigned long word, unsigned toPosition) {
	unsigned long nWord = htonl(word);
	insert((unsigned char*) &nWord, 4, toPosition);
}

void OutPacketBuffer::extract(unsigned char* to, unsigned numBytes,
		unsigned fromPosition) {
	unsigned realFromPosition = mPacketStart + fromPosition;
	if (realFromPosition + numBytes > mLimit) { // sanity check
		if (realFromPosition > mLimit)
			return; // we can't do this
		numBytes = mLimit - realFromPosition;
	}

	memmove(to, &mBuf[realFromPosition], numBytes);
}

unsigned long OutPacketBuffer::extractWord(unsigned fromPosition) {
	unsigned long nWord;
	extract((unsigned char*) &nWord, 4, fromPosition);
	return ntohl(nWord);
}

void OutPacketBuffer::skipBytes(unsigned numBytes) {
	if (numBytes > totalBytesAvailable()) {
		numBytes = totalBytesAvailable();
	}

	increment(numBytes);
}

void OutPacketBuffer::setOverflowData(unsigned overflowDataOffset,
		unsigned overflowDataSize, struct timeval const& presentationTime,
		unsigned durationInMicroseconds) {
	mOverflowDataOffset = overflowDataOffset;
	mOverflowDataSize = overflowDataSize;
	mOverflowPresentationTime = presentationTime;
	mOverflowDurationInMicroseconds = durationInMicroseconds;
}

void OutPacketBuffer::useOverflowData() {
	enqueue(&mBuf[mPacketStart + mOverflowDataOffset], mOverflowDataSize);
	mCurOffset -= mOverflowDataSize; // undoes increment performed by "enqueue"
	resetOverflowData();
}

void OutPacketBuffer::adjustPacketStart(unsigned numBytes) {
	mPacketStart += numBytes;
	if (mOverflowDataOffset >= numBytes) {
		mOverflowDataOffset -= numBytes;
	} else {
		mOverflowDataOffset = 0;
		mOverflowDataSize = 0; // an error otherwise
	}
}

void OutPacketBuffer::resetPacketStart() {
	if (mOverflowDataSize > 0) {
		mOverflowDataOffset += mPacketStart;
	}
	mPacketStart = 0;
}

//==============================
unsigned long our_random32()
{
	return 12345678;
}

bool RTPSource::hasBeenSynchronizedUsingRTCP() {
  return mCurPacketHasBeenSynchronizedUsingRTCP;
}

bool RTPSource::isRTPSource() const {
  return true;
}

RTPSource::RTPSource(
		unsigned char rtpPayloadFormat,
		unsigned long rtpTimestampFrequency)
		:mCurPacketHasBeenSynchronizedUsingRTCP(false), mLastReceivedSSRC(0),
		mRTPPayloadFormat(rtpPayloadFormat), mTimestampFrequency(rtpTimestampFrequency),
		mSSRC(our_random32()), mEnableRTCPReports(true)
{
		mReceptionStatsDB = new RTPReceptionStatsDB();
}

RTPSource::~RTPSource() {
  delete mReceptionStatsDB;
}

void RTPSource::getAttributes() const {

  /*envir().setResultMsg("");*/ // Fix later to get attributes from  header #####
}


////////// RTPReceptionStatsDB //////////

RTPReceptionStatsDB::RTPReceptionStatsDB()
  : /*fTable(HashTable::create(ONE_WORD_HASH_KEYS)),*/ mTotNumPacketsReceived(0) {
  reset();
}

void RTPReceptionStatsDB::reset() {
  mNumActiveSourcesSinceLastReset = 0;

  rec_tbl_iter it;
  for (it = mTable.begin(); it != mTable.end(); it++) {
	  RTPReceptionStats* stats = it->second;
	  stats->reset();
  }
}

RTPReceptionStatsDB::~RTPReceptionStatsDB() {
  // First, remove and delete all stats records from the table:
  RTPReceptionStats* stats;
  rec_tbl_iter it;
  for (it = mTable.begin(); it != mTable.end(); it++) {
	  RTPReceptionStats* stats = it->second;
	  if(stats) delete stats;
  }

  // Then, delete the table itself:
  //delete mTable;
}

void RTPReceptionStatsDB::noteIncomingPacket(
		unsigned long   SSRC,
		unsigned short  seqNum,
		unsigned long   rtpTimestamp,
		unsigned        timestampFrequency,
		bool            useForJitterCalculation,
		struct timeval& resultPresentationTime,
		bool&           resultHasBeenSyncedUsingRTCP,
		unsigned        packetSize)
{
	++mTotNumPacketsReceived;
	RTPReceptionStats* stats = lookup(SSRC);
	if (stats == NULL) {
		// This is the first time we've heard from this SSRC.
		// Create a new record for it:
		stats = new RTPReceptionStats(SSRC, seqNum);
		if (stats == NULL)
			return;
		add(SSRC, stats);
	}

	if (stats->numPacketsReceivedSinceLastReset() == 0) {
		++mNumActiveSourcesSinceLastReset;
	}

	stats->noteIncomingPacket(
			seqNum,
			rtpTimestamp,
			timestampFrequency,
			useForJitterCalculation,
			resultPresentationTime,
			resultHasBeenSyncedUsingRTCP, packetSize);
}

void RTPReceptionStatsDB::noteIncomingSR(
		unsigned long SSRC,
		unsigned long ntpTimestampMSW,
		unsigned long ntpTimestampLSW,
		unsigned long rtpTimestamp) {
	RTPReceptionStats* stats = lookup(SSRC);
	if (stats == NULL) {
		// This is the first time we've heard of this SSRC.
		// Create a new record for it:
		stats = new RTPReceptionStats(SSRC);
		if (stats == NULL) return;
		add(SSRC, stats);
	}
	stats->noteIncomingSR(ntpTimestampMSW, ntpTimestampLSW, rtpTimestamp);
}

void RTPReceptionStatsDB::removeRecord(unsigned long SSRC) {
	rec_tbl_iter it = mTable.find(SSRC);
	if (it != mTable.end()) {
		RTPReceptionStats* stats = it->second;
		if (stats != NULL) {
			delete stats;
		}
		mTable.erase(it);
	}
}

RTPReceptionStats* RTPReceptionStatsDB::lookup(unsigned long SSRC) {
	RTPReceptionStats* stats = NULL;
	rec_tbl_iter it = mTable.find(SSRC);
	if (it != mTable.end()) {
		stats = it->second;
	}
	return stats;
}

void RTPReceptionStatsDB::add(unsigned long SSRC, RTPReceptionStats* stats) {
  long SSRC_long = (long)SSRC;
  mTable[SSRC] = stats;
}

////////// RTPReceptionStats //////////

RTPReceptionStats::RTPReceptionStats(unsigned long SSRC, unsigned short initialSeqNum) {
  initSeqNum(initialSeqNum);
  init(SSRC);
}

RTPReceptionStats::RTPReceptionStats(unsigned long SSRC) {
  init(SSRC);
}

RTPReceptionStats::~RTPReceptionStats() {
}

void RTPReceptionStats::init(unsigned long SSRC) {
	mSSRC = SSRC;
	mTotNumPacketsReceived = 0;
	mTotBytesReceived_hi = mTotBytesReceived_lo = 0;
	mBaseExtSeqNumReceived = 0;
	mHighestExtSeqNumReceived = 0;
	mHaveSeenInitialSequenceNumber = false;
	mLastTransit = ~0;
	mPreviousPacketRTPTimestamp = 0;
	mJitter = 0.0;
	mLastReceivedSR_NTPmsw = mLastReceivedSR_NTPlsw = 0;
	mLastReceivedSR_time.tv_sec = mLastReceivedSR_time.tv_usec = 0;
	mLastPacketReceptionTime.tv_sec = mLastPacketReceptionTime.tv_usec = 0;
	mMinInterPacketGapUS = 0x7FFFFFFF;
	mMaxInterPacketGapUS = 0;
	mTotalInterPacketGaps.tv_sec = mTotalInterPacketGaps.tv_usec = 0;
	mHasBeenSynchronized = false;
	mSyncTime.tv_sec = mSyncTime.tv_usec = 0;
	reset();
}

void RTPReceptionStats::initSeqNum(unsigned short initialSeqNum) {
    mBaseExtSeqNumReceived = 0x10000 | initialSeqNum;
    mHighestExtSeqNumReceived = 0x10000 | initialSeqNum;
    mHaveSeenInitialSequenceNumber = true;
}

#ifndef MILLION
#define MILLION 1000000
#endif

void RTPReceptionStats::noteIncomingPacket(unsigned short seqNum,
		unsigned long rtpTimestamp, unsigned timestampFrequency,
		bool useForJitterCalculation, struct timeval& resultPresentationTime,
		bool& resultHasBeenSyncedUsingRTCP, unsigned packetSize) {
	if (!mHaveSeenInitialSequenceNumber)
		initSeqNum(seqNum);

	++mNumPacketsReceivedSinceLastReset;
	++mTotNumPacketsReceived;
	unsigned long prevTotBytesReceived_lo = mTotBytesReceived_lo;
	mTotBytesReceived_lo += packetSize;
	if (mTotBytesReceived_lo < prevTotBytesReceived_lo) { // wrap-around
		++mTotBytesReceived_hi;
	}

	// Check whether the new sequence number is the highest yet seen:
	unsigned oldSeqNum = (mHighestExtSeqNumReceived & 0xFFFF);
	unsigned seqNumCycle = (mHighestExtSeqNumReceived & 0xFFFF0000);
	unsigned seqNumDifference = (unsigned) ((int) seqNum - (int) oldSeqNum);
	unsigned newSeqNum = 0;
	if (seqNumLT((unsigned short) oldSeqNum, seqNum)) {
		// This packet was not an old packet received out of order, so check it:
		if (seqNumDifference >= 0x8000) {
			// The sequence number wrapped around, so start a new cycle:
			seqNumCycle += 0x10000;
		}
		newSeqNum = seqNumCycle | seqNum;
		if (newSeqNum > mHighestExtSeqNumReceived) {
			mHighestExtSeqNumReceived = newSeqNum;
		}
	} else if (mTotNumPacketsReceived > 1) {
		// This packet was an old packet received out of order
		if ((int) seqNumDifference >= 0x8000) {
			// The sequence number wrapped around, so switch to an old cycle:
			seqNumCycle -= 0x10000;
		}
		newSeqNum = seqNumCycle | seqNum;
		if (newSeqNum < mBaseExtSeqNumReceived) {
			mBaseExtSeqNumReceived = newSeqNum;
		}
	}

	// Record the inter-packet delay
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);
	if (mLastPacketReceptionTime.tv_sec != 0
			|| mLastPacketReceptionTime.tv_usec != 0) {
		unsigned gap = (timeNow.tv_sec - mLastPacketReceptionTime.tv_sec)
				* MILLION + timeNow.tv_usec - mLastPacketReceptionTime.tv_usec;
		if (gap > mMaxInterPacketGapUS) {
			mMaxInterPacketGapUS = gap;
		}
		if (gap < mMinInterPacketGapUS) {
			mMinInterPacketGapUS = gap;
		}
		mTotalInterPacketGaps.tv_usec += gap;
		if (mTotalInterPacketGaps.tv_usec >= MILLION) {
			++mTotalInterPacketGaps.tv_sec;
			mTotalInterPacketGaps.tv_usec -= MILLION;
		}
	}
	mLastPacketReceptionTime = timeNow;

	// Compute the current 'jitter' using the received packet's RTP timestamp,
	// and the RTP timestamp that would correspond to the current time.
	// (Use the code from appendix A.8 in the RTP spec.)
	// Note, however, that we don't use this packet if its timestamp is
	// the same as that of the previous packet (this indicates a multi-packet
	// fragment), or if we've been explicitly told not to use this packet.
	if (useForJitterCalculation
			&& rtpTimestamp != mPreviousPacketRTPTimestamp) {
		unsigned arrival = (timestampFrequency * timeNow.tv_sec);
		arrival += (unsigned) ((2.0 * timestampFrequency * timeNow.tv_usec
				+ 1000000.0) / 2000000);
		// note: rounding
		int transit = arrival - rtpTimestamp;
		if (mLastTransit == (~0))
			mLastTransit = transit; // hack for first time
		int d = transit - mLastTransit;
		mLastTransit = transit;
		if (d < 0)
			d = -d;
		mJitter += (1.0 / 16.0) * ((double) d - mJitter);
	}

	// Return the 'presentation time' that corresponds to "rtpTimestamp":
	if (mSyncTime.tv_sec == 0 && mSyncTime.tv_usec == 0) {
		// This is the first timestamp that we've seen, so use the current
		// 'wall clock' time as the synchronization time.  (This will be
		// corrected later when we receive RTCP SRs.)
		mSyncTimestamp = rtpTimestamp;
		mSyncTime = timeNow;
	}

	int timestampDiff = rtpTimestamp - mSyncTimestamp;
	// Note: This works even if the timestamp wraps around
	// (as long as "int" is 32 bits)

	// Divide this by the timestamp frequency to get real time:
	double timeDiff = timestampDiff / (double) timestampFrequency;

	// Add this to the 'sync time' to get our result:
	unsigned const million = 1000000;
	unsigned seconds, uSeconds;
	if (timeDiff >= 0.0) {
		seconds = mSyncTime.tv_sec + (unsigned) (timeDiff);
		uSeconds = mSyncTime.tv_usec
				+ (unsigned) ((timeDiff - (unsigned) timeDiff) * million);
		if (uSeconds >= million) {
			uSeconds -= million;
			++seconds;
		}
	} else {
		timeDiff = -timeDiff;
		seconds = mSyncTime.tv_sec - (unsigned) (timeDiff);
		uSeconds = mSyncTime.tv_usec
				- (unsigned) ((timeDiff - (unsigned) timeDiff) * million);
		if ((int) uSeconds < 0) {
			uSeconds += million;
			--seconds;
		}
	}
	resultPresentationTime.tv_sec = seconds;
	resultPresentationTime.tv_usec = uSeconds;
	resultHasBeenSyncedUsingRTCP = mHasBeenSynchronized;

	// Save these as the new synchronization timestamp & time:
	mSyncTimestamp = rtpTimestamp;
	mSyncTime = resultPresentationTime;

	mPreviousPacketRTPTimestamp = rtpTimestamp;
}

void RTPReceptionStats::noteIncomingSR(unsigned long ntpTimestampMSW,
		unsigned long ntpTimestampLSW, unsigned long rtpTimestamp) {
	mLastReceivedSR_NTPmsw = ntpTimestampMSW;
	mLastReceivedSR_NTPlsw = ntpTimestampLSW;

	gettimeofday(&mLastReceivedSR_time, NULL);

	// Use this SR to update time synchronization information:
	mSyncTimestamp = rtpTimestamp;
	mSyncTime.tv_sec = ntpTimestampMSW - 0x83AA7E80; // 1/1/1900 -> 1/1/1970
	double microseconds = (ntpTimestampLSW * 15625.0) / 0x04000000; // 10^6/2^32
	mSyncTime.tv_usec = (unsigned) (microseconds + 0.5);
	mHasBeenSynchronized = true;
}

double RTPReceptionStats::totNumKBytesReceived() const {
	double const hiMultiplier = 0x20000000 / 125.0; // == (2^32)/(10^3)
	return mTotBytesReceived_hi * hiMultiplier + mTotBytesReceived_lo / 1000.0;
}

unsigned RTPReceptionStats::jitter() const {
  return (unsigned)mJitter;
}

void RTPReceptionStats::reset() {
	mNumPacketsReceivedSinceLastReset = 0;
	mLastResetExtSeqNumReceived = mHighestExtSeqNumReceived;
}

typedef std::map<unsigned long, unsigned long> MemberMap;
typedef std::map<unsigned long, unsigned long>::iterator MemberMap_Iter;

class RTCPMemberDatabase {
public:
	RTCPMemberDatabase(CJdRtcp& ourRTCPInstance) :
			mOurRTCPInstance(ourRTCPInstance) {
	}

	virtual ~RTCPMemberDatabase() {
		//delete fTable;
	}

	bool isMember(unsigned long ssrc) const {
		return mTable.find(ssrc) != mTable.end();
	}

	bool noteMembership(unsigned long ssrc, unsigned curTimeCount) {
		bool isNew = !isMember(ssrc);

		// Record the current time, so we can age stale members
		mTable[ssrc] = (long) curTimeCount;

		return isNew;
	}

	bool remove(unsigned long ssrc) {
		Remove(ssrc);
		return true;
	}

	unsigned numMembers() const {
		return mTable.size();
	}

	void reapOldMembers(unsigned threshold);

private:
	void Remove(unsigned long Ssrc) {
		MemberMap_Iter it = mTable.find(Ssrc);
		if(it != mTable.end())
			mTable.erase(it);
	}
	CJdRtcp& mOurRTCPInstance;
	MemberMap mTable;
};

void RTCPMemberDatabase::reapOldMembers(unsigned threshold) {
	bool foundOldMember;
	unsigned long oldSSRC = 0;

	do {
		foundOldMember = 0;
		MemberMap_Iter it;

		for(it = mTable.begin(); it!= mTable.end(); ++it){
			if (it->second <  threshold) {
				// this SSRC is old
				oldSSRC = it->first;
				foundOldMember = true;
			}
		}
		if(it != mTable.end()) {
			mTable.erase(it);
			if (foundOldMember) {
				mOurRTCPInstance.removeSSRC(oldSSRC, true);
			}
		}
	} while (foundOldMember);
}


CJdRtcp::CJdRtcp()
{
	m_maxBufferLen = 2048;
	m_pBuffer = (char *)malloc(m_maxBufferLen);
}

double CJdRtcp::rtcp_interval(int members,
                        int senders,
                        double rtcp_bw,
                        int we_sent,
                        double avg_rtcp_size,
                        int initial)
   {
       /*
        * Minimum average time between RTCP packets from this site (in
        * seconds).  This time prevents the reports from `clumping' when
        * sessions are small and the law of large numbers isn't helping
        * to smooth out the traffic.  It also keeps the report interval
        * from becoming ridiculously small during transient outages like
        * a network partition.
        */
       double const RTCP_MIN_TIME = 5.;
       /*
        * Fraction of the RTCP bandwidth to be shared among active
        * senders.  (This fraction was chosen so that in a typical
        * session with one or two active senders, the computed report
        * time would be roughly equal to the minimum report time so that
        * we don't unnecessarily slow down receiver reports.) The
        * receiver fraction must be 1 - the sender fraction.
        */
       double const RTCP_SENDER_BW_FRACTION = 0.25;
       double const RTCP_RCVR_BW_FRACTION = (1-RTCP_SENDER_BW_FRACTION);
       /*
        * To compensate for "unconditional reconsideration" converging to a
        * value below the intended average.
        */
       double const COMPENSATION = 2.71828 - 1.5;

       double t;                   /* interval */
       double rtcp_min_time = RTCP_MIN_TIME;
       int n;                      /* no. of members for computation */

       /*
        * Very first call at application start-up uses half the min
        * delay for quicker notification while still allowing some time
        * before reporting for randomization and to learn about other
        * sources so the report interval will converge to the correct
        * interval more quickly.
        */
       if (initial) {
           rtcp_min_time /= 2;
       }

       /*
        * If there were active senders, give them at least a minimum
        * share of the RTCP bandwidth.  Otherwise all participants share
        * the RTCP bandwidth equally.
        */
       n = members;
       if (senders > 0 && senders < members * RTCP_SENDER_BW_FRACTION) {
           if (we_sent) {
               rtcp_bw *= RTCP_SENDER_BW_FRACTION;
               n = senders;
           } else {
               rtcp_bw *= RTCP_RCVR_BW_FRACTION;
               n -= senders;
           }
       }

       /*
        * The effective number of sites times the average packet size is
        * the total number of octets sent when each site sends a report.
        * Dividing this by the effective bandwidth gives the time
        * interval over which those packets must be sent in order to
        * meet the bandwidth target, with a minimum enforced.  In that
        * time interval we send one report so this time is also our
        * average time between reports.
        */
       t = avg_rtcp_size * n / rtcp_bw;
       if (t < rtcp_min_time) t = rtcp_min_time;

       /*
        * To avoid traffic bursts from unintended synchronization with
        * other sites, we then pick our actual next report interval as a
        * random number uniformly distributed between 0.5*t and 1.5*t.
        */
       t = t * (drand48() + 0.5);
       t = t / COMPENSATION;
       return t;
   }

   void CJdRtcp::OnExpire(void *e,
                 int    members,
                 int    senders,
                 double rtcp_bw,
                 int    we_sent,
                 double *avg_rtcp_size,
                 int    *initial,
                 time_tp   tc,
                 time_tp   *tp,
                 int    *pmembers)
   {
       /* This function is responsible for deciding whether to send
        * an RTCP report or BYE packet now, or to reschedule transmission.
        * It is also responsible for updating the pmembers, initial, tp,
        * and avg_rtcp_size state variables. This function should be called
        * upon expiration of the event timer used by Schedule(). */

       double t;     /* Interval */
       double tn;    /* Next transmit time */

       /* In the case of a BYE, we use "unconditional reconsideration" to
        * reschedule the transmission of the BYE if necessary */

       if (TypeOfEvent(e) == EVENT_BYE) {
           t = rtcp_interval(members,
                             senders,
                             rtcp_bw,
                             we_sent,
                             *avg_rtcp_size,
                             *initial);
           tn = *tp + t;
           if (tn <= tc) {
               SendBYEPacket(e);
               exit(1);
           } else {
               Schedule(tn, e);
           }

       } else if (TypeOfEvent(e) == EVENT_REPORT) {
           t = rtcp_interval(members,
                             senders,
                             rtcp_bw,
                             we_sent,
                             *avg_rtcp_size,
                             *initial);
           tn = *tp + t;

           if (tn <= tc) {
               SendRTCPReport(e);
               *avg_rtcp_size = (1./16.)*SentPacketSize(e) +
                   (15./16.)*(*avg_rtcp_size);
               *tp = tc;

               /* We must redraw the interval. Don't reuse the
                  one computed above, since its not actually
                  distributed the same, as we are conditioned
                  on it being small enough to cause a packet to
                  be sent */

               t = rtcp_interval(members,
                                 senders,
                                 rtcp_bw,
                                 we_sent,
                                 *avg_rtcp_size,
                                 *initial);

               Schedule(t+tc,e);
               *initial = 0;
           } else {
               Schedule(tn, e);
           }
           *pmembers = members;
       }
   }


   void CJdRtcp::OnReceive(void * p,
                  void * e,
                  int *members,
                  int *pmembers,
                  int *senders,
                  double *avg_rtcp_size,
                  double *tp,
                  double tc,
                  double tn)
   {
       /* What we do depends on whether we have left the group, and
        * are waiting to send a BYE (TypeOfEvent(e) == EVENT_BYE) or
        * an RTCP report. p represents the packet that was just received. */

       if (PacketType(p) == PACKET_RTCP_REPORT) {
           if (NewMember(p) && (TypeOfEvent(e) == EVENT_REPORT)) {
               AddMember(p);
               *members += 1;
           }
           *avg_rtcp_size = (1./16.)*ReceivedPacketSize(p) +
               (15./16.)*(*avg_rtcp_size);
       } else if (PacketType(p) == PACKET_RTP) {
           if (NewMember(p) && (TypeOfEvent(e) == EVENT_REPORT)) {
               AddMember(p);
               *members += 1;
           }
           if (NewSender(p) && (TypeOfEvent(e) == EVENT_REPORT)) {
               AddSender(p);
               *senders += 1;
           }
       } else if (PacketType(p) == PACKET_BYE) {
           *avg_rtcp_size = (1./16.)*ReceivedPacketSize(p) +
               (15./16.)*(*avg_rtcp_size);

           if (TypeOfEvent(e) == EVENT_REPORT) {
               if (NewSender(p) == false) {
                   RemoveSender(p);
                   *senders -= 1;
               }

               if (NewMember(p) == false) {
                   RemoveMember(p);
                   *members -= 1;
               }

               if(*members < *pmembers) {
                   tn = tc + (((double) *members)/(*pmembers))*(tn - tc);
                   *tp = tc - (((double) *members)/(*pmembers))*(tc - *tp);

                   /* Reschedule the next report for time tn */

                   Reschedule(tn, e);
                   *pmembers = *members;
               }

           } else if (TypeOfEvent(e) == EVENT_BYE) {
               *members += 1;
           }
       }
}


void CJdRtcp::removeSSRC(unsigned long ssrc, bool alsoRemoveStats) {
	mKnownMembers->remove(ssrc);

	if (alsoRemoveStats) {
		// Also, remove records of this SSRC from any reception or transmission stats
/*		if (mSource != NULL)
			mSource->receptionStatsDB().removeRecord(ssrc);*/
		if (mSink != NULL)
			mSink->transmissionStatsDB().removeRecord(ssrc);
	}
}

static double dTimeNow()
{
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    return (double) (timeNow.tv_sec + timeNow.tv_usec/1000000.0);
}


void CJdRtcp::processIncomingReport(
			unsigned packetSize,
		struct sockaddr_in const& fromAddressAndPort, int tcpSocketNum,
		unsigned char tcpStreamChannelId)
{
	do {
		bool callByeHandler = false;
		unsigned char* pkt = (unsigned char*)m_pBuffer;

		int totPacketSize = IP_UDP_HDR_SIZE + packetSize;

		// Check the RTCP packet for validity:
		// It must at least contain a header (4 bytes), and this header
		// must be version=2, with no padding bit, and a payload type of
		// SR (200), RR (201), or APP (204):
		if (packetSize < 4) {
			// TODO Error
			break;
		}
		unsigned rtcpHdr = ntohl(*(unsigned long*) pkt);
		if ((rtcpHdr & 0xE0FE0000) != (0x80000000 | (RTCP_PT_SR << 16))
				&& (rtcpHdr & 0xE0FF0000) != (0x80000000 | (RTCP_PT_APP << 16))) {
			// TODO Error
			break;
		}

		// Process each of the individual RTCP 'subpackets' in (what may be)
		// a compound RTCP packet.
		int typeOfPacket = PACKET_UNKNOWN_TYPE;
		unsigned reportSenderSSRC = 0;
		bool packetOK = false;
		while (1) {
			unsigned char rc = (rtcpHdr >> 24) & 0x1F;
			unsigned char pt = (rtcpHdr >> 16) & 0xFF;
			unsigned length = 4 * (rtcpHdr & 0xFFFF); // doesn't count hdr
			pkt += 4; // skip over the header
			if (length > packetSize) {
				// TODO Error
				break;
			}
			// Assume that each RTCP subpacket begins with a 4-byte SSRC:
			if (length < 4) {
				// TODO Error
				break;
			}
			length -= 4;
			reportSenderSSRC = ntohl(*(unsigned long*) pkt);
			pkt += 4;

			bool subPacketOK = false;

			switch (pt) {
			case RTCP_PT_SR: {
				if (length < 20) {
					// TODO Error
					break;
				}
				length -= 20;

				// Extract the NTP timestamp, and note this:
				unsigned NTPmsw = ntohl(*(unsigned long*) pkt);
				pkt += 4;
				unsigned NTPlsw = ntohl(*(unsigned long*) pkt);
				pkt += 4;
				unsigned rtpTimestamp = ntohl(*(unsigned long*) pkt);
				pkt += 4;
				if (mSource != NULL) {
					RTPReceptionStatsDB& receptionStats = mSource->receptionStatsDB();
					receptionStats.noteIncomingSR(reportSenderSSRC, NTPmsw, NTPlsw,
							rtpTimestamp);
				}
				pkt += 8; // skip over packet count, octet count

				// If a 'SR handler' was set, call it now:
				if (fSRHandlerTask != NULL)
					(*fSRHandlerTask)(fSRHandlerClientData);

				// The rest of the SR is handled like a RR (so, no "break;" here)
			}
/*			case RTCP_PT_RR: {
				unsigned reportBlocksSize = rc * (6 * 4);
				if (length < reportBlocksSize)
					break;
				length -= reportBlocksSize;

				if (mSink != NULL) {
					// Use this information to update stats about our transmissions:
					RTPTransmissionStatsDB& transmissionStats =
							mSink->transmissionStatsDB();
					for (unsigned i = 0; i < rc; ++i) {
						unsigned senderSSRC = ntohl(*(unsigned long*) pkt);
						pkt += 4;
						// We care only about reports about our own transmission, not others'
						if (senderSSRC == mSink->SSRC()) {
							unsigned lossStats = ntohl(*(unsigned long*) pkt);
							pkt += 4;
							unsigned highestReceived = ntohl(*(unsigned long*) pkt);
							pkt += 4;
							unsigned jitter = ntohl(*(unsigned long*) pkt);
							pkt += 4;
							unsigned timeLastSR = ntohl(*(unsigned long*) pkt);
							pkt += 4;
							unsigned timeSinceLastSR = ntohl(*(unsigned long*) pkt);
							pkt += 4;
							transmissionStats.noteIncomingRR(reportSenderSSRC,
									fromAddressAndPort, lossStats, highestReceived,
									jitter, timeLastSR, timeSinceLastSR);
						} else {
							pkt += (4 * 5);
						}
					}
				} else {
					pkt += reportBlocksSize;
				}

				if (pt == RTCP_PT_RR) { // i.e., we didn't fall through from 'SR'
					noteArrivingRR(fromAddressAndPort, tcpSocketNum,
							tcpStreamChannelId);
				}

				subPacketOK = true;
				typeOfPacket = PACKET_RTCP_REPORT;
				break;
			}*/
			case RTCP_PT_BYE: {
				// If a 'BYE handler' was set, arrange for it to be called at the end of this routine.
				// (Note: We don't call it immediately, in case it happens to cause "this" to be deleted.)
				if (fByeHandlerTask != NULL
						&& (!fByeHandleActiveParticipantsOnly
								|| (mSource != NULL
										&& mSource->receptionStatsDB().lookup(
												reportSenderSSRC) != NULL)
								|| (mSink != NULL
										&& mSink->transmissionStatsDB().lookup(
												reportSenderSSRC) != NULL))) {
					callByeHandler = true;
				}

				// We should really check for & handle >1 SSRCs being present #####

				subPacketOK = true;
				typeOfPacket = PACKET_BYE;
				break;
			}
/*
			case RTCP_PT_APP: {
				unsigned char& subtype = rc; // In "APP" packets, the "rc" field gets used as "subtype"
				if (length < 4) {
					break;
				}
				unsigned long nameBytes = (pkt[0] << 24) | (pkt[1] << 16)
						| (pkt[2] << 8) | (pkt[3]);
				pkt += 4; // skip over "name", to the 'application-dependent data'

				// If an 'APP' packet handler was set, call it now:
				if (fAppHandlerTask != NULL) {
					(*fAppHandlerTask)(fAppHandlerClientData, subtype, nameBytes,
							pkt, length);
				}
				subPacketOK = true;
				typeOfPacket = PACKET_RTCP_APP;
				break;
			}*/
				// Other RTCP packet types that we don't yet handle:
			case RTCP_PT_SDES: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_RTPFB: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_PSFB: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_XR: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_AVB: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_RSI: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_TOKEN: {
				subPacketOK = true;
				break;
			}
			case RTCP_PT_IDMS: {
				subPacketOK = true;
				break;
			}
			default: {
				subPacketOK = true;
				break;
			}
			}
			if (!subPacketOK)
				break;

			// need to check for (& handle) SSRC collision! #####

			// Skip over any remaining bytes in this subpacket:
			pkt += length;

			// Check whether another RTCP 'subpacket' follows:
			if (packetSize == 0) {
				packetOK = true;
				break;
			} else if (packetSize < 4) {
				break;
			}
			rtcpHdr = ntohl(*(unsigned long*) pkt);
			if ((rtcpHdr & 0xC0000000) != 0x80000000) {
				break;
			}
		}

		if (!packetOK) {
			break;
		} else {
		}

		onReceive(typeOfPacket, totPacketSize, reportSenderSSRC);

		// Finally, if we need to call a "BYE" handler, do so now (in case it causes "this" to get deleted):
		if (callByeHandler && fByeHandlerTask != NULL/*sanity check*/) {
			TaskFunc* byeHandler = fByeHandlerTask;
			fByeHandlerTask = NULL; // because we call the handler only once, by default
			(*byeHandler)(fByeHandlerClientData);
		}
	} while (0);
}

void CJdRtcp::onReceive(int typeOfPacket, int totPacketSize, unsigned long ssrc) {
  mTypeOfPacket = typeOfPacket;
  mLastReceivedSize = totPacketSize;
  mLastReceivedSSRC = ssrc;

  int members = (int)mKnownMembers->numMembers();
  int senders = (mSink != NULL) ? 1 : 0;

  OnReceive(this, // p
	    this, // e
	    &members, // members
	    &mPrevNumMembers, // pmembers
	    &senders, // senders
	    &mAveRTCPSize, // avg_rtcp_size
	    &mPrevReportTime, // tp
	    dTimeNow(), // tc
	    mNextReportTime);
}
void CJdRtcp::Process(void *_pRtp)
{
	JdRtp *pRtp = (JdRtp *)pRtp;
	int bytesRead = pRtp->ReadRtcp(m_pBuffer, m_maxBufferLen);
	if(bytesRead > 0){
		unsigned packetSize = bytesRead;
		 struct sockaddr_in const fromAddressAndPort;
		 int tcpSocketNum;
		 unsigned char tcpStreamChannelId;
		processIncomingReport(packetSize, fromAddressAndPort, tcpSocketNum, tcpStreamChannelId);
	}
}

void CJdRtcp::removeSSRC(unsigned long ssrc, bool alsoRemoveStats) {
  mKnownMembers->remove(ssrc);

  if (alsoRemoveStats) {
    // Also, remove records of this SSRC from any reception or transmission stats

	  //if (mSource != NULL) mSource->receptionStatsDB().removeRecord(ssrc);
    if (mSink != NULL) mSink->transmissionStatsDB().removeRecord(ssrc);
  }
}


void CJdRtcp::addRR() {
  // ASSERT: mSource != NULL

  enqueueCommonReportPrefix(RTCP_PT_RR, mSource->SSRC());
  enqueueCommonReportSuffix();
}

void CJdRtcp::enqueueCommonReportPrefix(unsigned char packetType,
					     unsigned long SSRC,
					     unsigned numExtraWords) {
	unsigned numReportingSources;
	if (mSource == NULL) {
		numReportingSources = 0; // we don't receive anything
	} else {
/*

		RTPReceptionStatsDB& allReceptionStats = mSource->receptionStatsDB();
		numReportingSources =
				allReceptionStats.numActiveSourcesSinceLastReset();
*/
		numReportingSources = 1; // TODO
		// This must be <32, to fit in 5 bits:
		if (numReportingSources >= 32) {
			numReportingSources = 32;
		}
		// Later: support adding more reports to handle >32 sources (unlikely)#####
	}

	unsigned rtcpHdr = 0x80000000; // version 2, no padding
	rtcpHdr |= (numReportingSources << 24);
	rtcpHdr |= (packetType << 16);
	rtcpHdr |= (1 + numExtraWords + 6 * numReportingSources);
	// each report block is 6 32-bit words long
	mOutBuf->enqueueWord(rtcpHdr);

	mOutBuf->enqueueWord(SSRC);
}

void CJdRtcp::enqueueCommonReportSuffix()
{
	// Output the report blocks for each source:
	if (mSource != NULL) {
		RTPReceptionStatsDB *stats = mSource->receptionStatsDB();
		if (stats){
			enqueueReportBlock(stats);
		}
	}
}

void
CJdRtcp::enqueueReportBlock(RTPReceptionStats* stats)
{
	mOutBuf->enqueueWord(stats->SSRC());

	unsigned highestExtSeqNumReceived = stats->highestExtSeqNumReceived();

	unsigned totNumExpected = highestExtSeqNumReceived 	- stats->baseExtSeqNumReceived();
	int totNumLost = totNumExpected - stats->totNumPacketsReceived();
	// 'Clamp' this loss number to a 24-bit signed value:
	if (totNumLost > 0x007FFFFF) {
		totNumLost = 0x007FFFFF;
	} else if (totNumLost < 0) {
		if (totNumLost < -0x00800000)
			totNumLost = 0x00800000; // unlikely, but...
		totNumLost &= 0x00FFFFFF;
	}

	unsigned numExpectedSinceLastReset = highestExtSeqNumReceived
			- stats->lastResetExtSeqNumReceived();
	int numLostSinceLastReset = numExpectedSinceLastReset
			- stats->numPacketsReceivedSinceLastReset();
	unsigned char lossFraction;
	if (numExpectedSinceLastReset == 0 || numLostSinceLastReset < 0) {
		lossFraction = 0;
	} else {
		lossFraction = (unsigned char) ((numLostSinceLastReset << 8)
				/ numExpectedSinceLastReset);
	}

	mOutBuf->enqueueWord((lossFraction << 24) | totNumLost);
	mOutBuf->enqueueWord(highestExtSeqNumReceived);

	mOutBuf->enqueueWord(stats->jitter());

	unsigned NTPmsw = stats->lastReceivedSR_NTPmsw();
	unsigned NTPlsw = stats->lastReceivedSR_NTPlsw();
	unsigned LSR = ((NTPmsw & 0xFFFF) << 16) | (NTPlsw >> 16); // middle 32 bits
	mOutBuf->enqueueWord(LSR);

	// Figure out how long has elapsed since the last SR rcvd from this src:
	struct timeval const& LSRtime = stats->lastReceivedSR_time(); // "last SR"
	struct timeval timeNow, timeSinceLSR;
	gettimeofday(&timeNow, NULL);
	if (timeNow.tv_usec < LSRtime.tv_usec) {
		timeNow.tv_usec += 1000000;
		timeNow.tv_sec -= 1;
	}
	timeSinceLSR.tv_sec = timeNow.tv_sec - LSRtime.tv_sec;
	timeSinceLSR.tv_usec = timeNow.tv_usec - LSRtime.tv_usec;
	// The enqueued time is in units of 1/65536 seconds.
	// (Note that 65536/1000000 == 1024/15625)
	unsigned DLSR;
	if (LSR == 0) {
		DLSR = 0;
	} else {
		DLSR = (timeSinceLSR.tv_sec << 16)
				| ((((timeSinceLSR.tv_usec << 11) + 15625) / 31250) & 0xFFFF);
	}
	mOutBuf->enqueueWord(DLSR);
}

bool CJdRtcp::addReport(bool alwaysAdd)
{
  // Include a SR or a RR, depending on whether we have an associated sink or source:
/*
  if (fSink != NULL) {
    if (!alwaysAdd) {
      if (!fSink->enableRTCPReports()) return False;

      // Hack: Don't send a SR during those (brief) times when the timestamp of the
      // next outgoing RTP packet has been preset, to ensure that that timestamp gets
      // used for that outgoing packet. (David Bertrand, 2006.07.18)
      if (fSink->nextTimestampHasBeenPreset()) return False;
    }

    addSR();
  }
*/
  if (mSource != NULL) {
 /*   if (!alwaysAdd) {
      if (!fSource->enableRTCPReports()) return False;
    }*/

    addRR();
  }

  return true;
}
