/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef MMA8452_H__
#define MMA8452_H__


#include "nrf_twi_mngr.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MMA8452_ADDR        ( 0x1D )

#define STATUS 0x00                         // Type 'read' : Real time status, should return 0x00
#define OUT_X_MSB 0x01                      // Type 'read' : x axis - 8 most significatn bit of a 12 bit sample
#define OUT_X_LSB 0x02                      // Type 'read' : x axis - 4 least significatn bit of a 12 bit sample
#define OUT_Y_MSB 0x03                      // Type 'read' : y axis - 8 most significatn bit of a 12 bit sample
#define OUT_Y_LSB 0x04                      // Type 'read' : y axis - 4 least significatn bit of a 12 bit sample
#define OUT_Z_MSB 0x05                      // Type 'read' : z axis - 8 most significatn bit of a 12 bit sample
#define OUT_Z_LSB 0x06                      // Type 'read' : z axis - 4 least significatn bit of a 12 bit sample
 
#define SYSMOD 0x0B                         // Type 'read' : This tells you if device is active, sleep or standy 0x00=STANDBY 0x01=WAKE 0x02=SLEEP
#define WHO_AM_I 0x0D                       // Type 'read' : This should return the device id of 0x2A
 
#define PL_STATUS 0x10                      // Type 'read' : This shows portrait landscape mode orientation
#define PL_CFG 0x11                         // Type 'read/write' : This allows portrait landscape configuration
#define PL_COUNT 0x12                       // Type 'read' : This is the portraint landscape debounce counter
#define PL_BF_ZCOMP 0x13                    // Type 'read' :
#define PL_THS_REG 0x14                     // Type 'read' :
 
#define FF_MT_CFG 0X15                      // Type 'read/write' : Freefaul motion functional block configuration
#define FF_MT_SRC 0X16                      // Type 'read' : Freefaul motion event source register
#define FF_MT_THS 0X17                      // Type 'read' : Freefaul motion threshold register
#define FF_COUNT  0X18                       // Type 'read' : Freefaul motion debouce counter
 
#define ASLP_COUNT 0x29                     // Type 'read/write' : Counter settings for auto sleep
#define CTRL_REG_1 0x2A                     // Type 'read/write' :
#define CTRL_REG_2 0x2B                     // Type 'read/write' :
#define CTRL_REG_3 0x2C                     // Type 'read/write' :
#define CTRL_REG_4 0x2D                     // Type 'read/write' :
#define CTRL_REG_5 0x2E                     // Type 'read/write' :
 
// // Defined in table 13 of the Freescale PDF
#define STANDBY 0x00                        // State value returned after a SYSMOD request, it can be in state STANDBY, WAKE or SLEEP
#define WAKE 0x01                           // State value returned after a SYSMOD request, it can be in state STANDBY, WAKE or SLEEP
#define SLEEP 0x02                          // State value returned after a SYSMOD request, it can be in state STANDBY, WAKE or SLEEP
#define ACTIVE 0x01                         // Stage value returned and set in Control Register 1, it can be STANDBY=00, or ACTIVE=01
 
#define TILT_STATUS 0x03        // Tilt Status (Read only)
#define SRST_STATUS 0x04        // Sample Rate Status Register (Read only)
#define SPCNT_STATUS 0x05       // Sleep Count Register (Read/Write)
#define INTSU_STATUS 0x06       // Interrupt Setup Register
#define MODE_STATUS 0x07        // Mode Register (Read/Write)
#define SR_STATUS 0x08          // Auto-Wake and Active Mode Portrait/Landscape Samples per Seconds Register (Read/Write)
#define PDET_STATUS 0x09        // Tap/Pulse Detection Register (Read/Write)
#define PD_STATUS 0xA           // Tap/Pulse Debounce Count Register (Read/Write)

#define MMA8452_NUMBER_OF_REGISTERS 6


#define MMA8452_GET_ACC(reg_data)  (int8_t)(reg_data)


extern uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND mma8452_xout_reg_addr;

#define MMA8452_READ(p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(MMA8452_ADDR, p_reg_addr, 1,        NRF_TWI_MNGR_NO_STOP), \
    NRF_TWI_MNGR_READ (MMA8452_ADDR, p_buffer,   byte_cnt, 0)

#define MMA8452_READ_XYZ(p_buffer) \
    MMA8452_READ(&mma8452_xout_reg_addr, p_buffer, 6)

#define MMA8452_INIT_TRANSFER_COUNT 1

extern nrf_twi_mngr_transfer_t const
    mma8452_init_transfers[MMA8452_INIT_TRANSFER_COUNT];

#ifdef __cplusplus
}
#endif

#endif // MMA8452_H__
