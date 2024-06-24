#ifndef TWCC_MANAGER_H
#define TWCC_MANAGER_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* API includes. */
#include "rtcp_data_types.h"
#include "rtcp_endianness.h"

#define RTCP_TWCC_PACKET_UNITIALIZED_TIME                   0
#define RTCP_TWCC_PACKET_LOST_TIME                          ( ( uint64_t ) ( -1LL ) )
#define RTCP_MILLISECONDS_PER_SECOND                        1000LL
#define RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND                  ( ( int64_t ) 10000000 )
#define RTCP_TWCC_ESTIMATOR_TIME_WINDOW                     ( 1 * RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND )
// https://tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions-01
// Deltas are represented as multiples of 250us:
#define RTCP_TWCC_TICKS_PER_SECOND                          ( 1000000LL / 250 )
#define RTCP_CONVERT_TIMESCALE( pts, from_timescale, to_timescale )       ( pts * to_timescale / from_timescale )

#define RTCP_VECTOR_SYMBOL_LOCATION( symbolSize )           ( symbolSize ? 3u : 1u )
#define RTCP_VECTOR_SYMBOL_SIZE( symbolSize )               ( symbolSize ? 2u : 1u )
#define RTCP_VECTOR_SYMBOL_BITMASK( i, symbolSize )         ( 14u - ( ( i + 1 ) * RTCP_VECTOR_SYMBOL_SIZE( symbolSize ) ) )
/*-----------------------------------------------------------*/

typedef struct TwccBandwidthInfo
{
    uint64_t sentBytes;
    uint64_t receivedBytes;
    uint64_t sentPackets;
    uint64_t receivedPackets;
    int64_t duration;
} TwccBandwidthInfo_t;

typedef struct TwccPacketInfo // RTP packet info hence not prefixing with RTCP
{
    uint64_t localTimeKvs;
    uint64_t remoteTimeKvs;
    uint32_t packetSize;
    uint16_t packetSequenceNumber;
} TwccPacketInfo_t;

typedef struct RtcpTwccManagerCtx
{
    TwccPacketInfo_t * pTwccPktInfoArray;    // If we allocate 256 elements size of this array becomes ~ 5.5KB , if 512 elements ~11Kb
                                             // 512 is a better choice according to demoRun
    size_t twccPacketInfoArrayLength;
    size_t writeIndex;
    size_t readIndex;
    uint16_t firstReportedSeqNum;
    uint16_t lastReportedSeqNum;
    RtcpReadWriteFunctions_t readWriteFunctions;
} RtcpTwccManagerCtx_t;
/*-----------------------------------------------------------*/

RtcpResult_t RtcpTwcc_Init( RtcpTwccManagerCtx_t * pTwccCtx,
                            TwccPacketInfo_t * pTwccBuffer,
                            size_t twccBufferSize );

RtcpResult_t RtcpTwcc_AddPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                     uint32_t payloadLength,
                                     uint32_t sentTime,
                                     uint16_t seqNumber );

RtcpResult_t RtcpTwcc_GetPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                     TwccPacketInfo_t * pTwccPacketInfo );

RtcpResult_t RtcpTwcc_ExtractPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                         TwccPacketInfo_t * pTwccPacketInfo );

RtcpResult_t RtcpTwcc_GetSeqNum_PacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                            TwccPacketInfo_t * pTwccPacketInfo,
                                            uint16_t seqNum );

RtcpResult_t RtcpTwcc_OlderPacketInfoDeletion( RtcpTwccManagerCtx_t * pTwccCtx,
                                               uint64_t currentPacketSentTime,
                                               uint16_t seqNumber );

RtcpResult_t RtcpTwcc_ParseRtcpChunk( RtcpTwccManagerCtx_t * pTwccCtx,
                                      RtcpTwccPacket_t * rtcpTwccPacket );

RtcpResult_t RtcpTwcc_GetBandwidthParameters( RtcpTwccManagerCtx_t * pTwccCtx,
                                              TwccBandwidthInfo_t * pBandwidthInfo );

#endif /* TWCC_MANAGER_H */