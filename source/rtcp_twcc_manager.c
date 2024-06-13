/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* API includes. */
#include "rtcp_twcc_manager.h"

/*-----------------------------------------------------------*/

#define WRAP( x, n ) \
    ( ( x ) % ( n ) )

#define WRAP_DEC( x, n ) \
    ( ( x - 1 + n ) % n )

#define INC_READ_INDEX( pTwccCtx ) \
    WRAP( ( pTwccCtx )->readIndex + 1,\
          ( pTwccCtx )->twccPacketInfoArrayLength )

#define INC_WRITE_INDEX( pTwccCtx ) \
    WRAP( ( pTwccCtx )->writeIndex + 1,\
          ( pTwccCtx )->twccPacketInfoArrayLength )

#define IS_TWCC_BUFFER_FULL( pTwccCtx ) \
    ( INC_WRITE_INDEX( pTwccCtx ) == ( pTwccCtx )->readIndex )

#define IS_TWCC_BUFFER_EMPTY( pTwccCtx ) \
    ( ( pTwccCtx )->readIndex == ( pTwccCtx )->writeIndex )

#define RTCP_READ_UINT16    ( pTwccCtx->readWriteFunctions.readUint16Fn )
/*-----------------------------------------------------------*/

RtcpResult_t RtcpTwcc_Init( RtcpTwccManagerCtx_t * pTwccCtx,
                            TwccPacketInfo_t * pTwccPacketInfoArray,
                            size_t twccPacketInfoArrayLength )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pTwccCtx == NULL ) ||
        ( pTwccPacketInfoArray == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pTwccCtx->pTwccPktInfoArray = pTwccPacketInfoArray;
        pTwccCtx->twccPacketInfoArrayLength = twccPacketInfoArrayLength;

        memset( pTwccCtx->pTwccPktInfoArray,
                0,
                sizeof( TwccPacketInfo_t ) * twccPacketInfoArrayLength );
        pTwccCtx->readIndex = 0;
        pTwccCtx->writeIndex = 0;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t RtcpTwcc_AddPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                     uint32_t payloadLength,
                                     uint32_t sentTime,
                                     uint16_t seqNumber )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pTwccPacketInfo;

    if( pTwccCtx == NULL )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( IS_TWCC_BUFFER_FULL( pTwccCtx ) )
        {
            pTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->readIndex ] );
            memset( pTwccPacketInfo,
                    0,
                    sizeof( pTwccPacketInfo ) );
            pTwccCtx->readIndex = INC_READ_INDEX( pTwccCtx );
        }

        pTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->writeIndex ] );

        pTwccPacketInfo->packetSize = payloadLength;
        pTwccPacketInfo->localTimeKvs = sentTime;
        pTwccPacketInfo->remoteTimeKvs = RTCP_TWCC_PACKET_LOST_TIME;
        pTwccPacketInfo->packetSequenceNumber = seqNumber;
        pTwccCtx->writeIndex = INC_WRITE_INDEX( pTwccCtx );

        // result = RtcpTwcc_DeletePacketInfo( pTwccCtx,
        //                                     sentTime,
        //                                     seqNumber );
    }

    return result;
}
/*-----------------------------------------------------------*/

/* Read the packet info present at the read Index. */
RtcpResult_t RtcpTwcc_GetPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                     TwccPacketInfo_t * pTwccPacketInfo )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pReadTwccPacketInfo;

    if( ( pTwccCtx == NULL ) ||
        ( pTwccPacketInfo == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( IS_TWCC_BUFFER_EMPTY( pTwccCtx ) )
        {
            result = RTCP_RESULT_TWCC_BUFFER_EMPTY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pReadTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->readIndex ] );
        pTwccPacketInfo->localTimeKvs = pReadTwccPacketInfo->localTimeKvs;
        pTwccPacketInfo->packetSequenceNumber = pReadTwccPacketInfo->packetSequenceNumber;
        pTwccPacketInfo->packetSize = pReadTwccPacketInfo->packetSize;
        pTwccPacketInfo->remoteTimeKvs = pReadTwccPacketInfo->remoteTimeKvs;
    }

    return result;
}
/*-----------------------------------------------------------*/

