/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.

    Based on mbed-os\targets\TARGET_GigaDevice\TARGET_GD32F30X\i2c_api.c
*/

#include "utility/twi.h"
#include "pinmap.h"
#include "twi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
#if defined(I2C0)
    I2C0_INDEX,
#endif
#if defined(I2C1)
    I2C1_INDEX,
#endif
    I2C_NUM
};

static struct i2c_s *obj_s_buf[I2C_NUM] = {NULL};
#define BUSY_TIMEOUT  ((SystemCoreClock / obj_s->freq) * 2 * 10)
#define FLAG_TIMEOUT  (0xF0000U)
#define I2C_S(obj)    (struct i2c_s *) (obj)

/** Initialize the I2C peripheral
 *
 * @param obj       The I2C object
 * @param sda       The sda pin
 * @param scl       The scl pin
 * @param address   The I2C own address
 */
void i2c_init(i2c_t *obj, PinName sda, PinName scl, uint8_t address)
{
    struct i2c_s *obj_s = I2C_S(obj);
    /* find the I2C by pins */
    uint32_t i2c_sda = pinmap_peripheral(sda, PinMap_I2C_SDA);
    uint32_t i2c_scl = pinmap_peripheral(scl, PinMap_I2C_SCL);

    obj_s->sda = sda;
    obj_s->scl = scl;
    obj_s->i2c = pinmap_merge(i2c_sda, i2c_scl);
    i2c_deinit(obj_s->i2c);
    switch(obj_s->i2c) {
        case I2C0:
            /* enable I2C0 clock and configure the pins of I2C0 */
            obj_s->index = 0;
            rcu_periph_clock_enable(RCU_I2C0);
            break;
        case I2C1:
            /* enable I2C1 clock and configure the pins of I2C1 */
            obj_s->index = 1;
            rcu_periph_clock_enable(RCU_I2C1);
            break;
        default:
            break;
    }

    /* configure the pins of I2C */
    pinmap_pinout(sda, PinMap_I2C_SDA);
    pinmap_pinout(scl, PinMap_I2C_SCL);

    rcu_i2c0_clock_config(RCU_I2C0SRC_CKAPB1);
    i2c_timing_config(obj->i2c, 0x00, 0x00, 0x00);

    /* I2C clock configure */
    i2c_master_clock_config(obj->i2c,200,200);
    /* enable I2C */
    i2c_enable(obj->i2c);
    /* enable acknowledge */
    i2c_nack_disable(obj->i2c);
    /* get obj_s_buf */
    obj_s_buf[obj_s->index] = obj_s;
}

/** Enable the I2C interrupt
 *
 * @param obj       The I2C object
 */
void i2c_slaves_interrupt_enable(i2c_t *obj)
{
    struct i2c_s *obj_s = I2C_S(obj);
    switch(obj_s->i2c) {
        case I2C0:
            /* enable I2C0 interrupt */
            eclic_irq_enable(I2C0_EV_IRQn, 8, 0);
            eclic_irq_enable(I2C0_ER_IRQn, 8, 0);
            break;
        case I2C1:
            /* enable I2C1 interrupt */
            eclic_irq_enable(I2C1_EV_IRQn, 8, 0);
            eclic_irq_enable(I2C1_ER_IRQn, 8, 0);
            break;
        default:
            break;
    }

    i2c_interrupt_enable(obj_s->i2c, I2C_INT_ERR);
    i2c_interrupt_enable(obj_s->i2c, I2C_INT_RBNE);
    i2c_interrupt_enable(obj_s->i2c, I2C_INT_ADDM);
    i2c_interrupt_enable(obj_s->i2c, I2C_INT_STPDET);
}

/** Write one byte (master)
 *
 * @param obj  The I2C object
 * @param data Byte to be written
 * @return 0 if NACK was received, 1 if ACK was received, 2 for timeout.
 */
static int i2c_byte_write(i2c_t *obj, int data)
{
    int timeout;
    struct i2c_s *obj_s = I2C_S(obj);

    i2c_data_transmit(obj_s->i2c, (uint8_t)data);

    /* wait until the byte is transmitted or timeout*/
    timeout = FLAG_TIMEOUT;
    while(!i2c_flag_get(obj_s->i2c, I2C_FLAG_TBE) && (--timeout != 0));

    if (timeout == 0){
        return 2;
    }

    return 1;
}

/** Send STOP command
 *
 * @param obj The I2C object
 */
static int i2c_stop(i2c_t *obj)
{
    struct i2c_s *obj_s = I2C_S(obj);

    /* generate a STOP condition */
    i2c_stop_on_bus(obj_s->i2c);

    /* wait for STOP bit reset */
    while((I2C_CTL0(obj_s->i2c) & I2C_CTL0_STPDETIE));

    return 0;
}

