/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IFADDRS_ANDROID_H_included
#define IFADDRS_ANDROID_H_included

#include <cstring>
#include <new>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>

#include "posix_ifaddrs_localarray.h" //"LocalArray.h"
#include "posix_ifaddrs_scopedfd.h" //"ScopedFd.h"

#define CHECK_AND_FREE(x) if(x) free(x)
// Android (bionic) doesn't have getifaddrs(3)/freeifaddrs(3).
// We fake it here, so java_net_NetworkInterface.cpp can use that API
// with all the non-portable code being in this file.

// Source-compatible subset of the BSD struct.
struct ifaddrs {
    // Pointer to next struct in list, or NULL at end.
    ifaddrs* ifa_next;

    // Interface name.
    char* ifa_name;

    // Interface flags.
    unsigned int ifa_flags;

    // Interface address.
    sockaddr* ifa_addr;

    ifaddrs(ifaddrs* next)
    : ifa_next(next), ifa_name(NULL), ifa_flags(0), ifa_addr(NULL)
    {
    }

    ~ifaddrs() {
    	CHECK_AND_FREE(ifa_next);
    	CHECK_AND_FREE(ifa_name);
    	CHECK_AND_FREE(ifa_addr);
    }

    // Sadly, we can't keep the interface index for portability with BSD.
    // We'll have to keep the name instead, and re-query the index when
    // we need it later.
    bool setNameAndFlagsByIndex(int interfaceIndex) {
        // Get the name.
        char buf[IFNAMSIZ];
        char* name = if_indextoname(interfaceIndex, buf);
        if (name == NULL) {
            return false;
        }
        ifa_name = new char[strlen(name) + 1];
        strcpy(ifa_name, name);

        // Get the flags.
        ScopedFd fd(socket(AF_INET, SOCK_DGRAM, 0));
        if (fd.get() == -1) {
            return false;
        }
        ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, name);
        int rc = ioctl(fd.get(), SIOCGIFFLAGS, &ifr);
        if (rc == -1) {
            return false;
        }
        ifa_flags = ifr.ifr_flags;
        return true;
    }

    // Netlink gives us the address family in the header, and the
    // sockaddr_in or sockaddr_in6 bytes as the payload. We need to
    // stitch the two bits together into the sockaddr that's part of
    // our portable interface.
    void setAddress(int family, void* data, size_t byteCount) {
        sockaddr_storage* ss = new sockaddr_storage;
        ss->ss_family = family;
        if (family == AF_INET) {
            void* dst = &reinterpret_cast<sockaddr_in*>(ss)->sin_addr;
            memcpy(dst, data, byteCount);
        } else if (family == AF_INET6) {
            void* dst = &reinterpret_cast<sockaddr_in6*>(ss)->sin6_addr;
            memcpy(dst, data, byteCount);
        }
        ifa_addr = reinterpret_cast<sockaddr*>(ss);
    }
};

// FIXME: use iovec instead.
struct addrReq_struct {
    nlmsghdr netlinkHeader;
    ifaddrmsg msg;
};

inline bool sendNetlinkMessage(int fd, const void* data, size_t byteCount) {
    ssize_t sentByteCount = TEMP_FAILURE_RETRY(send(fd, data, byteCount, 0));
    return (sentByteCount == static_cast<ssize_t>(byteCount));
}

inline ssize_t recvNetlinkMessage(int fd, char* buf, size_t byteCount) {
    return TEMP_FAILURE_RETRY(recv(fd, buf, byteCount, 0));
}

