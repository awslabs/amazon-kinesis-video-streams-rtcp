/* Standard includes. */
#include <string.h>

/* API includes. */
#include "rtcp_api.h"
#include "rtcp_data_types.h"

/*-----------------------------------------------------------*/

/*
 * Helper macros.
 */
#define RTCP_WRITE_UINT16   ( pCtx->readWriteFunctions.writeUint16Fn )
#define RTCP_WRITE_UINT32   ( pCtx->readWriteFunctions.writeUint32Fn )
#define RTCP_WRITE_UINT64   ( pCtx->readWriteFunctions.writeUint64Fn )
#define RTCP_READ_UINT16    ( pCtx->readWriteFunctions.readUint16Fn )
#define RTCP_READ_UINT32    ( pCtx->readWriteFunctions.readUint32Fn )
#define RTCP_READ_UINT64    ( pCtx->readWriteFunctions.readUint64Fn )

#define RTCP_BYTES_TO_WORDS( bytes )    \
    ( ( bytes ) / 4 )

#define RTCP_WORDS_TO_BYTES( words )    \
    ( ( words ) * 4 )

/*-----------------------------------------------------------*/

/*
 * RTCP Packet type and FMT values.
 */
#define RTCP_PACKET_TYPE_FIR                            192 /* https://datatracker.ietf.org/doc/html/rfc2032#section-5.2.1 */
#define RTCP_PACKET_TYPE_SENDER_REPORT                  200 /* https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.1 */
#define RTCP_PACKET_TYPE_RECEIVER_REPORT                201 /* https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.2 */
#define RTCP_PACKET_TYPE_SOURCE_DESCRIPTION             202
#define RTCP_PACKET_TYPE_TRANSPORT_SPECIFIC_FEEDBACK    205 /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.2 */
#define RTCP_PACKET_TYPE_PAYLOAD_SPECIFIC_FEEDBACK      206 /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.3 */

#define RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_PLI      1  /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.3 */
#define RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_SLI      2  /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.3 */
#define RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_RPSI     3  /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.3 */
#define RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_REMB     15 /* https://datatracker.ietf.org/doc/html/draft-alvestrand-rmcat-remb-03#section-2.2 */
#define RTCP_FMT_TRANSPORT_SPECIFIC_FEEDBACK_NACK   1  /* https://datatracker.ietf.org/doc/html/rfc4585#section-6.2.1 */
#define RTCP_FMT_TRANSPORT_SPECIFIC_FEEDBACK_TWCC   15 /* https://datatracker.ietf.org/doc/html/draft-holmer-rmcat-transport-wide-cc-extensions-01#section-3.1 */

/*-----------------------------------------------------------*/

/* RTCP Header:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|    FMT     |       PT      |             length         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define RTCP_HEADER_LENGTH                      4
#define RTCP_HEADER_VERSION                     2

#define RTCP_HEADER_VERSION_BITMASK             0xC0000000
#define RTCP_HEADER_VERSION_LOCATION            30

#define RTCP_HEADER_PADDING_BITMASK             0x20000000
#define RTCP_HEADER_PADDING_LOCATION            29

#define RTCP_HEADER_RC_BITMASK                  0x1F000000
#define RTCP_HEADER_RC_LOCATION                 24

#define RTCP_HEADER_PACKET_TYPE_BITMASK         0x00FF0000
#define RTCP_HEADER_PACKET_TYPE_LOCATION        16

#define RTCP_HEADER_PACKET_LENGTH_BITMASK       0x0000FFFF
#define RTCP_HEADER_PACKET_LENGTH_LOCATION      0

/*-----------------------------------------------------------*/

/*
 * Sender Report (SR) RFC - https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.1
 * Receiver Report (RR) RFC - https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.2
 */
#define RTCP_SENDER_SSRC_LENGTH                     4
#define RTCP_SENDER_INFO_LENGTH                     20
#define RTCP_RECEPTION_REPORT_LENGTH                24

#define RTCP_SENDER_REPORT_MIN_PAYLOAD_LENGTH       24
#define RTCP_RECEIVER_REPORT_MIN_PAYLOAD_LENGTH     4

#define RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET    31

#define RTCP_FRACTION_LOST_BITMASK                  0xFF000000
#define RTCP_FRACTION_LOST_LOCATION                 24

#define RTCP_PACKET_LOST_BITMASK                    0x00FFFFFF
#define RTCP_PACKET_LOST_LOCATION                   0

/*-----------------------------------------------------------*/

/*
 * Full INTRA-frame Request (FIR) packet.
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc2032#section-5.2.1
 */
#define RTCP_FIR_PACKET_PAYLOAD_LENGTH          4
#define RTCP_FIR_PACKET_SENDER_SSRC_OFFSET      0

/*-----------------------------------------------------------*/

/*
 * Picture Loss Indication (PLI):
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.3.1
 */
#define RTCP_PLI_PACKET_PAYLOAD_LENGTH          8
#define RTCP_PLI_PACKET_SENDER_SSRC_OFFSET      0
#define RTCP_PLI_PACKET_MEDIA_SSRC_OFFSET       4

