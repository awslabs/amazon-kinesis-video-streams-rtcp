#ifndef RTCP_DATA_TYPES_H
#define RTCP_DATA_TYPES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* Endianness includes. */
#include "rtcp_endianness.h"

//MAP HERE -                        
// #define STATUS_RTCP_INPUT_PACKET_TOO_SMALL      
// #define STATUS_RTCP_INPUT_PACKET_INVALID_VERSION 
// #define STATUS_RTCP_INPUT_PACKET_LEN_MISMATCH    
// #define STATUS_RTCP_INPUT_NACK_LIST_INVALID     
// #define STATUS_RTCP_INPUT_SSRC_INVALID           
// #define STATUS_RTCP_INPUT_PARTIAL_PACKET  =  RTP_RESULT_MALFORMED_PACKET     
// #define STATUS_RTCP_INPUT_REMB_TOO_SMALL        
// #define STATUS_RTCP_INPUT_REMB_INVALID         

#define RTCP_HEADER_LENGTH                      4
#define RTCP_HEADER_VERSION                     2
#define RTCP_HEADER_VERSION_MASK                0xC0
#define RTCP_HEADER_VERSION_LOCATION            6
#define RTCP_HEADER_PADDING_MASK                0x20
#define RTCP_HEADER_PADDING_LOCATION            5

#define RTCP_PACKET_RRC_BITMASK                 0x1F
#define RTCP_PACKET_LEN_WORD_SIZE               4

typedef enum RtcpResult
{
    RTCP_RESULT_OK,
    RTCP_RESULT_BAD_PARAM,
    RTCP_RESULT_OUT_OF_MEMORY,
    RTCP_RESULT_WRONG_VERSION,
    RTCP_RESULT_MALFORMED_PACKET,
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

/*-----------------------------------------------------------*/

#endif /* RTCP_DATA_TYPES_H */