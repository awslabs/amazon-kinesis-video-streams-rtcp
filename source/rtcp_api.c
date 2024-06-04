/* Standard includes. */
#include <string.h>

/* API includes. */
#include "rtcp_data_types.h"

/* Read, Write macros. */
#define RTCP_WRITE_UINT16   ( pCtx->readWriteFunctions.writeUint16Fn )
#define RTCP_WRITE_UINT32   ( pCtx->readWriteFunctions.writeUint32Fn )
#define RTCP_WRITE_UINT64   ( pCtx->readWriteFunctions.writeUint64Fn )
#define RTCP_READ_UINT16    ( pCtx->readWriteFunctions.readUint16Fn )
#define RTCP_READ_UINT32    ( pCtx->readWriteFunctions.readUint32Fn )
#define RTCP_READ_UINT64    ( pCtx->readWriteFunctions.readUint64Fn )

#define RTCP_HEADER_VERSION_MASK                0xC0000000
#define RTCP_HEADER_VERSION_LOCATION            30
#define RTCP_HEADER_PADDING_MASK                0x20000000
#define RTCP_HEADER_PADDING_LOCATION            29
#define RTCP_PACKET_RRC_BITMASK                 0x1F000000
#define RTCP_PACKET_RRC_BITMASK_LOCATION        24
#define RTCP_PACKET_TYPE_BITMASK                0x00FF0000
#define RTCP_PACKET_TYPE_BITMASK_LOCATION       16
#define RTCP_PACKET_LENGTH_BITMASK              0x0000FFFF

#define RTCP_REMB_PACKET_SSRC_LEN_LOCATION      24
#define RTCP_REMB_PACKET_SSRC_LEN_BITMASK       0xFF000000
#define RTCP_REMB_PACKET_EXPONENT_LOCATION      18
#define RTCP_REMB_PACKET_EXPONENT_BITMASK       0x00FC0000
#define RTCP_REMB_PACKET_MANTISSA_BITMASK       0x0003FFFF

#define RTCP_FRACTION_LOST_LOCATION             24
#define RTCP_FRACTION_LOST_BITMASK              0xFF000000
#define RTCP_REMB_PACKET_LOST_BITMASK           0x00FFFFFF

#define RTCP_PACKET_LEN_WORD_SIZE               4

/*-----------------------------------------------------------*/