/*-----------------------------------------------------------*/

/*
 * Slice Loss Indication (SLI):
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.3.2
 */
#define RTCP_SLI_PACKET_MIN_PAYLOAD_LENGTH      12

/*-----------------------------------------------------------*/

/*
 * Receiver Estimated Max Bitrate (REMB):
 *
 * RFC - https://datatracker.ietf.org/doc/html/draft-alvestrand-rmcat-remb-03#section-2.2
 */
#define RTCP_REMB_PACKET_MIN_PAYLOAD_LENGTH     20
#define RTCP_REMB_PACKET_IDENTIFIER_OFFSET      8
#define RTCP_REMB_PACKET_IDENTIFIER_LENGTH      4
#define RTCP_REMB_PACKET_NUM_SSRC_OFFSET        12

#define RTCP_REMB_PACKET_NUM_SSRC_BITMASK       0xFF000000
#define RTCP_REMB_PACKET_NUM_SSRC_LOCATION      24

#define RTCP_REMB_PACKET_BR_EXPONENT_BITMASK    0x00FC0000
#define RTCP_REMB_PACKET_BR_EXPONENT_LOCATION   18

#define RTCP_REMB_PACKET_BR_MANTISSA_BITMASK    0x0003FFFF
#define RTCP_REMB_PACKET_BR_MANTISSA_LOCATION   0

/*-----------------------------------------------------------*/

/*
 * Generic NACK:
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.2.1
 */
#define RTCP_NACK_PACKET_MIN_PAYLOAD_LENGTH     12

/*-----------------------------------------------------------*/

/*
 * Transport-wide Congestion Control (TWCC):
 *
 * RFC - https://datatracker.ietf.org/doc/html/draft-holmer-rmcat-transport-wide-cc-extensions-01#section-3.1
 */
#define RTCP_TWCC_PACKET_MIN_PAYLOAD_LENGTH         18

#define RTCP_TWCC_REFERENCE_TIME_BITMASK            0xFFFFFF00
#define RTCP_TWCC_REFERENCE_TIME_LOCATION           8

#define RTCP_TWCC_FEEDBACK_PACKET_COUNT_BITMASK     0x000000FF
#define RTCP_TWCC_FEEDBACK_PACKET_COUNT_LOCATION    0

#define RTCP_TWCC_PACKET_CHUNK_TYPE_BITMASK         0x8000
#define RTCP_TWCC_PACKET_CHUNK_TYPE_LOCATION        15

#define RTCP_TWCC_PACKET_CHUNK_STATUS_BITMASK       0x6000
#define RTCP_TWCC_PACKET_CHUNK_STATUS_LOCATION      13

#define RTCP_TWCC_PACKET_CHUNK_RUN_LENGTH_BITMASK   0x1FFF
#define RTCP_TWCC_PACKET_CHUNK_RUN_LENGTH_LOCATION  0

#define RTCP_TWCC_PACKET_CHUNK_SYMBOL_SIZE_BITMASK  0x4000
#define RTCP_TWCC_PACKET_CHUNK_SYMBOL_SIZE_LOCATION 14

#define RTCP_TWCC_PACKET_CHUNK_SYMBOL_LIST_BITMASK  0x3FFF
#define RTCP_TWCC_PACKET_CHUNK_SYMBOL_LIST_LOCATION 0

#define RTCP_TWCC_PACKET_CHUNK_TYPE_RUN_LENGTH      0
#define RTCP_TWCC_PACKET_CHUNK_TYPE_STATUS_VECTOR   1

#define RTCP_TWCC_PACKET_STATUS_NOT_RECEIVED        0
#define RTCP_TWCC_PACKET_STATUS_SMALL_DELTA         1
#define RTCP_TWCC_PACKET_STATUS_LARGE_DELTA         2

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_TYPE( packetChunk )          \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_TYPE_BITMASK ) >> \
      RTCP_TWCC_PACKET_CHUNK_TYPE_LOCATION )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_STATUS( packetChunk )            \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_STATUS_BITMASK ) >>   \
      RTCP_TWCC_PACKET_CHUNK_STATUS_LOCATION )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_RUN_LENGTH( packetChunk )            \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_RUN_LENGTH_BITMASK ) >>   \
      RTCP_TWCC_PACKET_CHUNK_RUN_LENGTH_LOCATION )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_SIZE( packetChunk )   \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_SYMBOL_SIZE_BITMASK ) == 0 ? 1 : 2 )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_COUNT( packetChunk )   \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_SYMBOL_SIZE_BITMASK ) == 0 ? 14 : 7 )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_LIST( packetChunk )           \
    ( ( ( packetChunk ) &  RTCP_TWCC_PACKET_CHUNK_SYMBOL_LIST_BITMASK ) >>  \
      RTCP_TWCC_PACKET_CHUNK_SYMBOL_LIST_LOCATION )