/* Extract the packet info for a specific sequence number */
RtcpResult_t RtcpTwcc_GetSeqNum_PacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                            TwccPacketInfo_t * pTwccPacketInfo,
                                            uint16_t seqNum )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pReadTwccPacketInfo;
    size_t i;

    if( ( pTwccCtx == NULL ) ||
        ( pTwccPacketInfo == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( IS_TWCC_BUFFER_EMPTY( pTwccCtx ) )
        {
            result = RTCP_RESULT_TWCC_BUFFER_EMPTY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        result = RTCP_RESULT_TWCC_NO_PACKET_FOUND;
        for( i = pTwccCtx->readIndex;
             i != pTwccCtx->writeIndex;
             i = WRAP( ( i + 1 ),
                       pTwccCtx->twccPacketInfoArrayLength ) )
        {
            if( pTwccCtx->pTwccPktInfoArray[ i ].packetSequenceNumber == seqNum )
            {
                pTwccPacketInfo->localTimeKvs = pTwccCtx->pTwccPktInfoArray[ i ].localTimeKvs;
                pTwccPacketInfo->packetSequenceNumber = pTwccCtx->pTwccPktInfoArray[ i ].packetSequenceNumber;
                pTwccPacketInfo->packetSize = pTwccCtx->pTwccPktInfoArray[ i ].packetSize;
                pTwccPacketInfo->remoteTimeKvs = pTwccCtx->pTwccPktInfoArray[ i ].remoteTimeKvs;

                result = RTCP_RESULT_OK;
                break;
            }
        }

    }

    return result;
}
/*-----------------------------------------------------------*/

/* Extract the packet info for a specific sequence number */
RtcpResult_t RtcpTwcc_ExtractPacketInfo( RtcpTwccManagerCtx_t * pTwccCtx,
                                         TwccPacketInfo_t * pTwccPacketInfo )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pReadTwccPacketInfo;

    if( ( pTwccCtx == NULL ) ||
        ( pTwccPacketInfo == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( IS_TWCC_BUFFER_EMPTY( pTwccCtx ) )
        {
            result = RTCP_RESULT_TWCC_BUFFER_EMPTY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pReadTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->readIndex ] );
        pTwccPacketInfo->localTimeKvs = pReadTwccPacketInfo->localTimeKvs;
        pTwccPacketInfo->packetSequenceNumber = pReadTwccPacketInfo->packetSequenceNumber;
        pTwccPacketInfo->packetSize = pReadTwccPacketInfo->packetSize;
        pTwccPacketInfo->remoteTimeKvs = pReadTwccPacketInfo->remoteTimeKvs;

        memset( pReadTwccPacketInfo,
                0,
                sizeof( TwccPacketInfo_t ) );
        pTwccCtx->readIndex = INC_READ_INDEX( pTwccCtx );
    }

    return result;
}

/*-----------------------------------------------------------*/

/* Remove all the packets older than a nanosecond. */
RtcpResult_t RtcpTwcc_OlderPacketInfoDeletion( RtcpTwccManagerCtx_t * pTwccCtx,
                                               uint64_t currentPacketSentTime,
                                               uint16_t seqNumber )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pTwccPacketInfo;
    uint16_t seqNumRead;
    uint64_t ageOfOldest = 0, firstRtpTime = 0;
    uint8_t isCheckComplete = 0;

    if( pTwccCtx == NULL )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->readIndex ] );
        firstRtpTime = pTwccPacketInfo->localTimeKvs;
        seqNumRead = pTwccPacketInfo->packetSequenceNumber;

        while( isCheckComplete == 0 &&
               seqNumRead != seqNumber )
        {
            if( currentPacketSentTime >= firstRtpTime )
            {
                ageOfOldest = currentPacketSentTime - firstRtpTime;
                if( ageOfOldest > RTCP_TWCC_ESTIMATOR_TIME_WINDOW )
                {
                    memset( pTwccPacketInfo,
                            0,
                            sizeof( pTwccPacketInfo ) );
                    pTwccCtx->readIndex = INC_READ_INDEX( pTwccCtx );
                }
                else {
                    isCheckComplete = 1;
                }
                pTwccPacketInfo = &( pTwccCtx->pTwccPktInfoArray[ pTwccCtx->readIndex ] );
                firstRtpTime = pTwccPacketInfo->localTimeKvs;
                seqNumRead = pTwccPacketInfo->packetSequenceNumber;
            }
            else
            {
                isCheckComplete = 1;
            }
        }
    }

    return result;
}
/*-----------------------------------------------------------*/

