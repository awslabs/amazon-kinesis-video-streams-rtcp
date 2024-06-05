/* Standard includes. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RTP includes. */
#include "rtcp_api.h"

#define RTCP_READ_UINT32    ( ctx.readWriteFunctions.readUint32Fn )

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
/*-----------------------------------------------------------*/

void deserialize_rembValueGet()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    size_t ssrcListLen = 0;
    uint64_t maximumBitRate = 0;
    uint32_t * pSsrcList1, * pSsrcList2;
    uint8_t bufferNoUniqueIdentifier[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t singleSSRC[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x45, 0x4d, 0x42, 0x01, 0x12, 0x76, 0x28, 0x6c, 0x76, 0xe8, 0x55 };
    uint8_t multipleSSRC[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x45, 0x4d, 0x42,
                               0x02, 0x12, 0x76, 0x28, 0x6c, 0x76, 0xe8, 0x55, 0x42, 0x42, 0x42, 0x42 };
    uint8_t invalidSSRCLength[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x45,
                                    0x4d, 0x42, 0xFF, 0x12, 0x76, 0x28, 0x6c, 0x76, 0xe8, 0x55 };

    memset( &rtcpPacket,
            0x00,
            sizeof( RtcpPacket_t ) );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    result = Rtcp_ParseRembPacket( &ctx,
                                   bufferNoUniqueIdentifier,
                                   sizeof( bufferNoUniqueIdentifier ),
                                   &ssrcListLen,
                                   &pSsrcList1,
                                   &maximumBitRate );
    assert( result == RTCP_RESULT_INPUT_REMB_INVALID );

    result = Rtcp_ParseRembPacket( &ctx,
                                   singleSSRC,
                                   sizeof( singleSSRC ),
                                   &ssrcListLen,
                                   &pSsrcList1,
                                   &maximumBitRate );

    assert( RTCP_RESULT_OK == RTCP_RESULT_OK );
    assert( ssrcListLen == 1 );
    assert( maximumBitRate == 2581120 );
    assert( RTCP_READ_UINT32( ( uint8_t * )&( pSsrcList1[0] ) ) == 0x6c76e855 );

    result = Rtcp_ParseRembPacket( &ctx,
                                   multipleSSRC,
                                   sizeof( multipleSSRC ),
                                   &ssrcListLen,
                                   &pSsrcList2,
                                   &maximumBitRate );
    assert( RTCP_RESULT_OK == RTCP_RESULT_OK );
    assert( ssrcListLen == 2 );
    assert( maximumBitRate == 2581120 );
    assert( RTCP_READ_UINT32( ( uint8_t * )&( pSsrcList2[0] ) ) == 0x6c76e855 );
    assert( RTCP_READ_UINT32( ( uint8_t * )&( pSsrcList2[1] ) ) == 0x42424242 );

    result = Rtcp_ParseRembPacket( &ctx,
                                   invalidSSRCLength,
                                   sizeof( invalidSSRCLength ),
                                   &ssrcListLen,
                                   &pSsrcList2,
                                   &maximumBitRate );
    assert( RTCP_RESULT_INPUT_REMB_INVALID == result );
}
/*-----------------------------------------------------------*/

void deserialize_senderReport()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpSenderReport_t senderReport;
    uint8_t payload[] = { 0x2c, 0x38, 0xaf, 0xd2, 0xe9, 0xf8, 0x11, 0x68, 0x33, 0x33, 0xe8,
                          0x64, 0x00, 0x03, 0x77, 0xca, 0x00, 0x00, 0x01, 0x4c, 0x00, 0x01, 0x0b, 0x2f };
    size_t paylaodLength = sizeof( payload );

    memset( &senderReport,
            0x00,
            sizeof( RtcpSenderReport_t ) );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    result = Rtcp_ParseSenderReport( &ctx,
                                     &( payload[0] ),
                                     paylaodLength,
                                     &senderReport );
    assert( RTCP_RESULT_OK == result );

    assert( senderReport.ssrc == 0x2c38afd2 );
    assert( senderReport.ntpTime == 0xe9f811683333e864 );
    assert( senderReport.rtpTime == 0x377ca );
    assert( senderReport.octetCount == 0x10b2f );
    assert( senderReport.packetCount == 0x14c );
}
/*-----------------------------------------------------------*/