#define RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_FROM_LIST( symbolList, i, symbolSize )                \
    ( ( symbolSize ) == 1 ? ( ( ( symbolList ) >> ( 14 - ( ( i + 1 ) * ( symbolSize ) ) ) ) & 1 )   \
                          : ( ( ( symbolList ) >> ( 14 - ( ( i + 1 ) * ( symbolSize ) ) ) ) & 3 ) )

#define RTCP_TWCC_MS_TO_HUNDRED_OF_NANOS( ms )  \
    ( ( ( ms ) * RTCP_TWCC_HUNDREDS_OF_NANOS_IN_A_SECOND ) / 1000 )

#define RTCP_TWCC_US_TO_HUNDRED_OF_NANOS( us )  \
    ( ( ( us ) * RTCP_TWCC_HUNDREDS_OF_NANOS_IN_A_SECOND ) / 1000000 )

/*-----------------------------------------------------------*/

static void WriteSenderInfo( RtcpContext_t * pCtx,
                             const RtcpSenderInfo_t * pSenderInfo,
                             uint8_t * pBuffer,
                             size_t currentIndex )
{
    RTCP_WRITE_UINT64( &( pBuffer[ currentIndex ] ),
                       pSenderInfo->ntpTime );
    currentIndex += 8;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pSenderInfo->rtpTime );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pSenderInfo->packetCount );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pSenderInfo->octetCount );
}

/*-----------------------------------------------------------*/

static void ReadSenderInfo( RtcpContext_t * pCtx,
                            const RtcpPacket_t * pRtcpPacket,
                            size_t currentIndex,
                            RtcpSenderInfo_t * pSenderInfo )
{
    pSenderInfo->ntpTime = RTCP_READ_UINT64( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 8;

    pSenderInfo->rtpTime = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pSenderInfo->packetCount = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pSenderInfo->octetCount = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
}

/*-----------------------------------------------------------*/

static void WriteReceptionReport( RtcpContext_t * pCtx,
                                  const RtcpReceptionReport_t * pReceptionReport,
                                  uint8_t * pBuffer,
                                  size_t currentIndex )
{
    uint32_t packetLost;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pReceptionReport->sourceSsrc );
    currentIndex += 4;

    packetLost = ( ( pReceptionReport->fractionLost << RTCP_FRACTION_LOST_LOCATION ) &
                   RTCP_FRACTION_LOST_BITMASK );
    packetLost |= ( pReceptionReport->cumulativePacketsLost &
                    RTCP_PACKET_LOST_BITMASK );
    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       packetLost );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pReceptionReport->extendedHighestSeqNumReceived );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pReceptionReport->interArrivalJitter );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pReceptionReport->lastSR );
    currentIndex += 4;

    RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                       pReceptionReport->delaySinceLastSR );
}

/*-----------------------------------------------------------*/

static void ReadReceptionReport( RtcpContext_t * pCtx,
                                 const RtcpPacket_t * pRtcpPacket,
                                 size_t currentIndex,
                                 RtcpReceptionReport_t * pReceptionReport )
{
    uint32_t packetLost;

    pReceptionReport->sourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    packetLost = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pReceptionReport->fractionLost = ( uint8_t ) ( ( packetLost & RTCP_FRACTION_LOST_BITMASK ) >>
                                                   RTCP_FRACTION_LOST_LOCATION );
    pReceptionReport->cumulativePacketsLost = ( packetLost & RTCP_PACKET_LOST_BITMASK ) >>
                                              RTCP_PACKET_LOST_LOCATION;

    pReceptionReport->extendedHighestSeqNumReceived = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pReceptionReport->interArrivalJitter = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pReceptionReport->lastSR = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
    currentIndex += 4;

    pReceptionReport->delaySinceLastSR = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
}

/*-----------------------------------------------------------*/