/* Parse RTCP TWCC packet chunk information to get firstreported and lastreported sequence number. */
RtcpResult_t RtcpTwcc_ParseRtcpChunk( RtcpTwccManagerCtx_t * pTwccCtx,
                                      RtcpTwccPacket_t * rtcpTwccPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t * pTwccPacketInfo;
    size_t packetsRemaining;
    uint8_t statusSymbol, symbolSize;
    uint16_t startSeqNum, recvDelta;
    uint32_t packetChunk, i, runLengthChunkPackets, statusSymbolCount;
    uint32_t chunkOffset, recvOffset, status;
    uint64_t referenceTime;

    if( ( pTwccCtx == NULL ) ||
        ( rtcpTwccPacket == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        packetsRemaining = rtcpTwccPacket->packetStatusCount;
        startSeqNum = rtcpTwccPacket->baseSeqNum;
        pTwccCtx->firstReportedSeqNum = startSeqNum;
        referenceTime = RTCP_CONVERT_TIMESCALE( rtcpTwccPacket->referenceTime * 64,
                                                RTCP_MILLISECONDS_PER_SECOND,
                                                RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND );

        while( packetsRemaining > 0 )
        {
            packetChunk = RTCP_READ_UINT16( &( rtcpTwccPacket->pPacketChunkStart[ chunkOffset ] ) );
            if( CHUNK_TYPE( packetChunk ) == RTCP_TWCC_RUN_LENGTH_CHUNK )
            {
                runLengthChunkPackets = ( packetChunk & RTCP_RUN_LENGTH_BITMASK );
                statusSymbol = ( packetChunk & RTCP_RUN_LENGTH_STATUS_SYMBOL_BITMASK ) >> RTCP_RUN_LENGTH_STATUS_SYMBOL_LOCATION;
                result = RtcpTwcc_GetSeqNum_PacketInfo( pTwccCtx,
                                                        pTwccPacketInfo,
                                                        startSeqNum );
                for( i = 0; i < runLengthChunkPackets; i++ )
                {
                    switch( statusSymbol )
                    {
                        case RTCP_TWCC_STATUS_SYMBOL_SMALLDELTA:
                            recvDelta = ( uint16_t ) rtcpTwccPacket->pRecvDeltaStart[ recvOffset ];
                            recvOffset += 1;
                            referenceTime += RTCP_CONVERT_TIMESCALE( recvDelta,
                                                                    RTCP_TWCC_TICKS_PER_SECOND,
                                                                    RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND );
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = referenceTime;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        case RTCP_TWCC_STATUS_SYMBOL_LARGEDELTA:
                            recvDelta = RTCP_READ_UINT16( &( rtcpTwccPacket->pRecvDeltaStart[ recvOffset ] ) );
                            recvOffset += 2;
                            referenceTime += RTCP_CONVERT_TIMESCALE( recvDelta,
                                                                    RTCP_TWCC_TICKS_PER_SECOND,
                                                                    RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND );
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = referenceTime;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        case RTCP_TWCC_STATUS_SYMBOL_NOTRECEIVED:
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = RTCP_TWCC_PACKET_LOST_TIME;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        default:
                            /* runLength unhandled statusSymbol */
                            break;
                    }
                }

                startSeqNum++;
                packetsRemaining--;
            }
            else
            {
                symbolSize = ( packetChunk & RTCP_VECTOR_SYMBOL_SIZE_BITMASK ) >> RTCP_VECTOR_SYMBOL_SIZE_LOCATION;
                statusSymbolCount = ( symbolSize == 1 ) ? 14 : 7;
                status = ( packetsRemaining < statusSymbolCount ) ? packetsRemaining : statusSymbolCount;

                for( i = 0; i < status; i++ )
                {
                    statusSymbol = ( packetChunk >> RTCP_VECTOR_SYMBOL_BITMASK( i,
                                                                                symbolSize ) ) & RTCP_VECTOR_SYMBOL_LOCATION( symbolSize );
                    result = RtcpTwcc_GetSeqNum_PacketInfo( pTwccCtx,
                                                            pTwccPacketInfo,
                                                            startSeqNum );
                    switch( statusSymbol )
                    {
                        case RTCP_TWCC_STATUS_SYMBOL_SMALLDELTA:
                            recvDelta = ( uint16_t ) rtcpTwccPacket->pRecvDeltaStart[ recvOffset ];
                            recvOffset += 1;
                            referenceTime += RTCP_CONVERT_TIMESCALE( recvDelta,
                                                                    RTCP_TWCC_TICKS_PER_SECOND,
                                                                    RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND );
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = referenceTime;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        case RTCP_TWCC_STATUS_SYMBOL_LARGEDELTA:
                            recvDelta = RTCP_READ_UINT16( &( rtcpTwccPacket->pRecvDeltaStart[ recvOffset ] ) );
                            recvOffset += 2;
                            referenceTime += RTCP_CONVERT_TIMESCALE( recvDelta,
                                                                    RTCP_TWCC_TICKS_PER_SECOND,
                                                                    RTCP_HUNDREDS_OF_NANOS_IN_A_SECOND );
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = referenceTime;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        case RTCP_TWCC_STATUS_SYMBOL_NOTRECEIVED:
                            if( result == RTCP_RESULT_OK )
                            {
                                pTwccPacketInfo->remoteTimeKvs = RTCP_TWCC_PACKET_LOST_TIME;
                            }
                            pTwccCtx->lastReportedSeqNum = startSeqNum;
                            break;

                        default:
                            /* runLength unhandled statusSymbol */
                            break;
                    }

                    startSeqNum++;
                    packetsRemaining--;
                }
            }

            chunkOffset += RTCP_TWCC_PACKET_CHUNK_SIZE;
        }

    }

    return result;
}
/*-----------------------------------------------------------*/

/* Extract bandwidth information based on RTCP TWCC packet received. */
RtcpResult_t RtcpTwcc_GetBandwidthParameters( RtcpTwccManagerCtx_t * pTwccCtx,
                                              TwccBandwidthInfo_t * pBandwidthInfo )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    TwccPacketInfo_t twccPacketInfo;
    RtcpTwccPacket_t rtcpTwccPacket;
    size_t i;
    uint16_t baseSeqNum = 0, seqNum;
    uint64_t localStartTimeKvs, localEndTimeKvs;

    if( ( pTwccCtx == NULL ) ||
        ( pBandwidthInfo == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        memset( pBandwidthInfo,
                0,
                sizeof( TwccBandwidthInfo_t ) );
        baseSeqNum = pTwccCtx->firstReportedSeqNum;

        //Get oldest localStartTimeKvs
        seqNum = WRAP_DEC( baseSeqNum,
                           pTwccCtx->twccPacketInfoArrayLength );

        while( seqNum != ( pTwccCtx->lastReportedSeqNum + 1 ) )
        {
            result = RtcpTwcc_GetSeqNum_PacketInfo( pTwccCtx,
                                                    &twccPacketInfo,
                                                    seqNum );
            if( result == RTCP_RESULT_TWCC_NO_PACKET_FOUND )
            {
                localStartTimeKvs = RTCP_TWCC_PACKET_UNITIALIZED_TIME;
            }
            else
            {
                localStartTimeKvs = twccPacketInfo.localTimeKvs;
                break;
            }

            WRAP( ( seqNum + 1 ),
                  pTwccCtx->twccPacketInfoArrayLength );

            if( localStartTimeKvs == RTCP_TWCC_PACKET_UNITIALIZED_TIME )
            {
                // Time not yet set. If prev seqNum was not present / deleted
                result = RtcpTwcc_GetSeqNum_PacketInfo( pTwccCtx,
                                                        &twccPacketInfo,
                                                        seqNum );
                if( result == RTCP_RESULT_OK )
                {
                    localStartTimeKvs = twccPacketInfo.localTimeKvs;
                    break;
                }
            }
        }

        for( seqNum = baseSeqNum; seqNum != ( pTwccCtx->lastReportedSeqNum + 1 ); seqNum++ )
        {
            result = RtcpTwcc_GetSeqNum_PacketInfo( pTwccCtx,
                                                    &twccPacketInfo,
                                                    seqNum );
            if( result == RTCP_RESULT_OK )
            {
                localEndTimeKvs = twccPacketInfo.localTimeKvs;
                pBandwidthInfo->duration = localEndTimeKvs - localStartTimeKvs;
                pBandwidthInfo->sentBytes += twccPacketInfo.packetSize;
                pBandwidthInfo->sentPackets += 1;
                if( twccPacketInfo.remoteTimeKvs != RTCP_TWCC_PACKET_LOST_TIME )
                {
                    pBandwidthInfo->receivedBytes += twccPacketInfo.packetSize;
                    pBandwidthInfo->receivedPackets += 1;
                }
            }
        }
    }

    return result;
}