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

#define RTCP_PACKET_LEN_WORD_SIZE               4

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