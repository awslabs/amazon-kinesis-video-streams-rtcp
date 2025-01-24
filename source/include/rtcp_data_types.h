#ifndef RTCP_DATA_TYPES_H
#define RTCP_DATA_TYPES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* Endianness includes. */
#include "rtcp_endianness.h"

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

#define RTCP_TWCC_PACKET_LOST_TIME                  ( ( uint64_t ) ( -1 ) )
#define RTCP_TWCC_PACKET_UNINITIALIZED_TIME         0
#define RTCP_TWCC_HUNDREDS_OF_NANOS_IN_A_SECOND     10000000
#define RTCP_TWCC_ESTIMATOR_TIME_WINDOW             ( 1 * RTCP_TWCC_HUNDREDS_OF_NANOS_IN_A_SECOND )

/*-----------------------------------------------------------*/

typedef enum RtcpResult
{
    RTCP_RESULT_OK,
    RTCP_RESULT_BAD_PARAM,
    RTCP_RESULT_OUT_OF_MEMORY,
    RTCP_RESULT_WRONG_VERSION,
    RTCP_RESULT_MALFORMED_PACKET,
    RTCP_RESULT_INPUT_PACKET_TOO_SMALL,
    RTCP_RESULT_INPUT_REMB_PACKET_INVALID
} RtcpResult_t;

typedef enum RtcpPacketType
{
    RTCP_PACKET_UNKNOWN,
    RTCP_PACKET_FIR,
    RTCP_PACKET_SENDER_REPORT,
    RTCP_PACKET_RECEIVER_REPORT,
    RTCP_PACKET_PAYLOAD_FEEDBACK_PLI,
    RTCP_PACKET_PAYLOAD_FEEDBACK_SLI,
    RTCP_PACKET_PAYLOAD_FEEDBACK_REMB,
    RTCP_PACKET_TRANSPORT_FEEDBACK_NACK,
    RTCP_PACKET_TRANSPORT_FEEDBACK_TWCC
} RtcpPacketType_t;

/*-----------------------------------------------------------*/

typedef struct RtcpContext
{
    RtcpReadWriteFunctions_t readWriteFunctions;
} RtcpContext_t;

typedef struct RtcpHeader
{
    uint8_t padding;
    uint8_t receptionReportCount;
    RtcpPacketType_t packetType;
} RtcpHeader_t;

/* Need to use struct RtcpDataPacket here instead of struct RtcpPacket as per the
 * naming convention, to avoid conflict with an existing struct in the current
 * WebRTC SDK. */
typedef struct RtcpDataPacket
{
    RtcpHeader_t header;
    const uint8_t * pPayload;
    size_t payloadLength;
} RtcpPacket_t;

/*-----------------------------------------------------------*/

/*
 * Sender Report (SR) RTCP Packet:
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.1
 *
 *         0                   1                   2                   3
 *         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * header |V=2|P|    RC   |   PT=SR=200   |             length            |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                         SSRC of sender                        |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * sender |              NTP timestamp, most significant word             |
 * info   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |             NTP timestamp, least significant word             |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                         RTP timestamp                         |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                     sender's packet count                     |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                      sender's octet count                     |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * report |                 SSRC_1 (SSRC of first source)                 |
 * block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   1    | fraction lost |       cumulative number of packets lost       |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |           extended highest sequence number received           |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                      interarrival jitter                      |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                         last SR (LSR)                         |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                   delay since last SR (DLSR)                  |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * report |                 SSRC_2 (SSRC of second source)                |
 * block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   2    :                               ...                             :
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *        |                  profile-specific extensions                  |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct RtcpSenderInfo
{
    uint64_t ntpTime;
    uint32_t rtpTime;
    uint32_t packetCount;
    uint32_t octetCount;
} RtcpSenderInfo_t;

typedef struct RtcpReceptionReport
{
    uint32_t sourceSsrc;
    uint8_t fractionLost;
    uint32_t cumulativePacketsLost;
    uint32_t extendedHighestSeqNumReceived;
    uint32_t interArrivalJitter;
    uint32_t lastSR;
    uint32_t delaySinceLastSR;
} RtcpReceptionReport_t;

typedef struct RtcpSenderReport
{
    uint32_t senderSsrc;
    RtcpSenderInfo_t senderInfo;
    RtcpReceptionReport_t * pReceptionReports;
    uint8_t numReceptionReports;
} RtcpSenderReport_t;

/*-----------------------------------------------------------*/

/*
 * Receiver Report (RR) RTCP Packet:
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.2
 *
 *         0                   1                   2                   3
 *         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * header |V=2|P|    RC   |   PT=RR=201   |             length            |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                     SSRC of packet sender                     |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * report |                 SSRC_1 (SSRC of first source)                 |
 * block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   1    | fraction lost |       cumulative number of packets lost       |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |           extended highest sequence number received           |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                      interarrival jitter                      |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                         last SR (LSR)                         |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                   delay since last SR (DLSR)                  |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 * report |                 SSRC_2 (SSRC of second source)                |
 * block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   2    :                               ...                             :
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *        |                  profile-specific extensions                  |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct RtcpReceiverReport
{
    uint32_t senderSsrc;
    RtcpReceptionReport_t * pReceptionReports;
    uint8_t numReceptionReports;
} RtcpReceiverReport_t;

/*-----------------------------------------------------------*/

