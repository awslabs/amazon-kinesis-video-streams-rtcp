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
 * @brief Validate Twcc Manager Init fail functionality for Bad Parameters.
 */
void test_twccInit_BadParam( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultFail1,resultFail2,resultFail3;

    resultFail1 = RtcpTwccManager_Init( NULL,
                                        &( twccPacketInfoArray[ 0 ] ),
                                        TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail1 );

    resultFail2 = RtcpTwccManager_Init( &( twccManager ),
                                        NULL,
                                        TWCC_PACKET_INFO_ARRAY_LENGTH );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail2 );

    resultFail3 = RtcpTwccManager_Init( &( twccManager ),
                                        &( twccPacketInfoArray[ 0 ] ),
                                        0 );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail3 );
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
void test_twccAddPacket_BadParam( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultInitPass, resultFail1, resultFail2;
    TwccPacketInfo_t twccPacketInfo = { 0 };

    resultInitPass = RtcpTwccManager_Init( &( twccManager ),
                                           &( twccPacketInfoArray[ 0 ] ),
                                           TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       resultInitPass );

    resultFail1 = RtcpTwccManager_AddPacketInfo( NULL,
                                                 &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail1 );

    resultFail2 = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                 NULL );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail2 );
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

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = ( rand() % ( 1000 ) );
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );
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

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = ( rand() % ( 1000 ) );
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager check Older packet deletion functionality for some Deletions only.
 */
void test_twccOlderPacketInfoDeletion_PartialDeletion( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t foundTwccPacketInfo;

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

    twccPacketInfo.localSentTime = time( NULL );
    twccPacketInfo.packetSize = 0;
    twccPacketInfo.packetSeqNum += seqNum;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 1; i < TWCC_PACKET_INFO_ARRAY_LENGTH * 2; i++ )
    {
        twccPacketInfo.packetSize = i;
        if( i % 2 == 0 ) {
            twccPacketInfo.localSentTime -= ( RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1 );
        }
        else{
            twccPacketInfo.localSentTime += RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1;
        }
        twccPacketInfo.packetSeqNum += seqNum;

        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );

        /* Only the last added packet should be in the Manager. */
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 twccPacketInfo.packetSeqNum,
                                                 &( foundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                           foundTwccPacketInfo.packetSize );
        TEST_ASSERT_EQUAL( twccPacketInfo.localSentTime,
                           foundTwccPacketInfo.localSentTime );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSeqNum,
                           foundTwccPacketInfo.packetSeqNum );
    }
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
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    TwccPacketInfo_t foundTwccPacketInfo;

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

    twccPacketInfo.localSentTime = time( NULL );
    twccPacketInfo.packetSize = 0;
    twccPacketInfo.packetSeqNum += seqNum;

    result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                            &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 1; i < TWCC_PACKET_INFO_ARRAY_LENGTH * 2; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime += RTCP_TWCC_ESTIMATOR_TIME_WINDOW + 1;
        twccPacketInfo.packetSeqNum += seqNum;

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
                                                 &( foundTwccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSize,
                           foundTwccPacketInfo.packetSize );
        TEST_ASSERT_EQUAL( twccPacketInfo.localSentTime,
                           foundTwccPacketInfo.localSentTime );
        TEST_ASSERT_EQUAL( twccPacketInfo.packetSeqNum,
                           foundTwccPacketInfo.packetSeqNum );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet fail functionality for Bad Parameters.
 */
void test_twccFindPacket_BadParam( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultInitPass, resultFail1, resultFail2;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    uint64_t localSentTime = time( NULL );

    resultInitPass = RtcpTwccManager_Init( &( twccManager ),
                                           &( twccPacketInfoArray[ 0 ] ),
                                           TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       resultInitPass );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = localSentTime + ( int64_t ) i;
        twccPacketInfo.packetSeqNum += seqNum;

        resultInitPass = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                        &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           resultInitPass );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );

    resultFail1 = RtcpTwccManager_FindPacketInfo( NULL,
                                                  seqNum * ( i + 1 ),
                                                  &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail1 );

    resultFail2 = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                  seqNum * ( i + 1 ),
                                                  NULL );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail2 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality for Empty Packets.
 */