// Manually request network info from driver
inline int getifaddrs(ifaddrs** result) {
        int sockfd;
        int io;
        struct ifreq * ifr;
        struct ifconf ifc;
        struct ifaddrs * ifAddrStruct=NULL;
        struct ifaddrs * previous=NULL;

        *result == NULL;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0)
            return -1;
            //MIDSLOG("", "socket error \n");

    // find number of interfaces.
        memset(&ifc, 0, sizeof(ifc));
        ifc.ifc_ifcu.ifcu_req = NULL;
        ifc.ifc_len = 0;
        if( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
            return -1;

        ifr = (ifreq *)malloc(ifc.ifc_len);
        ifc.ifc_ifcu.ifcu_req = ifr;

        io = ioctl(sockfd, SIOCGIFCONF, &ifc);
        if(io == 0){
            close(sockfd);
            int numif = ifc.ifc_len / sizeof(struct ifreq);
            for (int i = 0; i < numif; i++) {
                ifAddrStruct = (ifaddrs *)malloc(sizeof(ifaddrs));
               // memset(ifAddrStruct->ifa_addr,0,sizeof(ifaddrs));
                ifAddrStruct->ifa_next = NULL;
                if(previous)
                        previous->ifa_next = ifAddrStruct;
                else
                     *result = ifAddrStruct;
                int deviceStrLen = strlen(ifr[i].ifr_name) + 1;
                ifAddrStruct->ifa_name =  (char *)malloc(deviceStrLen);
                memset((char *)ifAddrStruct->ifa_name,0,deviceStrLen);
                strncpy(ifAddrStruct->ifa_name,(char *)ifr[i].ifr_name,deviceStrLen);
                ifAddrStruct->ifa_addr = (sockaddr *)malloc(sizeof(sockaddr_in));
                memcpy(ifAddrStruct->ifa_addr,&ifr[i].ifr_addr,sizeof(sockaddr_in));

                previous = ifAddrStruct;

                //(*result)->ifa_addr =  (char *)((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr.s_addr;
                //char * addr = (char *)((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr.s_addr;
            }//end for
            return 0;
        }
        return -1;
}
//netlink version
// Source-compatible with the BSD function.
inline int getifaddrs2(ifaddrs** result) {
    // Simplify cleanup for callers.
    *result = NULL;

    // Create a netlink socket.
    ScopedFd fd(socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE));
    if (fd.get() < 0) {
        return -1;
    }

    // Ask for the address information.
    addrReq_struct addrRequest;
    memset(&addrRequest, 0, sizeof(addrRequest));
    addrRequest.netlinkHeader.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH;
    addrRequest.netlinkHeader.nlmsg_type = RTM_GETADDR;
    addrRequest.netlinkHeader.nlmsg_len = NLMSG_ALIGN(NLMSG_LENGTH(sizeof(addrRequest)));
    addrRequest.msg.ifa_family = AF_UNSPEC; // All families.
    addrRequest.msg.ifa_index = 0; // All interfaces.
    if (!sendNetlinkMessage(fd.get(), &addrRequest, addrRequest.netlinkHeader.nlmsg_len)) {
        return -1;
    }

    // Read the responses.
    LocalArray<0> buf(65536); // We don't necessarily have std::vector.
    ssize_t bytesRead;
    while ((bytesRead  = recvNetlinkMessage(fd.get(), &buf[0], buf.size())) > 0) {
        nlmsghdr* hdr = reinterpret_cast<nlmsghdr*>(&buf[0]);
		for (; NLMSG_OK(hdr, (unsigned int)bytesRead); hdr = NLMSG_NEXT(hdr, bytesRead)) {
            switch (hdr->nlmsg_type) {
            case NLMSG_DONE:
                return 0;
            case NLMSG_ERROR:
                return -1;
            case RTM_NEWADDR:
                {
                    ifaddrmsg* address = reinterpret_cast<ifaddrmsg*>(NLMSG_DATA(hdr));
                    rtattr* rta = IFA_RTA(address);
                    size_t ifaPayloadLength = IFA_PAYLOAD(hdr);
                    while (RTA_OK(rta, ifaPayloadLength)) {
                        if (rta->rta_type == IFA_LOCAL) {
                            int family = address->ifa_family;
                            if (family == AF_INET || family == AF_INET6) {
								if((*result) == NULL) {
									*result = new ifaddrs(*result);
								}
                                (*result)->setAddress(family, RTA_DATA(rta), RTA_PAYLOAD(rta));
                            }
                        } else if(rta->rta_type == IFA_LABEL) {
							if((*result) == NULL) {
								*result = new ifaddrs(*result);
							}
							(*result)->ifa_name = new char[strlen((char*)RTA_DATA(rta)) + 1];
							strcpy((*result)->ifa_name, (char*)RTA_DATA(rta));
						}
                        rta = RTA_NEXT(rta, ifaPayloadLength);
                    }
                }
                break;
            }
        }
    }
    // We only get here if recv fails before we see a NLMSG_DONE.
    return -1;
}

// Source-compatible with the BSD function.
inline void freeifaddrs(ifaddrs* addresses) {
	CHECK_AND_FREE(addresses);
}

#endif  // IFADDRS_ANDROID_H_included
