/**
 * @file i2c_a7.c
 * @author NXP Semiconductors
 * @version 1.0
 * @par License
 * Copyright 2017 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 *
 * @par Description
 * i.MX6UL board specific i2c code
 * @par History
 *
 **/
#include "i2c_a7.h"
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <time.h>

//#define LOG_I2C 1
#define ENABLE_PRINTF 0 


static int axSmDevice;
static int axSmDevice_addr = 0x48;      // 7-bit address
static char devName[] = "/dev/i2c-0";   // Change this when connecting to another host i2c master port


/**
* Opens the communication channel to I2C device
*/
i2c_error_t axI2CInit()
{
    unsigned long funcs;

    /*
     * Open the file in /dev/i2c-1
     */
    if(ENABLE_PRINTF)printf("I2CInit: opening %s\n", devName);

    if ((axSmDevice = open(devName, O_RDWR)) < 0)
    {
        if(ENABLE_PRINTF)printf("opening failed...\n");
        perror("Failed to open the i2c bus");
        return I2C_FAILED;
    }

    if (ioctl(axSmDevice, I2C_SLAVE, axSmDevice_addr) < 0)
    {
        if(ENABLE_PRINTF)printf("I2C driver failed setting address\n");
    }

    // clear PEC flag
    if (ioctl(axSmDevice, I2C_PEC, 0) < 0)
    {
        if(ENABLE_PRINTF)printf("I2C driver: PEC flag clear failed\n");
    }
    else
    {
        if(ENABLE_PRINTF)printf("I2C driver: PEC flag cleared\n");
    }

    // Query functional capacity of I2C driver
    if (ioctl(axSmDevice, I2C_FUNCS, &funcs) < 0)
    {
        if(ENABLE_PRINTF)printf("Fatal: Cannot get i2c adapter functionality\n");
        return I2C_FAILED;
    }
    else
    {
        if (funcs & I2C_FUNC_I2C)
        {
            if(ENABLE_PRINTF)printf("I2C driver supports plain i2c-level commands.\n");
            if ( (funcs & I2C_FUNC_SMBUS_READ_BLOCK_DATA) == I2C_FUNC_SMBUS_READ_BLOCK_DATA )
            {
                if(ENABLE_PRINTF)printf("I2C driver supports Read Block.\n");
            }
            else
            {
                if(ENABLE_PRINTF)printf("Fatal: I2C driver does not support Read Block!\n");
                return I2C_FAILED;
            }
        }
        else
        {
            if(ENABLE_PRINTF)printf("Fatal: I2C driver CANNOT support plain i2c-level commands!\n");
            return I2C_FAILED;
        }
    }

   return I2C_OK;
}

/**
* Closes the communication channel to I2C device (not implemented)
*/
void axI2CTerm(int mode)
{
    AX_UNUSED_ARG(mode);
    if(ENABLE_PRINTF)printf("axI2CTerm: not implemented.\n");
    return;
}

/**
 * Write a single byte to the slave device.
 * In the context of the SCI2C protocol, this command is only invoked
 * to trigger a wake-up of the attached secure module. As such this
 * wakeup command 'wakes' the device, but does not receive a valid response.
 * \note \par bus is currently not used to distinguish between I2C masters.
*/
i2c_error_t axI2CWriteByte(unsigned char bus, unsigned char addr, unsigned char *pTx)
{
    int nrWritten = -1;
    i2c_error_t rv;

    if (bus != I2C_BUS_0)
    {
        if(ENABLE_PRINTF)printf("axI2CWriteByte on wrong bus %x (addr %x)\n", bus, addr);
    }

    nrWritten = write(axSmDevice, pTx, 1);
    if (nrWritten < 0)
    {
        if(ENABLE_PRINTF)printf("Failed writing data (nrWritten=%d).\n", nrWritten);
        rv = I2C_FAILED;
    }
    else
    {
        if (nrWritten == 1)
        {
            rv = I2C_OK;
        }
        else
        {
            rv = I2C_FAILED;
        }
    }

    return rv;
}

