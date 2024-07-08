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
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = {0};

    result = RtcpTwccManager_Init( &twccCtx,
                                   &( twccBuffer[0] ),
                                   SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_OK );

    for( i = 0; i < 4; i++ )
    {
        twccPacketInfo.packetSize = ( rand() % ( 1000 ) );
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &twccCtx,
                                                &twccPacketInfo );
        TEST_ASSERT_EQUAL( result,
                           RTCP_TWCC_MANAGER_RESULT_OK );
    }

    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       4 );
    TEST_ASSERT_EQUAL( twccCtx.count,
                       4 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager check Older packet deletion functionality.
 */
void test_twccOlderPacketInfoDeletion( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = {0};

    result = RtcpTwccManager_Init( &twccCtx,
                                   &( twccBuffer[0] ),
                                   SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_OK );

    twccPacketInfo.localSentTime = time( NULL );
    twccPacketInfo.packetSize = 0;
    twccPacketInfo.packetSeqNum += seqNum;
    result = RtcpTwccManager_AddPacketInfo( &twccCtx,
                                            &twccPacketInfo );
    TEST_ASSERT_EQUAL( result,
                           RTCP_TWCC_MANAGER_RESULT_OK );

    for( i = 1; i < SIZE_OF_TWCC_INFO_PKT_ARRAY; i++ ) {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime += RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1;
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &twccCtx,
                                                &twccPacketInfo );
        TEST_ASSERT_EQUAL( result,
                           RTCP_TWCC_MANAGER_RESULT_OK );
        /* Delete every other packet as diference between timestamp b/w 2 packets is > RTCP_TWCC_ESTIMATOR_TIME_WINDOW
         * Hence count is always 1. */
        TEST_ASSERT_EQUAL( twccCtx.readIndex,
                           i );
            TEST_ASSERT_EQUAL( twccCtx.count,
                           1 );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality.
 */
void test_twccFindPacket_NotFound( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = {0};

    result = RtcpTwccManager_Init( &twccCtx,
                                   &( twccBuffer[0] ),
                                   SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_OK );

    for( i = 0; i < 10; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &twccCtx,
                                                &twccPacketInfo );
        TEST_ASSERT_EQUAL( result,
                           RTCP_TWCC_MANAGER_RESULT_OK );
    }
    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       10 );

    /* Get packet info from the read index */
    result = RtcpTwccManager_FindPacketInfo( &twccCtx,
                                             2,
                                             &twccPacketInfo );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality.
 */
void test_twccFindPacket_Found( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccCtx;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = {0};

    result = RtcpTwccManager_Init( &twccCtx,
                                   &( twccBuffer[0] ),
                                   SIZE_OF_TWCC_INFO_PKT_ARRAY );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_OK );

    for( i = 0; i < 10; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &twccCtx,
                                                &twccPacketInfo );
        TEST_ASSERT_EQUAL( result,
                           RTCP_TWCC_MANAGER_RESULT_OK );
    }
    TEST_ASSERT_EQUAL( twccCtx.readIndex,
                       0 );
    TEST_ASSERT_EQUAL( twccCtx.writeIndex,
                       10 );

    /* Get packet info from the read index */
    result = RtcpTwccManager_FindPacketInfo( &twccCtx,
                                             512,
                                             &twccPacketInfo );
    TEST_ASSERT_EQUAL( result,
                       RTCP_TWCC_MANAGER_RESULT_OK );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                       1 );
    TEST_ASSERT_EQUAL( twccPacketInfo.packetSeqNum,
                       512 );
}

/*-----------------------------------------------------------*/