/** Write bytes at a given address
 *
 * @param obj     The I2C object
 * @param address 7-bit address (last bit is 0)
 * @param data    The buffer for sending
 * @param length  Number of bytes to write
 * @param stop    Stop to be generated after the transfer is done
 * @return Status
 */
i2c_status_enum i2c_master_transmit(i2c_t *obj, uint8_t address, uint8_t *data, uint16_t length,
                                    uint8_t stop)
{
    i2c_status_enum ret = I2C_OK;
    uint32_t timeout = 0;
    uint32_t count = 0;

    if(length > I2C_BUFFER_SIZE) {
        ret = I2C_DATA_TOO_LONG;
        return ret;
    }

    /* When size is 0, this is usually an I2C scan / ping to check if device is there and ready */
    if(length == 0) {
        /*
            Although we do not send actual data in this case, we must call i2c_transfer_byte_number_config
            to set a non-zero number to ensure we can send start flag successfully !!!

            when i2c_transfer_byte_number_config is called, BYTENUM of I2C_CTL1 register is set to non-zero,
            once stop signal is sent, BYTENUM will be set to zero automatically by hardware.
        */
        i2c_transfer_byte_number_config(obj->i2c,1);
        ret = i2c_wait_standby_state(obj, address);
    } else {
        i2c_transfer_byte_number_config(obj->i2c,length);
        timeout = FLAG_TIMEOUT;
        while((i2c_flag_get(obj->i2c, I2C_FLAG_I2CBSY)) && (--timeout != 0));
        if(0 == timeout) {
            ret = I2C_BUSY;
        }
        /* generate a START condition */
        i2c_start_on_bus(obj->i2c);

        /* ensure the i2c has been started successfully */
        timeout = FLAG_TIMEOUT;
        while((!i2c_flag_get(obj->i2c, I2C_FLAG_TBE)) && (--timeout != 0));
        if(0 == timeout) {
            ret = I2C_TIMEOUT;
            return ret;
        }

        /* after sending slave addr, we receive NACK, which means the slave do not exist */
        if((I2C_STAT(obj->i2c) & I2C_STAT_NACK) || (I2C_STAT(obj->i2c) & I2C_STAT_STPDET))
        {
            ret = I2C_NACK_ADDR;

            if(I2C_STAT(obj->i2c) & I2C_STAT_NACK)
                i2c_flag_clear(obj->i2c, I2C_STATC_NACKC);

            if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
                i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);

            return ret;
        }

        for(count = 0; count < length; count++) {
            if(1 != i2c_byte_write(obj, data[count])) {
                ret = I2C_NACK_DATA;
                return ret;
            }
        }
        /* if not sequential write, then send stop */
        if(stop) {

            timeout = FLAG_TIMEOUT;
            while(!i2c_flag_get(obj->i2c, I2C_STAT_TC) && (--timeout != 0));
            if (timeout == 0){
                ret = I2C_TIMEOUT;
                return ret;
            }

            i2c_stop_on_bus(obj->i2c);

            timeout = FLAG_TIMEOUT;
            // /* wait until the stop condition is finished */
            while(!(I2C_STAT(obj->i2c) & I2C_STAT_STPDET) && (--timeout != 0));

            if (timeout == 0){
                ret = I2C_TIMEOUT;
                return ret;
            }

            if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
                i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);
        }
    }
    return ret;
}

i2c_status_enum i2c_slave_transmit(i2c_t *obj, uint8_t *data, uint16_t length)
{
    i2c_status_enum ret = I2C_OK;
    uint32_t timeout = 0;
    uint32_t count = 0;

    if(length > I2C_BUFFER_SIZE) {
        ret = I2C_DATA_TOO_LONG;
        return ret;
    }

    timeout = FLAG_TIMEOUT;
    while((i2c_flag_get(obj->i2c, I2C_FLAG_I2CBSY)) && (--timeout != 0));
    if(0 == timeout) {
        ret = I2C_BUSY;
    }

    timeout = FLAG_TIMEOUT;
    while((!i2c_flag_get(obj->i2c, I2C_FLAG_TBE)) && (--timeout != 0));
    if(0 == timeout) {
        ret = I2C_TIMEOUT;
        return ret;
    }

    for(count = 0; count < length; count++) {
        if(1 != i2c_byte_write(obj, data[count])) {
            ret = I2C_NACK_DATA;
            return ret;
        }
    }
    return ret;
}

/** Read one byte
 *
 * @param obj  The I2C object
 * @param last Acknowledge
 * @return The read byte
 */