static RtcpResult_t ParseTwccPacketChunks( RtcpContext_t * pCtx,
                                           const RtcpPacket_t * pRtcpPacket,
                                           size_t packetChunkStartIndex,
                                           size_t receiveDeltaStartIndex,
                                           RtcpTwccPacket_t * pTwccPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentPacketChunkIndex = packetChunkStartIndex;
    size_t currentReceiveDeltaIndex = receiveDeltaStartIndex;
    size_t numArrivalInfos = 0;
    uint16_t packetsRemaining = pTwccPacket->packetStatusCount;
    uint16_t i, packetChunk, statusSymbol, numPacketsInRunLengthChunk, recvDelta;
    uint16_t remoteSeqNum, symbolSize, symbolCount, symbolList;
    uint64_t referenceTime, remoteArrivalTime;

    remoteSeqNum = pTwccPacket->baseSeqNum;
    referenceTime = RTCP_TWCC_MS_TO_HUNDRED_OF_NANOS( pTwccPacket->referenceTime * 64 ); /* Reference time is represented in multiples of 64ms. */

    while( ( result == RTCP_RESULT_OK ) &&
           ( packetsRemaining > 0 ) &&
           ( ( currentPacketChunkIndex + 1 ) < pRtcpPacket->payloadLength ) ) /* +1 because we read 2 bytes at a time. */
    {
        packetChunk = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentPacketChunkIndex ] ) );
        currentPacketChunkIndex += 2;

        if( RTCP_TWCC_PACKET_CHUNK_EXTRACT_TYPE( packetChunk ) == RTCP_TWCC_PACKET_CHUNK_TYPE_RUN_LENGTH )
        {
            statusSymbol = RTCP_TWCC_PACKET_CHUNK_EXTRACT_STATUS( packetChunk );
            numPacketsInRunLengthChunk = RTCP_TWCC_PACKET_CHUNK_EXTRACT_RUN_LENGTH( packetChunk );

            for( i = 0; i < numPacketsInRunLengthChunk; i++ )
            {
                switch( statusSymbol )
                {
                    case RTCP_TWCC_PACKET_STATUS_NOT_RECEIVED:
                    {
                        remoteArrivalTime = RTCP_TWCC_PACKET_LOST_TIME;
                    }
                    break;

                    case RTCP_TWCC_PACKET_STATUS_SMALL_DELTA:
                    {
                        if( currentReceiveDeltaIndex < pRtcpPacket->payloadLength )
                        {
                            recvDelta = ( uint16_t ) ( pRtcpPacket->pPayload[ currentReceiveDeltaIndex ] );
                            currentReceiveDeltaIndex += 1;

                            referenceTime += RTCP_TWCC_US_TO_HUNDRED_OF_NANOS( recvDelta * 250 ); /* Deltas are represented as multiples of 250us. */
                            remoteArrivalTime = referenceTime;
                        }
                        else
                        {
                            result = RTCP_RESULT_MALFORMED_PACKET;
                        }
                    }
                    break;

                    case RTCP_TWCC_PACKET_STATUS_LARGE_DELTA:
                    {
                        if( ( currentReceiveDeltaIndex + 1 ) < pRtcpPacket->payloadLength )
                        {
                            recvDelta = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentReceiveDeltaIndex ] ) );
                            currentReceiveDeltaIndex += 2;

                            referenceTime += RTCP_TWCC_US_TO_HUNDRED_OF_NANOS( recvDelta * 250 ); /* Deltas are represented as multiples of 250us. */
                            remoteArrivalTime = referenceTime;
                        }
                        else
                        {
                            result = RTCP_RESULT_MALFORMED_PACKET;
                        }
                    }
                    break;
                }

                if( pTwccPacket->pArrivalInfoList != NULL )
                {
                    if( numArrivalInfos < pTwccPacket->arrivalInfoListLength )
                    {
                        pTwccPacket->pArrivalInfoList[ numArrivalInfos ].seqNum = remoteSeqNum;
                        pTwccPacket->pArrivalInfoList[ numArrivalInfos ].remoteArrivalTime = remoteArrivalTime;
                        numArrivalInfos += 1;
                    }
                    else
                    {
                        result = RTCP_RESULT_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    numArrivalInfos += 1;
                }

                packetsRemaining -= 1;
                remoteSeqNum += 1;
            }
        }
        else
        {
            symbolSize = RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_SIZE( packetChunk );
            symbolCount = RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_COUNT( packetChunk );
            symbolCount = ( packetsRemaining < symbolCount ) ? packetsRemaining : symbolCount;
            symbolList = RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_LIST( packetChunk );

            for( i = 0; i < symbolCount; i++ )
            {
                statusSymbol = RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_FROM_LIST( symbolList, i, symbolSize );

                switch( statusSymbol )
                {
                    case RTCP_TWCC_PACKET_STATUS_NOT_RECEIVED:
                    {
                        remoteArrivalTime = RTCP_TWCC_PACKET_LOST_TIME;
                    }
                    break;

                    case RTCP_TWCC_PACKET_STATUS_SMALL_DELTA:
                    {
                        if( currentReceiveDeltaIndex < pRtcpPacket->payloadLength )
                        {
                            recvDelta = ( uint16_t ) ( pRtcpPacket->pPayload[ currentReceiveDeltaIndex ] );
                            currentReceiveDeltaIndex += 1;

                            referenceTime += RTCP_TWCC_US_TO_HUNDRED_OF_NANOS( recvDelta * 250 ); /* Deltas are represented as multiples of 250us. */
                            remoteArrivalTime = referenceTime;
                        }
                        else
                        {
                            result = RTCP_RESULT_MALFORMED_PACKET;
                        }
                    }
                    break;

                    case RTCP_TWCC_PACKET_STATUS_LARGE_DELTA:
                    {
                        if( ( currentReceiveDeltaIndex + 1 ) < pRtcpPacket->payloadLength )
                        {
                            recvDelta = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentReceiveDeltaIndex ] ) );
                            currentReceiveDeltaIndex += 2;

                            referenceTime += RTCP_TWCC_US_TO_HUNDRED_OF_NANOS( recvDelta * 250 ); /* Deltas are represented as multiples of 250us. */
                            remoteArrivalTime = referenceTime;
                        }
                        else
                        {
                            result = RTCP_RESULT_MALFORMED_PACKET;
                        }
                    }
                    break;
                }

                if( pTwccPacket->pArrivalInfoList != NULL )
                {
                    if( numArrivalInfos < pTwccPacket->arrivalInfoListLength )
                    {
                        pTwccPacket->pArrivalInfoList[ numArrivalInfos ].seqNum = remoteSeqNum;
                        pTwccPacket->pArrivalInfoList[ numArrivalInfos ].remoteArrivalTime = remoteArrivalTime;
                        numArrivalInfos += 1;
                    }
                    else
                    {
                        result = RTCP_RESULT_OUT_OF_MEMORY;
                    }
                }
                else
                {
                    numArrivalInfos += 1;
                }

                packetsRemaining -= 1;
                remoteSeqNum += 1;
            }
        }
    }

    pTwccPacket->arrivalInfoListLength = numArrivalInfos;

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_Init( RtcpContext_t * pCtx )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( pCtx == NULL )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        Rtcp_InitReadWriteFunctions( &( pCtx->readWriteFunctions ) );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_SerializeSenderReport( RtcpContext_t * pCtx,
                                         const RtcpSenderReport_t * pSenderReport,
                                         uint8_t * pBuffer,
                                         size_t * pBufferLength )
{
    uint32_t firstWord;
    size_t i, serializedReportLength = 0, currentIndex = 0;
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pCtx == NULL ) ||
        ( pSenderReport == NULL ) ||
        ( pBuffer == NULL ) ||
        ( pBufferLength == NULL ) ||
        ( pSenderReport->numReceptionReports > RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        serializedReportLength = RTCP_HEADER_LENGTH +
                                 RTCP_SENDER_SSRC_LENGTH +
                                 RTCP_SENDER_INFO_LENGTH +
                                 pSenderReport->numReceptionReports * RTCP_RECEPTION_REPORT_LENGTH;

        if( *pBufferLength < serializedReportLength )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        firstWord = RTCP_HEADER_VERSION << RTCP_HEADER_VERSION_LOCATION;
        firstWord |= ( pSenderReport->numReceptionReports << RTCP_HEADER_RC_LOCATION );
        firstWord |= ( RTCP_PACKET_TYPE_SENDER_REPORT << RTCP_HEADER_PACKET_TYPE_LOCATION );
        firstWord |= ( ( RTCP_BYTES_TO_WORDS( serializedReportLength ) - 1U ) << RTCP_HEADER_PACKET_LENGTH_LOCATION );

        /* Write RTCP Packet header. */
        RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                           firstWord );
        currentIndex += 4;

        /* Write sender SSRC. */
        RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                           pSenderReport->senderSsrc );
        currentIndex += 4;

        /* Write sender info. */
        WriteSenderInfo( pCtx,
                         &( pSenderReport->senderInfo ),
                         pBuffer,
                         currentIndex );
        currentIndex += RTCP_SENDER_INFO_LENGTH;

        /* Write reception reports. */
        for( i = 0; i < pSenderReport->numReceptionReports; i++ )
        {
            WriteReceptionReport( pCtx,
                                  &( pSenderReport->pReceptionReports[ i ]  ),
                                  pBuffer,
                                  currentIndex );
            currentIndex += RTCP_RECEPTION_REPORT_LENGTH;
        }

        /* Update the output parameter to return the serialized report length.
         */
        *pBufferLength = serializedReportLength;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_SerializeReceiverReport( RtcpContext_t * pCtx,
                                           const RtcpReceiverReport_t * pReceiverReport,
                                           uint8_t * pBuffer,
                                           size_t * pBufferLength )
{
    uint32_t firstWord;
    size_t i, serializedReportLength = 0, currentIndex = 0;
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pCtx == NULL ) ||
        ( pReceiverReport == NULL ) ||
        ( pBuffer == NULL ) ||
        ( pBufferLength == NULL ) ||
        ( pReceiverReport->numReceptionReports > RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        serializedReportLength = RTCP_HEADER_LENGTH +
                                 RTCP_SENDER_SSRC_LENGTH +
                                 pReceiverReport->numReceptionReports * RTCP_RECEPTION_REPORT_LENGTH;

        if( *pBufferLength < serializedReportLength )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        firstWord = RTCP_HEADER_VERSION << RTCP_HEADER_VERSION_LOCATION;
        firstWord |= ( pReceiverReport->numReceptionReports << RTCP_HEADER_RC_LOCATION );
        firstWord |= ( RTCP_PACKET_TYPE_RECEIVER_REPORT << RTCP_HEADER_PACKET_TYPE_LOCATION );
        firstWord |= ( ( RTCP_BYTES_TO_WORDS( serializedReportLength ) - 1U ) << RTCP_HEADER_PACKET_LENGTH_LOCATION );

        /* Write RTCP Packet header. */
        RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                           firstWord );
        currentIndex += 4;

        /* Write sender SSRC. */
        RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                           pReceiverReport->senderSsrc );
        currentIndex += 4;

        /* Write reception reports. */
        for( i = 0; i < pReceiverReport->numReceptionReports; i++ )
        {
            WriteReceptionReport( pCtx,
                                  &( pReceiverReport->pReceptionReports[ i ]  ),
                                  pBuffer,
                                  currentIndex );
            currentIndex += RTCP_RECEPTION_REPORT_LENGTH;
        }

        /* Update the output parameter to return the serialized report length.
         */
        *pBufferLength = serializedReportLength;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_DeSerializePacket( RtcpContext_t * pCtx,
                                     const uint8_t * pSerializedPacket,
                                     size_t serializedPacketLength,
                                     RtcpPacket_t * pRtcpPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    uint32_t firstWord;
    size_t packetLengthInWords;
    uint8_t packetType, fmt;

    if( ( pCtx == NULL ) ||
        ( pSerializedPacket == NULL ) ||
        ( pRtcpPacket == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( serializedPacketLength < RTCP_HEADER_LENGTH )
    {
        result = RTCP_RESULT_INPUT_PACKET_TOO_SMALL;
    }

    if( result == RTCP_RESULT_OK )
    {
        firstWord = RTCP_READ_UINT32( &( pSerializedPacket[ 0 ] ) );

        if( ( ( firstWord & RTCP_HEADER_VERSION_BITMASK ) >>
              RTCP_HEADER_VERSION_LOCATION ) != RTCP_HEADER_VERSION )
        {
            result = RTCP_RESULT_WRONG_VERSION;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        packetLengthInWords = ( size_t ) ( ( firstWord & RTCP_HEADER_PACKET_LENGTH_BITMASK ) >>
                                           RTCP_HEADER_PACKET_LENGTH_LOCATION );

        if( serializedPacketLength < RTCP_WORDS_TO_BYTES( packetLengthInWords + 1 ) )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pRtcpPacket->header.padding = ( firstWord & RTCP_HEADER_PADDING_BITMASK ) >>
                                      RTCP_HEADER_PADDING_LOCATION;

         /* RC field is FMT in some Application Feedback and Transport Feedback
          * messages. */
        fmt = ( firstWord & RTCP_HEADER_RC_BITMASK ) >>
              RTCP_HEADER_RC_LOCATION;
        packetType = ( firstWord & RTCP_HEADER_PACKET_TYPE_BITMASK ) >>
                     RTCP_HEADER_PACKET_TYPE_LOCATION;

        pRtcpPacket->header.packetType = GetRtcpPacketType( packetType, fmt );
        pRtcpPacket->header.receptionReportCount = fmt;

        pRtcpPacket->pPayload = &( pSerializedPacket[ RTCP_HEADER_LENGTH ] );
        pRtcpPacket->payloadLength = RTCP_WORDS_TO_BYTES( packetLengthInWords );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseFirPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpFirPacket_t * pFirPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pFirPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_FIR_PACKET_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_FIR ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pFirPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ RTCP_FIR_PACKET_SENDER_SSRC_OFFSET ] ) );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParsePliPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpPliPacket_t * pPliPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pPliPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_PLI_PACKET_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_PAYLOAD_FEEDBACK_PLI ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pPliPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ RTCP_PLI_PACKET_SENDER_SSRC_OFFSET ] ) );
        pPliPacket->mediaSourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ RTCP_PLI_PACKET_MEDIA_SSRC_OFFSET ] ) );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseSliPacket( RtcpContext_t * pCtx,
                                  const RtcpPacket_t * pRtcpPacket,
                                  RtcpSliPacket_t * pSliPacket )
{
    size_t i, currentIndex = 0, numSliInfos = 0;
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pSliPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_SLI_PACKET_MIN_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_PAYLOAD_FEEDBACK_SLI ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pSliPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        pSliPacket->mediaSourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        if( pSliPacket->pSliInfos != NULL )
        {
            for( i = 0; ( i < pSliPacket->numSliInfos ) && ( currentIndex < pRtcpPacket->payloadLength ); i++ )
            {
                pSliPacket->pSliInfos[ i ] = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
                currentIndex += 4;
                numSliInfos += 1;
            }
            pSliPacket->numSliInfos = numSliInfos;
        }
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseRembPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpRembPacket_t * pRembPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    const uint8_t rembUniqueIdentifier[] = { 0x52, 0x45, 0x4d, 0x42 };
    size_t i, currentIndex = 0, numSsrc = 0;
    uint32_t word;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pRembPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_PAYLOAD_FEEDBACK_REMB ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if ( ( result == RTCP_RESULT_OK ) &&
         ( pRtcpPacket->payloadLength < RTCP_REMB_PACKET_MIN_PAYLOAD_LENGTH ) )
    {
        result = RTCP_RESULT_INPUT_REMB_INVALID;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( memcmp( &( rembUniqueIdentifier[ 0 ] ),
                    &( pRtcpPacket->pPayload[ RTCP_REMB_PACKET_IDENTIFIER_OFFSET ] ),
                    RTCP_REMB_PACKET_IDENTIFIER_LENGTH ) != 0 )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        word = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ RTCP_REMB_PACKET_NUM_SSRC_OFFSET ] ) );

        numSsrc = ( word & RTCP_REMB_PACKET_NUM_SSRC_BITMASK ) >>
                  RTCP_REMB_PACKET_NUM_SSRC_LOCATION;

        if( pRembPacket->ssrcListLength < numSsrc )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pRembPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        pRembPacket->mediaSourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        /* Skip REMB identifier as we have already checked it. */
        currentIndex += 4;

        /* Skip the word containing Num SSRC, BR Exp and BR Mantissa as we have
         * already read it.*/
        currentIndex += 4;

        pRembPacket->bitRateExponent = ( word & RTCP_REMB_PACKET_BR_EXPONENT_BITMASK ) >>
                                       RTCP_REMB_PACKET_BR_EXPONENT_LOCATION;
        pRembPacket->bitRateMantissa = ( word & RTCP_REMB_PACKET_BR_MANTISSA_BITMASK ) >>
                                       RTCP_REMB_PACKET_BR_MANTISSA_LOCATION;

        for( i = 0; i < numSsrc; i++ )
        {
            pRembPacket->pSsrcList[ i ] = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
            currentIndex += 4;
        }
        pRembPacket->ssrcListLength = numSsrc;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseSenderReport( RtcpContext_t * pCtx,
                                     const RtcpPacket_t * pRtcpPacket,
                                     RtcpSenderReport_t * pSenderReport )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t i, currentIndex = 0, expectedPayloadLength = 0;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pSenderReport == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_SENDER_REPORT_MIN_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_SENDER_REPORT ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( pSenderReport->numReceptionReports < ( size_t ) pRtcpPacket->header.receptionReportCount )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        expectedPayloadLength = RTCP_SENDER_SSRC_LENGTH +
                                RTCP_SENDER_INFO_LENGTH +
                                pRtcpPacket->header.receptionReportCount * RTCP_RECEPTION_REPORT_LENGTH;

        if( pRtcpPacket->payloadLength < expectedPayloadLength )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pSenderReport->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        ReadSenderInfo( pCtx,
                        pRtcpPacket,
                        currentIndex,
                        &( pSenderReport->senderInfo ) );
        currentIndex += RTCP_SENDER_INFO_LENGTH;

        for( i = 0; i < ( size_t ) pRtcpPacket->header.receptionReportCount; i++ )
        {
            ReadReceptionReport( pCtx,
                                 pRtcpPacket,
                                 currentIndex,
                                 &( pSenderReport->pReceptionReports[ i ] ) );
            currentIndex += RTCP_RECEPTION_REPORT_LENGTH;
        }
        pSenderReport->numReceptionReports = ( size_t ) pRtcpPacket->header.receptionReportCount;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseReceiverReport( RtcpContext_t * pCtx,
                                       const RtcpPacket_t * pRtcpPacket,
                                       RtcpReceiverReport_t * pReceiverReport )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t i, currentIndex = 0, expectedPayloadLength = 0;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pReceiverReport == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_RECEIVER_REPORT_MIN_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_RECEIVER_REPORT ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( pReceiverReport->numReceptionReports < ( size_t ) pRtcpPacket->header.receptionReportCount )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        expectedPayloadLength = RTCP_SENDER_SSRC_LENGTH +
                                pRtcpPacket->header.receptionReportCount * RTCP_RECEPTION_REPORT_LENGTH;

        if( pRtcpPacket->payloadLength < expectedPayloadLength )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pReceiverReport->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        for( i = 0; i < ( size_t ) pRtcpPacket->header.receptionReportCount; i++ )
        {
            ReadReceptionReport( pCtx,
                                 pRtcpPacket,
                                 currentIndex,
                                 &( pReceiverReport->pReceptionReports[ i ] ) );
            currentIndex += RTCP_RECEPTION_REPORT_LENGTH;
        }
        pReceiverReport->numReceptionReports = ( size_t ) pRtcpPacket->header.receptionReportCount;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseNackPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpNackPacket_t * pNackPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentIndex = 0;
    uint16_t i, startingSeqNum, bitmask, seqNumCount = 0;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pNackPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_NACK_PACKET_MIN_PAYLOAD_LENGTH ) ||
        ( ( pRtcpPacket->payloadLength % 4 ) != 0  ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_TRANSPORT_FEEDBACK_NACK ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pNackPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        pNackPacket->mediaSourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        while( ( currentIndex < pRtcpPacket->payloadLength ) &&
               ( result == RTCP_RESULT_OK ) )
        {
            startingSeqNum = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentIndex ] ) );
            currentIndex += 2;

            bitmask = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentIndex ] ) );
            currentIndex += 2;

            if( pNackPacket->pSeqNumList != NULL )
            {
                if( seqNumCount <= pNackPacket->seqNumListLength )
                {
                    pNackPacket->pSeqNumList[ seqNumCount ] = startingSeqNum;
                }
                else
                {
                    result = RTCP_RESULT_OUT_OF_MEMORY;
                    break;
                }
            }
            seqNumCount += 1;

            /* Iterate over 16 bits of bitmask. */
            for( i = 0; i < 16; i++ )
            {
                if( ( bitmask & ( 1 << i ) ) != 0 )
                {
                    if( pNackPacket->pSeqNumList != NULL )
                    {
                        if( seqNumCount <= pNackPacket->seqNumListLength )
                        {
                            pNackPacket->pSeqNumList[ seqNumCount ] = startingSeqNum + i + 1;
                        }
                        else
                        {
                            result = RTCP_RESULT_OUT_OF_MEMORY;
                            break;
                        }
                    }
                    seqNumCount += 1;
                }
            }
        }
        pNackPacket->seqNumListLength = seqNumCount;
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseTwccPacket( RtcpContext_t * pCtx,
                                   const RtcpPacket_t * pRtcpPacket,
                                   RtcpTwccPacket_t * pTwccPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    uint32_t word;
    uint16_t packetsToParse = 0, packetChunk, symbolCount;
    size_t currentIndex = 0, packetChunkStartIndex, receiveDeltaStartIndex;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pTwccPacket == NULL ) ||
        ( pRtcpPacket->pPayload == NULL ) ||
        ( pRtcpPacket->payloadLength < RTCP_TWCC_PACKET_MIN_PAYLOAD_LENGTH ) ||
        ( pRtcpPacket->header.packetType != RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pTwccPacket->senderSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        pTwccPacket->mediaSourceSsrc = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;

        pTwccPacket->baseSeqNum = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 2;

        pTwccPacket->packetStatusCount = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 2;

        word = RTCP_READ_UINT32( &( pRtcpPacket->pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pTwccPacket->referenceTime = ( word & RTCP_TWCC_REFERENCE_TIME_BITMASK ) >>
                                     RTCP_TWCC_REFERENCE_TIME_LOCATION;
        pTwccPacket->feedbackPacketCount = ( word & RTCP_TWCC_FEEDBACK_PACKET_COUNT_BITMASK ) >>
                                           RTCP_TWCC_FEEDBACK_PACKET_COUNT_LOCATION;

        packetChunkStartIndex = currentIndex;
        packetsToParse = pTwccPacket->packetStatusCount;

        while( ( packetsToParse > 0 ) &&
               ( ( currentIndex + 1 ) < pRtcpPacket->payloadLength ) ) /* +1 because we read 2 bytes at a time. */
        {
            packetChunk = RTCP_READ_UINT16( &( pRtcpPacket->pPayload[ currentIndex ] ) );
            currentIndex += 2;

            if( RTCP_TWCC_PACKET_CHUNK_EXTRACT_TYPE( packetChunk ) == RTCP_TWCC_PACKET_CHUNK_TYPE_RUN_LENGTH )
            {
                packetsToParse -= RTCP_TWCC_PACKET_CHUNK_EXTRACT_RUN_LENGTH( packetChunk );
            }
            else
            {
                symbolCount = RTCP_TWCC_PACKET_CHUNK_EXTRACT_SYMBOL_COUNT( packetChunk );
                packetsToParse -= ( packetsToParse < symbolCount ) ? packetsToParse : symbolCount;
            }
        }

        receiveDeltaStartIndex = currentIndex;

        result = ParseTwccPacketChunks( pCtx,
                                        pRtcpPacket,
                                        packetChunkStartIndex,
                                        receiveDeltaStartIndex,
                                        pTwccPacket );
    }

    return result;
}

/*-----------------------------------------------------------*/

RtcpPacketType_t GetRtcpPacketType( uint8_t packetType,
                                    uint8_t fmt )
{
    RtcpPacketType_t ret = RTCP_PACKET_UNKNOWN;

    switch( packetType )
    {
        case RTCP_PACKET_TYPE_FIR:
        {
            if( fmt == 0 )
            {
                ret = RTCP_PACKET_FIR;
            }
        }
        break;

        case RTCP_PACKET_TYPE_SENDER_REPORT:
        {
            ret = RTCP_PACKET_SENDER_REPORT;
        }
        break;

        case RTCP_PACKET_TYPE_RECEIVER_REPORT:
        {
            ret = RTCP_PACKET_RECEIVER_REPORT;
        }
        break;

        case RTCP_PACKET_TYPE_TRANSPORT_SPECIFIC_FEEDBACK:
        {
            if( fmt == RTCP_FMT_TRANSPORT_SPECIFIC_FEEDBACK_NACK )
            {
                ret = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;
            }
            else if( fmt == RTCP_FMT_TRANSPORT_SPECIFIC_FEEDBACK_TWCC )
            {
                ret = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;
            }
        }
        break;

        case RTCP_PACKET_TYPE_PAYLOAD_SPECIFIC_FEEDBACK:
        {
            if( fmt == RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_PLI )
            {
                ret = RTCP_PACKET_PAYLOAD_FEEDBACK_PLI;
            }
            else if( fmt == RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_SLI )
            {
                ret = RTCP_PACKET_PAYLOAD_FEEDBACK_SLI;
            }
            else if( fmt == RTCP_FMT_PAYLOAD_SPECIFIC_FEEDBACK_REMB )
            {
                ret = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;
            }
        }
        break;
    }

    return ret;
}

/*-----------------------------------------------------------*/