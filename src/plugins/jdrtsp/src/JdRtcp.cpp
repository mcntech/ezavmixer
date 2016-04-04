#include "JdRtcp.h"
#include <map>
#include "JdRtp.h"
#include <sys/endian.h>

typedef std::map<unsigned long, unsigned long> MemberMap;


////////// OutPacketBuffer //////////

// A data structure that a sink may use for an output packet:
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
	void enqueueWord(u_int32_t word);
	void insert(unsigned char const* from, unsigned numBytes,
			unsigned toPosition);
	void insertWord(u_int32_t word, unsigned toPosition);
	void extract(unsigned char* to, unsigned numBytes, unsigned fromPosition);
	u_int32_t extractWord(unsigned fromPosition);

	void skipBytes(unsigned numBytes);

	Boolean isPreferredSize() const {
		return fCurOffset >= fPreferred;
	}
	Boolean wouldOverflow(unsigned numBytes) const {
		return (fCurOffset + numBytes) > fMax;
	}
	unsigned numOverflowBytes(unsigned numBytes) const {
		return (fCurOffset + numBytes) - fMax;
	}
	Boolean isTooBigForAPacket(unsigned numBytes) const {
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
	Boolean haveOverflowData() const {
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

unsigned OutPacketBuffer::maxSize = 60000; // by default

OutPacketBuffer::OutPacketBuffer(unsigned preferredPacketSize,
		unsigned maxPacketSize, unsigned maxBufferSize) :
		fPreferred(preferredPacketSize), fMax(maxPacketSize), fOverflowDataSize(
				0) {
	if (maxBufferSize == 0)
		maxBufferSize = maxSize;
	unsigned maxNumPackets = (maxBufferSize + (maxPacketSize - 1))
			/ maxPacketSize;
	fLimit = maxNumPackets * maxPacketSize;
	fBuf = new unsigned char[fLimit];
	resetPacketStart();
	resetOffset();
	resetOverflowData();
}

OutPacketBuffer::~OutPacketBuffer() {
	delete[] fBuf;
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

void OutPacketBuffer::enqueueWord(u_int32_t word) {
	u_int32_t nWord = htonl(word);
	enqueue((unsigned char*) &nWord, 4);
}

void OutPacketBuffer::insert(unsigned char const* from, unsigned numBytes,
		unsigned toPosition) {
	unsigned realToPosition = fPacketStart + toPosition;
	if (realToPosition + numBytes > fLimit) {
		if (realToPosition > fLimit)
			return; // we can't do this
		numBytes = fLimit - realToPosition;
	}

	memmove(&fBuf[realToPosition], from, numBytes);
	if (toPosition + numBytes > fCurOffset) {
		fCurOffset = toPosition + numBytes;
	}
}

void OutPacketBuffer::insertWord(u_int32_t word, unsigned toPosition) {
	u_int32_t nWord = htonl(word);
	insert((unsigned char*) &nWord, 4, toPosition);
}

void OutPacketBuffer::extract(unsigned char* to, unsigned numBytes,
		unsigned fromPosition) {
	unsigned realFromPosition = fPacketStart + fromPosition;
	if (realFromPosition + numBytes > fLimit) { // sanity check
		if (realFromPosition > fLimit)
			return; // we can't do this
		numBytes = fLimit - realFromPosition;
	}

	memmove(to, &fBuf[realFromPosition], numBytes);
}

u_int32_t OutPacketBuffer::extractWord(unsigned fromPosition) {
	u_int32_t nWord;
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
	fOverflowDataOffset = overflowDataOffset;
	fOverflowDataSize = overflowDataSize;
	fOverflowPresentationTime = presentationTime;
	fOverflowDurationInMicroseconds = durationInMicroseconds;
}

void OutPacketBuffer::useOverflowData() {
	enqueue(&fBuf[fPacketStart + fOverflowDataOffset], fOverflowDataSize);
	fCurOffset -= fOverflowDataSize; // undoes increment performed by "enqueue"
	resetOverflowData();
}

void OutPacketBuffer::adjustPacketStart(unsigned numBytes) {
	fPacketStart += numBytes;
	if (fOverflowDataOffset >= numBytes) {
		fOverflowDataOffset -= numBytes;
	} else {
		fOverflowDataOffset = 0;
		fOverflowDataSize = 0; // an error otherwise
	}
}

void OutPacketBuffer::resetPacketStart() {
	if (fOverflowDataSize > 0) {
		fOverflowDataOffset += fPacketStart;
	}
	fPacketStart = 0;
}

class RTCPMemberDatabase {
public:
	RTCPMemberDatabase(CJdRtcp& ourRTCPInstance) :
			mOurRTCPInstance(ourRTCPInstance), mNumMembers(1 /*ourself*/) {
	}

	virtual ~RTCPMemberDatabase() {
		//delete fTable;
	}

	Boolean isMember(unsigned long ssrc) const {
		return mTable.find((ssrc) != NULL;
	}

	Boolean noteMembership(unsigned long ssrc, unsigned curTimeCount) {
		bool isNew = !isMember(ssrc);

		// Record the current time, so we can age stale members
		mTable[ssrc] = (long) curTimeCount;

		return isNew;
	}

	bool remove(unsigned long ssrc) {
		bool wasPresent = Remove(ssrc);
		return wasPresent;
	}

	unsigned numMembers() const {
		return mTable.size();
	}

	void reapOldMembers(unsigned threshold);

private:
	void Remove(unsigned long Ssrc) {
		MemberMap::iterator it = mTable.find(Ssrc);
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

		MemberMap::Iterator it;

		for(it=mTable.begin(), it!= mTable.end(), ++it){
			if (it->second <  threshold) {
				// this SSRC is old
				oldSSRC = it->first;
				foundOldMember = true;
			}
		}
		mTable.erase(it);

		if (foundOldMember) {
			mOurRTCPInstance.removeSSRC(oldSSRC, true);
		}
	} while (foundOldMember);
}

CJdRtcp::CJdRtcp
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
               if (NewSender(p) == FALSE) {
                   RemoveSender(p);
                   *senders -= 1;
               }

               if (NewMember(p) == FALSE) {
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
}

void CJdRtcp::removeSSRC(unsigned long ssrc, bool alsoRemoveStats) {
	mKnownMembers->remove(ssrc);

	if (alsoRemoveStats) {
		// Also, remove records of this SSRC from any reception or transmission stats
		if (fSource != NULL)
			fSource->receptionStatsDB().removeRecord(ssrc);
		if (fSink != NULL)
			fSink->transmissionStatsDB().removeRecord(ssrc);
	}
}

static double dTimeNow()
{
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    return (double) (timeNow.tv_sec + timeNow.tv_usec/1000000.0);
}


void CJdRtcp::processIncomingReport(unsigned packetSize, struct sockaddr_in const& fromAddressAndPort,
			int tcpSocketNum, unsigned char tcpStreamChannelId) {
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
				if (fSource != NULL) {
					RTPReceptionStatsDB& receptionStats =
							fSource->receptionStatsDB();
					receptionStats.noteIncomingSR(reportSenderSSRC, NTPmsw, NTPlsw,
							rtpTimestamp);
				}
				pkt += 8; // skip over packet count, octet count

				// If a 'SR handler' was set, call it now:
				if (fSRHandlerTask != NULL)
					(*fSRHandlerTask)(fSRHandlerClientData);

				// The rest of the SR is handled like a RR (so, no "break;" here)
			}
			case RTCP_PT_RR: {
				unsigned reportBlocksSize = rc * (6 * 4);
				if (length < reportBlocksSize)
					break;
				length -= reportBlocksSize;

				if (fSink != NULL) {
					// Use this information to update stats about our transmissions:
					RTPTransmissionStatsDB& transmissionStats =
							fSink->transmissionStatsDB();
					for (unsigned i = 0; i < rc; ++i) {
						unsigned senderSSRC = ntohl(*(unsigned long*) pkt);
						pkt += 4;
						// We care only about reports about our own transmission, not others'
						if (senderSSRC == fSink->SSRC()) {
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
			}
			case RTCP_PT_BYE: {
				// If a 'BYE handler' was set, arrange for it to be called at the end of this routine.
				// (Note: We don't call it immediately, in case it happens to cause "this" to be deleted.)
				if (fByeHandlerTask != NULL
						&& (!fByeHandleActiveParticipantsOnly
								|| (fSource != NULL
										&& fSource->receptionStatsDB().lookup(
												reportSenderSSRC) != NULL)
								|| (fSink != NULL
										&& fSink->transmissionStatsDB().lookup(
												reportSenderSSRC) != NULL))) {
					callByeHandler = true;
				}

				// We should really check for & handle >1 SSRCs being present #####

				subPacketOK = true;
				typeOfPacket = PACKET_BYE;
				break;
			}
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
			}
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
  int senders = (fSink != NULL) ? 1 : 0;

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
void CJdRtcp::Process(JdRtp *pRtp)
{
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
    if (fSource != NULL) fSource->receptionStatsDB().removeRecord(ssrc);
    if (fSink != NULL) fSink->transmissionStatsDB().removeRecord(ssrc);
  }
}


void CJdRtcp::addRR() {
  // ASSERT: fSource != NULL

  enqueueCommonReportPrefix(RTCP_PT_RR, fSource->SSRC());
  enqueueCommonReportSuffix();
}

void CJdRtcp::enqueueCommonReportPrefix(unsigned char packetType,
					     unsigned long SSRC,
					     unsigned numExtraWords) {
  unsigned numReportingSources;
  if (fSource == NULL) {
    numReportingSources = 0; // we don't receive anything
  } else {
    RTPReceptionStatsDB& allReceptionStats
      = fSource->receptionStatsDB();
    numReportingSources = allReceptionStats.numActiveSourcesSinceLastReset();
    // This must be <32, to fit in 5 bits:
    if (numReportingSources >= 32) { numReportingSources = 32; }
    // Later: support adding more reports to handle >32 sources (unlikely)#####
  }

  unsigned rtcpHdr = 0x80000000; // version 2, no padding
  rtcpHdr |= (numReportingSources<<24);
  rtcpHdr |= (packetType<<16);
  rtcpHdr |= (1 + numExtraWords + 6*numReportingSources);
      // each report block is 6 32-bit words long
  fOutBuf->enqueueWord(rtcpHdr);

  fOutBuf->enqueueWord(SSRC);
}

void RTCPInstance::enqueueCommonReportSuffix() {
  // Output the report blocks for each source:
  if (fSource != NULL) {
    RTPReceptionStatsDB& allReceptionStats
      = fSource->receptionStatsDB();

    RTPReceptionStatsDB::Iterator iterator(allReceptionStats);
    while (1) {
      RTPReceptionStats* receptionStats = iterator.next();
      if (receptionStats == NULL) break;
      enqueueReportBlock(receptionStats);
    }

    allReceptionStats.reset(); // because we have just generated a report
  }
}

void
RTCPInstance::enqueueReportBlock(RTPReceptionStats* stats) {
	fOutBuf->enqueueWord(stats->SSRC());

	unsigned highestExtSeqNumReceived = stats->highestExtSeqNumReceived();

	unsigned totNumExpected = highestExtSeqNumReceived
			- stats->baseExtSeqNumReceived();
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

	fOutBuf->enqueueWord((lossFraction << 24) | totNumLost);
	fOutBuf->enqueueWord(highestExtSeqNumReceived);

	fOutBuf->enqueueWord(stats->jitter());

	unsigned NTPmsw = stats->lastReceivedSR_NTPmsw();
	unsigned NTPlsw = stats->lastReceivedSR_NTPlsw();
	unsigned LSR = ((NTPmsw & 0xFFFF) << 16) | (NTPlsw >> 16); // middle 32 bits
	fOutBuf->enqueueWord(LSR);

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
	fOutBuf->enqueueWord(DLSR);
}

