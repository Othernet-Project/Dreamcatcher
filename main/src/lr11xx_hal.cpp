#include "Arduino.h"
#include "../../components/LR11XX-Driver/src/lr11xx_hal.h"
#include "SPI.h"

typedef struct {
    SPIClass* spi;
    uint8_t nreset;
    uint8_t busy;
    uint8_t dio1;
    uint8_t nss;
} lr11xx_context_t;

lr11xx_hal_status_t lr11xx_hal_write(const void *context, const uint8_t *command, const uint16_t command_length, const uint8_t *data, const uint16_t data_length)
{
    SPIClass *spi = ((lr11xx_context_t *)context)->spi;
    uint8_t busy = ((lr11xx_context_t *)context)->busy;
    uint8_t nss = ((lr11xx_context_t *)context)->nss;

    while (digitalRead(busy))
        ets_delay_us(10);

    spi->beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

    digitalWrite(nss, LOW);

    spi->writeBytes(command, command_length);
    spi->writeBytes(data, data_length);

    digitalWrite(nss, HIGH);

    spi->endTransaction();

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_read(const void *context, const uint8_t *command, const uint16_t command_length, uint8_t *data, const uint16_t data_length)
{
    SPIClass *spi = ((lr11xx_context_t *)context)->spi;
    uint8_t busy = ((lr11xx_context_t *)context)->busy;
    uint8_t nss = ((lr11xx_context_t *)context)->nss;
    uint8_t dummy = 0;
    memset(data, 0x0, data_length);

    while (digitalRead(busy))
        ets_delay_us(10);

    spi->beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

    digitalWrite(nss, LOW);

    spi->transfer((uint8_t *)command, command_length);
    digitalWrite(nss, HIGH);

    while (digitalRead(busy))
        ets_delay_us(10);

    digitalWrite(nss, LOW);
    spi->transfer(dummy);
    spi->transfer(data, data_length);

    digitalWrite(nss, HIGH);

    spi->endTransaction();

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_direct_read( const void* context, uint8_t* data, const uint16_t data_length ){
    SPIClass *spi = ((lr11xx_context_t *)context)->spi;
    uint8_t busy = ((lr11xx_context_t *)context)->busy;
    uint8_t nss = ((lr11xx_context_t *)context)->nss;
    memset(data, 0x0, data_length);

    while (digitalRead(busy))
        ets_delay_us(10);

    spi->beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
    
    digitalWrite(nss, LOW);
    //spi->transfer(LR11XX_NOP);
    spi->transfer(data, data_length);

    digitalWrite(nss, HIGH);

    spi->endTransaction();

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_write_read(const void *context, const uint8_t *command, uint8_t *data, const uint16_t data_length)
{
    SPIClass *spi = ((lr11xx_context_t *)context)->spi;
    uint8_t busy = ((lr11xx_context_t *)context)->busy;
    uint8_t nss = ((lr11xx_context_t *)context)->nss;

    while (digitalRead(busy))
        ets_delay_us(10);

    spi->beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));

    digitalWrite(nss, LOW);

    spi->transfer(data, data_length);

    digitalWrite(nss, HIGH);

    spi->endTransaction();

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_reset(const void *context)
{
    uint8_t nreset = ((lr11xx_context_t *)context)->nreset;
    digitalWrite(nreset, LOW);
    ets_delay_us(1000);
    digitalWrite(nreset, HIGH);

    return LR11XX_HAL_STATUS_OK;
}

lr11xx_hal_status_t lr11xx_hal_wakeup(const void *context)
{
    uint8_t nss = ((lr11xx_context_t *)context)->nss;
    digitalWrite(nss, LOW);
    ets_delay_us(1000);
    digitalWrite(nss, HIGH);

    return LR11XX_HAL_STATUS_OK;
}