i2c_error_t axI2CWrite(unsigned char bus, unsigned char addr, unsigned char * pTx, unsigned short txLen)
{
    int nrWritten = -1;
    i2c_error_t rv;
#ifdef LOG_I2C
    int i = 0;
#endif

    if (bus != I2C_BUS_0)
    {
        if(ENABLE_PRINTF)printf("axI2CWrite on wrong bus %x (addr %x)\n", bus, addr);
    }
#ifdef LOG_I2C
    if(ENABLE_PRINTF)printf("TX (axI2CWrite): ");
    for (i = 0; i < txLen; i++)
    {
        if(ENABLE_PRINTF)printf("%02X ", pTx[i]);
    }
    if(ENABLE_PRINTF)printf("\n");
#endif

   nrWritten = write(axSmDevice, pTx, txLen);
   if (nrWritten < 0)
   {
      if(ENABLE_PRINTF)printf("Failed writing data (nrWritten=%d).\n", nrWritten);
      rv = I2C_FAILED;
   }
   else
   {
        if (nrWritten == txLen) // okay
        {
            rv = I2C_OK;
        }
        else
        {
            rv = I2C_FAILED;
        }
   }
#ifdef LOG_I2C
    if(ENABLE_PRINTF)printf("    Done with rv = %02x ", rv);
    if(ENABLE_PRINTF)printf("\n");
#endif

   return rv;
}

i2c_error_t axI2CWriteRead(unsigned char bus, unsigned char addr, unsigned char * pTx,
      unsigned short txLen, unsigned char * pRx, unsigned short * pRxLen)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    int r = 0;
    int i = 0;

    usleep(100000);

    if (bus != I2C_BUS_0) // change if bus 0 is not the correct bus
    {
        if(ENABLE_PRINTF)printf("axI2CWriteRead on wrong bus %x (addr %x)\n", bus, addr);
    }

    messages[0].addr  = axSmDevice_addr;
    messages[0].flags = 0;
    messages[0].len   = txLen;
    messages[0].buf   = pTx;

    // NOTE:
    // By setting the 'I2C_M_RECV_LEN' bit in 'messages[1].flags' one ensures
    // the I2C Block Read feature is used.
    messages[1].addr  = axSmDevice_addr;
    messages[1].flags = I2C_M_RD | I2C_M_RECV_LEN;
    messages[1].len   = 256;
    messages[1].buf   = pRx;
    messages[1].buf[0] = 1;

    // NOTE:
    // By passing the two message structures via the packets structure as
    // a parameter to the ioctl call one ensures a Repeated Start is triggered.
    packets.msgs      = messages;
    packets.nmsgs     = 2;
#ifdef LOG_I2C
    if(ENABLE_PRINTF)printf("TX (%d byte): ", txLen);
    for (i = 0; i < txLen; i++)
    {
        if(ENABLE_PRINTF)printf("%02X ", packets.msgs[0].buf[i]);
    }
    if(ENABLE_PRINTF)printf("\n");
#endif

    // Send the request to the kernel and get the result back
    r = ioctl(axSmDevice, I2C_RDWR, &packets);

    // NOTE:
    // The ioctl return value in case of a NACK on the write address is '-1'
    // This impacts the error handling routine of the caller.
    if (r < 0)
    {
#ifdef LOG_I2C
        if(ENABLE_PRINTF)printf("axI2CWriteRead: ioctl cmd I2C_RDWR fails with value %d (hex: 0x%08X)\n", r, r);
        perror("Errorstring: ");
#endif
        //if(ENABLE_PRINTF) printf("axI2CWriteRead: ioctl value %d (hex: 0x%08X)\n", r, r);
        return I2C_FAILED;
    }
    else
    {
        int rlen = packets.msgs[1].buf[0]+1;

        //if(ENABLE_PRINTF)printf("packets.msgs[1].len is %d \n", packets.msgs[1].len);
#ifdef LOG_I2C
        if(ENABLE_PRINTF)printf("RX  (%d): ", rlen);
        for (i = 0; i < rlen; i++)
        {
            if(ENABLE_PRINTF)printf("%02X ", packets.msgs[1].buf[i]);
        }
        if(ENABLE_PRINTF)printf("\n");
#endif
        for (i = 0; i < rlen; i++)
        {
            pRx[i] = packets.msgs[1].buf[i];
        }
        *pRxLen = rlen;
    }

    return I2C_OK;
}