/*
 * Full INTRA-frame Request (FIR) packet:
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc2032#section-5.2.1
 *
 *  0                     1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|   MBZ   |  PT=RTCP_FIR  |           length              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                              SSRC                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct RtcpFirPacket
{
    uint32_t senderSsrc;
} RtcpFirPacket_t;

/*-----------------------------------------------------------*/

/*
 * Picture Loss Indication (PLI):
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.3.1
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|  FMT=1  |     PT=206    |          length               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of packet sender                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of media source                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct RtcpPliPacket
{
    uint32_t senderSsrc;
    uint32_t mediaSourceSsrc;
} RtcpPliPacket_t;

/*-----------------------------------------------------------*/

/*
 * Slice Loss Indication (SLI):
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.3.2
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|  FMT=2  |     PT=206    |          length               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of packet sender                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of media source                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
 * |            First        |        Number           | PictureID |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    | n SLIs.
 * |            First        |        Number           | PictureID |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
 */
typedef struct RtcpSliPacket
{
    uint32_t senderSsrc;
    uint32_t mediaSourceSsrc;
    uint32_t * pSliInfos;
    size_t numSliInfos;
} RtcpSliPacket_t;

#define RTCP_SLI_INFO_FIRST_BITMASK         0xFFF80000
#define RTCP_SLI_INFO_FIRST_LOCATION        19

#define RTCP_SLI_INFO_NUMBER_BITMASK        0x0007FFC0
#define RTCP_SLI_INFO_NUMBER_LOCATION       6

#define RTCP_SLI_INFO_PICTURE_ID_BITMASK    0x0000003F
#define RTCP_SLI_INFO_PICTURE_ID_LOCATION   0

#define RTCP_SLI_INFO_EXTRACT_FIRST( sliInfo )  \
    ( ( ( sliInfo ) & RTCP_SLI_INFO_FIRST_BITMASK ) >> RTCP_SLI_INFO_FIRST_LOCATION )

#define RTCP_SLI_INFO_EXTRACT_NUMBER( sliInfo ) \
    ( ( ( sliInfo ) & RTCP_SLI_INFO_NUMBER_BITMASK ) >> RTCP_SLI_INFO_NUMBER_LOCATION )

#define RTCP_SLI_INFO_EXTRACT_PICTURE_ID( sliInfo ) \
    ( ( ( sliInfo ) & RTCP_SLI_INFO_PICTURE_ID_BITMASK ) >> RTCP_SLI_INFO_PICTURE_ID_LOCATION )

/*-----------------------------------------------------------*/

/*
 * Receiver Estimated Max Bitrate (REMB):
 *
 * RFC - https://datatracker.ietf.org/doc/html/draft-alvestrand-rmcat-remb-03#section-2.2
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P| FMT=15  |   PT=206      |             length            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of packet sender                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of media source                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Unique identifier 'R' 'E' 'M' 'B'                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Num SSRC     | BR Exp    |  BR Mantissa                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
 * |   SSRC feedback                                               |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    | n SSRCs.
 * |   SSRC feedback                                               |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
 */
typedef struct RtcpRembPacket
{
    uint32_t senderSsrc;
    uint32_t mediaSourceSsrc;
    uint32_t bitRateMantissa;
    uint8_t bitRateExponent;
    uint32_t * pSsrcList;
    uint8_t ssrcListLength;
} RtcpRembPacket_t;

/*-----------------------------------------------------------*/

/*
 * Generic NACK:
 *
 * RFC - https://datatracker.ietf.org/doc/html/rfc4585#section-6.2.1
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|  FMT=1  |     PT=205    |          length               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of packet sender                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  SSRC of media source                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
 * |            PID                |             BLP               |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    | n NACKs.
 * |            PID                |             BLP               |    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <--+
*/
typedef struct RtcpNackPacket
{
    uint32_t senderSsrc;
    uint32_t mediaSourceSsrc;
    uint16_t * pSeqNumList;
    size_t seqNumListLength;
} RtcpNackPacket_t;

/*-----------------------------------------------------------*/

/*
 * Transport-wide Congestion Control (TWCC):
 *
 * RFC - https://datatracker.ietf.org/doc/html/draft-holmer-rmcat-transport-wide-cc-extensions-01#section-3.1
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|  FMT=15 |    PT=205     |           length              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     SSRC of packet sender                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      SSRC of media source                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      base sequence number     |      packet status count      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 reference time                | fb pkt. count |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          packet chunk         |         packet chunk          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * .                                                               .
 * .                                                               .
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         packet chunk          |  recv delta   |  recv delta   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * .                                                               .
 * .                                                               .
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           recv delta          |  recv delta   | zero padding  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct PacketArrivalInfo
{
    uint16_t seqNum;
    uint64_t remoteArrivalTime;
} PacketArrivalInfo_t;

typedef struct RtcpTwccPacket
{
    uint32_t senderSsrc;
    uint32_t mediaSourceSsrc;
    uint16_t baseSeqNum;
    uint16_t packetStatusCount;
    uint32_t referenceTime;
    uint8_t feedbackPacketCount;
    PacketArrivalInfo_t * pArrivalInfoList;
    size_t arrivalInfoListLength;
} RtcpTwccPacket_t;

/*-----------------------------------------------------------*/

#endif /* RTCP_DATA_TYPES_H */
