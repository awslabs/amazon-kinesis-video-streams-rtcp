#ifndef RTCP_DATA_TYPES_H
#define RTCP_DATA_TYPES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* Endianness includes. */
#include "rtcp_endianness.h"

#define RTCP_HEADER_LENGTH                      4
#define RTCP_HEADER_VERSION                     2

/* RFC https://datatracker.ietf.org/doc/html/draft-alvestrand-rmcat-remb-03#section-2.2
 *     https://datatracker.ietf.org/doc/html/rfc5104#section-4.2.1.1
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
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   SSRC feedback                                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  ...
 */

#define RTCP_REMB_IDENTIFIER_OFFSET             8
#define RTCP_REMB_NUM_SSRC_OFFSET               12
#define RTCP_REMB_SSRC_LIST_OFFSET              16
#define RTCP_REMB_MIN_PAYLOAD_SIZE              16

#define RTCP_SENDER_REPORT_MIN_LENGTH           24
#define RTCP_RECEIVER_REPORT_BLOCK_LENGTH       24
#define RTCP_RECEIVER_REPORT_MIN_LENGTH         4 + RTCP_RECEIVER_REPORT_BLOCK_LENGTH

#define RTCP_NACK_REPORT_MIN_LENGTH             8
#define RTCP_BLP_BIT_COUNT                      16

#define RTCP_TWCC_REPORT_MIN_LENGTH             18
#define RTCP_TWCC_PACKET_CHUNK_SIZE             2
#define RTCP_RUN_LENGTH_STATUS_SYMBOL_BITMASK   0x6000
#define RTCP_RUN_LENGTH_STATUS_SYMBOL_LOCATION  13
#define RTCP_RUN_LENGTH_BITMASK                 0x1FFF
#define RTCP_VECTOR_SYMBOL_SIZE_BITMASK         0x4000
#define RTCP_VECTOR_SYMBOL_SIZE_LOCATION        15
#define RTCP_PACKET_CHUNK_TYPE_BITMASK          0x8000
#define RTCP_PACKET_CHUNK_TYPE_LOCATION         15
#define CHUNK_TYPE( packetChunk )               ( ( packetChunk & RTCP_PACKET_CHUNK_TYPE_BITMASK ) >> RTCP_PACKET_CHUNK_TYPE_LOCATION )

/*-----------------------------------------------------------*/
typedef enum RtcpResult
{
    RTCP_RESULT_OK,
    RTCP_RESULT_BAD_PARAM,
    RTCP_RESULT_OUT_OF_MEMORY,
    RTCP_RESULT_WRONG_VERSION,
    RTCP_RESULT_MALFORMED_PACKET,
    RTCP_RESULT_INPUT_REMB_INVALID,
    RTCP_RESULT_INPUT_NACK_LIST_INVALID,
    RTCP_RESULT_TWCC_INPUT_PACKET_INVALID,
    RTCP_RESULT_TWCC_NO_PACKET_FOUND,
    RTCP_RESULT_TWCC_BUFFER_EMPTY,
    RTCP_RESULT_BUFFER_NOT_IN_RANGE
} RtcpResult_t;

/* RTCP Packet Types supported */
typedef enum {
    RTCP_PACKET_TYPE_FIR = 192, /* https://tools.ietf.org/html/rfc2032#section-5.2.1 */
    RTCP_PACKET_TYPE_SENDER_REPORT = 200,
    RTCP_PACKET_TYPE_RECEIVER_REPORT = 201, /* https://tools.ietf.org/html/rfc3550#section-6.4.2 */
    RTCP_PACKET_TYPE_SOURCE_DESCRIPTION = 202,
    RTCP_PACKET_TYPE_GENERIC_RTCP_FEEDBACK = 205,
    RTCP_PACKET_TYPE_PAYLOAD_SPECIFIC_FEEDBACK = 206,
} RTCP_PACKET_TYPE;

typedef enum {
    RTCP_TWCC_RUN_LENGTH_CHUNK,
    RTCP_TWCC_STATUS_VECTOR_CHUNK
} RTCP_TWCC_CHUNK_TYPE;


typedef enum {
    RTCP_TWCC_STATUS_SYMBOL_NOTRECEIVED = 0,
    RTCP_TWCC_STATUS_SYMBOL_SMALLDELTA,
    RTCP_TWCC_STATUS_SYMBOL_LARGEDELTA,
} RTCP_TWCC_STATUS_SYMBOL;
/*-----------------------------------------------------------*/

typedef struct RtcpContext
{
    RtcpReadWriteFunctions_t readWriteFunctions;
} RtcpContext_t;


/* RTCP HEADER
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |V=2|P|    Count   |       PT      |             length         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct RtcpHeader
{
    uint8_t padding;
    uint8_t receptionReportCount;
    RTCP_PACKET_TYPE packetType;
    uint16_t packetLength;
} RtcpHeader_t;

/* Need to use struct RtcpDataPacket here instead of struct RtcpPacket as per the
 * naming convention, to avoid conflict with an existing struct in the current
 * WebRTC SDK. */
typedef struct RtcpDataPacket
{
    RtcpHeader_t header;
    uint8_t * pPayload;
    size_t payloadLength;
} RtcpPacket_t;

/* RTCP sender report packet https://datatracker.ietf.org/doc/html/rfc3550#section-6.4.1
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |V=2|P|    RC   |   PT          |             length            | header
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                         SSRC of sender                        |
 *     +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *     |              NTP timestamp, most significant word             |  sender
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  info
 *     |             NTP timestamp, least significant word             |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                         RTP timestamp                         |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                     sender's packet count                     |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                      sender's octet count                     |
 *     +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 */
typedef struct RtcpSenderReport
{
    uint32_t ssrc;
    uint64_t ntpTime;
    uint32_t rtpTime;
    uint32_t packetCount;
    uint32_t octetCount;
} RtcpSenderReport_t;

/*        0                   1                   2                   3
 *        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * header |V=2|P|    RC   |   PT=RR=201   |             length            |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *        |                     SSRC of packet sender                     |
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *  report |                 SSRC_1 (SSRC of first source)                 |
 *  block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
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
 *  report |                 SSRC_2 (SSRC of second source)                |
 *  block  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   2    :                               ...                             :
 *        +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *        |                  profile-specific extensions                  |
 *        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct RtcpReceiverReport
{
    uint32_t ssrcSender;
    uint32_t ssrcSource;
    uint8_t fractionLost;
    uint32_t cumulativePacketsLost;
    uint32_t extHiSeqNumReceived;
    uint32_t interArrivalJitter;
    uint32_t lastSR;
    uint32_t delaySinceLastSR;
} RtcpReceiverReport_t;

typedef struct RtcpNackPacket
{
    uint32_t ssrcSender;
    uint32_t ssrcSource;
    uint32_t seqNumListLength;
    uint16_t * pSeqNumList;
} RtcpNackPacket_t;

typedef struct RtcpTwccPacket
{
    uint32_t ssrcSender;
    uint32_t ssrcSource;
    uint16_t baseSeqNum;
    uint16_t packetStatusCount;
    uint32_t referenceTime;
    uint8_t feedbackPacketCount;
    uint8_t * pPacketChunkStart;
    uint8_t * pRecvDeltaStart;
} RtcpTwccPacket_t;

/*-----------------------------------------------------------*/

#endif /* RTCP_DATA_TYPES_H */