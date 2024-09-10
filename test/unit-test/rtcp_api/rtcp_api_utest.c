/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <stdlib.h>
#include <time.h>

/* Data Types includes. */
#include "rtcp_data_types.h"

/* API includes. */
#include "rtcp_twcc_manager.h"
#include "rtcp_api.h"
#include "rtcp_endianness.h"

/* ===========================  EXTERN VARIABLES  =========================== */
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

#define NtpTime                                     0x1122334455667788ULL
#define RtpTime                                     0x99AABBCC
#define PacketCount                                 1000
#define OctetCount                                  100000

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

}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpSerializeSenderReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_SerializeSenderReport( NULL,
                                         &senderReport,
                                         buffer,
                                         &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &context,
                                         NULL,
                                         buffer,
                                         &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &context,
                                         &senderReport,
                                         NULL,
                                         &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeSenderReport( &context,
                                         &senderReport,
                                         buffer,
                                         NULL );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Too Many ReceptionReports.
 */
void test_rtcpSerializeSenderReport_ManyReceptionReports( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    senderReport.numReceptionReports = RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET + 1;

    result = Rtcp_SerializeSenderReport( &context,
                                         &senderReport,
                                         buffer,
                                         &bufferLength );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report fail functionality for Small Buffer.
 */
void test_rtcpSerializeSenderReport_SmallBuffer( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[10];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    // Initialize the RtcpSenderReport_t struct with sample data
    senderReport.senderSsrc = 0x12345678;
    senderReport.senderInfo.ntpTime = NtpTime;
    senderReport.senderInfo.rtpTime = RtpTime;
    senderReport.senderInfo.packetCount = PacketCount;
    senderReport.senderInfo.octetCount = OctetCount;
    senderReport.numReceptionReports = 2;

    senderReport.pReceptionReports = ( RtcpReceptionReport_t * ) malloc( senderReport.numReceptionReports * sizeof( RtcpReceptionReport_t ) );
    for(uint32_t i = 0; i < senderReport.numReceptionReports; i++)
    {
        senderReport.pReceptionReports[i].sourceSsrc = i + 1;
        senderReport.pReceptionReports[i].fractionLost = 0;
        senderReport.pReceptionReports[i].cumulativePacketsLost = 0;
        senderReport.pReceptionReports[i].extendedHighestSeqNumReceived = 0;
        senderReport.pReceptionReports[i].interArrivalJitter = 0;
        senderReport.pReceptionReports[i].lastSR = 0;
        senderReport.pReceptionReports[i].delaySinceLastSR = 0;
    }

    result = Rtcp_SerializeSenderReport( &context,
                                         &senderReport,
                                         buffer,
                                         &bufferLength );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );

    free( senderReport.pReceptionReports );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Sender Report functionality.
 */
void test_rtcpSerializeSenderReport( void )
{
    RtcpContext_t context;
    RtcpSenderReport_t senderReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    uint8_t expectedBuffer[] = {
        0x82, 0xC8, 0x00, 0x12, /* Header */
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime) */
        0x99, 0xAA, 0xBB, 0xCC, /* Sender Info (rtpTime) */
        0x00, 0x00, 0x03, 0xE8, /* Sender Info (packetCount) */
        0x00, 0x01, 0x86, 0xA0, /* Sender Info (octetCount) */
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Reception Report 1 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* Reception Report 2 */
    };
    size_t expectedLength = sizeof( expectedBuffer );

    result = Rtcp_Init( &( context ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    // Initialize the RtcpSenderReport_t struct with sample data
    senderReport.senderSsrc = 0x12345678;
    senderReport.senderInfo.ntpTime = NtpTime;
    senderReport.senderInfo.rtpTime = RtpTime;
    senderReport.senderInfo.packetCount = PacketCount;
    senderReport.senderInfo.octetCount = OctetCount;
    senderReport.numReceptionReports = 2;
    senderReport.pReceptionReports = ( RtcpReceptionReport_t * ) malloc( senderReport.numReceptionReports * sizeof( RtcpReceptionReport_t ) );

    for(uint32_t i = 0; i < senderReport.numReceptionReports; i++)
    {
        senderReport.pReceptionReports[i].sourceSsrc = i + 1;
        senderReport.pReceptionReports[i].fractionLost = 0;
        senderReport.pReceptionReports[i].cumulativePacketsLost = 0;
        senderReport.pReceptionReports[i].extendedHighestSeqNumReceived = 0;
        senderReport.pReceptionReports[i].interArrivalJitter = 0;
        senderReport.pReceptionReports[i].lastSR = 0;
        senderReport.pReceptionReports[i].delaySinceLastSR = 0;
    }

    // Call the function being tested
    result = Rtcp_SerializeSenderReport( &( context ),
                                         &senderReport,
                                         buffer,
                                         &bufferLength );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expectedBuffer,
                                   buffer,
                                   expectedLength );
    TEST_ASSERT_EQUAL_HEX32( 0x82C80012,
                             context.readWriteFunctions.readUint32Fn( buffer ) );
    TEST_ASSERT_EQUAL_HEX32( senderReport.senderSsrc,
                             context.readWriteFunctions.readUint32Fn( buffer + 4 ) );
    TEST_ASSERT_EQUAL_HEX64( NtpTime,
                             context.readWriteFunctions.readUint64Fn( buffer + 8 ) );
    TEST_ASSERT_EQUAL_HEX32( RtpTime,
                             context.readWriteFunctions.readUint32Fn( buffer + 16 ) );
    TEST_ASSERT_EQUAL_HEX32( PacketCount,
                             context.readWriteFunctions.readUint32Fn( buffer + 20 ) );
    TEST_ASSERT_EQUAL_HEX32( OctetCount,
                             context.readWriteFunctions.readUint32Fn( buffer + 24 ) );
    TEST_ASSERT_EQUAL( RTCP_HEADER_LENGTH + RTCP_SENDER_SSRC_LENGTH + RTCP_SENDER_INFO_LENGTH + ( senderReport.numReceptionReports * RTCP_RECEPTION_REPORT_LENGTH ),
                       bufferLength );
    free( senderReport.pReceptionReports );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Bad Parameters.
 */
void test_rtcpSerializeReceiverReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_SerializeReceiverReport( NULL,
                                           &receiverReport,
                                           buffer,
                                           &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &context,
                                           NULL,
                                           buffer,
                                           &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &context,
                                           &receiverReport,
                                           NULL,
                                           &bufferLength );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    result = Rtcp_SerializeReceiverReport( &context,
                                           &receiverReport,
                                           buffer,
                                           NULL );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Too Many ReceptionReports.
 */
void test_rtcpSerializeReceiverReport_ManyReceptionReports( void )
{
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    receiverReport.numReceptionReports = RTCP_MAX_RECEPTION_REPORTS_IN_ONE_PACKET + 1;

    result = Rtcp_SerializeReceiverReport( &context,
                                           &receiverReport,
                                           buffer,
                                           &bufferLength );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Serialize Receiver Report fail functionality for Small Buffer.
 */
void test_rtcpSerializeReceiverReport_SmallBuffer( void )
{
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[10];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    // Initialize of RtcpReceiverReport_t struct with sample data
    receiverReport.senderSsrc = 0x12345678;
    receiverReport.numReceptionReports = 2;

    receiverReport.pReceptionReports = ( RtcpReceptionReport_t * ) malloc( receiverReport.numReceptionReports * sizeof( RtcpReceptionReport_t ) );
    for(uint32_t i = 0; i < receiverReport.numReceptionReports; i++)
    {
        receiverReport.pReceptionReports[i].sourceSsrc = i + 1;
        receiverReport.pReceptionReports[i].fractionLost = 0;
        receiverReport.pReceptionReports[i].cumulativePacketsLost = 0;
        receiverReport.pReceptionReports[i].extendedHighestSeqNumReceived = 0;
        receiverReport.pReceptionReports[i].interArrivalJitter = 0;
        receiverReport.pReceptionReports[i].lastSR = 0;
        receiverReport.pReceptionReports[i].delaySinceLastSR = 0;
    }

    result = Rtcp_SerializeReceiverReport( &context,
                                           &receiverReport,
                                           buffer,
                                           &bufferLength );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );

    free( receiverReport.pReceptionReports );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet fail functionality for Bad Parameters.
 */
void test_rtcpDeSerializePacket_BadParams( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[256];
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;

    result = Rtcp_DeserializePacket( NULL,
                                     ( serializedPacket ),
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
                                     ( serializedPacket ),
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
    uint8_t serializedPacket[2] = {0};
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     ( serializedPacket ),
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
    RtcpPacket_t rtcpPacket;
    RtcpFirPacket_t rtcpFirPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 6;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
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

    result = Rtcp_Init( &( context ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = 6;
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_FIR;

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( payloadBuffer,
                                   rtcpPacket.pPayload,
                                   rtcpPacket.payloadLength );

    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Pli Packet fail functionality for Bad Parameters.
 */
void test_rtcpParsePliPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpPliPacket_t rtcpPliPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 10;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sli Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseSliPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpSliPacket_t rtcpSliPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 14;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseRembPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpRembPacket_t rtcpRembPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpParseSenderReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpSenderReport_t rtcpSenderReport;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 25;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Receiver Report fail functionality for Bad Parameters.
 */
void test_rtcpParseReceiverReport_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpReceiverReport_t rtcpReceiverReport;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 10;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseNackPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpNackPacket_t rtcpNackPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 40;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet fail functionality for Bad Parameters.
 */
void test_rtcpParseTwccPacket_BadParams( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

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
    uint8_t * payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );

    rtcpPacket.payloadLength = 20;
    payloadBuffer = ( uint8_t * )malloc( rtcpPacket.payloadLength );
    rtcpPacket.pPayload = payloadBuffer;
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;
    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );
    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
    free( payloadBuffer );
}

/*-----------------------------------------------------------*/