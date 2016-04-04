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
};

#endif
