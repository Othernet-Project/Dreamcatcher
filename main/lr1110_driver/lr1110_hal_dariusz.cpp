#include <cstring>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lr1110_hal.h"
#include "Arduino.h"

lr1110_hal_status_t lr1110_hal_write( const void* context, const uint8_t* command, const uint16_t command_length, const uint8_t* data, const uint16_t data_length )
{
    // TODO add semaphore
    spi_device_handle_t* spi = ((lr1110_context_t*)context)->spi;
    uint8_t* resp = &((lr1110_context_t*)context)->last_cmd_status;
    uint8_t busy = ((lr1110_context_t*)context)->busy;
    ((lr1110_context_t*)context)->last_cmd = *(uint16_t*)command;

    static spi_transaction_t trans;
    esp_err_t ret = ESP_OK;

    uint8_t *temp = (uint8_t*)calloc(1, command_length + data_length);
    uint8_t *stat = (uint8_t*)calloc(1, command_length + data_length);
    memcpy(temp, command, command_length);
    memcpy(&temp[command_length], data, data_length);
    memset(&trans, 0, sizeof(spi_transaction_t));

    trans.length = 8 * (command_length + data_length);
    trans.tx_buffer = temp;
    trans.rx_buffer = stat;
    ret = spi_device_transmit(*spi, &trans);
    Serial.printf("spi write: %d\n", ret);

    *resp = stat[0];
    free(temp);
    free(stat);

    while (digitalRead(busy))
        ets_delay_us(10);
    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_read( const void* context, const uint8_t* command, const uint16_t command_length, uint8_t* data, const uint16_t data_length )
{
    // TODO add semaphore
    spi_device_handle_t* spi = ((lr1110_context_t*)context)->spi;
    uint8_t* resp = &((lr1110_context_t*)context)->last_cmd_status;
    ((lr1110_context_t*)context)->last_cmd = *(uint16_t*)command;
    uint8_t busy = ((lr1110_context_t*)context)->busy;

    static spi_transaction_t trans[2];
    uint8_t* _data = (uint8_t*)calloc(1, data_length + 1);
    esp_err_t ret = ESP_OK;

    memset(&trans[0], 0, sizeof(spi_transaction_t));
    memset(&trans[1], 0, sizeof(spi_transaction_t));
    trans[0].length = 8 * command_length;
    trans[1].length = 8 * data_length + 8;
    trans[0].tx_buffer = command;
    trans[1].rx_buffer = _data;

    for (int x=0; x<2; x++) {
        ret += spi_device_transmit(*spi, &trans[x]);
        Serial.printf("spi read: %d\n", ret);
        ets_delay_us(100);
    }

    while (digitalRead(busy))
        ets_delay_us(10);
        
    memcpy(data, &_data[1], data_length);
    *resp = _data[1];
    free(_data);
    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_write_read( const void* context, const uint8_t* command, uint8_t* data, const uint16_t data_length )
{
    spi_device_handle_t* spi = ((lr1110_context_t*)context)->spi;
    uint8_t busy = ((lr1110_context_t*)context)->busy;

    static spi_transaction_t trans;
    esp_err_t ret = ESP_OK;

    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.length = 8 * data_length;
    trans.tx_buffer = command;
    trans.rx_buffer = data;
    ret = spi_device_transmit(*spi, &trans);
    Serial.printf("spi write_read: %d\n", ret);
    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_reset( const void* context )
{
    gpio_num_t nreset = (gpio_num_t)((lr1110_context_t*)context)->nreset;
    gpio_set_level(nreset, 0);
    ets_delay_us(100);
    gpio_set_level(nreset, 1);
    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_status_t lr1110_hal_wakeup( const void* context )
{
    // ???
    return LR1110_HAL_STATUS_OK;
}
