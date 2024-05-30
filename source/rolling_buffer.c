/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* API includes. */
#include "rolling_buffer.h"


/*-----------------------------------------------------------*/

RtcpResult_t RollingBuffer_Init( RollingBufferContext_t * pRBCtx,
                                 size_t * pBuffer,
                                 size_t bufferLength )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( ( pRBCtx == NULL ) ||
        ( pBuffer == NULL ) ||
        ( bufferLength < 0 ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        pRBCtx->pRollingBuffer = pBuffer;
        pRBCtx->totalLength = bufferLength;
        pRBCtx->headIndex = 0;
        pRBCtx->tailIndex = 0;
        pRBCtx->filledLength = 0;

        memset(pRBCtx->pRollingBuffer, 0, bufferLength);
    }

    return result;
}

/* TODO : application need to lock before calling this function to remove conflict between
        RTP & RTCP accessing the same function. */
RtcpResult_t RollingBuffer_AddData( RollingBufferContext_t * pRBCtx,
                                    size_t rtpPacketStart,
                                    size_t * pIndex )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t freeRtpPacket;

    if( pRBCtx == NULL ||
        ( pRBCtx != NULL &&
        pRBCtx->pRollingBuffer == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if ( pRBCtx->filledLength == 0 ) /* Rolling buffer is empty. */
        {
            pRBCtx->pRollingBuffer[ROLLING_BUFFER_MAP_INDEX(pRBCtx, pRBCtx->headIndex)] = rtpPacketStart;
            pRBCtx->headIndex = pRBCtx->tailIndex + 1;
            pRBCtx->filledLength += 1;
        }
        else
        {
            if( pRBCtx->filledLength == pRBCtx->totalLength ) /* Rolling buffer is full. */
            {
                /* Create Slot to enter new data by freeing data pointed by tailIndex. */
                freeRtpPacket = pRBCtx->pRollingBuffer[ROLLING_BUFFER_MAP_INDEX(pRBCtx, pRBCtx->tailIndex)];

                /* TODO free here or at application? */
                pRBCtx->tailIndex += 1;
                pRBCtx->filledLength -= 1;
            }

            pRBCtx->pRollingBuffer[ROLLING_BUFFER_MAP_INDEX(pRBCtx, pRBCtx->headIndex)] = rtpPacketStart;
            pRBCtx->headIndex += 1;
            pRBCtx->filledLength += 1;
        }

        if ( pIndex != NULL )
        {
            *pIndex = pRBCtx->headIndex - 1;
        }
    }

    return result;
}

RtcpResult_t RollingBuffer_AddDatatoIndex( RollingBufferContext_t * pRBCtx,
                                           size_t index,
                                           size_t rtpPacketStart )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t * pData;

    if( pRBCtx == NULL ||
        ( pRBCtx != NULL &&
        pRBCtx->pRollingBuffer == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if ( pRBCtx->headIndex > index &&
             pRBCtx->tailIndex <= index )
        {
            result = RTCP_RESULT_BUFFER_NOT_IN_RANGE;
        }
    }

    if( result == RTCP_RESULT_OK )
    {
        pData = &( pRBCtx->pRollingBuffer[ ROLLING_BUFFER_MAP_INDEX(pRBCtx, index) ]);
        if( * pData != ( size_t ) NULL ) {
           /* TODO free */
        }
        * pData = rtpPacketStart;
    }

    return result;

}

RtcpResult_t RollingBuffer_GetData( RollingBufferContext_t * pRBCtx,
                                    size_t index,
                                    size_t * pRtpPacketStart )
{
    RtcpResult_t result = RTCP_RESULT_OK;
    size_t * pData;

    if( pRBCtx == NULL ||
        ( pRBCtx != NULL &&
        pRBCtx->pRollingBuffer == NULL ) )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if( pRBCtx->headIndex > index &&
            pRBCtx->tailIndex <= index )
        {
            * pRtpPacketStart = pRBCtx->pRollingBuffer[ROLLING_BUFFER_MAP_INDEX(pRBCtx, index)];

            if( * pRtpPacketStart != ( size_t ) NULL )
            {
                pRBCtx->pRollingBuffer[ROLLING_BUFFER_MAP_INDEX(pRBCtx, index)] = ( size_t ) NULL;
            }
        }
        else
        {
            * pRtpPacketStart = ( size_t ) NULL;
        }
    }

    return result;
}

RtcpResult_t RollingBuffer_GetSize( RollingBufferContext_t * pRBCtx,
                                    size_t * pSize )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( pRBCtx == NULL )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        * pSize = pRBCtx->filledLength;
    }

    return result;
}

RtcpResult_t RollingBuffer_IsEmpty( RollingBufferContext_t * pRBCtx,
                                    uint8_t * pIsEmpty )
{
    RtcpResult_t result = RTCP_RESULT_OK;

    if( pRBCtx == NULL )
    {
        result = RTCP_RESULT_BAD_PARAM;
    }

    if( result == RTCP_RESULT_OK )
    {
        if(pRBCtx->filledLength == 0 )
        {
            * pIsEmpty = 1;
        }
        else
        {
            * pIsEmpty = 0;
        }
    }

    return result;
}