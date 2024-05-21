#ifndef RTCP_ENDIANNESS_H
#define RTCP_ENDIANNESS_H

/* Standard includes. */
#include <stdint.h>

/* Endianness Function types. */
typedef void ( * RtcpWriteUint16_t ) ( uint8_t * pDst,
                                      uint16_t val );
typedef uint16_t ( * RtcpReadUint16_t ) ( const uint8_t * pSrc );

typedef struct RtcpReadWriteFunctions
{
    RtcpWriteUint16_t writeUint16Fn;
    RtcpReadUint16_t readUint16Fn;
} RtcpReadWriteFunctions_t;

void Rtcp_InitReadWriteFunctions( RtcpReadWriteFunctions_t * pReadWriteFunctions );

#endif /* RTCP_ENDIANNESS_H */