void test_twccFindPacket_Empty( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };

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
                                                 &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_EMPTY,
                           result );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Find packet functionality for Non-retrival.
 */
void test_twccFindPacket_NotFound( void )
{
    uint16_t seqNum = 256;
    uint32_t i;
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = time( NULL );
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );

    result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                             2,
                                             &( twccPacketInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND,
                       result );
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
    RtcpTwccManagerResult_t result;
    TwccPacketInfo_t twccPacketInfo = { 0 };
    uint64_t localSentTime = time( NULL );

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        twccPacketInfo.packetSize = i;
        twccPacketInfo.localSentTime = localSentTime + ( int64_t ) i;
        twccPacketInfo.packetSeqNum += seqNum;
        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( twccPacketInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }

    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       twccManager.count );

    for( i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++ )
    {
        result = RtcpTwccManager_FindPacketInfo( &( twccManager ),
                                                 seqNum * ( i + 1 ),
                                                 &( twccPacketInfo ) );

        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
        TEST_ASSERT_EQUAL( i,
                           twccPacketInfo.packetSize );
        TEST_ASSERT_EQUAL( seqNum * ( i + 1 ),
                           twccPacketInfo.packetSeqNum );
        TEST_ASSERT_EQUAL( localSentTime + ( int64_t ) i,
                           twccPacketInfo.localSentTime );
    }
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet fail functionality for Bad Parameter.
 */
void test_twccHandlePacket_BadParam( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultInitPass, resultFail1, resultFail2, resultFail3;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t twccBandwidthInfo = { 0 };

    resultInitPass = RtcpTwccManager_Init( &( twccManager ),
                                           &( twccPacketInfoArray[ 0 ] ),
                                           TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       resultInitPass );

    resultFail1 = RtcpTwccManager_HandleTwccPacket( NULL,
                                                    &( twccPacket ),
                                                    &( twccBandwidthInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail1 );

    resultFail2 = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                                    NULL,
                                                    &( twccBandwidthInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail2 );

    resultFail3 = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                                    &( twccPacket ),
                                                    NULL );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_BAD_PARAM,
                       resultFail3 );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality for No Packets.
 */
void test_twccHandlePacket_NoPackets( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t resultInitPass, result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t twccBandwidthInfo = { 0 };
    TwccPacketInfo_t packetInfo = { 0 };
    PacketArrivalInfo_t arrivalInfoList[TWCC_PACKET_INFO_ARRAY_LENGTH];
    uint64_t localSentTime = time( NULL );
    uint16_t seqNum = 256;
    size_t i;

    resultInitPass = RtcpTwccManager_Init( &( twccManager ),
                                           &( twccPacketInfoArray[ 0 ] ),
                                           TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       resultInitPass );

    for(i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++)
    {
        packetInfo.packetSize = i;
        packetInfo.localSentTime = localSentTime + ( int64_t )i;
        packetInfo.packetSeqNum = seqNum - 1 + ( uint16_t )( i );

        arrivalInfoList[i].seqNum = packetInfo.packetSeqNum;
        arrivalInfoList[i].remoteArrivalTime = localSentTime + ( int64_t )i; // Set some packets as received
    }

    // Creating a valid RtcpTwccPacket_t
    twccPacket.arrivalInfoListLength = TWCC_PACKET_INFO_ARRAY_LENGTH;
    twccPacket.pArrivalInfoList = arrivalInfoList;

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               &( twccBandwidthInfo ) );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality.
 */
