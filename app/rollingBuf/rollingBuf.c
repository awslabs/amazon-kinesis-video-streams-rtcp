/* Standard includes. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RTP includes. */
#include "rolling_buffer.h"

static void appendDataToBufferAndVerify( void )
{
    RollingBufferContext_t rbCtx;
    RtcpResult_t result;
    size_t capacity = 2;
    size_t * pBbuffer;
    size_t first = (size_t) 1, second = (size_t) 2, third = (size_t) 3, fourth = (size_t) 4;
    size_t index;
    
    pBbuffer = malloc ( capacity * sizeof(size_t) );

    result = RollingBuffer_Init( &rbCtx, pBbuffer, capacity );
    assert( RTCP_RESULT_OK == result );

    assert(0 == rbCtx.headIndex);
    assert(0 == rbCtx.tailIndex);
    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, first, &index));

    assert(1 == rbCtx.headIndex);
    assert(0 == rbCtx.tailIndex);
    assert(first == rbCtx.pRollingBuffer[0]);
    assert(0 == index);
    
    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, second, &index));
    assert(2 == rbCtx.headIndex);
    assert(0 == rbCtx.tailIndex);
    assert(first == rbCtx.pRollingBuffer[0]);
    assert(second == rbCtx.pRollingBuffer[1]);
    assert(1 == index);

    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, third, &index));
    assert(3 == rbCtx.headIndex);
    assert(1 == rbCtx.tailIndex);
    assert(third == rbCtx.pRollingBuffer[0]);
    assert(second == rbCtx.pRollingBuffer[1]);
    assert(2 == index);

    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, fourth, &index));
    assert(4 == rbCtx.headIndex);
    assert(2 == rbCtx.tailIndex);
    assert(third == rbCtx.pRollingBuffer[0]);
    assert(fourth == rbCtx.pRollingBuffer[1]);
    assert(3 == index);
}

static void extractDataFromBufferAndInsertBack( void )
{
    RollingBufferContext_t rbCtx;
    RtcpResult_t result;
    size_t capacity = 3;
    size_t * pBbuffer;
    size_t first = (size_t) 1, second = (size_t) 2, third = (size_t) 3, fourth = (size_t) 4;
    size_t index, data;
    
    pBbuffer = malloc ( capacity * sizeof(size_t) );

    result = RollingBuffer_Init( &rbCtx, pBbuffer, capacity );
    assert( RTCP_RESULT_OK == result );

    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, first, &index));
    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, second, &index));
    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, third, &index));
    assert(RTCP_RESULT_OK == RollingBuffer_AddData( &rbCtx, fourth, &index));

    assert(4 == rbCtx.headIndex);
    assert(1 == rbCtx.tailIndex);
    assert(fourth == rbCtx.pRollingBuffer[0]);
    assert(second == rbCtx.pRollingBuffer[1]);
    assert(third == rbCtx.pRollingBuffer[2]);

    assert(RTCP_RESULT_OK == RollingBuffer_GetData( &rbCtx, 2, &data));
    assert(third == data);
    assert(RTCP_RESULT_OK == RollingBuffer_GetData( &rbCtx, 2, &data));
    assert( ( size_t ) NULL == data);
}

int main()
{
    appendDataToBufferAndVerify();
    extractDataFromBufferAndInsertBack();
    printf("Test passed\n");
    return 0;
}