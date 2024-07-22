#ifndef RTCP_ENDIANNESS_H
#define RTCP_ENDIANNESS_H

/* Standard includes. */
#include <stdint.h>

/* Endianness Function types. */
typedef void ( * RtcpWriteUint16_t ) ( uint8_t * pDst, uint16_t val );
typedef void ( * RtcpWriteUint32_t ) ( uint8_t * pDst, uint32_t val );
typedef void ( * RtcpWriteUint64_t ) ( uint8_t * pDst, uint64_t val );
typedef uint16_t ( * RtcpReadUint16_t ) ( const uint8_t * pSrc );
typedef uint32_t ( * RtcpReadUint32_t ) ( const uint8_t * pSrc );
typedef uint64_t ( * RtcpReadUint64_t ) ( const uint8_t * pSrc );

typedef struct RtcpReadWriteFunctions
{
    RtcpWriteUint16_t writeUint16Fn;
    RtcpWriteUint32_t writeUint32Fn;
    RtcpWriteUint64_t writeUint64Fn;
    RtcpReadUint16_t readUint16Fn;
    RtcpReadUint32_t readUint32Fn;
    RtcpReadUint64_t readUint64Fn;
} RtcpReadWriteFunctions_t;

void Rtcp_InitReadWriteFunctions( RtcpReadWriteFunctions_t * pReadWriteFunctions );

#endif /* RTCP_ENDIANNESS_H */
