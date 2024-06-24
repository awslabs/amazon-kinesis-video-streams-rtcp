/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* API includes. */
#include "rtcp_twcc_manager.h"
#include "rtcp_api.h"
#include "rtcp_endianness.h"

/* ===========================  EXTERN VARIABLES  =========================== */

#define SIZE_OF_TWCC_INFO_PKT_ARRAY         512
TwccPacketInfo_t twccBuffer[ SIZE_OF_TWCC_INFO_PKT_ARRAY ];

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate Twcc Manager Add packet functionality.
 */
void test_twccAddPacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );

    for( i = 0; i < 4; i++ )
    {
        payloadLength = ( rand() % ( 1000 ) );
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        TEST_ASSERT_EQUAL( result,
                           RTCP_RESULT_OK );
    }

    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       4 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Get packet functionality.
 */
void test_twccGetPacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );

    for( i = 0; i < 10; i++ )
    {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        TEST_ASSERT_EQUAL( result,
                           RTCP_RESULT_OK );
    }
    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       10 );

    /* Get packet info from the read index */
    result = RtcpTwcc_GetPacketInfo( &twccCtx,
                                     &twccPacketInfo );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                       0 );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSequenceNumber,
                       256 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Remove packet functionality.
 */
void test_twccRemovePacket( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );

    for( i = 0; i < 10; i++ ) {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        TEST_ASSERT_EQUAL( result,
                           RTCP_RESULT_OK );
    }
    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       10 );

    /* Get packet info from the read index */
    result = RtcpTwcc_ExtractPacketInfo( &twccCtx,
                                         &twccPacketInfo );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                       0 );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSequenceNumber,
                       256 );
    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       1 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       10 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Older packet deletion functionality.
 */
void test_twccOlderPacketInfoDeletion( void )
{
    uint16_t seqNum = 256, seqNumber = 0;
    uint64_t sentTime;
    uint32_t payloadLength, i;
    RtcpTwccManagerCtx_t twccCtx;
    RtcpResult_t result;

    result = RtcpTwcc_Init( &twccCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_RESULT_OK );

    for( i = 0; i < 60; i++ ) {
        payloadLength = i;
        sentTime = time( NULL );
        seqNumber += seqNum;
        result = RtcpTwcc_AddPacketInfo( &twccCtx,
                                         payloadLength,
                                         sentTime,
                                         seqNumber );
        TEST_ASSERT_EQUAL( result,
                           RTCP_RESULT_OK );

        result = RtcpTwcc_OlderPacketInfoDeletion( &twccCtx,
                                                   sentTime,
                                                   seqNumber );
        //sleep( 1 ); // Need to get different sentTime
        TEST_ASSERT_EQUAL( result,
                           RTCP_RESULT_OK );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP twcc packet parsing.
 */
void test_twccParseTwccPacket( void )
{
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpTwccPacket_t twccPacket;
    RtcpTwccManagerCtx_t twccManagerCtx;
    uint8_t twccpayload[] = { 0x1c, 0x8c, 0x77, 0xb6, 0x3a, 0x1b, 0x46, 0x4a, 0x00, 0x11, 0x00, 0x08, 0x63, 0xe3,
                              0x21, 0x01, 0x20, 0x08, 0xb3, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x02 };

    result = Rtcp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    memset( &twccPacket,
            0x00,
            sizeof( RtcpTwccPacket_t ) );

    result = Rtcp_ParseTwccPacket( &ctx,
                                   &( twccpayload[0] ),
                                   sizeof( twccpayload ),
                                   &twccPacket );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( twccPacket.pPacketChunkStart,
                       &( twccpayload[16] ) );
    TEST_ASSERT_EQUAL( twccPacket.pRecvDeltaStart,
                       &( twccpayload[18] ) );
    TEST_ASSERT_EQUAL( twccPacket.baseSeqNum,
                       17 );

    result = RtcpTwcc_Init( &twccManagerCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = RtcpTwcc_ParseRtcpChunk( &twccManagerCtx,
                                      &twccPacket );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( twccManagerCtx.firstReportedSeqNum,
                       17 );
    TEST_ASSERT_EQUAL( twccManagerCtx.lastReportedSeqNum,
                       24 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP twcc packet parsing.
 */
void test_twccParseTwccPacket2( void )
{
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpTwccPacket_t twccPacket;
    RtcpTwccManagerCtx_t twccManagerCtx;
    uint8_t twccpayload[] = { 0xcb, 0x00, 0x18, 0x1a, 0x6d, 0x06, 0xec, 0xda, 0x00, 0xa5, 0x00, 0x0c, 0x64, 0x5e, 0x11, 0x0f, 0x20,
                              0x0c, 0x84, 0x00, 0x00, 0x00, 0x00, 0x23, 0x50, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 };
    result = Rtcp_Init( &ctx );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    memset( &twccPacket,
            0x00,
            sizeof( RtcpTwccPacket_t ) );

    result = Rtcp_ParseTwccPacket( &ctx,
                                   &( twccpayload[0] ),
                                   sizeof( twccpayload ),
                                   &twccPacket );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( twccPacket.pPacketChunkStart,
                       &( twccpayload[16] ) );
    TEST_ASSERT_EQUAL( twccPacket.pRecvDeltaStart,
                       &( twccpayload[18] ) );
    TEST_ASSERT_EQUAL( twccPacket.baseSeqNum,
                       165 );

    result = RtcpTwcc_Init( &twccManagerCtx,
                            &( twccBuffer[0] ),
                            SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = RtcpTwcc_ParseRtcpChunk( &twccManagerCtx,
                                      &twccPacket );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( twccManagerCtx.firstReportedSeqNum,
                       165 );
    TEST_ASSERT_EQUAL( twccManagerCtx.lastReportedSeqNum,
                       176 );
}

/*-----------------------------------------------------------*/