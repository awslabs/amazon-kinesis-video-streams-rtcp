/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* API includes. */
#include "rtcp_twcc_manager.h"
#include "rtcp_api.h"
#include "rtcp_endianness.h"

/* ===========================  EXTERN VARIABLES  =========================== */

#define TWCC_PACKET_INFO_ARRAY_LENGTH         32
TwccPacketInfo_t twccPacketInfoArray[ TWCC_PACKET_INFO_ARRAY_LENGTH ];

void setUp( void )
{
    memset( &( twccPacketInfoArray[ 0 ] ),
            0,
            sizeof( twccPacketInfoArray ) );
}

void tearDown( void )
{
}

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate Twcc Manager Init fail functionality for Bad Parameters.
 */
void test_twccInit_BadParams( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;

    result = RtcpTwccManager_Init( NULL,
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   NULL,
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   0 );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Init functionality.
 */
void test_twccInit( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0,
                       twccManager.readIndex );
    TEST_ASSERT_EQUAL( 0,
                       twccManager.writeIndex );
    TEST_ASSERT_EQUAL( 0,
                       twccManager.count );
    TEST_ASSERT_EQUAL_PTR( &( twccPacketInfoArray[ 0 ] ),
                           twccManager.pTwccPacketInfoArray );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.twccPacketInfoArrayLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Add packet fail functionality for Bad Parameters.
 */
void test_twccAddPacket_BadParams( void )
{
    RtcpTwccManager_t twccManager = { 0 };
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };

    result = RtcpTwccManager_AddPacketInfo( NULL,
                                            &( twccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            NULL );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Add packet functionality.
 */
void test_twccAddPacket( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t * pFoundTwccPacketInfo;
    size_t packetSize = ( rand() % ( 1000 ) );
    uint64_t sentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = packetSize + ( size_t ) i;
        twccPacketInfo.localSentTime = sentTime + ( uint64_t ) i;
        twccPacketInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( i + 1,
                           twccManager.count );
    }

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 seqNum + ( uint16_t ) i,
                                                 &( pFoundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( sentTime + ( uint64_t ) i,
                           pFoundTwccPacketInfo->localSentTime );
        TEST_ASSERT_EQUAL( packetSize + ( size_t ) i,
                           pFoundTwccPacketInfo->packetSize );
        TEST_ASSERT_EQUAL( seqNum + ( uint16_t ) i,
                           pFoundTwccPacketInfo->packetSeqNum );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Add packet functionality for Overflow.
 */
void test_twccAddPacket_Overflow( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t * pFoundTwccPacketInfo;
    size_t packetSize = ( rand() % ( 1000 ) );
    uint64_t sentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = packetSize + ( size_t ) i;
        twccPacketInfo.localSentTime = sentTime + ( uint64_t ) i;
        twccPacketInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( i + 1,
                           twccManager.count );
    }

    twccPacketInfo.packetSize = packetSize + ( size_t ) i;
    twccPacketInfo.localSentTime = sentTime + ( uint64_t ) i;
    twccPacketInfo.packetSeqNum = seqNum + ( uint16_t ) i;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );

    /* The first (i.e. the oldest) packet is deleted, and hence not found. */
    result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                             seqNum,
                                             &( pFoundTwccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND,
                       result );

    for( i = 1; i <= TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 seqNum + ( uint16_t ) i,
                                                 &( pFoundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( sentTime + ( uint64_t ) i,
                           pFoundTwccPacketInfo->localSentTime );
        TEST_ASSERT_EQUAL( packetSize + ( size_t ) i,
                           pFoundTwccPacketInfo->packetSize );
        TEST_ASSERT_EQUAL( seqNum + ( uint16_t ) i,
                           pFoundTwccPacketInfo->packetSeqNum );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager check packet deletion functionality.
 *
 * This test validates that no packet is deleted when the added packet's
 * timestamp is less then any packet in the packetInfoArray.
 */
void test_twccOlderPacketInfoDeletion_AddOldPacket( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t * pFoundTwccPacketInfo;
    uint64_t sentTime = time( NULL );
    size_t packetSize = ( rand() % ( 1000 ) );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    twccPacketInfo.localSentTime = sentTime;
    twccPacketInfo.packetSize = packetSize;
    twccPacketInfo.packetSeqNum = seqNum;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    twccPacketInfo.packetSize = packetSize + 1;
    twccPacketInfo.localSentTime = sentTime - RTCP_TWCC_ESTIMATOR_TIME_WINDOW;
    twccPacketInfo.packetSeqNum = seqNum + 1;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    /* The last added packet added had a less time-stamp than the previous one,
     * hence no packet will get deleted. */
    TEST_ASSERT_EQUAL( 2,
                       twccManager.count );

    /* Ensure that both the packets are present in the TWCC manager. */
    for( i = 0; i < 2; i ++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                seqNum + ( uint16_t ) i,
                                                &( pFoundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( packetSize + i,
                           pFoundTwccPacketInfo->packetSize );
        TEST_ASSERT_EQUAL( sentTime - ( ( uint64_t ) i * RTCP_TWCC_ESTIMATOR_TIME_WINDOW ),
                           pFoundTwccPacketInfo->localSentTime );
        TEST_ASSERT_EQUAL( seqNum + i,
                           pFoundTwccPacketInfo->packetSeqNum );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager for Older packet deletion functionality.
 */
void test_twccOlderPacketInfoDeletion( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t * pFoundTwccPacketInfo;
    size_t packetSize = ( rand() % ( 1000 ) );
    uint64_t sentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    twccPacketInfo.localSentTime = sentTime;
    twccPacketInfo.packetSize = packetSize;
    twccPacketInfo.packetSeqNum = seqNum;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 1; i < TWCC_PACKET_INFO_ARRAY_LENGTH * 2; i++ )
    {
        twccPacketInfo.packetSize = packetSize + ( size_t ) i;
        twccPacketInfo.localSentTime = sentTime + ( ( uint64_t ) i * ( RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1 ) );
        twccPacketInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );

        /* The above operation should delete every other packet as timestamp
         * difference between 2 packets is > RTCP_TWCC_ESTIMATOR_TIME_WINDOW.
         * Therefore, the count should always be one. */
        TEST_ASSERT_EQUAL( 1,
                           twccManager.count );

        /* Only the last added packet should be in the Manager. */
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 twccPacketInfo.packetSeqNum,
                                                 &( pFoundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                           pFoundTwccPacketInfo->packetSize );
        TEST_ASSERT_EQUAL( twccPacketInfo.localSentTime,
                           pFoundTwccPacketInfo->localSentTime );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSeqNum,
                           pFoundTwccPacketInfo->packetSeqNum );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet fail functionality for Bad Parameters.
 */
void test_twccFindPacket_BadParams( void )
{
    uint16_t seqNum = 256;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t * pFoundTwccPacketInfo;

    result = RtcpTwccManager_FindPacketInfo( NULL,
                                             seqNum,
                                             &( pFoundTwccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                             seqNum,
                                             NULL );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality for Empty Twcc Manager.
 */
void test_twccFindPacket_Empty( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t * pFoundTwccPacketInfo;

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0,
                       twccManager.count );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 seqNum * ( i + 1 ),
                                                 &( pFoundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_EMPTY,
                           result );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality for non-existant sequence numbers.
 */
void test_twccFindPacket_NotFound( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t * pFoundTwccPacketInfo;

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( i + 1,
                           twccManager.count );
    }

    result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                             2,
                                             &( pFoundTwccPacketInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet fail functionality for bad parameters.
 */
void test_twccHandlePacket_BadParams( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    RtcpTwccPacket_t twccPacket = { 0 };
    TwccBandwidthInfo_t twccBandwidthInfo = { 0 };

    result = RtcpTwccManager_HandleTwccPacket( NULL,
                                               &( twccPacket ),
                                               &( twccBandwidthInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               NULL,
                                               &( twccBandwidthInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               NULL );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality for no packets.
 */
void test_twccHandlePacket_NoPackets( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultInitPass, result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t twccBandwidthInfo;
    PacketArrivalInfo_t arrivalInfoList[ TWCC_PACKET_INFO_ARRAY_LENGTH ];

    resultInitPass = RtcpTwccManager_Init( &( twccManager ),
                                           &( twccPacketInfoArray[ 0 ] ),
                                           TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       resultInitPass );

    memset( &( arrivalInfoList[ 0 ] ),
            0,
            TWCC_PACKET_INFO_ARRAY_LENGTH );

    twccPacket.arrivalInfoListLength = TWCC_PACKET_INFO_ARRAY_LENGTH;
    twccPacket.pArrivalInfoList = &( arrivalInfoList[ 0 ] );

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               &( twccBandwidthInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0,
                       twccBandwidthInfo.duration );
    TEST_ASSERT_EQUAL( 0,
                       twccBandwidthInfo.sentBytes );
    TEST_ASSERT_EQUAL( 0,
                       twccBandwidthInfo.sentPackets );
    TEST_ASSERT_EQUAL( 0,
                       twccBandwidthInfo.receivedBytes );
    TEST_ASSERT_EQUAL( 0,
                       twccBandwidthInfo.receivedPackets );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality.
 */
void test_twccHandlePacket( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t bandwidthInfo = { 0 };
    TwccPacketInfo_t packetInfo = { 0 };
    PacketArrivalInfo_t arrivalInfoList[ TWCC_PACKET_INFO_ARRAY_LENGTH ];
    size_t packetSize = ( rand() % ( 1000 ) );
    uint64_t sentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        packetInfo.packetSize = packetSize;
        packetInfo.localSentTime = sentTime + ( uint64_t ) i;
        packetInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( packetInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );

        arrivalInfoList[ i ].seqNum = packetInfo.packetSeqNum;
        if( ( i % 2 ) == 0 )
        {
            arrivalInfoList[ i ].remoteArrivalTime = RTCP_TWCC_PACKET_LOST_TIME; /* Set half of the packets as lost. */
        }
        else
        {
            arrivalInfoList[ i ].remoteArrivalTime = packetInfo.localSentTime + 1; /* Set half of the packets as received. */
        }
    }

    twccPacket.arrivalInfoListLength = TWCC_PACKET_INFO_ARRAY_LENGTH;
    twccPacket.pArrivalInfoList = &( arrivalInfoList[ 0 ] );

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               &( bandwidthInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH - 1,
                       bandwidthInfo.duration );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       bandwidthInfo.sentPackets );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH * packetSize,
                       bandwidthInfo.sentBytes );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH / 2,
                       bandwidthInfo.receivedPackets );
    TEST_ASSERT_EQUAL( ( TWCC_PACKET_INFO_ARRAY_LENGTH / 2 ) * packetSize,
                       bandwidthInfo.receivedBytes );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality when sequence number is not found in the manager.
 */
void test_twccHandlePacket_SeqNumNotFound( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t bandwidthInfo;
    TwccPacketInfo_t packetInfo;
    PacketArrivalInfo_t arrivalInfo;
    size_t packetSize = ( rand() % ( 1000 ) );
    uint64_t sentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        packetInfo.packetSize = packetSize + ( size_t ) i;
        packetInfo.localSentTime = sentTime + ( uint64_t ) i;
        packetInfo.packetSeqNum = seqNum + ( uint16_t ) i;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( packetInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }

    /* Create a RtcpTwccPacket_t with a sequence number that doesn't exist in
     * the RtcpTwccManager_t. */
    arrivalInfo.seqNum = seqNum + TWCC_PACKET_INFO_ARRAY_LENGTH;
    arrivalInfo.remoteArrivalTime = 0;

    twccPacket.arrivalInfoListLength = 1;
    twccPacket.pArrivalInfoList = &( arrivalInfo );

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               &( bandwidthInfo ) );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0,
                       bandwidthInfo.duration );
    TEST_ASSERT_EQUAL( 0,
                       bandwidthInfo.sentPackets );
    TEST_ASSERT_EQUAL( 0,
                       bandwidthInfo.receivedPackets );
    TEST_ASSERT_EQUAL( 0,
                       bandwidthInfo.sentBytes );
    TEST_ASSERT_EQUAL( 0,
                       bandwidthInfo.receivedBytes );
}

/*-----------------------------------------------------------*/
