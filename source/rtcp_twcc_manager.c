/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* API includes. */
#include "rtcp_twcc_manager.h"

/*-----------------------------------------------------------*/

#define WRAP( x, n ) \
    ( ( x ) % ( n ) )

#define INC_READ_INDEX( pTwccManager )      \
    WRAP( ( pTwccManager )->readIndex + 1,  \
          ( pTwccManager )->twccPacketInfoArrayLength )

#define INC_WRITE_INDEX( pTwccManager )     \
    WRAP( ( pTwccManager )->writeIndex + 1, \
          ( pTwccManager )->twccPacketInfoArrayLength )

#define IS_TWCC_MANAGER_FULL( pTwccManager ) \
    ( ( pTwccManager )->count == ( pTwccManager )->twccPacketInfoArrayLength )

#define IS_TWCC_MANAGER_EMPTY( pTwccManager ) \
    ( ( pTwccManager )->count == 0 )

/*-----------------------------------------------------------*/

static void DeleteOlderPacketInfos( RtcpTwccManager_t * pTwccManager,
                                    uint64_t currentPacketSentTime )
{
    size_t i, readIndex, count;
    TwccPacketInfo_t * pTwccPacketInfo;
    uint64_t packetAge;

    /* Make local copies as these values may get modified in the loop below. */
    count = pTwccManager->count;
    readIndex = pTwccManager->readIndex;

    /* If i == count - 1 then the newly added packet is the one to be deleted,
     * which is never the case. Hence i < count - 1. */
    for( i = 0; i < count - 1 ; i++ )
    {
        pTwccPacketInfo = &( pTwccManager->pTwccPacketInfoArray[ WRAP( readIndex + i,
                                                                       pTwccManager->twccPacketInfoArrayLength ) ] );

        if( currentPacketSentTime >= pTwccPacketInfo->localSentTime )
        {
            packetAge = currentPacketSentTime - pTwccPacketInfo->localSentTime;

            if( packetAge > RTCP_TWCC_ESTIMATOR_TIME_WINDOW )
            {
                memset( pTwccPacketInfo,
                        0,
                        sizeof( TwccPacketInfo_t ) );

                /* We can modify these because we have already stored the
                 * values at the beginning which we use for iterating. */
                pTwccManager->readIndex = INC_READ_INDEX( pTwccManager );
                pTwccManager->count -= 1;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

/*-----------------------------------------------------------*/

RtcpTwccManagerResult_t RtcpTwccManager_Init( RtcpTwccManager_t * pTwccManager,
                                              TwccPacketInfo_t * pTwccPacketInfoArray,
                                              size_t twccPacketInfoArrayLength )
{
    RtcpTwccManagerResult_t result = RTCP_TWCC_MANAGER_RESULT_OK;

    if( ( pTwccManager == NULL ) ||
        ( pTwccPacketInfoArray == NULL ) ||
        ( twccPacketInfoArrayLength == 0 ) )
    {
        result = RTCP_TWCC_MANAGER_RESULT_BAD_PARAM;
    }

    if( result == RTCP_TWCC_MANAGER_RESULT_OK )
    {
        pTwccManager->pTwccPacketInfoArray = pTwccPacketInfoArray;
        pTwccManager->twccPacketInfoArrayLength = twccPacketInfoArrayLength;

        memset( pTwccManager->pTwccPacketInfoArray,
                0,
                sizeof( TwccPacketInfo_t ) * twccPacketInfoArrayLength );
        pTwccManager->readIndex = 0;
        pTwccManager->writeIndex = 0;
        pTwccManager->count = 0;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpTwccManagerResult_t RtcpTwccManager_AddPacketInfo( RtcpTwccManager_t * pTwccManager,
                                                       const TwccPacketInfo_t * pTwccPacketInfoToAdd )
{
    RtcpTwccManagerResult_t result = RTCP_TWCC_MANAGER_RESULT_OK;
    TwccPacketInfo_t * pTwccPacketInfo;

    if( ( pTwccManager == NULL ) ||
        ( pTwccPacketInfoToAdd == NULL ) )
    {
        result = RTCP_TWCC_MANAGER_RESULT_BAD_PARAM;
    }

    if( result == RTCP_TWCC_MANAGER_RESULT_OK )
    {
        if( IS_TWCC_MANAGER_FULL( pTwccManager ) )
        {
            pTwccPacketInfo = &( pTwccManager->pTwccPacketInfoArray[ pTwccManager->readIndex ] );
            memset( pTwccPacketInfo,
                    0,
                    sizeof( TwccPacketInfo_t ) );
            pTwccManager->readIndex = INC_READ_INDEX( pTwccManager );
            pTwccManager->count -= 1;
        }

        pTwccPacketInfo = &( pTwccManager->pTwccPacketInfoArray[ pTwccManager->writeIndex ] );

        pTwccPacketInfo->packetSize = pTwccPacketInfoToAdd->packetSize;
        pTwccPacketInfo->localSentTime = pTwccPacketInfoToAdd->localSentTime;
        pTwccPacketInfo->packetSeqNum = pTwccPacketInfoToAdd->packetSeqNum;

        pTwccManager->writeIndex = INC_WRITE_INDEX( pTwccManager );
        pTwccManager->count += 1;

        DeleteOlderPacketInfos( pTwccManager, pTwccPacketInfoToAdd->localSentTime );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpTwccManagerResult_t RtcpTwccManager_FindPacketInfo( RtcpTwccManager_t * pTwccManager,
                                                        uint16_t seqNum,
                                                        TwccPacketInfo_t ** ppOutTwccPacketInfo )
{
    RtcpTwccManagerResult_t result = RTCP_TWCC_MANAGER_RESULT_OK;
    TwccPacketInfo_t * pTwccPacketInfo;
    size_t i;

    if( ( pTwccManager == NULL ) ||
        ( ppOutTwccPacketInfo == NULL ) )
    {
        result = RTCP_TWCC_MANAGER_RESULT_BAD_PARAM;
    }

    if( result == RTCP_TWCC_MANAGER_RESULT_OK )
    {
        if( IS_TWCC_MANAGER_EMPTY( pTwccManager ) )
        {
            result = RTCP_TWCC_MANAGER_RESULT_EMPTY;
        }
    }

    if( result == RTCP_TWCC_MANAGER_RESULT_OK )
    {
        result = RTCP_TWCC_MANAGER_RESULT_PACKET_NOT_FOUND;

        for( i = 0; i < pTwccManager->count; i++ )
        {
            pTwccPacketInfo = &( pTwccManager->pTwccPacketInfoArray[ WRAP( pTwccManager->readIndex + i,
                                                                           pTwccManager->twccPacketInfoArrayLength ) ] );

            if( pTwccPacketInfo->packetSeqNum == seqNum )
            {
                *ppOutTwccPacketInfo = pTwccPacketInfo;
                result = RTCP_TWCC_MANAGER_RESULT_OK;
                break;
            }
        }
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpTwccManagerResult_t RtcpTwccManager_HandleTwccPacket( RtcpTwccManager_t * pTwccManager,
                                                          const RtcpTwccPacket_t * pTwccPacket,
                                                          TwccBandwidthInfo_t * pBandwidthInfo )
{
    RtcpTwccManagerResult_t findPacketResult, result = RTCP_TWCC_MANAGER_RESULT_OK;
    size_t i;
    uint8_t localStartTimeRecorded = 0;
    uint64_t localStartTime, localEndTime;
    TwccPacketInfo_t * pTwccPacketInfo;
    PacketArrivalInfo_t * pArrivalInfo;

    if( ( pTwccManager == NULL ) ||
        ( pTwccPacket == NULL ) ||
        ( pBandwidthInfo == NULL ) )
    {
        result = RTCP_TWCC_MANAGER_RESULT_BAD_PARAM;
    }

    if( result == RTCP_TWCC_MANAGER_RESULT_OK )
    {
        memset( pBandwidthInfo,
                0,
                sizeof( TwccBandwidthInfo_t ) );

        for( i = 0; i < pTwccPacket->arrivalInfoListLength; i++ )
        {
            pArrivalInfo = &( pTwccPacket->pArrivalInfoList[ i ] );

            if( localStartTimeRecorded == 0 )
            {
                findPacketResult = RtcpTwccManager_FindPacketInfo( pTwccManager,
                                                                   pArrivalInfo->seqNum - 1,
                                                                   &( pTwccPacketInfo ) );

                if( findPacketResult != RTCP_TWCC_MANAGER_RESULT_OK )
                {
                    localStartTime = RTCP_TWCC_PACKET_UNINITIALIZED_TIME;
                }
                else
                {
                    localStartTime = pTwccPacketInfo->localSentTime;
                    localStartTimeRecorded = 1;
                }

                if( localStartTime == RTCP_TWCC_PACKET_UNINITIALIZED_TIME )
                {
                    findPacketResult = RtcpTwccManager_FindPacketInfo( pTwccManager,
                                                                       pArrivalInfo->seqNum,
                                                                       &( pTwccPacketInfo ) );

                    if( findPacketResult == RTCP_TWCC_MANAGER_RESULT_OK )
                    {
                        localStartTime = pTwccPacketInfo->localSentTime;
                        localStartTimeRecorded = 1;
                    }
                }
            }

            findPacketResult = RtcpTwccManager_FindPacketInfo( pTwccManager,
                                                               pArrivalInfo->seqNum,
                                                               &( pTwccPacketInfo ) );

            if( findPacketResult == RTCP_TWCC_MANAGER_RESULT_OK )
            {
                localEndTime = pTwccPacketInfo->localSentTime;
                pBandwidthInfo->duration = localEndTime - localStartTime;
                pBandwidthInfo->sentBytes += pTwccPacketInfo->packetSize;
                pBandwidthInfo->sentPackets += 1;

                if( pArrivalInfo->remoteArrivalTime != RTCP_TWCC_PACKET_LOST_TIME )
                {
                    pBandwidthInfo->receivedBytes += pTwccPacketInfo->packetSize;
                    pBandwidthInfo->receivedPackets += 1;
                }
            }
        }
    }

    return result;
}

/*-----------------------------------------------------------*/
