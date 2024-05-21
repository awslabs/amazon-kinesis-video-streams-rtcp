/* Standard includes. */
#include <string.h>

/* API includes. */
#include "rtcp_data_types.h"

/* Read, Write macros. */
#define RTCP_WRITE_UINT16   ( pCtx->readWriteFunctions.writeUint16Fn )
#define RTCP_READ_UINT16    ( pCtx->readWriteFunctions.readUint16Fn )

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
    uint16_t packetLen, total_length_bytes;
    RtcpResult_t result = RTCP_RESULT_OK;
    uint8_t byte;

    if( ( pCtx == NULL ) ||
        ( pSerializedPacket == NULL ) ||
        ( serializedPacketLength < RTCP_HEADER_LENGTH ) ||
        ( pRtcpPacket == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        byte = pSerializedPacket[ currentIndex ];
        
        if( ( ( byte & RTCP_HEADER_VERSION_MASK ) >>
                RTCP_HEADER_VERSION_LOCATION ) != RTCP_HEADER_VERSION )
        {
            result = RTCP_RESULT_WRONG_VERSION;
        }
        else
        {
            pRtcpPacket->header.receptionReportCount = byte & RTCP_PACKET_RRC_BITMASK;
        }
        currentIndex+=1;
    }

    if( result == RTCP_RESULT_OK )
    {
        pRtcpPacket->header.packetType = pSerializedPacket[currentIndex];
        currentIndex+=1;
 
        packetLen = RTCP_READ_UINT16( &( pSerializedPacket[ currentIndex ] ) );
        pRtcpPacket->header.packetLength = packetLen;

        /* The length of this RTCP packet in 32-bit words minus one. */
        total_length_bytes = ( packetLen + 1) * RTCP_PACKET_LEN_WORD_SIZE;
        if( serializedPacketLength < total_length_bytes )
        {
            result = RTCP_RESULT_MALFORMED_PACKET;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pRtcpPacket->payloadLength = total_length_bytes - RTCP_HEADER_LENGTH;
        pRtcpPacket->pPayload = pSerializedPacket + RTCP_PACKET_LEN_WORD_SIZE;
    }

    return result;
}