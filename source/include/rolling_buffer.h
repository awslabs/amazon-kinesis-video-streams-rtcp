#ifndef RTCP_ROLLING_BUFFER_H
#define RTCP_ROLLING_BUFFER_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/* API includes. */
#include "rtcp_data_types.h"

#define ROLLING_BUFFER_MAP_INDEX(pRBCtx, index)     ((index) % (pRBCtx)->totalLength)

/*-----------------------------------------------------------*/

typedef struct RollingBufferContext
{
    size_t * pRollingBuffer;
    size_t totalLength;
    size_t filledLength;
    size_t headIndex; /* Head index point to next empty slot to put data */
    size_t tailIndex; /* Tail index point to oldest slot with data inside */
} RollingBufferContext_t;

/*-----------------------------------------------------------*/

RtcpResult_t RollingBuffer_Init( RollingBufferContext_t * pRBCtx,
                                  size_t * pBuffer,
                                  size_t bufferLength );

RtcpResult_t RollingBuffer_AddData( RollingBufferContext_t * pRBCtx,
                                    size_t rtpPacket,
                                    size_t * pIndex );

RtcpResult_t RollingBuffer_GetData( RollingBufferContext_t * pRBCtx,
                                    size_t index,
                                    size_t * pRtpPacket );

RtcpResult_t RollingBuffer_IsEmpty( RollingBufferContext_t * pRBCtx,
                                    uint8_t * pIsEmpty );

RtcpResult_t RollingBuffer_GetSize( RollingBufferContext_t * pRBCtx,
                                    size_t * pSize );

#endif /* RTCP_ROLLING_BUFFER_H */