void test_twccHandlePacket( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t bandwidthInfo = { 0 };
    TwccPacketInfo_t packetInfo = { 0 };
    PacketArrivalInfo_t arrivalInfoList[TWCC_PACKET_INFO_ARRAY_LENGTH];
    uint64_t localSentTime = time( NULL );
    uint16_t seqNum = 256;
    size_t i;

    result = RtcpTwccManager_Init( &( twccManager ),
                                   &( twccPacketInfoArray[ 0 ] ),
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for(i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++)
    {
        packetInfo.packetSize = i;
        packetInfo.localSentTime = localSentTime + ( int64_t )i;
        packetInfo.packetSeqNum = seqNum - 1 + ( uint16_t )( i );
        result = RtcpTwccManager_AddPacketInfo( &( twccManager ),
                                                &( packetInfo ) );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );

        arrivalInfoList[i].seqNum = packetInfo.packetSeqNum;
        if( i % 2 == 0 ) {
            arrivalInfoList[i].remoteArrivalTime = RTCP_TWCC_PACKET_LOST_TIME; // Set some packets as lost
        }
        else {
            arrivalInfoList[i].remoteArrivalTime = localSentTime + ( int64_t )i; // Set some packets as received
        }
    }

    // Creating a valid RtcpTwccPacket_t
    twccPacket.arrivalInfoListLength = TWCC_PACKET_INFO_ARRAY_LENGTH;
    twccPacket.pArrivalInfoList = arrivalInfoList;

    result = RtcpTwccManager_HandleTwccPacket( &( twccManager ),
                                               &( twccPacket ),
                                               &bandwidthInfo );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_GREATER_THAN( 0,
                              bandwidthInfo.duration );
    TEST_ASSERT_EQUAL( TWCC_PACKET_INFO_ARRAY_LENGTH,
                       bandwidthInfo.sentPackets );
    TEST_ASSERT_GREATER_THAN( 0,
                              bandwidthInfo.sentBytes );
    TEST_ASSERT_GREATER_THAN( 0,
                              bandwidthInfo.receivedBytes );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate Twcc Manager Handle packet functionality when unidentified Sequence Number Packet is found.
 */
void test_twccHandlePacket_unidentifiedSeqNum( void )
{
    RtcpTwccManager_t twccManager;
    RtcpTwccManagerResult_t result;
    RtcpTwccPacket_t twccPacket;
    TwccBandwidthInfo_t bandwidthInfo;
    TwccPacketInfo_t packetInfo;
    PacketArrivalInfo_t arrivalInfo;
    uint64_t localSentTime = time( NULL );
    uint16_t seqNum = 256;
    size_t i;

    result = RtcpTwccManager_Init( &twccManager,
                                   &twccPacketInfoArray[0],
                                   TWCC_PACKET_INFO_ARRAY_LENGTH );
    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );

    for(i = 0; i < TWCC_PACKET_INFO_ARRAY_LENGTH; i++)
    {
        packetInfo.packetSize = i;
        packetInfo.localSentTime = localSentTime + ( int64_t )i;
        packetInfo.packetSeqNum = seqNum + ( uint16_t )i;
        result = RtcpTwccManager_AddPacketInfo( &twccManager,
                                                &packetInfo );
        TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                           result );
    }

    // Create a RtcpTwccPacket_t with a sequence number that doesn't exist in the RtcpTwccManager_t
    twccPacket.arrivalInfoListLength = 1;
    arrivalInfo.seqNum = seqNum + TWCC_PACKET_INFO_ARRAY_LENGTH;
    arrivalInfo.remoteArrivalTime = 0;
    twccPacket.pArrivalInfoList = &arrivalInfo;

    RtcpTwccManager_HandleTwccPacket( &twccManager,
                                      &twccPacket,
                                      &bandwidthInfo );

    TEST_ASSERT_EQUAL( RTCP_TWCC_MANAGER_RESULT_OK,
                       result );
    TEST_ASSERT_GREATER_OR_EQUAL_INT( 0,
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