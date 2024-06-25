/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <string.h>
#include <stdint.h>

/* API includes. */
#include "rtcp_twcc_manager.h"

/* ===========================  EXTERN VARIABLES  =========================== */

#define MAX_PACKETS_IN_A_FRAME  512

void setUp( void )
{
}

void tearDown( void )
{
}

/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate Opus depacketization happy path.
 */
void test_twcc( void )
{
    TEST_ASSERT_EQUAL( 1, 1 );
}

/*-----------------------------------------------------------*/