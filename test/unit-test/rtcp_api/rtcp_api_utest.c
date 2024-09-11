/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <stdlib.h>
#include <time.h>

/* API includes. */
#include "rtcp_api.h"

/* ===========================  EXTERN VARIABLES  =========================== */
#define RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET    31

#define NtpTime                                     0x1122334455667788ULL
#define RtpTime                                     0x99AABBCC
#define PacketCount                                 0x44332211
#define OctetCount                                  0xAABBCCDD

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate RTCP Init fail functionality for Bad Parameters.
 */
void test_rtcpInit_BadParams( void )
{
    RtcpResult_t result;

    result = Rtcp_Init( NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Init functionality.
 */
void test_rtcpInit( void )
{
    RtcpContext_t context;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.readUint16Fn );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.readUint32Fn );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.readUint64Fn );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.writeUint16Fn );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.writeUint32Fn );
    TEST_ASSERT_NOT_NULL( context.readWriteFunctions.writeUint64Fn );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpSerializeSenderReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport = { 0 };
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_SerializeSenderReport( NULL,
                                         &( senderReport ),
                                         &( buffer[ 0 ] ),
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &( context ),
                                         NULL,
                                         &( buffer[ 0 ] ),
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &( context ),
                                         &( senderReport ),
                                         NULL,
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &( context ),
                                         &( senderReport ),
                                         &( buffer[ 0 ] ),
                                         NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Too Many ReceptionReports.
 */
void test_rtcpSerializeSenderReport_TooManyReceptionReports( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    senderReport.numReceptionReports = RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET + 1;

    result = Rtcp_SerializeSenderReport( &( context ),
                                         &( senderReport ),
                                         &( buffer[ 0 ] ),
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Small Buffer.
 */
void test_rtcpSerializeSenderReport_SmallBuffer( void )
{
    uint32_t i;
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[ 10 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    senderReport.senderSsrc = 0x12345678;
    senderReport.senderInfo.ntpTime = NtpTime;
    senderReport.senderInfo.rtpTime = RtpTime;
    senderReport.senderInfo.packetCount = PacketCount;
    senderReport.senderInfo.octetCount = OctetCount;
    senderReport.numReceptionReports = 2;
    senderReport.pReceptionReports = &( receptionReports[ 0 ] );

    for( i = 0; i < senderReport.numReceptionReports; i++ )
    {
        senderReport.pReceptionReports[ i ].sourceSsrc = i + 1;
        senderReport.pReceptionReports[ i ].fractionLost = 0;
        senderReport.pReceptionReports[ i ].cumulativePacketsLost = 0;
        senderReport.pReceptionReports[ i ].extendedHighestSeqNumReceived = 0;
        senderReport.pReceptionReports[ i ].interArrivalJitter = 0;
        senderReport.pReceptionReports[ i ].lastSR = 0;
        senderReport.pReceptionReports[ i ].delaySinceLastSR = 0;
    }

    result = Rtcp_SerializeSenderReport( &( context ),
                                         &( senderReport ),
                                         &( buffer[ 0 ] ),
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report functionality.
 */
void test_rtcpSerializeSenderReport( void )
{
    uint32_t i;
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    uint8_t serializedReport[] =
    {
        0x82, 0xC8, 0x00, 0x12, /* Header: V=2, P=0, RC=2, PT=SR=200, Length=18 words. */
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime). */
        0x99, 0xAA, 0xBB, 0xCC, /* Sender Info (rtpTime). */
        0x44, 0x33, 0x22, 0x11, /* Sender Info (packetCount). */
        0xAA, 0xBB, 0xCC, 0xDD, /* Sender Info (octetCount). */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0xDE, 0xAD, 0xBE, 0xEF, /* Delay since last SR = 0xDEADBEEF. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0xDE, 0xAD, 0xBE, 0xEF, /* Delay since last SR = 0xDEADBEEF. */
    };
    size_t serializedReportLength = sizeof( serializedReport );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    senderReport.senderSsrc = 0x12345678;
    senderReport.senderInfo.ntpTime = NtpTime;
    senderReport.senderInfo.rtpTime = RtpTime;
    senderReport.senderInfo.packetCount = PacketCount;
    senderReport.senderInfo.octetCount = OctetCount;
    senderReport.numReceptionReports = 2;
    senderReport.pReceptionReports = &( receptionReports[ 0 ] );

    for( i = 0; i < senderReport.numReceptionReports; i++ )
    {
        senderReport.pReceptionReports[ i ].sourceSsrc = i + 1;
        senderReport.pReceptionReports[ i ].fractionLost = 0x11;
        senderReport.pReceptionReports[ i ].cumulativePacketsLost = 0xA0A1A2;
        senderReport.pReceptionReports[ i ].extendedHighestSeqNumReceived = 0xD1D2D3D4;
        senderReport.pReceptionReports[ i ].interArrivalJitter = 0xB1B2B3B4;
        senderReport.pReceptionReports[ i ].lastSR = 0xC1C2C3C4;
        senderReport.pReceptionReports[ i ].delaySinceLastSR = 0xDEADBEEF;
    }

    result = Rtcp_SerializeSenderReport( &( context ),
                                         &( senderReport ),
                                         &( buffer[ 0 ] ),
                                         &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( serializedReportLength,
                       bufferLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( &( serializedReport[ 0 ] ),
                                   &( buffer[ 0 ] ),
                                   serializedReportLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Bad Parameters.
 */
void test_rtcpSerializeReceiverReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport = { 0 };
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_SerializeReceiverReport( NULL,
                                           &( receiverReport ),
                                           &( buffer[ 0 ] ),
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           NULL,
                                           &( buffer[ 0 ] ),
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
                                           NULL,
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
                                           &( buffer[ 0 ] ),
                                           NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Too Many ReceptionReports.
 */
void test_rtcpSerializeReceiverReport_TooManyReceptionReports( void )
{
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    receiverReport.numReceptionReports = RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET + 1;

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
                                           &( buffer[ 0 ] ),
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Small Buffer.
 */
void test_rtcpSerializeReceiverReport_SmallBuffer( void )
{
    uint32_t i;
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[ 10 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    receiverReport.senderSsrc = 0x12345678;
    receiverReport.numReceptionReports = 2;
    receiverReport.pReceptionReports = &( receptionReports[ 0 ] );

    for( i = 0; i < receiverReport.numReceptionReports; i++ )
    {
        receiverReport.pReceptionReports[ i ].sourceSsrc = i + 1;
        receiverReport.pReceptionReports[ i ].fractionLost = 0;
        receiverReport.pReceptionReports[ i ].cumulativePacketsLost = 0;
        receiverReport.pReceptionReports[ i ].extendedHighestSeqNumReceived = 0;
        receiverReport.pReceptionReports[ i ].interArrivalJitter = 0;
        receiverReport.pReceptionReports[ i ].lastSR = 0;
        receiverReport.pReceptionReports[ i ].delaySinceLastSR = 0;
    }

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
                                           &( buffer[ 0 ] ),
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet fail functionality for Bad Parameters.
 */
void test_rtcpDeSerializePacket_BadParams( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[ 256 ] = { 0 };
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;

    result = Rtcp_DeserializePacket( NULL,
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     NULL,
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet fail functionality for small Packet.
 */
void test_rtcpDeSerializePacket_SmallPacket( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[ 2 ] = { 0 };
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_INPUT_PACKET_TOO_SMALL,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Fir Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseFirPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpFirPacket_t rtcpFirPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 6 ];

    result = Rtcp_ParseFirPacket( NULL,
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseFirPacket( &( context ),
                                  NULL,
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 6;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Fir Packet functionality.
 */
void test_rtcpParseFirPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpFirPacket_t rtcpFirPacket;
    RtcpResult_t result;
    uint8_t firPacketPayload[] =
    {
        0x11, 0x22, 0x33, 0x44 /* SSRC. */
    };
    size_t firPacketPayloadLength = sizeof( firPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;
    rtcpPacket.header.packetType = RTCP_PACKET_FIR;
    rtcpPacket.pPayload = &( firPacketPayload[ 0 ] );
    rtcpPacket.payloadLength = firPacketPayloadLength;

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x11223344,
                       rtcpFirPacket.senderSsrc );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Pli Packet fail functionality for Bad Parameters.
 */
void test_rtcpParsePliPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpPliPacket_t rtcpPliPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 10 ];

    result = Rtcp_ParsePliPacket( NULL,
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParsePliPacket( &( context ),
                                  NULL,
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 10;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sli Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseSliPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpSliPacket_t rtcpSliPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 14 ];

    result = Rtcp_ParseSliPacket( NULL,
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseSliPacket( &( context ),
                                  NULL,
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 14;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseRembPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpRembPacket_t rtcpRembPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer [ 14 ];

    result = Rtcp_ParseRembPacket( NULL,
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseRembPacket( &( context ),
                                   NULL,
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 14;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpParseSenderReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpSenderReport_t rtcpSenderReport;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 25 ];

    result = Rtcp_ParseSenderReport( NULL,
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseSenderReport( &( context ),
                                     NULL,
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 25;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Receiver Report fail functionality for Bad Parameters.
 */
void test_rtcpParseReceiverReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpReceiverReport_t rtcpReceiverReport;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 10 ];

    result = Rtcp_ParseReceiverReport( NULL,
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseReceiverReport( &( context ),
                                       NULL,
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 10;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseNackPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpNackPacket_t rtcpNackPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 40 ];

    result = Rtcp_ParseNackPacket( NULL,
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseNackPacket( &( context ),
                                   NULL,
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 40;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseTwccPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;
    uint8_t payloadBuffer[ 20 ];

    result = Rtcp_ParseTwccPacket( NULL,
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseTwccPacket( &( context ),
                                   NULL,
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   NULL );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.pPayload = NULL;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/
