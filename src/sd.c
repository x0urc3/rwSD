/* rwSD Library
   Copyright (C) 2022 Khairulmizam Samsudin <xource@gmail.com

   This file is part of the rwSD Library

   This Library is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the rwSD Library. If not, see
   <http://www.gnu.org/licenses/>.
*/
#include <util/delay.h>
#include "spi_driver.h"
#include "sd.h"

void SD_setup(void) {
    SPI_init();

#ifdef SD_PIN_VCC
    SD_DDR |= _BV(SD_PIN_VCC);
#endif // SD_PIN_VCC
}

/*
 * SD power up sequence
 * - VCC high if SD_PIN_VCC is defined
 * - CS held high i.e. CS disable
 * - Provide at least 1ms and 74 clock cycles while keeping CMD line to high
 */
void SD_powerUp(void) {

#ifdef SD_PIN_VCC
    SD_ENABLE();
#endif // SD_PIN_VCC

    SPI_CS_DISABLE();
    _delay_ms(1);

    for (uint8_t i = 0; i < 10; i++)
        SPI_rw(0xff);

    SPI_CS_DISABLE();
}

void SD_writeCmd(uint8_t codeword, uint32_t arg, uint8_t crc) {

    SPI_rw(0xff);

    SPI_rw(codeword|SD_CMD_PREAMBLE);
    SPI_rw((uint8_t)(arg >> 24));
    SPI_rw((uint8_t)(arg >> 16));
    SPI_rw((uint8_t)(arg >> 8));
    SPI_rw((uint8_t)(arg));

    SPI_rw(crc|SD_CMD_POSTAMBLE);
}

uint8_t SD_readR1(void) {

    uint8_t r1, i = 0;
    do {
        r1 = SPI_rw(0xff);
        i++;
    } while ( (r1 & 0x80) && i < 10);

    return r1;
}

/* Read SD command response
 *  - MSB byte is always R1
 */
void SD_readResponse(uint8_t *res, const uint8_t size) {

    res[0] = SD_readR1();

    for (uint8_t i = 1; i < size; i++)
        res[i] = SPI_rw(0xff);
}

uint8_t SD_init(void) {
    SD_powerUp();

    uint8_t r1, r7[5];

    SPI_CS_ENABLE();

    SD_writeCmd(CMD0, 0, CMD0_CRC);
    r1 = SD_readR1();
    if (r1 != R1_IDLE_STATE)
        return CMD0_ERROR;

    SD_writeCmd(CMD8, CMD8_ARG, CMD8_CRC);
    SD_readResponse(r7, sizeof(r7));
    if (r7[3] != VHS_27_36 || r7[4] != CHECK_PATTERN)
        return CMD8_ERROR;
    uint32_t arg_acmd41;
    if (r7[0] == R1_IDLE_STATE)
        arg_acmd41 = ACMD41_ARG1;
    if (r7[0] == R1_ILLEGAL_COMMAND)
        arg_acmd41 = ACMD41_ARG0;

    uint8_t i=20;
    do {
        SD_writeAcmd(ACMD41, arg_acmd41, 0);
        r1 = SD_readR1();
        i--;
    } while ((r1 != R1_OK) && i);
    if (r1 != R1_OK)
        return ACMD41_ERROR;

    SPI_CS_DISABLE();

    return 0;
}
