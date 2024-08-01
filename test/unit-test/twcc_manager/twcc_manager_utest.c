/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <stdlib.h>
#include <time.h>

/* API includes. */
#include "rtcp_twcc_manager.h"
#include "rtcp_api.h"
#include "rtcp_endianness.h"

/* ===========================  EXTERN VARIABLES  =========================== */

#define TWCC_PACKET_INFO_ARRAY_LENGTH         32
TwccPacketInfo_t twccPacketInfoArray[ TWCC_PACKET_INFO_ARRAY_LENGTH ];

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate Twcc Manager Add packet functionality.
 */
void test_twccAddPacket( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
    TEST_ASSERT_EQUAL( 0, twccManager.readIndex );
    TEST_ASSERT_EQUAL( 0, twccManager.writeIndex );
    TEST_ASSERT_EQUAL( 0, twccManager.count );
    TEST_ASSERT_EQUAL_PTR( &( twccPacketInfoArray[ 0 ] ), twccManager.pTwccPacketInfoArray );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH, twccManager.twccPacketInfoArrayLength );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = ( rand() % ( 1000 ) );
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH, twccManager.count );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager check Older packet deletion functionality.
 */
void test_twccOlderPacketInfoDeletion( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t foundTwccPacketInfo;

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
    TEST_ASSERT_EQUAL( 0, twccManager.readIndex );
    TEST_ASSERT_EQUAL( 0, twccManager.writeIndex );
    TEST_ASSERT_EQUAL( 0, twccManager.count );
    TEST_ASSERT_EQUAL_PTR( &( twccPacketInfoArray[ 0 ] ), twccManager.pTwccPacketInfoArray );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH, twccManager.twccPacketInfoArrayLength );

    twccPacketInfo.localSentTime = time( NULL );
    twccPacketInfo.packetSize = 0;
    twccPacketInfo.packetSeqNum += seqNum;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );

    for( i = 1; i < TWCC_PACKET_INFO_ARRAY_LENGTH * 2; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime += RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1;
        twccPacketInfo.packetSeqNum += seqNum;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );

        /* The above operation should delete every other packet as timestamp
         * difference between 2 packets is > RTCP_TWCC_ESTIMATOR_TIME_WINDOW.
         * Therefore, the count should always be one. */
        TEST_ASSERT_EQUAL( 1, twccManager.count );

        /* Only the last added packet should be in the Manager. */
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 twccPacketInfo.packetSeqNum,
                                                 &( foundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSize, foundTwccPacketInfo.packetSize );
        TEST_ASSERT_EQUAL( twccPacketInfo.localSentTime, foundTwccPacketInfo.localSentTime );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSeqNum, foundTwccPacketInfo.packetSeqNum );
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
    RtcpTwccManager_t twccManager;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH, twccManager.count);

    result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                             2,
                                             &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND, result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality.
 */
void test_twccFindPacket_Found( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    uint64_t localSentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = localSentTime + ( int64_t ) i;
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH, twccManager.count );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                seqNum * ( i + 1 ),
                                                &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK, result );
        TEST_ASSERT_EQUAL( i, twccPacketInfo.packetSize );
        TEST_ASSERT_EQUAL( seqNum * ( i + 1 ), twccPacketInfo.packetSeqNum );
        TEST_ASSERT_EQUAL( localSentTime + ( int64_t ) i, twccPacketInfo.localSentTime );
    }
}

/*-----------------------------------------------------------*/