static RtcpResult_t Rtcp_GetMediaSSRC( RtcpContext_t * pCtx,
                                       uint8_t * pPayload,
                                       size_t paylaodLength,
                                       uint32_t * pMediaSSRC )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentIndex = 0;

    if( ( pCtx == NULL ) ||
        ( pPayload == NULL ) ||
        ( paylaodLength == 0 ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        RTCP_WRITE_UINT32( &( pPayload[ currentIndex + sizeof( uint32_t ) ] ),
                           *pMediaSSRC );
    }

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

RtcpResult_t Rtcp_DeSerialize( RtcpContext_t * pCtx,
                               uint8_t * pSerializedPacket,
                               uint16_t serializedPacketLength,
                               RtcpPacket_t * pRtcpPacket )
{
    size_t currentIndex = 0;
    uint16_t packetLen, total_length;
    RtcpResult_t result = RTCP_RESULT_OK;
    uint32_t firstWord;

    if( ( pCtx == NULL ) ||
        ( pSerializedPacket == NULL ) ||
        ( serializedPacketLength < RTCP_HEADER_LENGTH ) ||
        ( pRtcpPacket == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        firstWord = RTCP_READ_UINT32( &( pSerializedPacket[ currentIndex ] ) );

        if( ( ( firstWord & RTCP_HEADER_VERSION_MASK ) >>
              RTCP_HEADER_VERSION_LOCATION ) != RTCP_HEADER_VERSION )
        {
            result = RTCP_RESULT_WRONG_VERSION;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pRtcpPacket->header.padding = ( firstWord & RTCP_HEADER_PADDING_MASK ) >> RTCP_HEADER_PADDING_LOCATION;
        pRtcpPacket->header.receptionReportCount = ( firstWord & RTCP_PACKET_RRC_BITMASK ) >> RTCP_PACKET_RRC_BITMASK_LOCATION;

        pRtcpPacket->header.packetType = ( firstWord & RTCP_PACKET_TYPE_BITMASK ) >> RTCP_PACKET_TYPE_BITMASK_LOCATION;

        packetLen = firstWord & RTCP_PACKET_LENGTH_BITMASK;
        pRtcpPacket->header.packetLength = packetLen;

        /* The length of this RTCP packet in 32-bit words minus one. */
        total_length = ( packetLen + 1 ) * RTCP_PACKET_LEN_WORD_SIZE;
        if( serializedPacketLength < total_length )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pRtcpPacket->payloadLength = total_length - RTCP_HEADER_LENGTH;
        pRtcpPacket->pPayload = pSerializedPacket + RTCP_PACKET_LEN_WORD_SIZE;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_Serialize( RtcpContext_t * pCtx,
                             const RtcpPacket_t * pRtcpPacket,
                             uint8_t * pBuffer,
                             size_t * pLength )
{
    size_t currentIndex = 0;
    RtcpResult_t result = RTCP_RESULT_OK;
    uint16_t packetLen;
    uint32_t firstWord = 0;

    if( ( pCtx == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( pLength == NULL ) ||
        ( ( pLength != NULL ) &&
          ( *pLength < RTCP_HEADER_LENGTH ) ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        firstWord = RTCP_HEADER_VERSION << RTCP_HEADER_VERSION_LOCATION;
        firstWord |= pRtcpPacket->header.padding << RTCP_HEADER_PADDING_LOCATION;
        firstWord |= pRtcpPacket->header.packetType << RTCP_PACKET_TYPE_BITMASK_LOCATION;

        packetLen = ( pRtcpPacket->header.packetLength / RTCP_PACKET_LEN_WORD_SIZE ) - 1;
        firstWord |= packetLen;

        RTCP_WRITE_UINT32( &( pBuffer[ currentIndex ] ),
                           firstWord );
        currentIndex += 4;

        /* Fill payload */
        if( ( pRtcpPacket->pPayload != NULL ) &&
            ( pRtcpPacket->payloadLength > 0 ) )
        {
            memcpy( ( void * ) &( pBuffer[ currentIndex ] ),
                    ( const void * ) &( pRtcpPacket->pPayload[ 0 ] ),
                    pRtcpPacket->payloadLength );
            currentIndex += pRtcpPacket->payloadLength;
        }
        *pLength = currentIndex;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_CreatePayloadSenderReport( RtcpContext_t * pCtx,
                                             RtcpPacket_t * pRtcpPacket,
                                             size_t paylaodLength,
                                             const RtcpSenderReport_t * pRtcpSenderReport )
{
    size_t currentIndex = 0;
    RtcpResult_t result = RTCP_RESULT_OK;
    uint8_t * pPayload = pRtcpPacket->pPayload;

    if( ( pCtx == NULL ) ||
        ( pRtcpSenderReport == NULL ) ||
        ( pRtcpPacket == NULL ) ||
        ( ( pRtcpPacket != NULL ) &&
          ( pPayload == NULL ) ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( paylaodLength < sizeof( RtcpSenderReport_t ) )
        {
            result = RTCP_RESULT_OUT_OF_MEMORY;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        RTCP_WRITE_UINT32( &( pPayload[ currentIndex ] ),
                           pRtcpSenderReport->ssrc );
        currentIndex += 4;
        RTCP_WRITE_UINT64( &( pPayload[ currentIndex ] ),
                           pRtcpSenderReport->ntpTime );
        currentIndex += 8;
        RTCP_WRITE_UINT32( &( pPayload[ currentIndex ] ),
                           pRtcpSenderReport->rtpTime );
        currentIndex += 4;
        RTCP_WRITE_UINT32( &( pPayload[ currentIndex ] ),
                           pRtcpSenderReport->packetCount );
        currentIndex += 4;
        RTCP_WRITE_UINT32( &( pPayload[ currentIndex ] ),
                           pRtcpSenderReport->octetCount );
        currentIndex += 4;

        pRtcpPacket->payloadLength = currentIndex;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseFIRPacket( RtcpContext_t * pCtx,
                                  uint8_t * pPayload,
                                  size_t paylaodLength,
                                  uint32_t * pMediaSSRC )
{
    return Rtcp_GetMediaSSRC( pCtx,
                              pPayload,
                              paylaodLength,
                              pMediaSSRC );
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParsePLIPacket( RtcpContext_t * pCtx,
                                  uint8_t * pPayload,
                                  size_t paylaodLength,
                                  uint32_t * pMediaSSRC )
{
    return Rtcp_GetMediaSSRC( pCtx,
                              pPayload,
                              paylaodLength,
                              pMediaSSRC );
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseSLIPacket( RtcpContext_t * pCtx,
                                  uint8_t * pPayload,
                                  size_t paylaodLength,
                                  uint32_t * pMediaSSRC )
{
    return Rtcp_GetMediaSSRC( pCtx,
                              pPayload,
                              paylaodLength,
                              pMediaSSRC );
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseRembPacket( RtcpContext_t * pCtx,
                                   uint8_t * pPayload,
                                   size_t paylaodLength,
                                   size_t * pSsrcListLength,
                                   uint32_t ** ppSsrcList,
                                   uint64_t * pBitRate )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    const uint8_t rembUniqueIdentifier[] = { 0x52, 0x45, 0x4d, 0x42 };
    uint32_t rembIdentifierRead, rembIdentifier;
    uint32_t word = 0, mantissa = 0;
    uint8_t exponent = 0;
    size_t rembPayloadSize = RTCP_REMB_MIN_PAYLOAD_SIZE, i;

    if( ( pCtx == NULL ) ||
        ( pPayload == NULL ) ||
        ( pSsrcListLength == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }
    else if( paylaodLength < rembPayloadSize )
    {
        result = RTCP_RESULT_INPUT_REMB_INVALID;
    }

    if( result == RTCP_RESULT_OK )
    {
        rembIdentifier = RTCP_READ_UINT32( &( rembUniqueIdentifier[0] ) );
        rembIdentifierRead = RTCP_READ_UINT32( &( pPayload[ RTCP_REMB_IDENTIFIER_OFFSET ] ) );
        if( rembIdentifierRead != rembIdentifier )
        {
            result = RTCP_RESULT_INPUT_REMB_INVALID;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        word = RTCP_READ_UINT32( &( pPayload[ RTCP_REMB_IDENTIFIER_OFFSET + 4 ] ) );

        *pSsrcListLength = ( ( word & RTCP_REMB_PACKET_SSRC_LEN_BITMASK ) >> RTCP_REMB_PACKET_SSRC_LEN_LOCATION );
        rembPayloadSize += ( *pSsrcListLength ) * sizeof( uint32_t );

        exponent = ( ( word & RTCP_REMB_PACKET_EXPONENT_BITMASK ) >> RTCP_REMB_PACKET_EXPONENT_LOCATION );
        mantissa = ( word & RTCP_REMB_PACKET_MANTISSA_BITMASK );
        *pBitRate = mantissa << exponent;

        if( paylaodLength < rembPayloadSize )
        {
            result = RTCP_RESULT_INPUT_REMB_INVALID;
        }
    }

    if( ( result == RTCP_RESULT_OK ) &&
        ( *pSsrcListLength != 0 ) )
    {
        *ppSsrcList = ( uint32_t * ) &( pPayload[ RTCP_REMB_SSRC_LIST_OFFSET ] );
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseSenderReport( RtcpContext_t * pCtx,
                                     uint8_t * pPayload,
                                     size_t paylaodLength,
                                     RtcpSenderReport_t * pSenderReport )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentIndex = 0;

    if( ( pCtx == NULL ) ||
        ( pPayload == NULL ) ||
        ( pSenderReport == NULL ) ||
        ( paylaodLength < RTCP_SENDER_REPORT_MIN_LENGTH ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pSenderReport->ssrc = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pSenderReport->ntpTime = RTCP_READ_UINT64( &( pPayload[ currentIndex ] ) );
        currentIndex += 8;
        pSenderReport->rtpTime = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pSenderReport->packetCount = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pSenderReport->octetCount = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseReceiverReport( RtcpContext_t * pCtx,
                                       uint8_t * pPayload,
                                       size_t paylaodLength,
                                       RtcpReceiverReport_t * pReceiverReport )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentIndex = 0;
    uint32_t word;

    if( ( pCtx == NULL ) ||
        ( pPayload == NULL ) ||
        ( pReceiverReport == NULL ) ||
        ( paylaodLength < RTCP_RECEIVER_REPORT_MIN_LENGTH ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pReceiverReport->ssrcSender = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pReceiverReport->ssrcSource = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;

        word = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        pReceiverReport->fractionLost = ( ( word & RTCP_FRACTION_LOST_BITMASK ) >> RTCP_FRACTION_LOST_LOCATION );
        pReceiverReport->cumulativePacketsLost = word & RTCP_REMB_PACKET_LOST_BITMASK;
        currentIndex += 4;

        pReceiverReport->extHiSeqNumReceived = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pReceiverReport->interArrivalJitter = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pReceiverReport->lastSR = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pReceiverReport->delaySinceLastSR = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
    }

    return result;
}
/*-----------------------------------------------------------*/

RtcpResult_t Rtcp_ParseNackPacket( RtcpContext_t * pCtx,
                                   uint8_t * pPayload,
                                   size_t paylaodLength,
                                   RtcpNackPacket_t * pNackPacket )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t currentIndex = 0, j;
    uint16_t currentSeqNumber, BLP, seqNumberCount = 0;

    if( ( pCtx == NULL ) ||
        ( pPayload == NULL ) ||
        ( pNackPacket == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( ( paylaodLength < RTCP_NACK_REPORT_MIN_LENGTH ) ||
            ( paylaodLength % 4 != 0 ) )
        {
            result = RTCP_RESULT_INPUT_NACK_LIST_INVALID;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pNackPacket->ssrcSender = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;
        pNackPacket->ssrcSource = RTCP_READ_UINT32( &( pPayload[ currentIndex ] ) );
        currentIndex += 4;

        for(; currentIndex < paylaodLength; currentIndex += 4 )
        {
            currentSeqNumber = RTCP_READ_UINT16( &( pPayload[ currentIndex ] ) );
            BLP = RTCP_READ_UINT16( &( pPayload[ currentIndex + 2 ] ) );

            if( ( pNackPacket->pSeqNumList != NULL ) && ( seqNumberCount <= pNackPacket->seqNumListLength ) )
            {
                pNackPacket->pSeqNumList[ seqNumberCount ] = currentSeqNumber;
            }
            seqNumberCount += 1;

            for( j = 0; j < RTCP_BLP_BIT_COUNT; j++ )
            {
                if( ( BLP & ( 1 << j ) ) != 0 )
                {
                    if( ( pNackPacket->pSeqNumList != NULL ) && ( seqNumberCount <= pNackPacket->seqNumListLength ) )
                    {
                        pNackPacket->pSeqNumList[ seqNumberCount ] = currentSeqNumber + j + 1;
                    }
                    seqNumberCount += 1;
                }
            }
        }
        pNackPacket->seqNumListLength = seqNumberCount;
    }

    return result;
}
/*-----------------------------------------------------------*/