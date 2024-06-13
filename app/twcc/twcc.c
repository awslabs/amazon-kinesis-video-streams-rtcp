/* Standard includes. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


/* RTP includes. */
#include "rtcp_twcc_manager.h"

#define SIZE_OF_TWCC_INFO_PKT_ARRAY         512

static void twccAddPacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ];

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    assert( result == RTCP_RESULT_OK );

    for( i = 0; i < 4; i++ ) {
        payloadLength = ( rand() % ( 1000 ) );
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        assert( result == RTCP_RESULT_OK );
    }
    assert( twccCtx.readIndex == 0 );
    assert( twccCtx.writeIndex == 4 );
}

static void twccGetPacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ], twccPacketInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    assert( result == RTCP_RESULT_OK );

    for( i = 0; i < 10; i++ ) {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        assert( result == RTCP_RESULT_OK );
    }
    assert( twccCtx.readIndex == 0 );
    assert( twccCtx.writeIndex == 10 );

    /* Get packet info from the read index */
    result = RtcpTwcc_GetPacketInfo( &twccCtx,
                                     &twccPacketInfo );
    assert( result == RTCP_RESULT_OK );
    assert( twccPacketInfo.packetSize == 0 );
    assert( twccPacketInfo.packetSequenceNumber == 256 );
}

static void twccRemovePacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ], twccPacketInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    assert( result == RTCP_RESULT_OK );

    for( i = 0; i < 10; i++ ) {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        assert( result == RTCP_RESULT_OK );
    }
    assert( twccCtx.readIndex == 0 );
    assert( twccCtx.writeIndex == 10 );

    /* Get packet info from the read index */
    result = RtcpTwcc_ExtractPacketInfo( &twccCtx,
                                         &twccPacketInfo );
    assert( result == RTCP_RESULT_OK );
    assert( twccPacketInfo.packetSize == 0 );
    assert( twccPacketInfo.packetSequenceNumber == 256 );
    assert( twccCtx.readIndex == 1 );
    assert( twccCtx.writeIndex == 10 );

}


static void twccOlderPacketInfoDeletion( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ], twccPacketInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    assert( result == RTCP_RESULT_OK );

    for( i = 0; i < 1200; i++ ) {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        assert( result == RTCP_RESULT_OK );

        result = RtcpTwcc_OlderPacketInfoDeletion( &twccCtx,
                                                   sentTime,
                                                   seqNumber );
        sleep( 2 ); // Need to get different sentTime
        assert( result == RTCP_RESULT_OK );
    }
}

static void twccbandwidth( RtcpTwccPacket_t * pRtcpTwccPacket ) // Receive pRtcpTwccPacket from Rtcp_ParseTwccPacket
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ], twccPacketInfo;
    TwccBandwidthInfo_t bandwidthInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    assert( result == RTCP_RESULT_OK );

    result = RtcpTwcc_ParseRtcpChunk( &twccCtx,
                                      pRtcpTwccPacket );
    assert( result == RTCP_RESULT_OK );

    RtcpTwcc_GetBandwidthParameters( &twccCtx,
                                     &bandwidthInfo );
    assert( result == RTCP_RESULT_OK );

    printf( "sentBytes %ld receivedBytes %ld sentPackets %ld receivedPackets %ld duration %ld\n",
            bandwidthInfo.sentBytes,
            bandwidthInfo.receivedBytes,
            bandwidthInfo.sentPackets,
            bandwidthInfo.receivedPackets,
            bandwidthInfo.duration );

}
int main()
{
    twccAddPacket();
    twccGetPacket();
    twccRemovePacket();
    twccOlderPacketInfoDeletion();

    printf( "Test passed\n" );
    return 0;
}