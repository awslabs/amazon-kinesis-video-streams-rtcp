/* API includes. */
#include "rtcp_endianness.h"

#define SWAP_BYTES_16( value )          \
    ( ( ( ( value ) >> 8 ) & 0xFF ) |   \
      ( ( ( value ) & 0xFF ) << 8 ) )

#define SWAP_BYTES_32( value )           \
    ( ( ( ( value ) >> 24 ) & 0xFF )  |  \
      ( ( ( value ) >> 8 ) & 0xFF00 ) |  \
      ( ( ( value ) & 0xFF00 ) << 8 ) |  \
      ( ( ( value ) & 0xFF ) << 24 ) )

#define SWAP_BYTES_64( value )                              \
    ( ( ( ( uint64_t )( value ) >> 56 ) & 0xFF )        |   \
      ( ( ( uint64_t )( value ) >> 40 ) & 0xFF00 )      |   \
      ( ( ( uint64_t )( value ) >> 24 ) & 0xFF0000 )    |   \
      ( ( ( uint64_t )( value ) >> 8 ) & 0xFF000000 )   |   \
      ( ( ( uint64_t )( value ) & 0xFF000000 ) << 8 )   |   \
      ( ( ( uint64_t )( value ) & 0xFF0000 ) << 24 )    |   \
      ( ( ( uint64_t )( value ) & 0xFF00 ) << 40 )      |   \
      ( ( ( uint64_t )( value ) & 0xFF ) << 56 ) )

/*-----------------------------------------------------------*/

void RtcpWriteUint16Swap( uint8_t * pDst, uint16_t val )
{
    *( ( uint16_t * )( pDst ) ) = SWAP_BYTES_16( val );
}

/*-----------------------------------------------------------*/

void RtcpWriteUint32Swap( uint8_t * pDst, uint32_t val )
{
    *( ( uint32_t * )( pDst ) ) = SWAP_BYTES_32( val );
}

/*-----------------------------------------------------------*/

void RtcpWriteUint64Swap( uint8_t * pDst, uint64_t val )
{
    *( ( uint64_t * )( pDst ) ) = SWAP_BYTES_64( val );
}

/*-----------------------------------------------------------*/

uint16_t RtcpReadUint16Swap( const uint8_t * pSrc )
{
    return SWAP_BYTES_16( *( ( uint16_t * )( pSrc ) ) );
}

/*-----------------------------------------------------------*/

uint32_t RtcpReadUint32Swap( const uint8_t * pSrc )
{
    return SWAP_BYTES_32( *( ( uint32_t * )( pSrc ) ) );
}

/*-----------------------------------------------------------*/

uint64_t RtcpReadUint64Swap( const uint8_t * pSrc )
{
    return SWAP_BYTES_64( *( ( uint64_t * )( pSrc ) ) );
}

/*-----------------------------------------------------------*/

void RtcpWriteUint16NoSwap( uint8_t * pDst, uint16_t val )
{
    *( ( uint16_t * )( pDst ) ) = ( val );
}

/*-----------------------------------------------------------*/

void RtcpWriteUint32NoSwap( uint8_t * pDst, uint32_t val )
{
    *( ( uint32_t * )( pDst ) ) = ( val );
}

/*-----------------------------------------------------------*/

void RtcpWriteUint64NoSwap( uint8_t * pDst, uint64_t val )
{
    *( ( uint64_t * )( pDst ) ) = ( val );
}

/*-----------------------------------------------------------*/

uint16_t RtcpReadUint16NoSwap( const uint8_t * pSrc )
{
    return *( ( uint16_t * )( pSrc ) );
}

/*-----------------------------------------------------------*/

uint32_t RtcpReadUint32NoSwap( const uint8_t * pSrc )
{
    return *( ( uint32_t * )( pSrc ) );
}

/*-----------------------------------------------------------*/

uint64_t RtcpReadUint64NoSwap( const uint8_t * pSrc )
{
    return *( ( uint64_t * )( pSrc ) );
}

/*-----------------------------------------------------------*/

void Rtcp_InitReadWriteFunctions( RtcpReadWriteFunctions_t * pReadWriteFunctions )
{
    uint8_t isLittleEndian;

    isLittleEndian = ( *( uint8_t * )( &( uint16_t ){ 1 } ) == 1 );

    if( isLittleEndian != 0 )
    {
        pReadWriteFunctions->writeUint16Fn = RtcpWriteUint16Swap;
        pReadWriteFunctions->writeUint32Fn = RtcpWriteUint32Swap;
        pReadWriteFunctions->writeUint64Fn = RtcpWriteUint64Swap;
        pReadWriteFunctions->readUint16Fn = RtcpReadUint16Swap;
        pReadWriteFunctions->readUint32Fn = RtcpReadUint32Swap;
        pReadWriteFunctions->readUint64Fn = RtcpReadUint64Swap;
    }
    else
    {
        pReadWriteFunctions->writeUint16Fn = RtcpWriteUint16NoSwap;
        pReadWriteFunctions->writeUint32Fn = RtcpWriteUint32NoSwap;
        pReadWriteFunctions->writeUint64Fn = RtcpWriteUint64NoSwap;
        pReadWriteFunctions->readUint16Fn = RtcpReadUint16NoSwap;
        pReadWriteFunctions->readUint32Fn = RtcpReadUint32NoSwap;
        pReadWriteFunctions->readUint64Fn = RtcpReadUint64NoSwap;
    }
}

/*-----------------------------------------------------------*/
