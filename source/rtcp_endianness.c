/* API includes. */
#include "rtcp_endianness.h"

#define SWAP_BYTES_16( value )          \
    ( ( ( ( value ) >> 8 ) & 0xFF ) |   \
      ( ( ( value ) & 0xFF ) << 8 ) )

/*-----------------------------------------------------------*/

void WriteUint16Swap( uint8_t * pDst, uint16_t val )
{
    *( ( uint16_t * )( pDst ) ) = SWAP_BYTES_16( val );
}

/*-----------------------------------------------------------*/

uint16_t ReadUint16Swap( const uint8_t * pSrc )
{
    return SWAP_BYTES_16( *( ( uint16_t * )( pSrc ) ) );
}

/*-----------------------------------------------------------*/

void WriteUint16NoSwap( uint8_t * pDst, uint16_t val )
{
    *( ( uint16_t * )( pDst ) ) = ( val );
}

/*-----------------------------------------------------------*/

uint16_t ReadUint16NoSwap( const uint8_t * pSrc )
{
    return *( ( uint16_t * )( pSrc ) );
}

/*-----------------------------------------------------------*/

void Rtcp_InitReadWriteFunctions( RtcpReadWriteFunctions_t * pReadWriteFunctions )
{
    uint8_t isLittleEndian;

    isLittleEndian = ( *( uint8_t * )( &( uint16_t ){ 1 } ) == 1 );

    if( isLittleEndian != 0 )
    {
        pReadWriteFunctions->writeUint16Fn = WriteUint16Swap;
        pReadWriteFunctions->readUint16Fn = ReadUint16Swap;
    }
    else
    {
        pReadWriteFunctions->writeUint16Fn = WriteUint16NoSwap;
        pReadWriteFunctions->readUint16Fn = ReadUint16NoSwap;
    }
}

/*-----------------------------------------------------------*/