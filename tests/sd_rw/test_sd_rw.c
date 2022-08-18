#include "avrunit.h"
#include "sd.h"

static int test_cmd17 (void) {
    AU_UNIT_START;

    SD_setup();
    SD_init();

    SPI_CS_ENABLE();
    SD_writeCmd(CMD17, 0xf, 0);

    uint8_t r1 = SD_readR1();

    int timeout = SD_MAX_WRITE_CYCLE;
    uint8_t token = 0xff;
    do {
        token = SPI_rw(0xff);
        timeout--;
    } while ( (token == 0xff) && timeout);

    //flush data and CRC
    for (int j = 0; j < (SD_BLOCK_LENGTH+2); j++) {
        SPI_rw(0xff);
    }

    SPI_CS_DISABLE();

    AU_ASSERT(r1 == R1_OK);
    AU_ASSERT(token == TOKEN_BLOCK_START);
    AU_ASSERT(token != TOKEN_DATA_ERROR_OUT_OF_RANGE);

    AU_UNIT_END;
}

static int test_cmd17_out_of_range (void) {
    AU_UNIT_START;

    SD_setup();
    SD_init();

    SPI_CS_ENABLE();
    SD_writeCmd(CMD17, 0xffffffff, 0);

    uint8_t r1 = SD_readR1();

    int timeout = SD_MAX_WRITE_CYCLE;
    uint8_t token = 0xff;
    do {
        token = SPI_rw(0xff);
        timeout--;
    } while ( (token == 0xff) && timeout);

    //flush data and CRC
    for (int j = 0; j < (SD_BLOCK_LENGTH+2); j++) {
        SPI_rw(0xff);
    }

    SPI_CS_DISABLE();

    AU_ASSERT(r1 == R1_OK);
    AU_ASSERT(token == TOKEN_DATA_ERROR_OUT_OF_RANGE);

    AU_UNIT_END;
}

static int test_cmd24 (void) {
    AU_UNIT_START;

    SD_setup();
    SD_init();

    SPI_CS_ENABLE();
    SD_writeCmd(CMD24, 0xff, 0);

    uint8_t r1 = SD_readR1();

    SPI_rw(TOKEN_BLOCK_START);
    for (int j = 0; j < (SD_BLOCK_LENGTH); j++) {
        SPI_rw(0x88);
    }

    int timeout = SD_MAX_WRITE_CYCLE;
    uint8_t token = 0xff;
    do {
        token = SPI_rw(0xff);
        timeout--;
    } while ( (token == 0xff) && timeout);

    SPI_CS_DISABLE();

    AU_ASSERT(r1 == R1_OK);
    AU_ASSERT(token & TOKEN_WRITE_ACCEPT);

    AU_UNIT_END;
}

static int test_cmd13 (void) {
//    USART_init();

    AU_UNIT_START;

    SPI_CS_ENABLE();

    uint8_t r2[2];
    SD_writeCmd(CMD13, 0, 0);
    SD_readResponse(r2, sizeof(r2));

    SPI_CS_DISABLE();

    AU_ASSERT(r2[0] == R1_OK);
    AU_ASSERT(r2[1] == 0);

    AU_UNIT_END;
//    au_wait_serial('r');
//    au_send_serial((uint8_t *)r2, sizeof(r2));
}

int main (void) {

    AU_RUN_TEST(0x01, test_cmd17);
    AU_RUN_TEST(0x02, test_cmd17_out_of_range);
    AU_RUN_TEST(0x11, test_cmd24);
    AU_RUN_TEST(0x41, test_cmd13);

    AU_OUTPUT();

    return (0);
}
