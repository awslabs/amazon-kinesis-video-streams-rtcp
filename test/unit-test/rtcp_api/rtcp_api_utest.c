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
 * @brief Validate RTCP Serialize Receiver Report functionality.
 */
void test_rtcpSerializeReceiverReport( void )
{
    uint32_t i;
    RtcpContext_t context;
    RtcpReceiverReport_t receiverReport;
    uint8_t buffer[256];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    uint8_t expectedBuffer[] = {
        0x82, 0xC9, 0x00, 0x0D, /* Header */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t expectedLength = sizeof( expectedBuffer );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    receiverReport.senderSsrc = 0x87654321;
    receiverReport.numReceptionReports = 2;
    receiverReport.pReceptionReports = &( receptionReports[ 0 ] );

    for( i = 0; i < receiverReport.numReceptionReports; i++)
    {
        receiverReport.pReceptionReports[i].sourceSsrc = i + 1;
        receiverReport.pReceptionReports[i].fractionLost = 0x11;
        receiverReport.pReceptionReports[i].cumulativePacketsLost = 0xA0A1A2;
        receiverReport.pReceptionReports[i].extendedHighestSeqNumReceived = 0xD1D2D3D4;
        receiverReport.pReceptionReports[i].interArrivalJitter = 0xB1B2B3B4;
        receiverReport.pReceptionReports[i].lastSR = 0xC1C2C3C4;
        receiverReport.pReceptionReports[i].delaySinceLastSR = 0x5A5B5C5D;
    }

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
                                           &( buffer[ 0 ] ),
                                           &( bufferLength ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( expectedLength,
                       bufferLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( &( expectedBuffer[ 0 ] ),
                                   &( buffer[ 0 ] ),
                                   expectedLength );
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
    RtcpPacket_t rtcpPacket = { 0 };
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
    RtcpPacket_t rtcpPacket = { 0 };
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
 * @brief Validate RTCP DeSerialize fail Packet functionality for Wrong Version.
 */
void test_rtcpDeSerializePacket_WrongVersion( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] = {
        0x03, 0xC9, 0x00, 0x0D, /* Header with wrong version */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* ... Reception Reports ... */
    };
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );



    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_WRONG_VERSION,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize fail Packet functionality for Malformed Packet.
 */

/* This test covers when the serialized packet length is smaller than expected hence the packed is malformed. */

void test_rtcpDeSerializePacket_MalformedPacked( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] = {
        0x82, 0xC9, 0x00, 0x0D, /* Header */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1 */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        /* Reception Report 2 */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] = {
        0x82, 0xC9, 0x00, 0x0D, /* Header */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t serializedPacketLength = sizeof( serializedPacket );
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 2,
                       rtcpPacket.header.receptionReportCount );
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
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
 * @brief Validate RTCP Parse Pli Packet functionality.
 */
void test_rtcpParsePliPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpPliPacket_t rtcpPliPacket;
    RtcpResult_t result;
    uint8_t pliPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x87, 0x65, 0x43, 0x21  /* Media Source SSRC */
    };
    size_t pliPacketPayloadLength = sizeof( pliPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_PLI;
    rtcpPacket.pPayload = &( pliPacketPayload[ 0 ] );
    rtcpPacket.payloadLength = pliPacketPayloadLength;


    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpPliPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x87654321,
                       rtcpPliPacket.mediaSourceSsrc );
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
 * @brief Validate RTCP Parse Sli Packet functionality.
 */
void test_rtcpParseSliPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    uint8_t sliPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x87, 0x65, 0x43, 0x21, /* Media Source SSRC */
        0x00, 0x00, 0x00, 0x01, /* SLI Info 1 */
        0xE0, 0xBE, 0x18, 0x9F, /* SLI Info 2 */
    };
    /*
       For SLI Info 2 : SLI Info Field contains :
       First : 13 Bits     -> 7191 = ( 1110 0000 1011 1 )
       Number : 13 Bits    -> 6242 = ( 110 0001 1000 10 )
       PictureID : 6 Bits  -> 31   = ( 01 1111 )

       The SLI Info 2 field will look like :
       1110 0000 1011 1110 0001 1000 1001 1111
       E    0     B    E    1    8    9    F

       Hence the Packet is : 0xE0BE189F
     */
    size_t sliPacketPayloadLength = sizeof( sliPacketPayload );
    uint32_t sliInfo[ 2 ];
    RtcpSliPacket_t rtcpSliPacket = {
        .pSliInfos = &( sliInfo[ 0 ] ),
        .numSliInfos = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_SLI;
    rtcpPacket.pPayload = &( sliPacketPayload[ 0 ] );
    rtcpPacket.payloadLength = sliPacketPayloadLength;

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpSliPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x87654321,
                       rtcpSliPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 2,
                       rtcpSliPacket.numSliInfos );
    TEST_ASSERT_EQUAL( 0x00000001,
                       rtcpSliPacket.pSliInfos[0] );
    TEST_ASSERT_EQUAL( 0xE0Be189F,
                       rtcpSliPacket.pSliInfos[1] );
    TEST_ASSERT_EQUAL( 7191,
                       RTCP_SLI_INFO_EXTRACT_FIRST( rtcpSliPacket.pSliInfos[1] ) );
    TEST_ASSERT_EQUAL( 6242,
                       RTCP_SLI_INFO_EXTRACT_NUMBER( rtcpSliPacket.pSliInfos[1] ) );
    TEST_ASSERT_EQUAL( 31,
                       RTCP_SLI_INFO_EXTRACT_PICTURE_ID( rtcpSliPacket.pSliInfos[1] ) );
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
    uint8_t rembPacketPayload[ 14 ];

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
    rtcpPacket.pPayload = &( rembPacketPayload[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet fail functionality for invalid packets where payload is too small.
 */
void test_rtcpParseRembPacket_InvalidPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpRembPacket_t rtcpRembPacket = { 0 };
    uint8_t rembPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x52, 0x45, 0x4D, 0x42  /* REMB Unique Identifier */
    };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = 12;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;
    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_INPUT_REMB_PACKET_INVALID,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet fail functionality for malformed packets with incorrect remb identifier.
 */
void test_rtcpParseRembPacket_MalformedPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpRembPacket_t rtcpRembPacket = { 0 };
    uint8_t rembPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x52, 0x45, 0x4D, 0x43, /* Incorrect REMB Unique Identifier */
        0x02, 0x00, 0x00, 0x00, /* Num SSRC (2) and BR Exp (0) */
        0x01, 0x02, 0x03, 0x04  /* SSRC 1 */
    };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet fail functionality for invalid packets where SSRC List Length is too small.
 */
void test_rtcpParseRembPacket_InvalidSSRCLength( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    uint8_t rembPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x52, 0x45, 0x4D, 0x42, /* REMB Unique Identifier */
        0x02, 0x00, 0x00, 0x00, /* Num SSRC (2) and BR Exp (0) */
        0x01, 0x02, 0x03, 0x04, /* SSRC 1 */
        0x05, 0x06, 0x07, 0x08  /* SSRC 2 */
    };
    RtcpResult_t result;
    uint32_t ssrcList[ 1 ];
    RtcpRembPacket_t rtcpRembPacket = {
        .pSsrcList = &( ssrcList[ 0 ] ), /* SSRC list length too small */
        .ssrcListLength = 1
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_INPUT_REMB_PACKET_INVALID,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet functionality .
 */
void test_rtcpParseRembPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    uint8_t rembPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x52, 0x45, 0x4D, 0x42, /* REMB Unique Identifier */
        0x02, 0x00, 0x00, 0x00, /* Num SSRC (2) and BR Exp (0) */
        0x01, 0x02, 0x03, 0x04, /* SSRC 1 */
        0x05, 0x06, 0x07, 0x08  /* SSRC 2 */
    };
    RtcpResult_t result;
    uint32_t ssrcList[ 2 ];
    RtcpRembPacket_t rtcpRembPacket = {
        .pSsrcList = &( ssrcList[ 0 ] ), /* SSRC list length too small */
        .ssrcListLength = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpRembPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpRembPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 2,
                       rtcpRembPacket.ssrcListLength );
    TEST_ASSERT_EQUAL( 0x01020304,
                       rtcpRembPacket.pSsrcList[0] );
    TEST_ASSERT_EQUAL( 0x05060708,
                       rtcpRembPacket.pSsrcList[1] );
                       
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
    uint8_t senderReportPayload[ 25 ];

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
    rtcpPacket.pPayload = &( senderReportPayload[ 0 ] );

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 25;
    rtcpPacket.pPayload = &( senderReportPayload[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpParseSenderReport_OutOfMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 1 ];
    RtcpSenderReport_t rtcpSenderReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 1
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t senderReportPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime) */
        0x99, 0xAA, 0xBB, 0xCC, /* Sender Info (rtpTime) */
        0x00, 0x00, 0x03, 0xE8, /* Sender Info (packetCount) */
        0x00, 0x01, 0x86, 0xA0, /* Sender Info (octetCount) */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };

    rtcpPacket.payloadLength = 48;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 2;
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for Bad Parameters.
 */
