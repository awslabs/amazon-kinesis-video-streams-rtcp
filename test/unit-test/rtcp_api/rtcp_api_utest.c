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
        0x82, 0xC8, 0x00, 0x12,                         /* Header: V=2, P=0, RC=2, PT=SR=200, Length=18 words. */
        0x12, 0x34, 0x56, 0x78,                         /* Sender SSRC. */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime). */
        0x99, 0xAA, 0xBB, 0xCC,                         /* Sender Info (rtpTime). */
        0x44, 0x33, 0x22, 0x11,                         /* Sender Info (packetCount). */
        0xAA, 0xBB, 0xCC, 0xDD,                         /* Sender Info (octetCount). */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01,                         /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0xDE, 0xAD, 0xBE, 0xEF,                         /* Delay since last SR = 0xDEADBEEF. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02,                         /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0xDE, 0xAD, 0xBE, 0xEF,                         /* Delay since last SR = 0xDEADBEEF. */
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
    uint8_t buffer[ 256 ];
    size_t bufferLength = sizeof( buffer );
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    uint8_t serializedReport[] =
    {
        0x82, 0xC9, 0x00, 0x0D, /* Header: V=2, P=0, RC=2, PT=RR=201, Length = 0xD words. */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
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
    size_t serializedReportLength = sizeof( serializedReport );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    receiverReport.senderSsrc = 0x87654321;
    receiverReport.numReceptionReports = 2;
    receiverReport.pReceptionReports = &( receptionReports[ 0 ] );

    for( i = 0; i < receiverReport.numReceptionReports; i++)
    {
        receiverReport.pReceptionReports[ i ].sourceSsrc = i + 1;
        receiverReport.pReceptionReports[ i ].fractionLost = 0x11;
        receiverReport.pReceptionReports[ i ].cumulativePacketsLost = 0xA0A1A2;
        receiverReport.pReceptionReports[ i ].extendedHighestSeqNumReceived = 0xD1D2D3D4;
        receiverReport.pReceptionReports[ i ].interArrivalJitter = 0xB1B2B3B4;
        receiverReport.pReceptionReports[ i ].lastSR = 0xC1C2C3C4;
        receiverReport.pReceptionReports[ i ].delaySinceLastSR = 0x5A5B5C5D;
    }

    result = Rtcp_SerializeReceiverReport( &( context ),
                                           &( receiverReport ),
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
    uint8_t serializedPacket[ 2 ] = { 0 }; /* RTCP packet smaller than RTCP_HEADER_LENGTH. */
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
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;
    uint8_t serializedPacket[] =
    {
        /* Header: V=0 (wrong), P=0, RC=3, PT=RR=201, Length = 0xD words. */
        0x03, 0xC9, 0x00, 0x0D,
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
        /* ... Reception Reports ... */
    };
    size_t serializedPacketLength = sizeof( serializedPacket );

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

void test_rtcpDeSerializePacket_MalformedPacked( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;

    /* Both the reception report have one word missing (Delay since last SR). As
     * a result, the actual length of the packet is less than the length of
     * the packet encoded in the header. */
    uint8_t serializedPacket[] =
    {
        0x82, 0xC9, 0x00, 0x0D, /* Header: V=2, P=0, RC=2, PT=RR=201, Length = 0xD words. */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1 */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x00, 0x00, 0x00, 0x00, /* Fraction lost and Cumulative packet lost. */
        0x00, 0x00, 0x00, 0x00, /* Extended highest sequence number received. */
        0x00, 0x00, 0x00, 0x00, /* Inter-arrival Jitter. */
        0x00, 0x00, 0x00, 0x00, /* Last SR. */
        /* Reception Report 2 */
        0x00, 0x00, 0x00, 0x02, /* SSRC of second source. */
        0x00, 0x00, 0x00, 0x00, /* Fraction lost and Cumulative packet lost. */
        0x00, 0x00, 0x00, 0x00, /* Extended highest sequence number received. */
        0x00, 0x00, 0x00, 0x00, /* Inter-arrival Jitter. */
        0x00, 0x00, 0x00, 0x00, /* Last SR. */
    };
    size_t serializedPacketLength = sizeof( serializedPacket );

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
 * @brief Validate RTCP DeSerialize Packet functionality for an unkown type packet.
 */
void test_rtcpDeSerializePacket_UNKOWN( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x9F, 0xFE, 0x00, 0x0D, /* Header: V=2, P=0, RC=31, PT=RR=254, Length = 0xD words. */
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

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
    TEST_ASSERT_EQUAL( RTCP_PACKET_UNKNOWN,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 31,
                       rtcpPacket.header.receptionReportCount );
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_FIR( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x80, 0xC0, 0x00, 0x0D, /* Header: V=2, P=0, FMT=0, PT=RR=192, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_FIR,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.receptionReportCount );
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_FIR_Invalid( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x82, 0xC0, 0x00, 0x0D, /* Header: V=2, P=0, FMT=2, PT=RR=192, Length = 0xD words. */
        /* FMT For FIR Packet should be 0 , Hence Invalid */
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

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
    TEST_ASSERT_EQUAL( RTCP_PACKET_UNKNOWN,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 2,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_SenderReport( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x82, 0xC8, 0x00, 0x0D, /* Header: V=2, P=0, FMT=2, PT=RR=200, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_SENDER_REPORT,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 2,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_TransportSpecificFeedback_Nack( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x81, 0xCD, 0x00, 0x0D, /* Header: V=2, P=0, FMT=1, PT=RR=205, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_TRANSPORT_FEEDBACK_NACK,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 1,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_TransportSpecificFeedback_TWCC( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x8F, 0xCD, 0x00, 0x0D, /* Header: V=2, P=0, FMT=15, PT=RR=205, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 15,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_TransportSpecificFeedback_unknown( void )
{
    /* Thought The Packet Type is of  Transport Specific Feedback, the Feedback Message Type ( FMT ) is Unknown */
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x9F, 0xCD, 0x00, 0x0D, /* Header: V=2, P=0, FMT=31, PT=RR=205, Length = 0xD words. */
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

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
    TEST_ASSERT_EQUAL( RTCP_PACKET_UNKNOWN,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 31,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_PayloadSpecificFeedback_SLI( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x82, 0xCE, 0x00, 0x0D, /* Header: V=2, P=0, FMT=2, PT=RR=206, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_PAYLOAD_FEEDBACK_SLI,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 2,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_PayloadSpecificFeedback_PLI( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x81, 0xCE, 0x00, 0x07, /* Header: V=2, P=0, FMT=1, PT=RR=206, Length = 0x7 words. */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_PAYLOAD_FEEDBACK_PLI,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 1,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_PayloadSpecificFeedback_REMB( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x8F, 0xCE, 0x00, 0x0D, /* Header: V=2, P=0, FMT=15, PT=RR=206, Length = 0xD words. */
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
    TEST_ASSERT_EQUAL( RTCP_PACKET_PAYLOAD_FEEDBACK_REMB,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 15,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_PayloadSpecificFeedback_unknown( void )
{
    RtcpContext_t context;
    uint8_t serializedPacket[] =
    {
        0x9F, 0xCE, 0x00, 0x0D, /* Header: V=2, P=0, FMT=31, PT=RR=206, Length = 0xD words. */
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

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
    TEST_ASSERT_EQUAL( RTCP_PACKET_UNKNOWN,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 31,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL_PTR( &( serializedPacket[ 4 ] ),
                           rtcpPacket.pPayload );
    TEST_ASSERT_EQUAL( serializedPacketLength - 4,
                       rtcpPacket.payloadLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP DeSerialize Packet functionality.
 */
void test_rtcpDeSerializePacket_ReceiverReport( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket = { 0 };
    RtcpResult_t result;
    uint8_t serializedPacket[] =
    {
        0x82, 0xC9, 0x00, 0x0D, /* Header: V=2, P=0, FMT=2, PT=RR=201, Length = 0xD words. */
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
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

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    result = Rtcp_DeserializePacket( &( context ),
                                     &( serializedPacket[ 0 ] ),
                                     serializedPacketLength,
                                     &( rtcpPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( RTCP_PACKET_RECEIVER_REPORT,
                       rtcpPacket.header.packetType );
    TEST_ASSERT_EQUAL( 0,
                       rtcpPacket.header.padding );
    TEST_ASSERT_EQUAL( 2,
                       rtcpPacket.header.receptionReportCount );  /* Feedback message type (FMT) */
    TEST_ASSERT_EQUAL( RTCP_PACKET_RECEIVER_REPORT,
                       rtcpPacket.header.packetType );
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

    /* Payload length less than RTCP_FIR_PACKET_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseFirPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpFirPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 6;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    /* Packet type not RTCP_PACKET_FIR. */
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

    /* Payload length less than RTCP_PLI_PACKET_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParsePliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpPliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 10;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    /* Packet type not RTCP_PACKET_PAYLOAD_FEEDBACK_PLI. */
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
    uint8_t pliPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x87, 0x65, 0x43, 0x21  /* Media Source SSRC. */
    };
    size_t pliPacketPayloadLength = sizeof( pliPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 1;
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

    /* Payload length less than RTCP_SLI_PACKET_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 14;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    /* Packet type not RTCP_PACKET_PAYLOAD_FEEDBACK_SLI. */
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
void test_rtcpParseSliPacket_NullInfo( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpSliPacket_t rtcpSliPacket;
    RtcpResult_t result;
    uint8_t sliPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x87, 0x65, 0x43, 0x21, /* Media Source SSRC */
        /* SLI Info 1: First = 7191, Number = 6242, Picture ID = 31. */
        0xE0, 0xBE, 0x18, 0x9F,
    };

    /*
     *  Even though in the payload an SLI Info is going, since the rtcpSliPacket has pSliInfos as NULL, no such SLI Info's are retrieved.
     */
    size_t sliPacketPayloadLength = sizeof( sliPacketPayload );


    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_SLI;
    rtcpPacket.pPayload = &( sliPacketPayload[ 0 ] );
    rtcpPacket.payloadLength = sliPacketPayloadLength;

    rtcpSliPacket.pSliInfos = NULL;
    rtcpSliPacket.numSliInfos = 0;

    result = Rtcp_ParseSliPacket( &( context ),
                                  &( rtcpPacket ),
                                  &( rtcpSliPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpSliPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x87654321,
                       rtcpSliPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0,
                       rtcpSliPacket.numSliInfos );
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
    uint32_t sliInfo[ 2 ];
    RtcpSliPacket_t rtcpSliPacket;
    uint8_t sliPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x87, 0x65, 0x43, 0x21, /* Media Source SSRC. */
        /* SLI Info 1: First = 7191, Number = 6242, Picture ID = 31. */
        0xE0, 0xBE, 0x18, 0x9F,
        /* SLI Info 2: First = 5287, Number = 6541, Picture ID = 28. */
        0xA5, 0x3E, 0x63, 0x5C,
    };

    /*
     * SLI Info 1:
     *
     * First [13 Bits]     = 7191 = 1110 0000 1011 1
     * Number [13 Bits]    = 6242 = 110 0001 1000 10
     * Picture ID [6 Bits  = 31   = 01 1111
     *
     * The combined SLI Info 2 field look like:
     *
     * Binary = 1110 0000 1011 1110 0001 1000 1001 1111
     * Hex    = E    0    B    E    1    8    9    F
     *
     * Hence the SLI Info value = 0xE0BE189F.
     */
    size_t sliPacketPayloadLength = sizeof( sliPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.header.padding = 0;
    rtcpPacket.header.receptionReportCount = 0;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_SLI;
    rtcpPacket.pPayload = &( sliPacketPayload[ 0 ] );
    rtcpPacket.payloadLength = sliPacketPayloadLength;

    rtcpSliPacket.pSliInfos = &( sliInfo[ 0 ] );
    rtcpSliPacket.numSliInfos = 2;

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
    TEST_ASSERT_EQUAL( 0xE0BE189F,
                       rtcpSliPacket.pSliInfos[ 0 ] );
    TEST_ASSERT_EQUAL( 0xA53E635C,
                       rtcpSliPacket.pSliInfos[ 1 ] );
    TEST_ASSERT_EQUAL( 7191,
                       RTCP_SLI_INFO_EXTRACT_FIRST( rtcpSliPacket.pSliInfos[ 0 ] ) );
    TEST_ASSERT_EQUAL( 6242,
                       RTCP_SLI_INFO_EXTRACT_NUMBER( rtcpSliPacket.pSliInfos[ 0 ] ) );
    TEST_ASSERT_EQUAL( 31,
                       RTCP_SLI_INFO_EXTRACT_PICTURE_ID( rtcpSliPacket.pSliInfos[ 0 ] ) );
    TEST_ASSERT_EQUAL( 5287,
                       RTCP_SLI_INFO_EXTRACT_FIRST( rtcpSliPacket.pSliInfos[ 1 ] ) );
    TEST_ASSERT_EQUAL( 6541,
                       RTCP_SLI_INFO_EXTRACT_NUMBER( rtcpSliPacket.pSliInfos[ 1 ] ) );
    TEST_ASSERT_EQUAL( 28,
                       RTCP_SLI_INFO_EXTRACT_PICTURE_ID( rtcpSliPacket.pSliInfos[ 1 ] ) );
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
    /* Packet type not RTCP_PACKET_PAYLOAD_FEEDBACK_REMB. */
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
    RtcpResult_t result;
    uint8_t rembPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        0x52, 0x45, 0x4D, 0x42  /* REMB Unique Identifier. */
    };
    size_t rembPacketPayloadLength = sizeof( rembPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    /* Payload less than RTCP_REMB_PACKET_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = rembPacketPayloadLength;
    rtcpPacket.pPayload = &( rembPacketPayload[ 0 ] );
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
    RtcpResult_t result;
    uint8_t rembPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        0x52, 0x45, 0x4D, 0x43, /* Incorrect REMB Unique Identifier. */
        /* Num SSRC = 2, BR Exp = 31, BR Mantissa = 8712. */
        0x02, 0xC5, 0x87, 0x12,
        0x01, 0x02, 0x03, 0x04, /* SSRC 1. */
        0x11, 0x22, 0x33, 0x44, /* SSRC 2. */
    };
    size_t rembPacketPayloadLength = sizeof( rembPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = rembPacketPayloadLength;
    rtcpPacket.pPayload = &( rembPacketPayload[ 0 ] );
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
    RtcpResult_t result;
    uint32_t ssrcList[ 1 ];
    RtcpRembPacket_t rtcpRembPacket;
    uint8_t rembPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        0x52, 0x45, 0x4D, 0x42, /* REMB Unique Identifier. */
        /* Num SSRC = 2, BR Exp = 31, BR Mantissa = 8712. */
        0x02, 0xC5, 0x87, 0x12,
        0x01, 0x02, 0x03, 0x04, /* SSRC 1. */
        0x05, 0x06, 0x07, 0x08  /* SSRC 2. */
    };
    size_t rembPacketPayloadLength = sizeof( rembPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = rembPacketPayloadLength;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;

    rtcpRembPacket.pSsrcList = &( ssrcList[ 0 ] );
    rtcpRembPacket.ssrcListLength = 1; /* SRC list too small. */

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_INPUT_REMB_PACKET_INVALID,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Remb Packet functionality.
 */
void test_rtcpParseRembPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    uint32_t ssrcList[ 2 ];
    RtcpRembPacket_t rtcpRembPacket;
    uint8_t rembPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        0x52, 0x45, 0x4D, 0x42, /* REMB Unique Identifier. */
        /* Num SSRC = 2, BR Exp = 49, BR Mantissa = 34578. */
        0x02, 0xC4, 0x87, 0x12,
        0x01, 0x02, 0x03, 0x04, /* SSRC 1. */
        0x05, 0x06, 0x07, 0x08  /* SSRC 2. */
    };
    size_t rembPacketPayloadLength = sizeof( rembPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = rembPacketPayloadLength;
    rtcpPacket.pPayload = rembPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_PAYLOAD_FEEDBACK_REMB;

    rtcpRembPacket.pSsrcList = &( ssrcList[ 0 ] );
    rtcpRembPacket.ssrcListLength = 2;

    result = Rtcp_ParseRembPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpRembPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpRembPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpRembPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 49,
                       rtcpRembPacket.bitRateExponent );
    TEST_ASSERT_EQUAL( 34578,
                       rtcpRembPacket.bitRateMantissa );
    TEST_ASSERT_EQUAL( 2,
                       rtcpRembPacket.ssrcListLength );
    TEST_ASSERT_EQUAL( 0x01020304,
                       rtcpRembPacket.pSsrcList[ 0 ] );
    TEST_ASSERT_EQUAL( 0x05060708,
                       rtcpRembPacket.pSsrcList[ 1 ] );
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

    /* Payload length less than RTCP_SENDER_REPORT_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( senderReportPayload[ 0 ] );

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 25;
    rtcpPacket.pPayload = &( senderReportPayload[ 0 ] );
    /* Packet type not RTCP_PACKET_SENDER_REPORT. */
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for out of memory.
 */
void test_rtcpParseSenderReport_OutOfMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 1 ];
    RtcpSenderReport_t rtcpSenderReport;
    uint8_t senderReportPayload[] =
    {
        0x12, 0x34, 0x56, 0x78,                         /* Sender SSRC. */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime). */
        0x99, 0xAA, 0xBB, 0xCC,                         /* Sender Info (rtpTime). */
        0x00, 0x00, 0x03, 0xE8,                         /* Sender Info (packetCount). */
        0x00, 0x01, 0x86, 0xA0,                         /* Sender Info (octetCount). */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01,                         /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D,                         /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02,                         /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D,                         /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t senderReportPayloadLength = sizeof( senderReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = senderReportPayloadLength;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 2;
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    rtcpSenderReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpSenderReport.numReceptionReports = 1; /* Too small. */

    result = Rtcp_ParseSenderReport( &( context ),
                                     &( rtcpPacket ),
                                     &( rtcpSenderReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Sender Report fail functionality for malformed packet.
 */
void test_rtcpParseSenderReport_MalformedPacket( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    RtcpSenderReport_t rtcpSenderReport;
    uint8_t senderReportPayload[] =
    {
        0x12, 0x34, 0x56, 0x78,                         /* Sender SSRC. */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime). */
        0x99, 0xAA, 0xBB, 0xCC,                         /* Sender Info (rtpTime). */
        0x00, 0x00, 0x03, 0xE8,                         /* Sender Info (packetCount). */
        0x00, 0x01, 0x86, 0xA0,                         /* Sender Info (octetCount). */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01,                         /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D,                         /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t senderReportPayloadLength = sizeof( senderReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = senderReportPayloadLength;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 2; /* Malformed because the payload only contains one report. */
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    rtcpSenderReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpSenderReport.numReceptionReports = 2;

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
    uint32_t i;
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    RtcpSenderReport_t rtcpSenderReport;
    uint8_t senderReportPayload[] =
    {
        0x12, 0x34, 0x56, 0x78,                         /* Sender SSRC. */
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, /* Sender Info (ntpTime). */
        0x99, 0xAA, 0xBB, 0xCC,                         /* Sender Info (rtpTime). */
        0x00, 0x00, 0x03, 0xE8,                         /* Sender Info (packetCount). */
        0x00, 0x01, 0x86, 0xA0,                         /* Sender Info (octetCount). */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01,                         /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D,                         /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 2. */
        0x00, 0x00, 0x00, 0x02,                         /* SSRC of second source. */
        0x11, 0xA0, 0xA1, 0xA2,                         /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4,                         /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4,                         /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4,                         /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D,                         /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t senderReportPayloadLength = sizeof( senderReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = senderReportPayloadLength;
    rtcpPacket.pPayload = senderReportPayload;
    rtcpPacket.header.receptionReportCount = 2;
    rtcpPacket.header.packetType = RTCP_PACKET_SENDER_REPORT;

    rtcpSenderReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpSenderReport.numReceptionReports = 2;

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
    TEST_ASSERT_EQUAL( 2,
                       rtcpSenderReport.numReceptionReports );
    TEST_ASSERT_EQUAL( 0x00000001,
                       rtcpSenderReport.pReceptionReports[ 0 ].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000002,
                       rtcpSenderReport.pReceptionReports[ 1 ].sourceSsrc );

    for( i = 0; i < 2; i++ )
    {
        TEST_ASSERT_EQUAL( 0x11,
                           rtcpSenderReport.pReceptionReports[ i ].fractionLost );
        TEST_ASSERT_EQUAL( 0xA0A1A2,
                           rtcpSenderReport.pReceptionReports[ i ].cumulativePacketsLost );
        TEST_ASSERT_EQUAL( 0xD1D2D3D4,
                           rtcpSenderReport.pReceptionReports[ i ].extendedHighestSeqNumReceived );
        TEST_ASSERT_EQUAL( 0xB1B2B3B4,
                           rtcpSenderReport.pReceptionReports[ i ].interArrivalJitter );
        TEST_ASSERT_EQUAL( 0xC1C2C3C4,
                           rtcpSenderReport.pReceptionReports[ i ].lastSR );
        TEST_ASSERT_EQUAL( 0x5A5B5C5D,
                           rtcpSenderReport.pReceptionReports[ i ].delaySinceLastSR );
    }
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

    /* Payload length less than RTCP_RECEIVER_REPORT_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( receiverReportPayload[ 0 ] );

    result = Rtcp_ParseReceiverReport( &( context ),
                                       &( rtcpPacket ),
                                       &( rtcpReceiverReport ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 10;
    rtcpPacket.pPayload = &( receiverReportPayload[ 0 ] );
    /* Packet type not RTCP_PACKET_RECEIVER_REPORT. */
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
    RtcpReceptionReport_t receptionReports[ 2 ];
    RtcpReceiverReport_t rtcpReceiverReport;
    uint8_t receiverReportPayload[] =
    {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x02, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t receiverReportPayloadLength = sizeof( receiverReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = receiverReportPayloadLength;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2;

    rtcpReceiverReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpReceiverReport.numReceptionReports = 1; /* Too small. */

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
    RtcpReceptionReport_t receptionReports[ 2 ];
    RtcpReceiverReport_t rtcpReceiverReport;
    uint8_t receiverReportPayload[] =
    {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
        /* Reception Report 1. */
        0x00, 0x00, 0x00, 0x01, /* SSRC of first source. */
        0x11, 0xA0, 0xA1, 0xA2, /* Fraction lost = 0x11, Cumulative packet lost = 0xA0A1A2. */
        0xD1, 0xD2, 0xD3, 0xD4, /* Extended highest sequence number received = 0xD1D2D3D4. */
        0xB1, 0xB2, 0xB3, 0xB4, /* Inter-arrival Jitter = 0xB1B2B3B4. */
        0xC1, 0xC2, 0xC3, 0xC4, /* Last SR = 0xC1C2C3C4. */
        0x5A, 0x5B, 0x5C, 0x5D, /* Delay since last SR = 0x5A5B5C5D. */
    };
    size_t receiverReportPayloadLength = sizeof( receiverReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = receiverReportPayloadLength;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2; /* Malformed because the payload only contains one report. */

    rtcpReceiverReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpReceiverReport.numReceptionReports = 2;

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
    uint32_t i;
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpResult_t result;
    RtcpReceptionReport_t receptionReports[ 2 ];
    RtcpReceiverReport_t rtcpReceiverReport;
    uint8_t receiverReportPayload[] =
    {
        0x87, 0x65, 0x43, 0x21, /* Sender SSRC. */
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
    size_t receiverReportPayloadLength = sizeof( receiverReportPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = receiverReportPayloadLength;
    rtcpPacket.pPayload = receiverReportPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_RECEIVER_REPORT;
    rtcpPacket.header.receptionReportCount = 2;

    rtcpReceiverReport.pReceptionReports = &( receptionReports[ 0 ] );
    rtcpReceiverReport.numReceptionReports = 2;

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
                       rtcpReceiverReport.pReceptionReports[ 0 ].sourceSsrc );
    TEST_ASSERT_EQUAL( 0x00000002,
                       rtcpReceiverReport.pReceptionReports[ 1 ].sourceSsrc );

    for( i = 0; i < 2; i++ )
    {
        TEST_ASSERT_EQUAL( 0x11,
                           rtcpReceiverReport.pReceptionReports[ i ].fractionLost );
        TEST_ASSERT_EQUAL( 0xA0A1A2,
                           rtcpReceiverReport.pReceptionReports[ i ].cumulativePacketsLost );
        TEST_ASSERT_EQUAL( 0xD1D2D3D4,
                           rtcpReceiverReport.pReceptionReports[ i ].extendedHighestSeqNumReceived );
        TEST_ASSERT_EQUAL( 0xB1B2B3B4,
                           rtcpReceiverReport.pReceptionReports[ i ].interArrivalJitter );
        TEST_ASSERT_EQUAL( 0xC1C2C3C4,
                           rtcpReceiverReport.pReceptionReports[ i ].lastSR );
        TEST_ASSERT_EQUAL( 0x5A5B5C5D,
                           rtcpReceiverReport.pReceptionReports[ i ].delaySinceLastSR );
    }
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

    /* Payload length less than RTCP_NACK_PACKET_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( nackPacketPayload[ 0 ] );

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    /* Payload length not a multiple of 4. */
    rtcpPacket.payloadLength = 14;
    rtcpPacket.pPayload = &( nackPacketPayload[ 0 ] );

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 40;
    rtcpPacket.pPayload = &( nackPacketPayload[ 0 ] );
    /* Packet type not RTCP_PACKET_TRANSPORT_FEEDBACK_NACK. */
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
    RtcpNackPacket_t rtcpNackPacket;
    uint8_t nackPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        /* NACK: PID = 0x063B, BLP = 0x0A09. */
        0x06, 0x3B, 0x0A, 0x09
    };

    /*
     * NACK details:
     *
     * PID = 0x063B = 1595.
     * BLP = 0x0A09 = 0000 1010 0000 1001.
     *
     * Bits which are set in BLP = 1, 4, 10, 12.
     *
     * Total PIDs reported in this NACK report:
     *  - PID      = 1595
     *  - PID + 1  = 1596
     *  - PID + 4  = 1599
     *  - PID + 10 = 1605
     *  - PID + 12 = 1607
     */
    size_t nackPacketPayloadLength = sizeof( nackPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = nackPacketPayloadLength;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    rtcpNackPacket.pSeqNumList = &( seqNumList[ 0 ] );
    rtcpNackPacket.seqNumListLength = 5;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpNackPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpNackPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 5,
                       rtcpNackPacket.seqNumListLength );
    TEST_ASSERT_EQUAL( 1595,
                       rtcpNackPacket.pSeqNumList[ 0 ] );
    TEST_ASSERT_EQUAL( 1596,
                       rtcpNackPacket.pSeqNumList[ 1 ] );
    TEST_ASSERT_EQUAL( 1599,
                       rtcpNackPacket.pSeqNumList[ 2 ] );
    TEST_ASSERT_EQUAL( 1605,
                       rtcpNackPacket.pSeqNumList[ 3 ] );
    TEST_ASSERT_EQUAL( 1607,
                       rtcpNackPacket.pSeqNumList[ 4 ] );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Nack Packet functionality.
 */
void test_rtcpParseNackPacket_NullSequenceList( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpNackPacket_t rtcpNackPacket;
    RtcpResult_t result;
    uint8_t nackPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        /* PID = 0x0001, BLP = 0x0003. Total lost PIDs = 3 ( PID + 2 in BLP ). */
        0x00, 0x01, 0x00, 0x03,
        /* PID = 0x0004, BLP = 0x0001. Total lost PIDs = 2 ( PID + 1 in BLP ). */
        0x00, 0x04, 0x00, 0x01,
    };
    size_t nackPacketPayloadLength = sizeof( nackPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = nackPacketPayloadLength;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    rtcpNackPacket.pSeqNumList = NULL;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpNackPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpNackPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 5,
                       rtcpNackPacket.seqNumListLength );
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
    uint16_t seqNumList[ 3 ];
    RtcpNackPacket_t rtcpNackPacket;
    uint8_t nackPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC. */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC. */
        /* PID = 0x0001, BLP = 0x0003. Total lost PIDs = 3 ( PID + 2 in BLP ). */
        0x00, 0x01, 0x00, 0x03,
        /* PID = 0x0004, BLP = 0x0001. Total lost PIDs = 2 ( PID + 1 in BLP ). */
        0x00, 0x04, 0x00, 0x01,
    };
    size_t nackPacketPayloadLength = sizeof( nackPacketPayload );

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    rtcpPacket.payloadLength = nackPacketPayloadLength;
    rtcpPacket.pPayload = nackPacketPayload;
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_NACK;

    rtcpNackPacket.pSeqNumList = &( seqNumList[ 0 ] );
    /* Out of memory when parsing PID. */
    rtcpNackPacket.seqNumListLength = 3;

    result = Rtcp_ParseNackPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpNackPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );

    /* Out of memory when parsing BLP. */
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

    /* Payload length less than RTCP_TWCC_PACKET_MIN_PAYLOAD_LENGTH. */
    rtcpPacket.payloadLength = 2;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );

    rtcpPacket.payloadLength = 20;
    rtcpPacket.pPayload = &( payloadBuffer[ 0 ] );
    /* Packet type not RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC. */
    rtcpPacket.header.packetType = RTCP_PACKET_UNKNOWN;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with Malformed Small Delta.
 */
void test_rtcpParseTwccPacket_RunLengthChunk_SmallDelta_Malformed( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x02,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x01, /* Reference Time (0), Feedback Packet Count (1) */
        0x20, 0x02,             /* Packet Status (small Delta), Run Length Chunk */
        0x02
    };

    /*
     * Packet Chunk :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|0 1|0 0 0 0 0 0 0 0 0 0 0 1 0|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * Since there are 2 small Delta's packets received, but in the payload there is not enough "recv delta" for both the received packets.
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with Malformed Big Delta.
 */
void test_rtcpParseTwccPacket_RunLengthChunk_BigDelta_Malformed( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x02,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x01, /* Reference Time (0), Feedback Packet Count (1) */
        0x40, 0x01,             /* Packet Status (Big Delta), Run Length Chunk */
        0x80,                   /* Large Receive Delta (0x80 = -128 * 64ms = -2097088ms) */
    };

    /*
     * Packet Chunk :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|1 0|0 0 0 0 0 0 0 0 0 0 0 0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * Since there is 1 Big Delta packet received, but in the payload there is not enough "recv delta" for the received packets ( Since 1 Big-Delta packet needs a 16 bit "recv delta" ).
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with Out of Memory.
 */
void test_rtcpParseTwccPacket_RunLengthChunk_OutOfMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    PacketArrivalInfo_t packetArrivalInfo[ 7 ];
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x04,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x01, /* Reference Time (0), Feedback Packet Count (1) */
        0x00, 0xDD,             /* Packet Status (Not Received), Run Length Chunk */
        0x20, 0x02,             /* Packet Status (small Delta), Run Length Chunk */
        0x40, 0x01,             /* Packet Status (Big Delta), Run Length Chunk */
        /* recv delta */
        0x02, 0x01,             /* First Receive Delta (2 * 64ms = 128ms) */ /* Second Receive Delta (1 * 64ms = 64ms) */
        0x80, 0x01              /* Large Receive Delta (0x8001 = -32767 * 64ms = -2097088ms) */
    };

    /*
     * rtcpTwccPacket Info List Length is 7, but the number of arrival info's are far greater than 7 thus out of memory.
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = &( packetArrivalInfo[ 0 ] );
    rtcpTwccPacket.arrivalInfoListLength = 7;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with non-empty ArrivalInfoList.
 */
void test_rtcpParseTwccPacket_RunLengthChunk_ArrivalInfoList( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    PacketArrivalInfo_t packetArrivalInfo[ 224 ];
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0xE0,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time (0), Feedback Packet Count (2) */
        0x00, 0xDD,             /* Packet Status (Not Received), Run Length Chunk */
        0x20, 0x02,             /* Packet Status (small Delta), Run Length Chunk */
        0x40, 0x01,             /* Packet Status (Big Delta), Run Length Chunk */
        /* recv delta */
        0x02, 0x01,             /* First Receive Delta (2 * 64ms = 128ms) */ /* Second Receive Delta (1 * 64ms = 64ms) */
        0x80, 0x01              /* Large Receive Delta (0x8001 = -32767 * 64ms = -2097088ms) */
    };

    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = &( packetArrivalInfo[ 0 ] );
    rtcpTwccPacket.arrivalInfoListLength = 224;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 224,
                       rtcpTwccPacket.packetStatusCount );
    TEST_ASSERT_EQUAL( 0,
                       rtcpTwccPacket.referenceTime );
    TEST_ASSERT_EQUAL( 2,
                       rtcpTwccPacket.feedbackPacketCount );
    TEST_ASSERT_EQUAL( 224,
                       rtcpTwccPacket.arrivalInfoListLength );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.pArrivalInfoList[ 0 ].seqNum );
    TEST_ASSERT_EQUAL( RTCP_TWCC_PACKET_LOST_TIME,
                       rtcpTwccPacket.pArrivalInfoList[ 0 ].remoteArrivalTime );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with EMPTY ArrivalInfoList.
 */
void test_rtcpParseTwccPacket_RunLengthChunk( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0xE0,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x03, /* Reference Time ( 0 ), Feedback Packet Count ( 3 ) */
        0x00, 0xDD,             /* Packet Status ( Not Received ), Run Length Chunk */
        0x20, 0x02,             /* Packet Status ( small Delta ), Run Length Chunk */
        0x40, 0x01,             /* Packet Status ( Big Delta ), Run Length Chunk */
        /* recv delta */
        0x02, 0x01,             /* First Receive Delta ( 2 * 64ms = 128ms ) */ /* Second Receive Delta ( 1 * 64ms = 64ms ) */
        0x80, 0x01,             /* Large Receive Delta ( 0x8001 = -32767 * 64ms = -2097088ms ) */
    };

    /*
     *  Packet Chunk ( Not Received - 0x00DD ) :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|0 0|0 0 0 0 0 1 1 0 1 1 1 0 1|                  Total 221 Packet status as not received
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     *  Packet Chunk ( Small Detla - 0x2002 ) :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|0 1|0 0 0 0 0 0 0 0 0 0 0 1 0|                  Total 2 Packet status as Small Delta
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     *  Packet Chunk ( Big Detla - 0x4001 ) :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|1 0|0 0 0 0 0 0 0 0 0 0 0 0 1|                  Total 1 Packet status as Big Delta
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     *  Hence total of 224 Status Info received.
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 224,
                       rtcpTwccPacket.packetStatusCount );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Run Length with Reserved Packets.
 */
void test_rtcpParseTwccPacket_RunLengthChunk_ReservedPackets( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x01,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x01, /* Reference Time (0), Feedback Packet Count (1) */
        0x60, 0x01,             /* Packet Status (Reserved), Run Length Chunk */
    };

    /*
     * Packet Chunk :
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |0|1 1|0 0 0 0 0 0 0 0 0 0 0 0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * Since there is 1 reserved packet received. Since Status symbol for 0x6001 is 11 [Reserved]
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 1,
                       rtcpTwccPacket.packetStatusCount );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with Malformed Small Delta.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_SmallDelta_Makformed( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x07,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x07, /* Reference Time ( 0 ), Feedback Packet Count ( 7 ) */
        0xD1, 0x05,             /* Packet Status ( small Delta ), Status Vector Chunk */
        /* recv delta */
        0x01,                   /* First Receive Delta ( 1 * 64ms = 64ms ) */
        0x02,                   /* First Receive Delta ( 2 * 64ms = 128ms ) */
    };

    /*
     * Packet Chunk (Small Detla - 0xD105):
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |1|1|0 1|0 0|0 1|0 0|0 0|0 1|0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * Since there is 4 Small Delta packet received, but in the payload there is not enough "recv delta" for the received packets.
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with Malformed Big Delta.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_BigDelta_Malformed( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x02,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time ( 0 ), Feedback Packet Count ( 2 ) */
        0xE0, 0x02,             /* Packet Status ( Big Delta ), Status Vector Chunk */
        0x80,                   /* Large Receive Delta  */
        /* No Receive Delta for the second BIG DELTA Packet received. */
    };

    /*
     * Packet Chunk (Big Detla - 0xE002):
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |1|1|1 0|0 0|0 0|0 0|0 0|0 0|1 0|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * Since there is 2 Big Delta packet received, but in the payload there is not enough "recv delta" for the received packets.
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_MALFORMED_PACKET,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with Basic Symbol.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_BasicSymbol( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x07,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time ( 0 ), Feedback Packet Count ( 2 ) */
        0x84, 0x01,             /* Packet Status (2 received ), Status Vector Chunk */
        /* recv delta */
        0x01, 0x02              /* Delta's for both the packets received */
    };

    /*
     * Packet Chunk ( Basic Symbol - 0x8401 { Basic Symbol implies that second bit is set to 0, so only packet "received" or "not-received" info is transferred.} ):
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |1|0|0 0 0 1 0 0 0 0 0 0 0 0 0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     *  There is 2 packet received.
     *
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 7,
                       rtcpTwccPacket.packetStatusCount );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with Detailed Symbol.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_DetailSymbol( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x07,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time ( 0 ), Feedback Packet Count ( 2 ) */
        0xED, 0x01,             /* Status Vector Chunk */
        /* recv delta */
        0x80, 0x01,             /* Large Receive Delta ( 0x8001 = -32767 * 64ms = -2097088ms ) */
        0x01, 0x02              /* First Receive Delta ( 1 * 64ms = 64ms ) */   /* Second Receive Delta ( 2 * 64ms = 128ms ) */
    };

    /*
     * Packet Chunk (0xED01):
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |1|1|1 0|1 1|0 1|0 0|0 0|0 0|0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * 1x Big-Delta Packet.
     * 1x Reserved Packet.
     * 1x Small-Delta Packet.
     * 3x Packet Not Received.
     * 1x Small-Delta Packet.
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = NULL;
    rtcpTwccPacket.arrivalInfoListLength = 0;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 7,
                       rtcpTwccPacket.packetStatusCount );
    TEST_ASSERT_EQUAL( 0,
                       rtcpTwccPacket.referenceTime );
    TEST_ASSERT_EQUAL( 2,
                       rtcpTwccPacket.feedbackPacketCount );
    TEST_ASSERT_EQUAL( 7,
                       rtcpTwccPacket.arrivalInfoListLength );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with OutofMemory.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_OutofMemory( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    PacketArrivalInfo_t packetArrivalInfo[ 6 ];
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x07,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time ( 0 ), Feedback Packet Count ( 2 ) */
        0xE1, 0x01,             /* Status Vector Chunk */
        /* recv delta */
        0x80, 0x01,
        0x01, 0x02
    };

    /*
     * Packet Chunk ( 0xE101 ):
     *
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |1|1|1 0|0 0|0 1|0 0|0 0|0 0|0 1|
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *
     * 1x Big-Delta Packet.
     * 1x Packet Not Received.
     * 1x Small-Delta Packet.
     * 3x Packet Not Received.
     * 1x Small-Delta Packet.
     *
     * Since status of 7 packets is received, but the pArrivalInfoList has size of 6 packets. Hence Out of Memory.
     */
    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = &( packetArrivalInfo[ 0 ] );
    rtcpTwccPacket.arrivalInfoListLength = 6;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OUT_OF_MEMORY,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate RTCP Parse Twcc Packet functionality for Status Vector Chunk with Arrival Info List.
 */
void test_rtcpParseTwccPacket_StatusVectorChunk_ArrivalInfoList( void )
{
    RtcpContext_t context;
    RtcpPacket_t rtcpPacket;
    PacketArrivalInfo_t packetArrivalInfo[ 7 ];
    RtcpTwccPacket_t rtcpTwccPacket;
    RtcpResult_t result;

    result = Rtcp_Init( &( context ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );

    uint8_t twccPacketPayload[] =
    {
        0x12, 0x34, 0x56, 0x78, /* Sender SSRC */
        0x9A, 0xBC, 0xDE, 0xF0, /* Media Source SSRC */
        0x00, 0x01,             /* Base Sequence Number */
        0x00, 0x07,             /* Packet Status Count */
        0x00, 0x00, 0x00, 0x02, /* Reference Time (0), Feedback Packet Count (2) */
        0xE1, 0x01,             /* Status Vector Chunk */
        /* recv delta */
        0x80, 0x01,
        0x01, 0x02
    };

    rtcpPacket.pPayload = twccPacketPayload;
    rtcpPacket.payloadLength = sizeof( twccPacketPayload );
    rtcpPacket.header.packetType = RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC;

    rtcpTwccPacket.pArrivalInfoList = &( packetArrivalInfo[ 0 ] );      /* A non NULL arrivalInfo List */
    rtcpTwccPacket.arrivalInfoListLength = 7;

    result = Rtcp_ParseTwccPacket( &( context ),
                                   &( rtcpPacket ),
                                   &( rtcpTwccPacket ) );

    TEST_ASSERT_EQUAL( RTCP_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 0x12345678,
                       rtcpTwccPacket.senderSsrc );
    TEST_ASSERT_EQUAL( 0x9ABCDEF0,
                       rtcpTwccPacket.mediaSourceSsrc );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.baseSeqNum );
    TEST_ASSERT_EQUAL( 7,
                       rtcpTwccPacket.packetStatusCount );
    TEST_ASSERT_EQUAL( 0,
                       rtcpTwccPacket.referenceTime );
    TEST_ASSERT_EQUAL( 2,
                       rtcpTwccPacket.feedbackPacketCount );
    TEST_ASSERT_EQUAL( 7,
                       rtcpTwccPacket.arrivalInfoListLength );
    TEST_ASSERT_EQUAL( 0x0001,
                       rtcpTwccPacket.pArrivalInfoList[ 0 ].seqNum );
    TEST_ASSERT_EQUAL( 293,
                       rtcpTwccPacket.pArrivalInfoList[ 0 ].remoteArrivalTime );
}

/*-----------------------------------------------------------*/