static int i2c_byte_read(i2c_t *obj, int last)
{
    int timeout;
    struct i2c_s *obj_s = I2C_S(obj);

    if(last) {
        /* disable acknowledge */
        i2c_nack_enable(obj_s->i2c);
    } else {
        /* enable acknowledge */
        i2c_nack_disable(obj_s->i2c);
    }

    /* wait until the byte is received */
    timeout = FLAG_TIMEOUT;
    while((i2c_flag_get(obj_s->i2c, I2C_FLAG_RBNE)) == RESET) {
        if((timeout--) == 0) {
            return -1;
        }
    }
    return (int)I2C_RDATA(obj_s->i2c);
}

/** read bytes in master mode at a given address
 *
 * @param obj     The I2C object
 * @param address 7-bit address (last bit is 1)
 * @param data    The buffer for receiving
 * @param length  Number of bytes to read
 * @param stop    Stop to be generated after the transfer is done
 * @return status
 */
i2c_status_enum i2c_master_receive(i2c_t *obj, uint8_t address, uint8_t *data, uint16_t length,
                                   int stop)
{
    i2c_status_enum ret = I2C_OK;
    uint32_t timeout = 0;
    uint32_t count = 0;

    i2c_transfer_byte_number_config(obj->i2c,length);
    i2c_master_addressing(obj->i2c, address, I2C_MASTER_RECEIVE);

    timeout = FLAG_TIMEOUT;
    while((i2c_flag_get(obj->i2c, I2C_FLAG_I2CBSY)) && (--timeout != 0));
    if(0 == timeout) {
        ret = I2C_BUSY;
    }

    /* generate a START condition */
    i2c_start_on_bus(obj->i2c);

    if((I2C_STAT(obj->i2c) & I2C_STAT_NACK) || (I2C_STAT(obj->i2c) & I2C_STAT_STPDET))
    {
        ret = I2C_NACK_ADDR;

        if(I2C_STAT(obj->i2c) & I2C_STAT_NACK)
            i2c_flag_clear(obj->i2c, I2C_STATC_NACKC);

        if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
            i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);

        return ret;
    }

    for(count = 0; count < length; count++) {
        while(!i2c_flag_get(obj->i2c, I2C_FLAG_RBNE));
        data[count] = i2c_data_receive(obj->i2c);
    }

    /* if not sequential read, then send stop */
    if(stop) {
        while(!i2c_flag_get(obj->i2c, I2C_STAT_TC));
        i2c_stop_on_bus(obj->i2c);

        timeout = FLAG_TIMEOUT;
        // /* wait until the stop condition is finished */
        while(!(I2C_STAT(obj->i2c) & I2C_STAT_STPDET) && (--timeout != 0));

        if (timeout == 0){
            ret = I2C_TIMEOUT;
            return ret;
        }

        if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
            i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);
    }
    return ret;
}

/** Checks if target device is ready for communication
 *
 * @param obj     The I2C object
 * @param address 7-bit address (last bit is 1)
 * @return status
 */
i2c_status_enum i2c_wait_standby_state(i2c_t *obj, uint8_t address)
{
    i2c_status_enum status = I2C_OK;
    uint32_t timeout;

    /* wait until I2C_FLAG_I2CBSY flag is reset */
    timeout = FLAG_TIMEOUT;
    while((i2c_flag_get(obj->i2c, I2C_FLAG_I2CBSY)) && (--timeout != 0));
    if(0 == timeout) {
        status = I2C_BUSY;
        return status;
    }
    /* send a start condition to I2C bus */
    i2c_start_on_bus(obj->i2c);

    if((I2C_STAT(obj->i2c) & I2C_STAT_NACK) || (I2C_STAT(obj->i2c) & I2C_STAT_STPDET))
    {
        status = I2C_NACK_ADDR;

        if(I2C_STAT(obj->i2c) & I2C_STAT_NACK)
            i2c_flag_clear(obj->i2c, I2C_STATC_NACKC);

        if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
            i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);

        return status;
    }

    // If code can reach here, it means ACK was received (there is a device at the address)
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(obj->i2c);
    timeout = FLAG_TIMEOUT;
    // /* wait until the stop condition is finished */
    while(!(I2C_STAT(obj->i2c) & I2C_STAT_STPDET) && (--timeout != 0));

    if(I2C_STAT(obj->i2c) & I2C_STAT_STPDET)
        i2c_flag_clear(obj->i2c, I2C_STATC_STPDETC);

    return status;
}

/** sets function called before a slave read operation
 *
 * @param obj      The I2C object
 * @param function Callback function to use
 */
void i2c_attach_slave_rx_callback(i2c_t *obj, void (*function)(uint8_t *, int))
{
    if(obj == NULL) {
        return;
    }

    /* Exit if a reception is already on-going */
    if(function == NULL) {
        return;
    }
    obj->slave_receive_callback = function;
}