void test_rtcpParseSenderReport_MalformedPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 2 ];
    RtcpSenderReport_t rtcpSenderReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t senderReportPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime) */
        0x99, 0xAA, 0xBB, 0xCC, /* Sender Info (rtpTime) */
        0x00, 0x00, 0x03, 0xE8, /* Sender Info (packetCount) */
        0x00, 0x01, 0x86, 0xA0, /* Sender Info (octetCount) */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };

    rtcpPacket.payloadLength = 36;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 2;
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report functionality.
 */
void test_rtcpParseSenderReport( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 4 ];
    RtcpSenderReport_t rtcpSenderReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 4
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t senderReportPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime) */
        0x99, 0xAA, 0xBB, 0xCC, /* Sender Info (rtpTime) */
        0x00, 0x00, 0x03, 0xE8, /* Sender Info (packetCount) */
        0x00, 0x01, 0x86, 0xA0, /* Sender Info (octetCount) */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 3. */
        0x00, 0x00, 0x00, 0x03, /* SSRC of third source. */
        0x22, 0xB0, 0xB1, 0xB2, /* Fraction lost = 0x22, Cumulative packet lost = 0xB0B1B2. */
        0xE1, 0xE2, 0xE3, 0xE4, /* Extended highest sequence number received = 0xE1E2E3E4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Inter-arrival Jitter = 0xC1C2C3C4. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Last SR = 0xD1D2D3D4. */
        0x6A, 0x6B, 0x6C, 0x6D, /* Delay since last SR = 0x6A6B6C6D. */
        /* Reception Report 4. */
        0x00, 0x00, 0x00, 0x04, /* SSRC of fourth source. */
        0x33, 0xC0, 0xC1, 0xC2, /* Fraction lost = 0x33, Cumulative packet lost = 0xC0C1C2. */
        0xF1, 0xF2, 0xF3, 0xF4, /* Extended highest sequence number received = 0xF1F2F3F4. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Inter-arrival Jitter = 0xD1D2D3D4. */
        0xE1, 0xE2, 0xE3, 0xE4, /* Last SR = 0xE1E2E3E4. */
        0x7A, 0x7B, 0x7C, 0x7D, /* Delay since last SR = 0x7A7B7C7D. */
    };

    rtcpPacket.payloadLength = 120;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 4;
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpSenderReport.senderSsrc );
    TEST_ASSERT_EQUAL( 0x1122334455667788,
                       rtcpSenderReport.senderInfo.ntpTime );
    TEST_ASSERT_EQUAL( 0x99AABBCC,
                       rtcpSenderReport.senderInfo.rtpTime );
    TEST_ASSERT_EQUAL( 1000,
                       rtcpSenderReport.senderInfo.packetCount );
    TEST_ASSERT_EQUAL( 100000,
                       rtcpSenderReport.senderInfo.octetCount );
    TEST_ASSERT_EQUAL( 4,
                       rtcpSenderReport.numReceptionReports );
    TEST_ASSERT_EQUAL( 0x00000001,
                       rtcpSenderReport.pReceptionReports[0].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000002,
                       rtcpSenderReport.pReceptionReports[1].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000003,
                       rtcpSenderReport.pReceptionReports[2].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000004,
                       rtcpSenderReport.pReceptionReports[3].sourceSsrc );
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
    uint8_t receiverReportPayload[ 10 ];

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
    rtcpPacket.pPayload = &( receiverReportPayload[ 0 ] );

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 10;
    rtcpPacket.pPayload = &( receiverReportPayload[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Receiver Report Fail functionality for out of memory.
 */
void test_rtcpParseReceiverReport_OutOfMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 1 ];
    RtcpReceiverReport_t rtcpReceiverReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 1
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t receiverReportPayload[] = {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Receiver Report Fail functionality for malformed packet.
 */
void test_rtcpParseReceiverReport_MalformedPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 2 ];
    RtcpReceiverReport_t rtcpReceiverReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t receiverReportPayload[] = {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Receiver Report functionality.
 */
void test_rtcpParseReceiverReport( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReport[ 2 ];
    RtcpReceiverReport_t rtcpReceiverReport = {
        .pReceptionReports = &( receptionReport[ 0 ] ),
        .numReceptionReports = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t receiverReportPayload[] = {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };

    rtcpPacket.payloadLength = 52;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2;

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x87654321,
                       rtcpReceiverReport.senderSsrc );
    TEST_ASSERT_EQUAL( 2,
                       rtcpReceiverReport.numReceptionReports );
    TEST_ASSERT_EQUAL( 0x00000001,
                       rtcpReceiverReport.pReceptionReports[0].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000002,
                       rtcpReceiverReport.pReceptionReports[1].sourceSsrc );
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
    uint8_t nackPacketPayload[ 40 ];

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
    rtcpPacket.pPayload = &( nackPacketPayload[ 0 ] );

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 40;
    rtcpPacket.pPayload = &( nackPacketPayload[ 0 ] );
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet functionality.
 */
void test_rtcpParseNackPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    uint16_t seqNumList[ 5 ];
    RtcpNackPacket_t rtcpNackPacket = {
        .pSeqNumList = &( seqNumList[ 0 ] ),
        .seqNumListLength = 5
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t nackPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x06, 0x3B, 0x0A, 0x09  /* Sequence Number 1 */

    };
    /*
       For NACK Sequence 1 : NACK Sequence Field contains :
       PID(RTP SeqNum) :                16 Bits     -> 1595 
       BLP(Bitmask of Lost Packets) :   16 Bits     -> ( 000 1010 0000 1001 )
       
       BLP : 
       MSB                  LSB
       ( 000 1010 0000 1001 )

       Total SeqNum Lost: 
       PID --> 1595
       PID + 1  --> 1596
       PID + 4  --> 1599
       PID + 10 --> 1605
       PID + 12 --> 1607

       The NACK Sequence 1 field will look like :
       <-------PID------> 0000 1010 0000 1001
       0    6     3    B   0    A    0    9
     */
    rtcpPacket.payloadLength = 12;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpNackPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpNackPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL(5, rtcpNackPacket.seqNumListLength);
    TEST_ASSERT_EQUAL( 1595,
                       rtcpNackPacket.pSeqNumList[0] );
    TEST_ASSERT_EQUAL( 1596,
                       rtcpNackPacket.pSeqNumList[1] );
    TEST_ASSERT_EQUAL( 1599,
                       rtcpNackPacket.pSeqNumList[2] );
    TEST_ASSERT_EQUAL( 1605,
                       rtcpNackPacket.pSeqNumList[3] );
    TEST_ASSERT_EQUAL( 1607,
                       rtcpNackPacket.pSeqNumList[4] );

}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet functionality.
 */
void test_rtcpParseNackPacket_EmptySequenceList( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpNackPacket_t rtcpNackPacket = {
        .pSeqNumList = NULL
    };
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t nackPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01, 0x00, 0x03, /* Sequence Number 1, Bitmask 0x0003      Total Lost packets = 3 ( PID + 2 of BLP )*/  
        0x00, 0x04, 0x00, 0x01  /* Sequence Number 4  Bitmask 0x0001      Total Lost packets = 2 ( PID + 1 of BLP )*/
    };

    rtcpPacket.payloadLength = 16;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpNackPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpNackPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL(5, rtcpNackPacket.seqNumListLength);
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet fail functionality for out of memory.
 */
void test_rtcpParseNackPacket_OutOfMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    uint16_t seqNumList[ 2 ];
    RtcpNackPacket_t rtcpNackPacket = {
        .pSeqNumList = &( seqNumList[ 0 ] ),
        .seqNumListLength = 2
    };

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t nackPacketPayload[] = {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01, 0x00, 0x03, /* Sequence Number 1, Bitmask 0x0003 */
        0x00, 0x04, 0x00, 0x01  /* Sequence Number 4 */
    };

    rtcpPacket.payloadLength = 16;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );

    rtcpNackPacket.seqNumListLength = 1;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
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
