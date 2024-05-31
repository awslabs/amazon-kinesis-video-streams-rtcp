/* Standard includes. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RTP includes. */
#include "rtcp_api.h"

static void deserialize_test1( void )
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;

    memset( &rtcpPacket,
            0x00,
            sizeof( RtcpPacket_t ) );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    // Assert that we don't parse buffers that aren't even large enough
    uint8_t headerTooSmall[] = {0x00, 0x00, 0x00};
    result = Rtcp_DeSerialize( &ctx,
                               headerTooSmall,
                               sizeof( headerTooSmall ),
                               &rtcpPacket );
    assert( RTCP_RESULT_BAD_PARAM == result );

    // Assert that we check version field
    uint8_t invalidVersionValue[] = {0x01, 0xcd, 0x00, 0x03, 0x2c, 0xd1, 0xa0, 0xde, 0x00, 0x00, 0xab, 0xe0, 0x00, 0xa4, 0x00, 0x00};
    result = Rtcp_DeSerialize( &ctx,
                               invalidVersionValue,
                               sizeof( invalidVersionValue ),
                               &rtcpPacket );
    assert( RTCP_RESULT_WRONG_VERSION == result );

    // Assert that we check the length field
    uint8_t invalidLengthValue[] = {0x81, 0xcd, 0x00, 0x00, 0x2c, 0xd1, 0xa0, 0xde, 0x00, 0x00, 0xab, 0xe0, 0x00, 0xa4, 0x00, 0x00};
    result = Rtcp_DeSerialize( &ctx,
                               invalidLengthValue,
                               sizeof( invalidLengthValue ),
                               &rtcpPacket );
    assert( RTCP_RESULT_OK == result );

    uint8_t validRtcpPacket[] = {0x81, 0xcd, 0x00, 0x03, 0x2c, 0xd1, 0xa0, 0xde, 0x00, 0x00, 0xab, 0xe0, 0x00, 0xa4, 0x00, 0x00};
    result = Rtcp_DeSerialize( &ctx,
                               validRtcpPacket,
                               sizeof( validRtcpPacket ),
                               &rtcpPacket );
    assert( RTCP_RESULT_OK == result );

}

/*-----------------------------------------------------------*/

static void deserialize_test2( void )
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;

    memset( &rtcpPacket,
            0x00,
            sizeof( RtcpPacket_t ) );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    // Compound RTCP Packet that contains SR, SDES and REMB
    uint8_t compoundPacket[] = { 0x80, 0xc8, 0x00, 0x06, 0xf1, 0x2d, 0x7b, 0x4b, 0xe1, 0xe3, 0x20, 0x43, 0xe5, 0x3d, 0x10, 0x2b, 0xbf,
                                 0x58, 0xf7, 0xef, 0x00, 0x00, 0x23, 0xf3, 0x00, 0x6c, 0xd3, 0x75,
                                 0x81, 0xca, 0x00, 0x06, 0xf1, 0x2d, 0x7b, 0x4b, 0x01, 0x10, 0x2f, 0x76, 0x6d, 0x4b, 0x51, 0x6e, 0x47,
                                 0x6e, 0x55, 0x70, 0x4f, 0x2b, 0x70, 0x38, 0x64, 0x52, 0x00, 0x00,
                                 0x8f, 0xce, 0x00, 0x06, 0xf1, 0x2d, 0x7b, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x52, 0x45, 0x4d, 0x42, 0x02,
                                 0x12, 0x2d, 0x97, 0x0c, 0xef, 0x37, 0x0d, 0x2d, 0x07, 0x3d, 0x1d };

    int currentOffset = 0;
    result = Rtcp_DeSerialize( &ctx,
                               compoundPacket + currentOffset,
                               sizeof( compoundPacket ) - currentOffset,
                               &rtcpPacket );
    assert( RTCP_RESULT_OK == result );
    assert( rtcpPacket.header.packetType == RTCP_PACKET_TYPE_SENDER_REPORT );

    currentOffset += ( rtcpPacket.payloadLength + RTCP_HEADER_LENGTH );
    result = Rtcp_DeSerialize( &ctx,
                               compoundPacket + currentOffset,
                               sizeof( compoundPacket ) - currentOffset,
                               &rtcpPacket );
    assert( RTCP_RESULT_OK == result );
    assert( rtcpPacket.header.packetType == RTCP_PACKET_TYPE_SOURCE_DESCRIPTION );

    currentOffset += ( rtcpPacket.payloadLength + RTCP_HEADER_LENGTH );
    result = Rtcp_DeSerialize( &ctx,
                               compoundPacket + currentOffset,
                               sizeof( compoundPacket ) - currentOffset,
                               &rtcpPacket );
    assert( RTCP_RESULT_OK == result );
    assert( rtcpPacket.header.packetType == RTCP_PACKET_TYPE_PAYLOAD_SPECIFIC_FEEDBACK );
    currentOffset += ( rtcpPacket.payloadLength + RTCP_HEADER_LENGTH );
    assert( currentOffset == sizeof( compoundPacket ) );
}

void serialize_senderReport()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpSenderReport_t senderReport;
    size_t paylaodLength = sizeof( RtcpSenderReport_t ), bufferLen = paylaodLength + RTCP_HEADER_LENGTH;
    uint8_t * pBuffer;
    uint8_t expectedBuff[] = { 0x80, 0xc8, 0x00, 0x06, 0x2c, 0x38, 0xaf, 0xd2, 0xe9, 0xf8, 0x11, 0x68, 0x33, 0x33, 0xe8,
                               0x64, 0x00, 0x03, 0x77, 0xca, 0x00, 0x00, 0x01, 0x4c, 0x00, 0x01, 0x0b, 0x2f };

    memset( &rtcpPacket,
            0x00,
            sizeof( RtcpPacket_t ) );
    memset( &senderReport,
            0x00,
            sizeof( RtcpSenderReport_t ) );

    rtcpPacket.pPayload = malloc( paylaodLength );
    pBuffer = malloc( bufferLen );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    rtcpPacket.header.packetLength = 28;
    rtcpPacket.header.packetType = RTCP_PACKET_TYPE_SENDER_REPORT;
    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;

    senderReport.ssrc = 0x2c38afd2;
    senderReport.ntpTime = 0xe9f811683333e864;
    senderReport.rtpTime = 0x377ca;
    senderReport.octetCount = 0x10b2f;
    senderReport.packetCount = 0x14c;

    result = Rtcp_CreatePayloadSenderReport( &ctx,
                                             &rtcpPacket,
                                             paylaodLength,
                                             &senderReport );
    assert( RTCP_RESULT_OK == result );

    result = Rtcp_Serialize( &ctx,
                             &rtcpPacket,
                             pBuffer,
                             &bufferLen );

    for( int i = 0; i < bufferLen; i++ )
    {
        assert( pBuffer[i] == expectedBuff[i] );
    }

}

/*-----------------------------------------------------------*/

int main( void )
{
    deserialize_test1();
    deserialize_test2();

    printf( "\nAll deserialize test PASS.\r\n" );

    serialize_senderReport();

    printf( "\nAll serialize test PASS.\r\n" );

    return 0;
}

/*-----------------------------------------------------------*/