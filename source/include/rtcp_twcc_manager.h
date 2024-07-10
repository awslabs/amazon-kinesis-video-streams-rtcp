#ifndef TWCC_MANAGER_H
#define TWCC_MANAGER_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* API includes. */
#include "rtcp_data_types.h"

/*-----------------------------------------------------------*/

typedef enum RtcpTwccManagerResult
{
    RTCP_TWCC_MANAGER_RESULT_OK,
    RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
    RTCP_TWCC_MANAGER_RESULT_EMPTY,
    RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND
} RtcpTwccManagerResult_t;

/*-----------------------------------------------------------*/

typedef struct TwccBandwidthInfo
{
    uint64_t sentBytes;
    uint64_t receivedBytes;
    uint64_t sentPackets;
    uint64_t receivedPackets;
    int64_t duration;
} TwccBandwidthInfo_t;

typedef struct TwccPacketInfo
{
    uint64_t localSentTime;
    size_t packetSize;
    uint16_t packetSeqNum;
} TwccPacketInfo_t;

typedef struct RtcpTwccManager
{
    TwccPacketInfo_t * pTwccPacketInfoArray;
    size_t twccPacketInfoArrayLength;
    size_t writeIndex;
    size_t readIndex;
    size_t count;
} RtcpTwccManager_t;

/*-----------------------------------------------------------*/

RtcpTwccManagerResult_t RtcpTwccManager_Init( RtcpTwccManager_t * pTwccManager,
                                              TwccPacketInfo_t * pTwccPacketInfoArray,
                                              size_t twccPacketInfoArrayLength );

RtcpTwccManagerResult_t RtcpTwccManager_AddPacketInfo( RtcpTwccManager_t * pTwccManager,
                                                       const TwccPacketInfo_t * pTwccPacketInfoToAdd );

RtcpTwccManagerResult_t RtcpTwccManager_FindPacketInfo( RtcpTwccManager_t * pTwccManager,
                                                        uint16_t seqNum,
                                                        TwccPacketInfo_t * pOutTwccPacketInfo );

RtcpTwccManagerResult_t RtcpTwccManager_HandleTwccPacket( RtcpTwccManager_t * pTwccManager,
                                                          const RtcpTwccPacket_t * pTwccPacket,
                                                          TwccBandwidthInfo_t * pBandwidthInfo );

/*-----------------------------------------------------------*/

#endif /* TWCC_MANAGER_H */