/** sets function called before a slave write operation
 *
 * @param obj      The I2C object
 * @param function Callback function to use
 */
void i2c_attach_slave_tx_callback(i2c_t *obj, void (*function)(void))
{
    if(obj == NULL) {
        return;
    }

    /* Exit if a reception is already on-going */
    if(function == NULL) {
        return;
    }
    obj->slave_transmit_callback = function;
}

void i2c_attach_slave_addr_match_callback(i2c_t *obj, void (*function)(void))
{
    if(obj == NULL) {
        return;
    }

    /* Exit if a reception is already on-going */
    if(function == NULL) {
        return;
    }
    obj->slave_addr_match_callback = function;
}

/** Write bytes to master
 *
 * @param obj    The I2C object
 * @param data   The buffer for transfer
 * @param length Number of bytes to read
 * @return status
 */
i2c_status_enum i2c_slave_write_buffer(i2c_t *obj, uint8_t *data, uint16_t length)
{
    struct i2c_s *obj_s = I2C_S(obj);
    uint8_t i = 0;
    i2c_status_enum ret = I2C_OK;

    if(length > I2C_BUFFER_SIZE) {
        ret = I2C_DATA_TOO_LONG;
    } else {
        /* check the communication status */
        for(i = 0; i < length; i++) {
            *obj_s->tx_buffer_ptr++ = *(data + i);
        }
        obj_s->tx_count = length;
        obj_s->tx_buffer_ptr = obj_s->tx_buffer_ptr - length;
    }
    return ret;
}

#ifdef I2C0
/** This function handles I2C interrupt handler
 *
 * @param i2c_periph The I2C peripheral
 */
static void i2c_irq(struct i2c_s *obj_s)
{
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_ADDSEND)) {
        if(i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
            obj_s->slave_addr_match_callback();
        }
        /* clear the ADDSEND bit */
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_ADDSEND);
        obj_s->rx_count = 0;
        if(i2c_flag_get(I2C0, I2C_FLAG_TR)) {
            obj_s->slave_transmit_callback();
        }
    } else if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_RBNE)) {
        /* if reception data register is not empty ,I2C will read data from register */
        *obj_s->rx_buffer_ptr++ = i2c_data_receive(I2C0);
        obj_s->rx_count++;
    } else if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_STPDET)) {

        i2c_enable(I2C0);
        if(!i2c_flag_get(I2C0, I2C_FLAG_TI)) {
            obj_s->rx_buffer_ptr = obj_s->rx_buffer_ptr - obj_s->rx_count ;
            obj_s->slave_receive_callback(obj_s->rx_buffer_ptr, obj_s->rx_count);
        }
        /* clear the STPDET bit */
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_STPDET);
    }
}

/** Handle I2C0 event interrupt request
 *
 */
void I2C0_EV_IRQHandler(void)
{
    i2c_irq(obj_s_buf[I2C0_INDEX]);
}

/** handle I2C0 error interrupt request
 *
 */
void I2C0_ER_IRQHandler(void)
{
    /* no acknowledge received */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_NACK)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_NACK);
    }

    /* SMBus alert */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_SMBALT)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_SMBALT);
    }

    /* over-run or under-run when SCL stretch is disabled */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_OUERR)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_OUERR);
    }

    /* arbitration lost */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_LOSTARB)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_LOSTARB);
    }

    /* bus error */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_BERR)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_BERR);
    }

    /* CRC value doesn't match */
    if(i2c_interrupt_flag_get(I2C0, I2C_INT_FLAG_PECERR)) {
        i2c_interrupt_flag_clear(I2C0, I2C_INT_FLAG_PECERR);
    }
}
#endif

#ifdef I2C1

/** Handle I2C1 event interrupt request
 *
 */
void I2C1_EV_IRQHandler(void)
{
    i2c_irq(obj_s_buf[I2C1_INDEX]);
}

/** handle I2C1 error interrupt request
 *
 */
void I2C1_ER_IRQHandler(void)
{
    /* no acknowledge received */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_NACK)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_NACK);
    }

    /* SMBus alert */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_SMBALT)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_SMBALT);
    }

    /* over-run or under-run when SCL stretch is disabled */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_OUERR)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_OUERR);
    }

    /* arbitration lost */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_LOSTARB)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_LOSTARB);
    }

    /* bus error */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_BERR)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_BERR);
    }

    /* CRC value doesn't match */
    if(i2c_interrupt_flag_get(I2C1, I2C_INT_FLAG_PECERR)) {
        i2c_interrupt_flag_clear(I2C1, I2C_INT_FLAG_PECERR);
    }
}

#endif
#ifdef __cplusplus
}
#endif