void deserialize_receiverReport()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpReceiverReport_t receiverReport;

    uint8_t payload[] = { 0x12, 0x34, 0x56, 0x78, 0x87, 0x65, 0x43, 0x21, 0x25, 0x00, 0x00, 0x01, // Fraction lost (25 in hex, approximately 10%)
                          0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05 };
    size_t paylaodLength = sizeof( payload );

    memset( &receiverReport,
            0x00,
            sizeof( RtcpReceiverReport_t ) );

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    result = Rtcp_ParseReceiverReport( &ctx,
                                       &( payload[0] ),
                                       paylaodLength,
                                       &receiverReport );
    assert( RTCP_RESULT_OK == result );

    assert( receiverReport.ssrcSender == 0x12345678 );
    assert( receiverReport.ssrcSource == 0x87654321 );
    assert( receiverReport.fractionLost == 0x25 );
    assert( receiverReport.cumulativePacketsLost == 1 );
    assert( receiverReport.extHiSeqNumReceived == 2 );
    assert( receiverReport.interArrivalJitter == 3 );
    assert( receiverReport.lastSR == 4 );
    assert( receiverReport.delaySinceLastSR == 5 );
}
/*-----------------------------------------------------------*/

void deserialize_nackPacket()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpNackPacket_t nackPacket;

    // Assert that NACK list meets the minimum length requirement
    uint8_t nackListTooSmall[] = {0x00, 0x00, 0x00};
    uint8_t nackListMalformed[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t nackListSsrcOnly[] = {0x2c, 0xd1, 0xa0, 0xde, 0x00, 0x00, 0xab, 0xe0};
    uint8_t singlePID[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xa8, 0x00, 0x00 };
    uint8_t compound[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xa8, 0x00, 0x00, 0x0c, 0xff, 0x00, 0x02};

    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    /* nackListTooSmall Packet parsing */
    memset( &nackPacket,
            0x00,
            sizeof( RtcpNackPacket_t ) );

    result = Rtcp_ParseNackPacket( &ctx,
                                   &( nackListTooSmall[0] ),
                                   sizeof( nackListTooSmall ),
                                   &nackPacket );
    assert( RTCP_RESULT_INPUT_NACK_LIST_INVALID == result );

    /* nackListMalformed Packet parsing */
    memset( &nackPacket,
            0x00,
            sizeof( RtcpNackPacket_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( nackListMalformed[0] ),
                                   sizeof( nackListMalformed ),
                                   &nackPacket );
    assert( RTCP_RESULT_INPUT_NACK_LIST_INVALID == result );

    /* nackListSsrcOnly Packet parsing */
    memset( &nackPacket,
            0x00,
            sizeof( RtcpNackPacket_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( nackListSsrcOnly[0] ),
                                   sizeof( nackListSsrcOnly ),
                                   &nackPacket );
    assert( RTCP_RESULT_OK == result );
    assert( nackPacket.ssrcSender == 0x2cd1a0de );
    assert( nackPacket.ssrcSource == 0x0000abe0 );

    /* singlePID Packet parsing */
    memset( &nackPacket,
            0x00,
            sizeof( RtcpNackPacket_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( singlePID[0] ),
                                   sizeof( singlePID ),
                                   &nackPacket );
    assert( RTCP_RESULT_OK == result );
    assert( nackPacket.seqNumListLength == 1 );

    nackPacket.pSeqNumList = malloc( nackPacket.seqNumListLength * sizeof( uint16_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( singlePID[0] ),
                                   sizeof( singlePID ),
                                   &nackPacket );
    assert( RTCP_RESULT_OK == result );
    assert( nackPacket.pSeqNumList[0] == 3240 );
    free( nackPacket.pSeqNumList );

    /* compound Packet parsing */
    memset( &nackPacket,
            0x00,
            sizeof( RtcpNackPacket_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( compound[0] ),
                                   sizeof( compound ),
                                   &nackPacket );
    assert( RTCP_RESULT_OK == result );
    assert( nackPacket.seqNumListLength == 3 );

    nackPacket.pSeqNumList = malloc( nackPacket.seqNumListLength * sizeof( uint16_t ) );
    result = Rtcp_ParseNackPacket( &ctx,
                                   &( compound[0] ),
                                   sizeof( compound ),
                                   &nackPacket );
    assert( RTCP_RESULT_OK == result );
    assert( nackPacket.pSeqNumList[0] == 3240 );
    assert( nackPacket.pSeqNumList[1] == 3327 );
    assert( nackPacket.pSeqNumList[2] == 3329 );
    free( nackPacket.pSeqNumList );
}
/*-----------------------------------------------------------*/

void deserialize_twccPacket()
{
    RtcpPacket_t rtcpPacket;
    RtcpContext_t ctx;
    RtcpResult_t result;
    RtcpTwccPacket_t twccPacket;

    uint8_t twccpayloadTooSmall[] = { 0x00, 0x00 };
    uint8_t twccpayload1[] = { 0x44, 0x87, 0xa9, 0xe7, 0x54, 0xb3, 0xe6, 0xfd,
                               0x01, 0x81, 0x00, 0x01, 0x14, 0x7a, 0x75, 0xa6, 0x20, 0x01, 0xc8, 0x01 };
    uint8_t twccpayload2[] = { 0x44, 0x87, 0xa9, 0xe7, 0x54, 0xb3, 0xe6, 0xfd,
                               0x12, 0x67, 0x00, 0x08, 0x14, 0x85, 0x60, 0xa8, 0xd6, 0x65, 0x20, 0x01,
                               0x6c, 0x00, 0xfd, 0x78, 0x04, 0x02, 0x90, 0x28, 0x00, 0x04, 0x00, 0x02 };
    uint8_t twccpayload3[] = { 0x44, 0x87, 0xa9, 0xe7, 0x54, 0xb3, 0xe6, 0xfd,
                               0x04, 0x02, 0x00, 0xe4, 0x14, 0x7c, 0x9f, 0x81, 0x20, 0x27, 0x00, 0xb7,
                               0xe6, 0x64, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00 };
    result = Rtcp_Init( &ctx );
    assert( RTCP_RESULT_OK == result );

    /* twccpayloadTooSmall Packet parsing */
    memset( &twccPacket,
            0x00,
            sizeof( RtcpTwccPacket_t ) );

    result = Rtcp_ParseTwccPacket( &ctx,
                                   &( twccpayloadTooSmall[0] ),
                                   sizeof( twccpayloadTooSmall ),
                                   &twccPacket );
    assert( RTCP_RESULT_INPUT_TWCCK_PACKET_INVALID == result );

    /* twccpayload2 Packet parsing */
    memset( &twccPacket,
            0x00,
            sizeof( RtcpTwccPacket_t ) );

    result = Rtcp_ParseTwccPacket( &ctx,
                                   &( twccpayload2[0] ),
                                   sizeof( twccpayload2 ),
                                   &twccPacket );
    assert( RTCP_RESULT_OK == result );
    assert( twccPacket.baseSeqNum == 0x1267 );
    assert( twccPacket.packetStatusCount == 8 );
    assert( twccPacket.referenceTime == 0x148560 );
    assert( twccPacket.feedbackPacketCount == 0xa8 );
    assert( twccPacket.pPacketChunkStart == &( twccpayload2[16] ) );
    assert( twccPacket.pRecvDeltaStart == &( twccpayload2[20] ) );

    /* twccpayload3 Packet parsing */
    memset( &twccPacket,
            0x00,
            sizeof( RtcpTwccPacket_t ) );

    result = Rtcp_ParseTwccPacket( &ctx,
                                   &( twccpayload3[0] ),
                                   sizeof( twccpayload3 ),
                                   &twccPacket );
    assert( RTCP_RESULT_OK == result );
    assert( twccPacket.baseSeqNum == 0x402 );
    assert( twccPacket.packetStatusCount == 0xe4 );
    assert( twccPacket.referenceTime == 0x147c9f );
    assert( twccPacket.feedbackPacketCount == 0x81 );
    assert( twccPacket.pPacketChunkStart == &( twccpayload3[16] ) );
    assert( twccPacket.pRecvDeltaStart == &( twccpayload3[22] ) );
}
/*-----------------------------------------------------------*/

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
    deserialize_rembValueGet();
    deserialize_senderReport();
    deserialize_receiverReport();
    deserialize_nackPacket();
    deserialize_twccPacket();

    printf( "\nAll deserialize test PASS.\r\n" );

    serialize_senderReport();

    printf( "\nAll serialize test PASS.\r\n" );

    return 0;
}

/*-----------------------------------------------------------*/