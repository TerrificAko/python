/*****************************************************************************
* File: zn_driver.c
*
* Descirption: this file contains the functions support driver.
*
* Copyright (C) : 2023 ultraception.com
*
* Author: ultraception FW team
* Mail:   ultraception@ultraception.com
* Date:   2023 - 5
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "zn_regs.h"
#include "zn_interface.h"
#include "base_addr.h"
#include "clock_init.h"
#include "zn_qspi_gd25q40.h"
#include "common.h"
#include "app_cfg.h"
#include "gconfig.h"
#ifdef _APPLE_TAG
#include "zn_app.h"
#include "dca.h"
#endif

#define SETTING_DELAY 100 //ms

#define PHASE_COMPEN_RX0_RX1  0.0
#define PHASE_COMPEN_RX0_RX2  -0.1575
#define DELAY_RX0_RX1         4           // pulseIdx0 - pulseIdx1

#define PI 3.1415926

volatile uint8_t g_aoa_threshold = 0;

int32_t g_tx_ant_delta = 0;

static zn_local_data_t s_global_zn_config = {0};

static uint8_t  s_read_mem_buf[2048]  = {0}; //2048
static uint8_t  s_cir_memory[100]     = {0}; //8192
static uint8_t  s_last_channel        = CHANNEL_9;
static uint32_t s_lna_on_cal_rx0[30]  = {0};
static uint32_t s_lna_off_cal_rx0[30] = {0};
static uint32_t s_lna_on_cal_rx1[30]  = {0};
static uint32_t s_lna_off_cal_rx1[30] = {0};

/*************************************************************************************************************
* Description: this function is used to read 32 - bit value from the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* Output param: None
* Return value: 32 bit register value
*************************************************************************************************************/
uint32_t zn_read_32bit_reg(uint32_t reg_file_id)
{
    return read_mreg32(reg_file_id);
}

/*************************************************************************************************************
* Description: this function is used to write 32 - bit value to the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* reg_val - the value to write
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_write_32bit_reg(uint32_t reg_file_id, uint32_t reg_val)
{
    write_mreg32(reg_file_id, reg_val);
}

/*************************************************************************************************************
* Description: this function is used to read 16 - bit value from the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* Output: None
* Return value: 16 bit register value
*************************************************************************************************************/
uint16_t zn_read_16bit_reg(uint32_t reg_file_id)
{
    return read_mreg16(reg_file_id);
}

/*************************************************************************************************************
* Description: this function is used to write 16 - bit value to the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* reg_val - the value to write
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_write_16bit_reg(uint32_t reg_file_id, uint32_t reg_val)
{
    write_mreg16(reg_file_id, reg_val);
}

/*************************************************************************************************************
* Description: this function is used to read an 8 - bit value from the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* Output param: None
* Return value: 8 - bit register value
*************************************************************************************************************/
uint8_t zn_read_8bit_reg(uint32_t reg_file_id)
{
    return read_mreg8(reg_file_id);
}

/*************************************************************************************************************
* Description: this function is used to write an 8 - bit value to the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* reg_val - the value to write
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_write_8bit_reg(uint32_t reg_file_id, uint8_t reg_val)
{
    write_mreg8(reg_file_id, reg_val);
}

/*************************************************************************************************************
* Description: this function is used to OR a 32 - bit value to the ZN device registers
* Input param:
* reg_file_id : ID of register file or buffer being accessed
* or_value : the value to OR to register
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_or_32bit_reg(const uint32_t reg_file_id, const uint32_t or_value)
{
    uint32_t reg_value = zn_read_32bit_reg(reg_file_id);

    reg_value |= or_value;
    zn_write_32bit_reg(reg_file_id, reg_value);

    return;
}

/*************************************************************************************************************
* Description: this function is used to AND a 32 - bit value to the ZN device registers
* Input param:
* reg_file_id : ID of register file or buffer being accessed
* and_value: the value to AND to register
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_and_32bit_reg(const uint32_t reg_file_id, const uint32_t and_value)
{
    uint32_t reg_value = zn_read_32bit_reg(reg_file_id);

    reg_value &= and_value;
    zn_write_32bit_reg(reg_file_id, reg_value);

    return;
}

/*************************************************************************************************************
* Description: this function is used to write a commond to the ZN device registers.
* Input param:
* cmd - fast command
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_write_fast_cmd(uint16_t cmd)
{
    uint32_t reg_value = zn_read_32bit_reg(FAST_CMD_ID);

    reg_value &= ~(FAST_CMD_BIT_MASK);
    reg_value |= ((cmd << FAST_CMD_BIT_OFFSET) & FAST_CMD_BIT_MASK);
    zn_write_32bit_reg(FAST_CMD_ID, reg_value);

    #if ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    zn_and_32bit_reg(FAST_CMD_VLD_ID, ~FAST_CMD_VLD_BIT_MASK);
    #endif

    zn_or_32bit_reg(FAST_CMD_VLD_ID, FAST_CMD_VLD_BIT_MASK);
}

/*************************************************************************************************************
* Description: this function is used to write to the ZN device registers.
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* length - number of bytes being written
* buffer - pointer to buffer containing the 'length' bytes to be written
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_write_to_device(uint32_t reg_file_id, uint16_t length, uint8_t *buffer)
{
    uint16_t idx = 0;
    uint16_t div_len = length / 4;
    uint16_t rem_len = length % 4;

    for(idx = 0; idx < div_len; idx++)
    {
        write_mreg32((reg_file_id + idx * 4), *((uint32_t *)(buffer + idx * 4)));
    }

    for(idx = 0; idx < rem_len; idx++)
    {
        write_mreg8((reg_file_id + div_len * 4 + idx), *(buffer + div_len * 4 + idx));
    }
}

/*************************************************************************************************************
* Description: this function is used to read from the ZN device registers
* Input param:
* reg_file_id - ID of register file or buffer being accessed
* length - number of bytes being read
* buffer - pointer to buffer in which to return the read data.
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_read_from_device(uint32_t reg_file_id, uint16_t length, uint8_t *buffer)
{
    uint16_t idx = 0;
    uint16_t div_len = length / 4;
    uint16_t rem_len = length % 4;

    for(idx = 0; idx < div_len; idx++)
    {
        *((uint32_t *)(buffer + idx * 4)) = read_mreg32(reg_file_id + idx * 4);
    }
    for(idx = 0; idx < rem_len; idx++)
    {
        *(buffer + div_len * 4 + idx) = read_mreg8(reg_file_id + div_len * 4 + idx);
    }
}

/*************************************************************************************************************
* Description: this function is used to init ZN global config
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_init_global_config(void)
{
    memset(&s_global_zn_config.cb_data, 0, sizeof(zn_cb_data_t));
    s_global_zn_config.cb_tx_done = NULL;
    s_global_zn_config.cb_rx_ok = NULL;
    s_global_zn_config.cb_rx_timeout = NULL;
    s_global_zn_config.cb_rx_err = NULL;
    s_global_zn_config.cb_rx_done = NULL;
    s_global_zn_config.cb_cp_err = NULL;
    s_global_zn_config.cb_prd = NULL;
    //s_global_zn_config.cb_cca_fail = NULL;
    s_global_zn_config.cb_rx_preamble_timeout = NULL;
    s_global_zn_config.cb_rx_sfd_timeout = NULL;
    s_global_zn_config.cb_rx_sfd_done = NULL;
    s_global_zn_config.cb_rx_phr_err = NULL;
}

/*************************************************************************************************************
* Description: this function is used to init ZN config
* Input param:
* channel_num - channel ID
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_analog_regs_init(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1608, 0x0060010c); //LED Type
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1600, 0x20002020);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x160c, 0x0c090c08); //LED Param0 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1610, 0x0a040c04); //LED Param1 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1614, 0x0a0f0a06); //LED Param2 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1634, 0x20002020); //CP_LED configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1638, 0x207003E8);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x163C, 0x00310460); //CP_LED Type
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1644, 0x0c090c08); //CP_LED Param0 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1648, 0x0a040c04); //CP_LED Param1 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x164C, 0x0a0f0a06); //CP_LED Param2 Configuration
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x00090e10); //VGA_Cal_Sel_RX1
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x00000000); //VGA_Cal_Sel_RX2
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0800, 0x508f808f); //Frequenc offset

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x09240000); //tia dcoc off
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, 0x60729304); //RX0 LNA stage2 ON
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, 0xa4a4e204); //RX1 LNA stage2 ON
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0820, 0x40a02400); //tia q fb cap, 0b 80, res double
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0850, 0xfb0c0d00); //tia i fb cap, 87 04, res double, rx2
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0874, 0x477e061f); //tia q fb cap, 0b 80, res double, rx2
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x087c, 0x00203f00);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1900, 0x2AFFCFC8);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1904, 0x57ffd4e4);

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1904, 0x76500024);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1908, 0x40fe8022);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007);

    //tx config
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1814, 0x00030000); //tx clk 125M 500M
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76392000); //tx_500M pulse 2ns
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x181c, 0x00000201);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1878, 0x00000000);
    //(channel_num == CHANNEL_5) ? (zn_write_32bit_reg(UWB_BASE_ADDR + 0x1828, 0x0009090b)):(zn_write_32bit_reg(UWB_BASE_ADDR + 0x1828, 0x0009090e));
    //(channelNum == CHANNEL_5) ? (zn_write_32bit_reg(UWB_BASE_ADDR + 0x1828, 0x00090903)):(zn_write_32bit_reg(UWB_BASE_ADDR + 0x1828, 0x0009090e)); //forhaoyun
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, 0xfc001000); //2014 add

    #ifdef _AOA_EN
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007); //pdoa digital config

    #ifdef _AOA_TYPE_2D
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x01c481c4); //pdoa digital en 2d
    #else
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x0bc489c4); //pdoa digital en 3d
    #endif

    #endif

    #elif (ZN_SOC == ZN3005)
    zn_or_32bit_reg(SYSCTRL_BASE_ADDR + 0x0c, (1 << 4));
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0300, 0x2AFFCFd0);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0304, 0x57ffd4e4);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03d0, 0x00030404);

    zn_write_32bit_reg(0X4301107c, 0x00b60000);
    mdelay(SETTING_DELAY);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x107c, 0x00b60000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1004, 0x000c1e00);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x0018FE00);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0xf8003000);
    mdelay(SETTING_DELAY);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x80000000);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, 0x00046400);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, 0x00000119);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, 0x6c000000);
    mdelay(SETTING_DELAY);
    zn_or_32bit_reg(UWB_BASE_ADDR + 0x1080, 0x0f); //tx rx
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1020, 0x439114ac);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1024, 0x00120f0c);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x100c, 0xE1FE0000);
    mdelay(SETTING_DELAY);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0108, 0x0000002f);
    mdelay(SETTING_DELAY);

    #ifdef _AOA_EN
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007); //pdoa digital config
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x01c481c4); //pdoa digital en
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, 0x01482800); //pdoa rf en
    #endif

    #elif (ZN_SOC == ZN3006)
//        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x00040009);//rfpll_tx_auto_enable
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x0034000b);//rfsw turn on, ldo voltage highest
//				zn_write_32bit_reg(UWB_BASE_ADDR + 0x100c, 0x40ff0000);//tx_gain_max
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x100c, 0x60ff0000);//pa_gain highest
//				zn_write_32bit_reg(UWB_BASE_ADDR + 0x1010, 0xe0000000);//tx_ldo_high
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1010, 0x40000000);//pa power ldo voltage setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1004, 0xe0000000);//buck_vref_prg
        mdelay(10);
        printf("\nread pll state:0x%x \r\n",zn_read_32bit_reg(UWB_BASE_ADDR + 0x1100));
                
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1020, 0x43112800);//8G
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1014, 0x18492018);//0x18492018  0x183cf018//xo ferq change
        
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x0108, 0x2f);//digital clk_tx_en
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x05bc, 0x02);//digital_clk_rx_en
                
        // rx0 AGC config
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x03D0, 0x02500404);//agc on
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03D4, 0x000801f4);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03D8, 0x00660020);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03DC, 0x03041333);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03E0, 0x00000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03E4, 0x0666080c);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03E8, 0x00280028);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03EC, 0x00280404);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05D8, 0x0938aec7);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05DC, 0x0677c810);
    //LNAgain
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05E0, 0xbc107420);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05E4, 0x8c105820);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05E8, 0xbd101860);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05EC, 0x87901a20);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05F0, 0x87e80fe0);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05F4, 0x87e20fc8);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05F8, 0x00000fc2);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05FC, 0x00360800);//AGC out Gain over,default:off 1dB
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x05FC, 0x00160800);//AGC out Gain over,default:off 1dB
    
    //RX1
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03F4, 0x02500404);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03F8, 0x000801f4);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03FC, 0x00660020);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0400, 0x03041333);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0404, 0x00000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0408, 0x0666080c);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x040C, 0x00280028);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0410, 0x00280404);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x0938aec7);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0604, 0x0677c810);
    //LNAgain
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0608, 0xbc107420);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x060C, 0x8c105820);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0610, 0xbd101860);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0614, 0x87901a20);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0618, 0x87e80fe0);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x061C, 0x87e20fc8);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0620, 0x00000fc2);
    
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0624, 0x00360800);//AGC out Gain over,default:off 1dB
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0624, 0x00160800);//AGC out Gain over,default:off 1dB
    
    // STS
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0214, 0x14eb220f);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0218, 0xf86050a8);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x021c, 0xd1d336aa);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0220, 0x14148674);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0204, 0x1f9a3de4);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0208, 0xd37ec3ca);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x020c, 0xc44fa8fb);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0210, 0x362ee834);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0200, 0x00400300);
    mdelay(100);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060,0x0006000b);//atest sw rfsw trx on
    mdelay(100);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c,0x1c400000);//rx vbgr ldoh1p8
    mdelay(100);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x103c,0xe0000000);//adc tune RX_CH1
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054,0xe0000000);//adc tune RX_CH2
    mdelay(100);
    printf("pll state:0x%08X! \r\n",zn_read_32bit_reg(UWB_BASE_ADDR + 0x1100));//pll state
    mdelay(100);
//    zn_or_32bit_reg(SYSCTRL_BASE_ADDR + 0x0c, (1 << 4));
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0300, 0x2AFFCFd0);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0304, 0x57ffd4e4);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x03d0, 0x00030404);

//    zn_write_32bit_reg(0X4301107c, 0x00b60000);
//    mdelay(SETTING_DELAY);
//    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x107c, 0x00b60000);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1004, 0x000c1e00);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x0018FE00);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0xf8003000);
//    mdelay(SETTING_DELAY);

//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x80000000);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, 0x00046400);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, 0x00000119);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, 0x6c000000);
//    mdelay(SETTING_DELAY);
//    zn_or_32bit_reg(UWB_BASE_ADDR + 0x1080, 0x0f); //tx rx
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1020, 0x439114ac);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1024, 0x00120f0c);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x100c, 0xE1FE0000);
//    mdelay(SETTING_DELAY);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0108, 0x0000002f);
//    mdelay(SETTING_DELAY);

    #ifdef _AOA_EN
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007); //pdoa digital config
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x01c481c4); //pdoa digital en
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, 0x01482800); //pdoa rf en
    #endif

    #endif
}

/*******************************************************************************************************************
* Description: This API function writes the supplied TX data into the ZN's TX buffer.
* Input param:
* tx_data_length - This is the total length of data (in bytes) to write to the tx buffer.
* tx_data_bytes - Pointer to the user s buffer containing the data to send.
* tx_buffer_offset - This specifies an offset in the ZN IC TX Buffer at which to start writing data.
* Output param: None
* Returns value: ZN_SUCCESS for success, or ZN_ERROR for error
*******************************************************************************************************************/
int32_t zn_write_tx_data(uint16_t tx_data_length, uint8_t *tx_data_bytes, uint16_t tx_buffer_offset)
{
    if (tx_data_length < TX_BUFFER_MAX_LEN)
    {
        zn_write_to_device(TX_BUF_BASE_ADDR + tx_buffer_offset, tx_data_length, tx_data_bytes);
        return ZN_SUCCESS;
    }
    else
    {
        return ZN_ERROR;
    }
}

/*******************************************************************************************************************
* Description: This function read the supplied TX data from the ZN's TX buffer.
* Input param:
* buffer - Pointer to the user's buffer containing the data to read.
* length - This is the total length of data (in bytes) to read from the tx buffer.
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_read_tx_data(uint8_t *buffer, uint16_t length)
{
    zn_read_from_device(TX_BUF_BASE_ADDR, length, buffer);
}

/*******************************************************************************************************************
* Description: This API function configures the TX frame control register before the transmission of a frame
* Input param:
* tx_frame_length - this is the length of TX message (including the 2 byte CRC) - max is 1023
* NOTE: standard PHR mode allows up to 127 bytes
* if > 127 is programmed, ZN_PHRMODE_EXT needs to be set in the phrMode configuration
* see zn_configure function
* tx_buffer_offset - the offset in the tx buffer to start writing the data
* ranging - 1 if this is a ranging frame, else 0
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_write_tx_frame_ctrl(uint16_t tx_frame_length, uint16_t tx_buffer_offset, uint8_t ranging)
{
    uint32_t reg32 = tx_frame_length 
                     | ((tx_buffer_offset << TX_DATA_CTRL_TXB_OFFSET_BIT_OFFSET) & TX_DATA_CTRL_TXB_OFFSET_BIT_MASK)
                     | ((ranging << TX_DATA_CTRL_TXTR_BIT_OFFSET) & (TX_DATA_CTRL_TXTR_BIT_MASK));

    zn_and_32bit_reg(TX_DATA_CTRL_ID, ~(TX_DATA_CTRL_TXB_OFFSET_BIT_MASK | TX_DATA_CTRL_TXFLEN_BIT_MASK | TX_DATA_CTRL_TXTR_BIT_MASK));
    zn_or_32bit_reg(TX_DATA_CTRL_ID, reg32);
}

/*******************************************************************************************************************
* Description: This is used to read the data from the RX buffer, from an offset location give by offset parameter
* Input param:
* buffer - the buffer into which the data will be read
* length - the length of data to read (in bytes)
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_read_rx_data(uint8_t *buffer, uint16_t length)
{
    zn_read_from_device(RX0_BUF_BASE_ADDR, length, buffer);
}

/*******************************************************************************************************************
* Description: This API function configures the delayed transmit time or the delayed RX on time
* Input param:
* start_time - the TX/RX start time (the 32 bits should be the high 32 bits of the system time at which to send the message,
* The time parameter used here is in 4ns units
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_set_delayed_trx_time(uint32_t start_time)
{
    zn_write_32bit_reg(DX_TIME_ID, start_time); // Note: bit 0 of this register is ignored
}

/*******************************************************************************************************************
* Description: This is used to read the TX timestamp (adjusted with the programmed antenna delay)
* Input param: None
* Output param:
* timestamp - a pointer to a 5-byte buffer which will store the read TX timestamp time
* Return value: None
*******************************************************************************************************************/
void zn_read_tx_timestamp(uint8_t *timestamp)
{
    zn_read_from_device(TX_STAMP_L8_ID, TX_STAMP_L8_LEN, timestamp);       // Read bytes directly into buffer
    zn_read_from_device(TX_STAMP_H32_ID, TX_STAMP_H32_LEN, timestamp + 1); // Read bytes directly into buffer

    return;
}

#if (ZN_SOC == ZN2014)
/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the TX timestamp raw (no adjusted with the programmed antenna delay)
* Input param: None
* Output param: None
* Return value: high 32-bits of TX timestamp raw
*******************************************************************************************************************/
uint32_t zn_read_tx_timestamp_raw(void)
{
    return zn_read_32bit_reg(TX_STAMP_RAW_ID); // Offset is 1 to get the 4 upper bytes out of 5
}
#endif

/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the TX timestamp (adjusted with the programmed antenna delay)
* Input param: None
* Output param: None
* Return value: high 32-bits of TX timestamp
*******************************************************************************************************************/
uint32_t zn_read_tx_timestamp_high32(void)
{
    return zn_read_32bit_reg(TX_STAMP_H32_ID);
}

/*******************************************************************************************************************
* Description: This is used to read the low 32 - bits of the TX timestamp (adjusted with the programmed antenna delay)
* Input param: None
* Output param: None
* Return value: low 32-bits of TX timestamp
*******************************************************************************************************************/
uint32_t zn_read_tx_timestamp_low32(void)
{
    uint32_t time_value = 0;

    time_value = read_mreg8(TX_STAMP_L8_ID);
    time_value |= (zn_read_32bit_reg(TX_STAMP_H32_ID) << TX_STAMP_L8_BIT_LEN);

    return time_value;
}

/*******************************************************************************************************************
* Description: This is used to read the TX timestamp (adjusted with the programmed antenna delay) for STS case
* Input param: None
* Output param:
* timestamp - a pointer to a 5-byte buffer which will store the read TX timestamp time
* Return value: None
*******************************************************************************************************************/
void zn_read_tx_timestamp_sts(uint8_t *timestamp)
{
    zn_read_from_device(IP_STS_TX_STAMP_L8, IP_STS_TX_STAMP_L8_LEN, timestamp);       // Read bytes directly into buffer
    zn_read_from_device(IP_STS_TX_STAMP_H32, IP_STS_TX_STAMP_H32_LEN, timestamp + 1); // Read bytes directly into buffer

    return;
}

/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the TX timestamp (adjusted with the programmed antenna delay) for STS case
* Input param: None
* Output param: None
* Return value: high 32-bits of TX timestamp
*******************************************************************************************************************/
uint32_t zn_read_tx_timestamp_high32_sts(void)
{
    return zn_read_32bit_reg(IP_STS_TX_STAMP_H32);
}

/*******************************************************************************************************************
* Description: This is used to read the low 32 - bits of the TX timestamp (adjusted with the programmed antenna delay) for STS case
* Input Param: None
* Output Param: None
* Return value: low 32-bits of TX timestamp
*******************************************************************************************************************/
uint32_t zn_read_tx_timestamp_low32_sts(void)
{
    uint32_t time_value = read_mreg8(IP_STS_TX_STAMP_L8);

    time_value |= (zn_read_32bit_reg(IP_STS_TX_STAMP_H32) << 8);

    return time_value;
}

/*******************************************************************************************************************
* Description: This is used to read the RX timestamp (adjusted time of arrival)
* Input param: None
* Output param:
* timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
* Return value: None
*******************************************************************************************************************/
void zn_read_rx_timestamp(uint8_t *timestamp)
{
    zn_read_from_device((IP_TOA_L8_ID + (IP_TOA_L8_BIT_OFFSET / IP_TOA_L8_BIT_LEN)), IP_TOA_L8_LEN, timestamp); // Get the adjusted time of arrival
    zn_read_from_device(IP_TOA_H32_ID, IP_TOA_H32_LEN, timestamp + 1); // Get the adjusted time of arrival
}

#if (ZN_SOC == ZN2014)
/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the RX timestamp raw (no adjusted with the programmed antenna delay)
* Input Param: None
* Output Param: None
* Return value: high 32-bits of RX timestamp raw
*******************************************************************************************************************/
uint32_t zn_read_rx_timestamp_raw(void)
{
    return zn_read_32bit_reg(RX_STAMP_RAW_ID);
}
#endif

/******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the RX timestamp
* Input param: None
* Output param: None
* Return value: high 32-bits of RX timestamp
*******************************************************************************************************************/
uint32_t zn_read_rx_timestamp_high32(void)
{
    return zn_read_32bit_reg(IP_TOA_H32_ID); // Offset is 1 to get the 4 upper bytes out of 5 byte tiemstamp
}

/*******************************************************************************************************************
* Description: This is used to read the low 32 - bits of the RX timestamp
* Input param: None
* Output param: None
* Return value: high 32-bits of RX timestamp
*******************************************************************************************************************/
uint32_t zn_read_rx_timestamp_low32(void)
{
    uint32_t time_value = read_mreg8(IP_TOA_L8_ID + 3);

    time_value |= (zn_read_32bit_reg(IP_TOA_H32_ID) << TX_STAMP_L8_BIT_LEN);

    return time_value;
}

/*******************************************************************************************************************
* Description: This is used to read the RX timestamp (adjusted time of arrival) w.r.t. STS CIR
* Input param: None
* Output param:
* timestamp - a pointer to a 5-byte buffer which will store the read RX timestamp time
* Return value: no return value
*******************************************************************************************************************/
void zn_read_rx_timestamp_sts(uint8_t *timestamp)
{
    zn_read_from_device((CP_TOA_L8_ID + 3), CP_TOA_L8_LEN, timestamp); // Get the adjusted time of arrival
    zn_read_from_device(CP_TOA_H32_ID, CP_TOA_H32_LEN, timestamp + 1); // Get the adjusted time of arrival
}

/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the RX timestamp for STS case
* Input param: None
* Output param: None
* Return value: high 32-bits of RX timestamp
*******************************************************************************************************************/
uint32_t zn_read_rx_timestamp_high32_sts(void)
{
    return zn_read_32bit_reg(CP_TOA_H32_ID);
}

/*******************************************************************************************************************
* Description: This is used to read the low 32 - bits of the RX timestamp for STS case
* Input param: None
* Output param: None
* Return value: low 32-bits of RX timestamp
*******************************************************************************************************************/
uint32_t zn_read_rx_timestamp_low32_sts(void)
{
    uint32_t time_value = read_mreg8(CP_TOA_L8_ID + 3);

    time_value |= (zn_read_32bit_reg(CP_TOA_H32_ID) << 8);

    return time_value;
}

/*******************************************************************************************************************
* Description: This is used to read the high 32 - bits of the system time
* Input Param: None
* Output Param: None
* Return value: 32 - bits of the system time
*******************************************************************************************************************/
uint32_t zn_get_sys_time(void)
{
    #if((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    uint32_t reg_val = zn_read_32bit_reg(FAST_CMD_VLD_ID);
    zn_write_32bit_reg(FAST_CMD_VLD_ID, (reg_val & (~FAST_CMD_VLD_SYS_TIMER_VLD_BIT_MASK)));
    zn_write_32bit_reg(FAST_CMD_VLD_ID, (reg_val | FAST_CMD_VLD_SYS_TIMER_VLD_BIT_MASK));
    #endif

    return zn_read_32bit_reg(SYS_TIME_ID);
}

/********************************************************************************************************************
* Description: This sets the receiver turn on delay time after a transmission of a frame
* Input Param:
* rx_delay_time - (20 bits) - the delay is in UWB microseconds
* Output Param: None
* Return value: None
********************************************************************************************************************/
void zn_set_rx_after_tx_delay(uint32_t rx_delay_time)
{
    uint32_t val = zn_read_32bit_reg(ACK_RESP_ID);            // Read ACK_RESP_T_ID register

    val &= (~ACK_RESP_WAIT4RESP_TIM_BIT_MASK);                // Clear the timer (19:0)

    val |= (rx_delay_time & ACK_RESP_WAIT4RESP_TIM_BIT_MASK); // In UWB microseconds (e.g. turn the receiver on 20uus after TX)

    zn_write_32bit_reg(ACK_RESP_ID, val);
}

/*******************************************************************************************************************
* Description: This function is used to register the different callbacks called when one of the corresponding event occurs.
*
* NOTE: Callbacks can be undefined (set to NULL).In this case, zn_isr() will process the event as usual but the 'null'
* callback will not be called.
*
* Input param:
* cb_tx_done - the pointer to the TX confirmation event callback function
* cb_rx_ok - the pointer to the RX good frame event callback function
* cb_rx_to - the pointer to the RX timeout events callback function
* cb_rx_err - the pointer to the RX error events callback function
* cb_rx_done - the pointer to the Rx Done events callback function
* cb_cp_err - the pointer to the cp error events callback function
* cb_prd - the pointer to the prd events callback function
* Output param: None
* Return value: None
*******************************************************************************************************************/
void zn_set_callbacks(zn_cb_t cb_tx_done, zn_cb_t cb_rx_ok, zn_cb_t cb_rx_to, zn_cb_t cb_rx_err, zn_cb_t cb_rx_done, zn_cb_t cb_cp_err, zn_cb_t cb_prd /*, zn_cb_t cb_cca_fail */)
{
    NVIC_DisableIRQ(UWB_IRQn);
    s_global_zn_config.cb_tx_done = cb_tx_done;
    s_global_zn_config.cb_rx_ok = cb_rx_ok;
    s_global_zn_config.cb_rx_timeout = cb_rx_to;
    s_global_zn_config.cb_rx_err = cb_rx_err;
    s_global_zn_config.cb_rx_done = cb_rx_done;
    s_global_zn_config.cb_cp_err = cb_cp_err;
    s_global_zn_config.cb_prd = cb_prd;
    //s_global_zn_config.cb_cca_fail = cb_cca_fail;
    NVIC_EnableIRQ(UWB_IRQn);
}

/*******************************************************************************************************************
* Description: This function is used to register the different callbacks called when one of the corresponding event occurs.
*
* NOTE: Callbacks can be undefined (set to NULL).In this case, zn_isr() will process the event as usual but the 'null'
* callback will not be called.
*
* Input Param:
* cb_rx_preamble_timeout - the pointer to the preamble timeout events callback function
* cb_rx_sfd_timeout - the pointer to the SFD timeout events callback function
* cb_rx_sfd_done - the pointer to the SFD Done events callback function
* cb_rx_phr_err - the pointer to the PHR error events callback function
* cb_rx_phd - the pointer to the phd events callback function
* Output param: None
* Return value: None
*******************************************************************************************************************/
void zn_set_callbacks_ex(zn_cb_t cb_rx_preamble_timeout, zn_cb_t cb_rx_sfd_timeout, zn_cb_t cb_rx_sfd_done, zn_cb_t cb_rx_phr_err, zn_cb_t cb_rx_phd)
{
    NVIC_DisableIRQ(UWB_IRQn);
    s_global_zn_config.cb_rx_preamble_timeout = cb_rx_preamble_timeout;
    s_global_zn_config.cb_rx_sfd_timeout = cb_rx_sfd_timeout;
    s_global_zn_config.cb_rx_sfd_done = cb_rx_sfd_done;
    s_global_zn_config.cb_rx_phr_err = cb_rx_phr_err;
    s_global_zn_config.cb_rx_phd = cb_rx_phd;
    NVIC_EnableIRQ(UWB_IRQn);
}

/*************************************************************************************************************
* Description: This call enables preamble timeout (SY_STAT_RXPTO event)
* Input Param:
* timeout - Preamble detection timeout, expressed in multiples of PAC size.The counter automatically adds 1 PAC
* size to the value set.Min value that can be set is 1 (i.e. a timeout of 2 PAC size).
* Output Param: None
* Return value: None
*************************************************************************************************************/
void zn_set_preamble_detect_timeout(uint16_t timeout)
{
    zn_write_32bit_reg(DRX_PRETOC_ID, (timeout << DRX_PRETOC_BIT_OFFSET));
}

/*************************************************************************************************************
* Description: This call enables sfd timeout (SY_STAT_RXPTO event)
* Input Param:
* timeout - sfd detection timeout, expressed in multiples of PAC size.The counter automatically adds 1 PAC
* size to the value set.Min value that can be set is 1 (i.e. a timeout of 2 PAC size).
* Output Param: None
* Return value: None
*************************************************************************************************************/
void zn_set_sfd_le_detect_timeout(uint16_t timeout)
{
    zn_write_32bit_reg(DRX_SFDTOC_ID, (timeout << DRX_PRETOC_BIT_OFFSET));
}

/*****************************************************************************************
* Description: This is the ZN's general Interrupt Service Routine.It will process
* Report_buf the following events:
* - RXFR + no data mode (through cbRxOk callback, but set data_length to 0)
* - RXFCG (through cbRxOk callback)
* - TXFRS (through cbTxDone callback)
* - RXRFTO/RXPTO (through cbRxTo callback)
* - RXPHE/RXFCE/RXRFSL/RXSFDTO/AFFREJ/LDEERR/LCSSERR (through cbRxTo cbRxErr)
* -
* For all events, corresponding interrupts are cleared and necessary resets are performed.
* In addition, in the RXFCG case, received frame information and frame control are read
* before calling the callback.
* If double buffering is activated, it will also toggle between reception buffers once
* the reception callback processing has ended.
* This version of the ISR supports double buffering but does not support automatic RX re - enabling!
* NOTE: In PC based system using (Cheetah or ARM) USB to SPI converter there can be no interrupts, however we still need something
* to take the place of it and operate in a polled way.In an embedded system this function should be configured to be triggered
* on any of the interrupts described above.
* Input param: None
* Output param: None
* Return value: None
*****************************************************************************************/
#if (ZN_SOC == ZN2014)
void zn_isr(void)
{
    uint32_t status   = zn_read_32bit_reg(SYS_STATUS_ID);
    uint32_t info32   = zn_read_32bit_reg(RX_INFO_BUF0_ID);
    uint32_t irq_flag = ~(SYS_STATUS_TXDONE_BIT_MASK | SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_EVENT_ENABLE_RX_ERR |SYS_MASK_RXDONE_ENABLE_BIT_MASK | SYS_STATUS_RXSTSE_BIT_MASK
                        | SYS_STATUS_RXPRD_BIT_MASK | SYS_STATUS_RXSFDD_BIT_MASK | SYS_STATUS_RXSFDTO_BIT_MASK | SYS_STATUS_RXPHD_BIT_MASK | SYS_STATUS_RXPHE_BIT_MASK);

    s_global_zn_config.cb_data.status = status;
    s_global_zn_config.cb_data.rx_flags = 0;

    if(g_work_mode == WORK_MODE_NULL)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, status);
        zn_write_32bit_reg(SYS_STATUS_ID, irq_flag);
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_IRQS_BIT_MASK);

        return;
    }

    // Handle TX confirmation event
    if(status & SYS_STATUS_TXDONE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_ALL_TX); // Clear TX event bits
        // Call the corresponding callback if present
        if(s_global_zn_config.cb_tx_done != NULL)
        {
            s_global_zn_config.cb_tx_done(&s_global_zn_config.cb_data);
        }
    }

    if (status & SYS_STATUS_RXFCG_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD); // Clear all receive status bits

        s_global_zn_config.cb_data.rx_flags = 0;
        s_global_zn_config.cb_data.data_length = (info32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;

        // Report ranging bit
        if(info32 & RX_INFO_BUF0_RNG_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_RNG;
        }
        //zn_read_from_device(RX_INFO_BUF0_ID, s_global_zn_config.cb_data.data_length, s_global_zn_config.cb_data.data);

        if (status & SYS_STATUS_RXCIADONE_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_CIA;
        }

        // Call the corresponding callback if present
        if(s_global_zn_config.cb_rx_ok != NULL)
        {
            s_global_zn_config.cb_rx_ok(&s_global_zn_config.cb_data);
        }
    }

    // Handle frame reception/preamble detect timeout events
    if(status & SYS_STATUS_ALL_RX_TO)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO); // Clear RX timeout event bits
        // Call the corresponding callback if present
        if(s_global_zn_config.cb_rx_timeout != NULL)
        {
            s_global_zn_config.cb_rx_timeout(&s_global_zn_config.cb_data);
        }
    }

    // Handle RX errors events
    if(status & SYS_EVENT_ENABLE_RX_ERR)//SYS_STATUS_ALL_RX_ERR
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR|SYS_STATUS_ALL_RX_TO); // Clear RX error event bits
        s_global_zn_config.cb_data.data_length = (info32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;
        // zn_read_from_device(RX_INFO_BUF0_ID, s_global_zn_config.cb_data.data_length, s_global_zn_config.cb_data.data);
        if(s_global_zn_config.cb_rx_err != NULL)
        {
            s_global_zn_config.cb_rx_err(&s_global_zn_config.cb_data);
        }
    }

    if(status & SYS_STATUS_RXSTSE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSTSE_BIT_MASK); //clear cperr bit
        if(s_global_zn_config.cb_cp_err != NULL)
        {
            s_global_zn_config.cb_cp_err(&s_global_zn_config.cb_data);
        }
    }

    if(status & SYS_STATUS_RXPRD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXPRD_BIT_MASK); //PRD
        if(s_global_zn_config.cb_prd != NULL)
        {
            s_global_zn_config.cb_prd(&s_global_zn_config.cb_data);
        }
    }

    if(status & SYS_STATUS_RXSFDD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSFDD_BIT_MASK); //SFDD
        if(s_global_zn_config.cb_rx_sfd_done != NULL)
        {
            s_global_zn_config.cb_rx_sfd_done(&s_global_zn_config.cb_data);
        }
    }

    // Handle RX frame done events
    if(status & SYS_MASK_RXDONE_ENABLE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_MASK_RXDONE_ENABLE_BIT_MASK); // Clear RX timeout event bits
        s_global_zn_config.cb_data.data_length = (info32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;
        //zn_read_from_device(RX_INFO_BUF0_ID, s_global_zn_config.cb_data.data_length, s_global_zn_config.cb_data.data);
        // Report ranging bit
        if(info32 & RX_INFO_BUF0_RNG_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_RNG;
        }
        // Call the corresponding callback if present
        if((s_global_zn_config.cb_rx_done != NULL)
           && (!(status & SYS_STATUS_ALL_RX_TO))
           && (!(status & SYS_EVENT_ENABLE_RX_ERR))
           && (!(status & SYS_STATUS_RXSTSE_BIT_MASK)))
        {
            s_global_zn_config.cb_rx_done(&s_global_zn_config.cb_data);
        }
    }

    if (status & SYS_STATUS_RXSFDTO_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSFDTO_BIT_MASK); //clear prd bit
        if(s_global_zn_config.cb_rx_sfd_timeout != NULL)
        {
            s_global_zn_config.cb_rx_sfd_timeout(&s_global_zn_config.cb_data);
        }
    }
    if (status & SYS_STATUS_RXPHD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXPHD_BIT_MASK); //clear prd bit
        if(s_global_zn_config.cb_rx_phd != NULL)
        {
            s_global_zn_config.cb_rx_phd(&s_global_zn_config.cb_data);
        }
    }
    if (status & SYS_STATUS_RXPHE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXPHE_BIT_MASK); //clear prd bit
        if(s_global_zn_config.cb_rx_phr_err != NULL)
        {
            s_global_zn_config.cb_rx_phr_err(&s_global_zn_config.cb_data);
        }
    }

    zn_write_32bit_reg(SYS_STATUS_ID, irq_flag);
    zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_IRQS_BIT_MASK);

    return;
}
#elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
void zn_isr(void)
{
    uint32_t status = zn_read_32bit_reg(SYS_STATUS_ID); 
    uint32_t finfo32 = zn_read_32bit_reg(RX_INFO_BUF0_ID);
      //printf("zn_isr:0X%0X \r\n",status);
    s_global_zn_config.cb_data.status = status;
    s_global_zn_config.cb_data.rx_flags = 0;
    
    if(status & SYS_STATUS_TXDONE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_TXDONE_BIT_MASK); // Clear TX event bits
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_TXDONE_BIT_MASK)));
        #endif

        // Call the corresponding callback if present
        if(s_global_zn_config.cb_tx_done != NULL)
        {
            s_global_zn_config.cb_tx_done(&s_global_zn_config.cb_data);
        }
    }

    if (status & SYS_STATUS_RXFCG_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK); // Clear all receive status bits
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXFCG_BIT_MASK)));
        #endif
        s_global_zn_config.cb_data.rx_flags = 0;
        s_global_zn_config.cb_data.data_length = (finfo32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;

        // Report ranging bit
        if(finfo32 & RX_INFO_BUF0_RNG_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_RNG;
        }
        //zn_readfromdevice(RX_INFO_BUF0_ID, s_global_zn_config.cb_data.data_length, s_global_zn_config.cb_data.data);
        
        if (status & SYS_STATUS_RXCIADONE_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_CIA;
        }

        // Call the corresponding callback if present
        if(s_global_zn_config.cb_rx_ok != NULL)
        {
            s_global_zn_config.cb_rx_ok(&s_global_zn_config.cb_data);
        }
    }

    // Handle frame reception/preamble detect timeout events
    if(status & SYS_STATUS_ALL_RX_TO)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO); // Clear RX timeout event bits
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~(SYS_STATUS_ALL_RX_TO))));
        #endif
        // Call the corresponding callback if present
        if(s_global_zn_config.cb_rx_timeout != NULL)
        {
            s_global_zn_config.cb_rx_timeout(&s_global_zn_config.cb_data);
        }
    }

    // Handle RX errors events
    if(status & SYS_EVENT_ENABLE_RX_ERR)//SYS_STATUS_ALL_RX_ERR
    {        
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_EVENT_ENABLE_RX_ERR); // Clear RX error event bits
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_EVENT_ENABLE_RX_ERR)));
        #endif
        s_global_zn_config.cb_data.data_length = (finfo32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;
        if(s_global_zn_config.cb_rx_err != NULL)
        {
            s_global_zn_config.cb_rx_err(&s_global_zn_config.cb_data);
        }
    }

    if(status & SYS_STATUS_RXSTSE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSTSE_BIT_MASK);//clear cperr bit
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXSTSE_BIT_MASK)));
        #endif
        if(s_global_zn_config.cb_cp_err != NULL)
        {
            s_global_zn_config.cb_cp_err(&s_global_zn_config.cb_data);
        }
    }

    if(status & SYS_STATUS_RXPRD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXPRD_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXPRD_BIT_MASK)));
        #endif

        //if(s_global_zn_config.cbRXPreambleDone != NULL)
        {
            //s_global_zn_config.cbRXPreambleDone(&s_global_zn_config.cb_data);
        } 
    }
    
    if(status & SYS_STATUS_RXSFDD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSFDD_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXSFDD_BIT_MASK)));
        #endif

        if(s_global_zn_config.cb_rx_sfd_done != NULL)
        {
            s_global_zn_config.cb_rx_sfd_done(&s_global_zn_config.cb_data);
        } 
    }
 
    if(status & SYS_STATUS_RXCCAFIAL_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXCCAFIAL_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXCCAFIAL_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXCPCIAFAIL_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXCPCIAFAIL_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXCPCIAFAIL_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXCIAFAIL_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXCIAFAIL_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXCIAFAIL_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXSFDTO_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSFDTO_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXSFDTO_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXRFSL_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXRFSL_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXRFSL_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXRSCS_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXRSCS_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXRSCS_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXPHD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXPHD_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXPHD_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXCPCIADONE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXCPCIADONE_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXCPCIADONE_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXCIADONE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXCIADONE_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXCIADONE_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXSTSD_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXSTSD_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXSTSD_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_TXSTS_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_TXSTS_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_TXSTS_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_TXPHS_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_TXPHS_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_TXPHS_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_TXPRS_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_TXPRS_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_TXPRS_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_TXFRB_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_TXFRB_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_TXFRB_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_RXOVRR_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXOVRR_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXOVRR_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_AAT_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_AAT_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_AAT_BIT_MASK)));
        #endif
    }
    if(status & SYS_STATUS_AFFREJ_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_AFFREJ_BIT_MASK);
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_AFFREJ_BIT_MASK)));
        #endif
    }
 
    // Handle RX frame done events
    if(status & SYS_STATUS_RXDONE_BIT_MASK)
    {
        zn_write_32bit_reg(SYS_STATUS_ID, SYS_STATUS_RXDONE_BIT_MASK); // Clear RX timeout event bits
        #if (ZN_SOC == ZN3005)
        zn_write_32bit_reg(SYS_STATUS_ID,(status & (~SYS_STATUS_RXDONE_BIT_MASK)));
        #endif

        s_global_zn_config.cb_data.data_length = (finfo32 & RX_INFO_BUF0_RXFLEN_BIT_MASK) >> RX_INFO_BUF0_RXFLEN_BIT_OFFSET;
        // Report ranging bit
        if(finfo32 & RX_INFO_BUF0_RNG_BIT_MASK)
        {
            s_global_zn_config.cb_data.rx_flags |= ZN_CB_DATA_RX_FLAG_RNG;
        }
        // Call the corresponding callback if present
        if((s_global_zn_config.cb_rx_done != NULL)
            /*&& (!(status & SYS_STATUS_ALL_RX_TO))
            && (!(status & SYS_EVENT_ENABLE_RX_ERR))
            && (!(status & SYS_STATUS_RXSTSE_BIT_MASK))*/)
        {
            s_global_zn_config.cb_rx_done(&s_global_zn_config.cb_data);
        }
    }
    zn_write_32bit_reg(SYS_STATUS_IRQS_ID, SYS_STATUS_IRQS_BIT_MASK);
    #if (ZN_SOC == ZN3005)
    zn_write_32bit_reg(SYS_STATUS_IRQS_ID, ~SYS_STATUS_IRQS_BIT_MASK);
    #endif

    return;
}
#endif

/*******************************************************************************************************************
* Description: This call initiates the transmission
* Input param:
* mode - if mode = ZN_START_TX_IMMEDIATE - immediate TX (no response expected)
* if mode = ZN_START_TX_DELAYED - delayed TX (no response expected) at specified time (time in DX_TIME register)
* Output param: None
* Return value: ZN_SUCCESS for success, or ZN_ERROR for error (e.g. a delayed transmission will be cancelled if the delayed time has passed)
*******************************************************************************************************************/
int32_t zn_start_tx(uint8_t mode)
{
    if(mode & ZN_START_TX_DELAYED)
    {
        zn_write_fast_cmd(CMD_DTX);
    }
    else
    {
        zn_write_fast_cmd(CMD_TX);
    }

    return ZN_SUCCESS;
}

/*******************************************************************************************************************
* Description: This is used to turn off the transceiver
* Input param: None
* Output param: None
* Return value: None
*******************************************************************************************************************/
void zn_force_trx_off(void)
{
    zn_write_fast_cmd(CMD_TXRXOFF);
}

/*******************************************************************************************************************
* Description: This call turns on the receiver, can be immediate or delayed (depending on the mode parameter).In the case of a
* "late" error the receiver will only be turned on if the ZN_IDLE_ON_DLY_ERR is not set.
* The receiver will stay turned on, listening to any messages until
* it either receives a good frame, an error (CRC, PHY header, Reed Solomon) or it times out (SFD, Preamble or Frame).
* Input param:
* mode - this can be one of the following allowed values:
* ZN_START_RX_IMMEDIATE 0x00 Enable the receiver immediately
* ZN_START_RX_DELAYED 0x01 Set up delayed RX, if "late" error triggers, then the RX will be enabled immediately
* Output param: None
* Return value: ZN_SUCCESS for success, or ZN_ERROR for error
*******************************************************************************************************************/
int32_t zn_rx_enable(uint8_t mode)
{
    if(mode == ZN_START_RX_IMMEDIATE)
    {
        zn_write_fast_cmd(CMD_RX);
    }
    else if(mode == ZN_START_RX_DELAYED)
    {
        zn_write_fast_cmd(CMD_DRX);
    }

    return ZN_SUCCESS;
}

/*************************************************************************************************************
* Description: This call enables RX timeout (SY_STAT_RFTO event)
* Input param: 
* time - how long the receiver remains on from the RX enable command
* The time parameter used here is in 1us units
* If set to 0 the timeout is disabled.
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_set_rx_timeout(uint32_t time)
{
    zn_write_32bit_reg(RX_FWTO_ID, time * 125);
    zn_or_32bit_reg(SYS_CFG_ID, SYS_CFG_RXWTOE_BIT_MASK); //set the RX FWTO bit
}

/*************************************************************************************************************
* Description: This call get device state
* Input param: None
* Output param: None
* Return value: 1:INIT 2:IDLE_RC 3:IDLE 4:TX state 5:RX state
*************************************************************************************************************/
uint8_t zn_get_pmsc_state(void)
{
    return ((zn_read_32bit_reg(SYS_STATE_ID) & SYS_STATE_PMSC_BIT_MASK) >> SYS_STATE_PMSC_BIT_OFFSET);
}

/*************************************************************************************************************
* Description: This function enables the specified events to trigger an interrupt.
* The following events can be found in SYS_ENABLE_LO register.
* Input param:
* bit_mask - sets the events which will generate interrupt
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_set_interrupt(uint32_t bit_mask)
{
    NVIC_DisableIRQ(UWB_IRQn);
    zn_write_32bit_reg(SYS_MASK_ID, bit_mask); // New value
    NVIC_EnableIRQ(UWB_IRQn);
}

/*************************************************************************************************************
* Description: This function is dc estimate.
* Input param: flag
* Output param: None
* Return value: dc estimate value
*************************************************************************************************************/
int32_t zn_dc_estimate(int32_t flag)
{
    int32_t dc = 0;

    #if (ZN_SOC == ZN2014)
    if(flag < 2)
    {
        zn_write_32bit_reg(ADC_SYNC_ID, 0x1c400000);
        mdelay(10);
        zn_write_32bit_reg(ADC_SYNC_ID, 0x1c480000);
    }
    else
    {
        zn_write_32bit_reg(ADC_SYNC_ID, 0x1c500000);
        mdelay(10);
        zn_write_32bit_reg(ADC_SYNC_ID, 0x1c580000);
    }

    if(flag == 0 || flag == 2)
    {
        zn_read_from_device(UWB_BASE_ADDR + 0xA000, 512, s_read_mem_buf);
    }
    else
    {
        zn_read_from_device(UWB_BASE_ADDR + 0xA800, 512, s_read_mem_buf);
    }
    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    if(flag < 2)
    {
              zn_write_32bit_reg(ADC_SYNC_ID, 0x0E200588);
              mdelay(10);
              zn_write_32bit_reg(ADC_SYNC_ID, 0x0E200488);
    }
    else
    {
        zn_write_32bit_reg(ADC_SYNC_ID, 0x0e220588);
              mdelay(10);
              zn_write_32bit_reg(ADC_SYNC_ID, 0x0e220488);
    }

    if(flag == 0 )
    {
        zn_read_from_device(AHB1_SLAVE_BASE_ADDR, 512, s_read_mem_buf);
    }
        else if(flag == 1 )
    {
        zn_read_from_device(AHB1_SLAVE_BASE_ADDR + 1024*2, 512, s_read_mem_buf);
    }
        else if(flag == 2 )
    {
        zn_read_from_device(AHB1_SLAVE_BASE_ADDR + 1024, 512, s_read_mem_buf);
    }
    else
    {
        zn_read_from_device(AHB1_SLAVE_BASE_ADDR + 1024*3, 512, s_read_mem_buf);
    }
    #endif

    for (uint16_t pack = 0; pack < 512 / 64; pack++)
    {
        for(uint16_t pos = 0; pos < 64; pos++)
        {
            dc += (s_read_mem_buf[pack * 64 + pos]>>2)- 16;
        }
    }

    return (dc >> 9);
}

/*************************************************************************************************************
* Description: This function is dc Check.
* Input param:
* flag - I or Q flag
* chain_flag - chain flag
* addr - register addr
* proc_flag - proc flag
* Output param:
* gain - gain value
* dc_tmp - dc value
* dc_v - dc value
* r - return value
* return_flag - return flag
* Return value: None
*************************************************************************************************************/
void zn_dc_check(int32_t flag/*I or Q */, int32_t chain_flag/* P or n for step 0 or Q*/, int32_t *gain, int32_t *dc_tmp, int32_t *dc_v, int32_t *r , int32_t *return_flag, int32_t addr, int32_t proc_flag)
{
    int32_t gain_used = (proc_flag == 0 ? 0 : *gain);
    int32_t gain_tmp  = gain_used;
    int32_t dc        = 0;
    int32_t i         = 0; 

    for (i = 1; i < 128; i++)
    {
        // set gain
        if(chain_flag == 0)
        {
            zn_write_32bit_reg(addr, gain_used + (i << 24));
        }
        else if(chain_flag == 1)
        {
            zn_write_32bit_reg(addr, gain_used + (i << 16));
        }
        else if(chain_flag == 2)
        {
            zn_write_32bit_reg(addr, gain_used + (i << 8));
        }
        else
        {
            zn_write_32bit_reg(addr, gain_used + i );
        }

        // read DC
        dc = zn_dc_estimate(flag);
        if((proc_flag == 0) && (i == 1))
        {
            *dc_tmp = dc;
        }

        if(abs(dc) <= abs(*dc_tmp))
        {
            *dc_tmp = dc;
            dc_v[chain_flag] = dc;
            r[chain_flag] = i;

            if(chain_flag == 0)
            {
                gain_tmp = gain_used + (i << 24);
            }
            else if(chain_flag == 1)
            {
                gain_tmp = gain_used + (i << 16);
            }
            else if(chain_flag == 2)
            {
                gain_tmp = gain_used + (i << 8);
            }
            else
            {
                gain_tmp = gain_used + i ;
            }

            if(abs(dc) < 1)
            {
                *return_flag = 1;
                *gain = gain_tmp;

                return;
            }
        }
    }
    *gain = gain_tmp;
}

/*************************************************************************************************************
* Description: This function is to get VGA dc value one time.
* Input param:
* iq_flag - iq flag
* addr - register addr
* Output param: None
* Return value: VGA dc value
*************************************************************************************************************/
int32_t zn_vga_dc_one_time(int32_t iq_flag, uint32_t addr)
{
    int32_t dc             = 0;
    int32_t dc_tmp         = 0;
    int32_t gain           = 0; 
    int32_t gain_tmp       = 0;
    int32_t reg_value      = 0;
    int32_t compelete_flag = 0;
    int32_t dc_v[4]        = {15, 15, 15, 15};
    int32_t r[4]           = {0};
    int32_t min_dc         = 0;
    int32_t min_r          = 0;
    int32_t flag           = 0;
    int32_t i              = 0;

    dc = zn_dc_estimate(iq_flag);
    dc_tmp = dc;
    dc_v[0] = dc_tmp;

    for (i = 1; i < 128; i++)
    {
        zn_write_32bit_reg(addr, i << 24);
        dc = zn_dc_estimate(iq_flag);

        if(abs(dc) < abs(dc_tmp))
        {
            dc_tmp = dc;
            dc_v[0] = dc;
            r[0] = i;
            gain = i << 24;
            if(abs(dc) < 1)
            {
                compelete_flag = 1;
                break;
            }
        }
    }

    if(compelete_flag == 0)
    {
        zn_dc_check(iq_flag, 1, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 0);
    }

    if(compelete_flag == 0)
    {
        zn_dc_check(iq_flag, 2, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 0);
    }

    if(compelete_flag == 0)
    {
        zn_dc_check(iq_flag, 3, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 0);
    }

    min_dc = dc_v[0];
    min_r = r[0];
    flag = 0;

    for(i = 1; i < 4; i++)
    {
        if(abs(dc_v[i]) < abs(min_dc))
        {
            min_r = r[i];
            flag = i;
            min_dc = dc_v[i];
        }
    }

    if(abs(min_dc) > 13)
    {
        // first
        gain = 127 << 24;
        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 2, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 3, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        // second
        gain = 127 << 16;
        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 2, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 3, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        // third
        gain = 127 << 8;
        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 0, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 1, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        // forth
        gain = 127;
        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 0, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }

        if(compelete_flag == 0)
        {
            zn_dc_check(iq_flag, 1, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
        }
    }
    else if(abs(min_dc) > 1)
    {
        if(flag == 0 || flag == 1)
        {
            gain = (flag == 0 ? min_r << 24 : min_r << 16);
            if(compelete_flag == 0)
            {
                zn_dc_check(iq_flag, 2, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
            }

            if(compelete_flag == 0)
            {
                zn_dc_check(iq_flag, 3, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
            }
        }
        else
        {
            gain = (flag == 2 ? min_r << 8 : min_r);
            if(compelete_flag == 0)
            {
                zn_dc_check(iq_flag, 0, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
            }

            if(compelete_flag == 0)
            {
                zn_dc_check(iq_flag, 1, &gain, &dc_tmp, dc_v, r, &compelete_flag, addr, 1);
            }
        }
    }

    zn_write_32bit_reg(addr, gain);

    return dc_tmp;
}

/*************************************************************************************************************
* Description: This function init the dci.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_dc_init(void)
{
    int32_t dc = 0;

    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000003);

    // I
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0604, 0);
    dc = zn_vga_dc_one_time(0, UWB_BASE_ADDR + 0x0604);
    printf("VGA dc compeleted of I, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0604), dc);

    // Q
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0608, 0);
    dc = zn_vga_dc_one_time(1, UWB_BASE_ADDR + 0x0608);
    printf("VGA dc compeleted of Q, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0608), dc);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x0000000F);

    // I
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x060C, 0);
    dc = zn_vga_dc_one_time(0, UWB_BASE_ADDR + 0x060C);
    printf("VGA dc compeleted of I RX2, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0604), dc);

    // Q
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0610, 0);
    dc = zn_vga_dc_one_time(1, UWB_BASE_ADDR + 0x0610);
    printf("VGA dc compeleted of Q RX2, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0608), dc);

    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000003);

    // I
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0604, 0);
    dc = zn_vga_dc_one_time(0, UWB_BASE_ADDR + 0x0604);
    printf("VGA dc compeleted of I, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0604), dc);

    // Q
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0608, 0);
    dc = zn_vga_dc_one_time(1, UWB_BASE_ADDR + 0x0608);
    printf("VGA dc compeleted of Q, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0608), dc);

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x0000000F);

    // I
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x060C, 0);
    dc = zn_vga_dc_one_time(0, UWB_BASE_ADDR + 0x060C);
    printf("VGA dc compeleted of I RX2, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0604), dc);

    // Q
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0610, 0);
    dc = zn_vga_dc_one_time(1, UWB_BASE_ADDR + 0x0610);
    printf("VGA dc compeleted of Q RX2, reg value is %X, dc is %d\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x0608), dc);
    #endif
}

/*************************************************************************************************************
* Description: This function is change 32 to 8.
* Input param:
* data_in - input data
* len - input data length
* Output param:
* data_out - output data
* Return value: output data length
*************************************************************************************************************/
uint32_t change_32_to_8(uint32_t *data_in, uint8_t *data_out, uint32_t len)
{
    uint32_t idx = 0;
    uint32_t jdx = 0;

    for(idx = 0; idx < len; idx++)
    {
        jdx = idx * 4;
        data_out[jdx++] = (uint8_t)(data_in[idx] & 0xff);
        data_out[jdx++] = (uint8_t)((data_in[idx] >> 8) & 0xff);
        data_out[jdx++] = (uint8_t)((data_in[idx] >> 16) & 0xff);
        data_out[jdx++] = (uint8_t)((data_in[idx] >> 24) & 0xff);
    }

    return jdx;
}

/*************************************************************************************************************
* Description: This function is read vga call from flash.
* Input param:
* channel_num - channel num
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_read_vga_cal_from_flash(uint8_t channel_num)
{
    int32_t  rx1_i_addr[15] = {0}; 
    int32_t  rx1_q_addr[15] = {0};
    int32_t  rx2_i_addr[15] = {0};
    int32_t  rx2_q_addr[15] = {0};
    uint32_t cal_data[70]   = {0};
    uint32_t flash_addr     = ((CHANNEL_9 == channel_num) ? FLASH_CH9_VGA : FLASH_CH5_VGA);
    uint8_t  index          = 0;

    __disable_irq();
    qspi_flash_init();
    qspi_memory_mapped_read(cal_data, flash_addr, 64);
    qspi_memory_mapped_read(&cal_data[64], flash_addr + QSPI_PAGE_SIZE, 6);

    #if (ZN_SOC == ZN2014)
    for(index = 0; index < 15; index++)
    {
        rx1_i_addr[index] = UWB_BASE_ADDR + 0x0C00 + (index << 2);
        rx1_q_addr[index] = UWB_BASE_ADDR + 0x0C3C + (index << 2);
        rx2_i_addr[index] = UWB_BASE_ADDR + 0x0D00 + (index << 2);
        rx2_q_addr[index] = UWB_BASE_ADDR + 0x0D3C + (index << 2);
    }

    for(index = 0; index < 15; index++)
    {
        zn_write_32bit_reg(rx1_i_addr[index], cal_data[index]);
        zn_write_32bit_reg(rx1_q_addr[index], cal_data[index + 15]);
        s_lna_on_cal_rx0[index] = cal_data[index];
        s_lna_on_cal_rx0[index + 15] = cal_data[index + 15];
        zn_write_32bit_reg(rx2_i_addr[index], cal_data[index + 30]);
        zn_write_32bit_reg(rx2_q_addr[index], cal_data[index + 45]);
    }

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, cal_data[60]);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x102C, cal_data[61]);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, cal_data[62]);

    //RX1 Fix_Gain
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0604, cal_data[64]);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0608, cal_data[65]);

    //RX2 Fix_Gain
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x060c, cal_data[66]);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0610, cal_data[67]);

    //pll cal data
    write_mreg32(UWB_BASE_ADDR + 0x1010, 0x00030700);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1014, cal_data[68]);
    g_tx_ant_delta = (int32_t)cal_data[69];
    printf("TIA load: reg1028 =%x, reg102C=%x, reg1030=%x\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x1028), zn_read_32bit_reg(UWB_BASE_ADDR + 0x102C), zn_read_32bit_reg(UWB_BASE_ADDR + 0x1030));
    printf("insideloop g_tx_ant_delt is %d \r\n", g_tx_ant_delta);
    #endif
    __enable_irq();
}

/*************************************************************************************************************
* Description: This function enables the fix vga cal.
* Input param: None
* Output param:
* rx0_i - rx0 i value
* rx0_q - rx0 q value
* rx1_i - rx1 i value
* rx1_q - rx1 q value
* Return value: None
*************************************************************************************************************/
#if (ZN_SOC == ZN2014)
void zn_vga_fix_gain_cal(uint32_t *rx0_i, uint32_t *rx0_q, uint32_t *rx1_i, uint32_t *rx1_q)
{
    int32_t dc_i     = 0; 
    int32_t dc_q     = 0; 
    int32_t reg_i    = 0; 
    int32_t reg_q    = 0;
    int32_t dc_i_tmp = 0; 
    int32_t dc_q_tmp = 0;
    uint8_t index    = 0; 

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x03000300); //set vga gain
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x0000000f); //set vga cal word
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x00080504); //turn off AGC
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000003);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01004780); // turn on calibration clock
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0x000000ff); // TIA - VGA calibration time setting
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); // VGA calibration current settings
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273804); // VGA vcm settings

    // RX0:
    reg_i = 0;
    reg_q = 0;
    dc_i_tmp = 15;
    dc_q_tmp = 15;

    for(index = 0 ; index < 100 ; index++)
    {
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000000); // auto calibration set to 0
        mdelay(1);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000000); // code overwrite
        mdelay(1);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000011); // auto calibration on

        // dc_i = zn_dc_estimate(0);
        // dc_q = zn_dc_estimate(1);

        dc_i = zn_dc_estimate(0);
        dc_i += zn_dc_estimate(0);
        dc_i = dc_i >> 1;
        
        dc_q = zn_dc_estimate(1);
        dc_q += zn_dc_estimate(1);
        dc_q = dc_q >> 1;

        if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
        {
            dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
            reg_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0624);
        }

        if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
        {
            dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
            reg_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0628);
        }
        if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
        {
            break;
        }
    }
    // write into auto reg
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0604, reg_i);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0608, reg_q);
    *rx0_i = reg_i;
    *rx0_q = reg_q;
    printf("fixed vga gain cal:dc_i =%d, dc_q=%d, I_reg=%X, Q_reg = %X\n", dc_i_tmp, dc_q_tmp, reg_i, reg_q);

    // turn off calibration clock
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01000780); //turn off cal clk
}

/*************************************************************************************************************
* Description: this function is used to calac pll clock
* Input param: None
* Output param:
* cal_data - pll clock value
* Return value: None
*************************************************************************************************************/
void zn_pll_clk_cal(uint32_t *cal_data)
{
    uint32_t cal_val = 0;
    uint32_t reg_val = 0;

    write_mreg32(UWB_BASE_ADDR + 0x1010, 0x00030300);

    while(!(read_mreg32(UWB_BASE_ADDR + 0x081c) & 0x2));
    udelay(100);
    while(!(read_mreg32(UWB_BASE_ADDR + 0x081c) & 0x2));

    cal_val = read_mreg32(UWB_BASE_ADDR + 0x1018) & 0xff;
    write_mreg32(UWB_BASE_ADDR + 0x1010, 0x00030700);
    reg_val = read_mreg32(UWB_BASE_ADDR + 0x1014);
    reg_val &= 0x00ffffff;
    reg_val |= cal_val << 24;
    write_mreg32(UWB_BASE_ADDR + 0x1014, reg_val);
    *cal_data = reg_val;
}

/*************************************************************************************************************
* Description: This function enables the auto vga cal.
* Input param:
* channel_num - frequency selection. CHANNEL_5 : 6.5G, CHANNEL_9 : 8G
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_vga_all_gain_cal(uint8_t channel_num)
{
    int32_t power_table[15] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37, 0x3F};

    int32_t dc_i     = 0;
    int32_t dc_q     = 0;
    int32_t reg_i    = 0;
    int32_t reg_q    = 0;
    int32_t dc_i_tmp = 0;
    int32_t dc_q_tmp = 0;

    int32_t addr_i   = 0; 
    int32_t addr_q   = 0;

    int32_t rx1_i_addr[15] = {0};
    int32_t rx1_q_addr[15] = {0}; 
    int32_t rx2_i_addr[15] = {0}; 
    int32_t rx2_q_addr[15] = {0};

    int32_t reg0    = 0; 
    int32_t reg1    = 0; 
    int32_t reg_tmp = 0;

    uint32_t mask               = 0;
    uint32_t cal_data[70]       = {0};
    uint32_t flash_wr_flag      = 0;
    uint32_t flash_addr         = ((CHANNEL_9 == channel_num) ? FLASH_CH9_VGA : FLASH_CH5_VGA);
    uint8_t  flash_data[276]    = {0};
    uint8_t  change_32_to_8_len = 0;

    uint8_t tia_word_p_i = 0; 
    uint8_t tia_word_n_i = 0; 
    uint8_t tia_word_p_q = 0; 
    uint8_t tia_word_n_q = 0; 
    uint8_t tia_word_p_2nd_i = 0; 
    uint8_t tia_word_n_2nd_i = 0; 
    uint8_t tia_word_p_2nd_q = 0; 
    uint8_t tia_word_n_2nd_q = 0;

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0x00101f00);

    for(int32_t i = 0; i < 15; i++)
    {
        rx1_i_addr[i] = UWB_BASE_ADDR + 0x0C00 + (i << 2);
        rx1_q_addr[i] = UWB_BASE_ADDR + 0x0C3C + (i << 2);
        rx2_i_addr[i] = UWB_BASE_ADDR + 0x0D00 + (i << 2);
        rx2_q_addr[i] = UWB_BASE_ADDR + 0x0D3C + (i << 2);
    }

    __disable_irq();
    qspi_memory_mapped_read(&flash_wr_flag, flash_addr, 1);
    printf("QSPI_MemoryMappedRead flash_wr_flag = 0x%x\r\n", flash_wr_flag);
    printf("start RX0 VGA auto gain calibration\n");

    if((g_vga_ctrl == VGA_ONCE_POWERON) || 0xffffffff == flash_wr_flag)
    {
        printf("TIA CALC\r\n");
        zn_rx_on();
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0x000000ff); // TIA - VGA calibration time setting

        // TIA calibration:
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0x00000010); // TIA - VGA calibration time setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01704780); // turn on calibration clock
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x01C70000); //I channel setting
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0x11000010); //I channel auto en
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x0e380000); //Q channel setting
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0x22000010); //Q channel auto en
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x0db60000); // TIA settings after calibration
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); //turn off cal clk
        printf("RX0 TIA CALIB Result: 0x%x\r\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x1034));
        printf("RX1 TIA CALIB Result: 0x%x\r\n", zn_read_32bit_reg(UWB_BASE_ADDR + 0x1038));

        // load register
        reg0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1034);
        reg1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1038);
        tia_word_p_i = reg0 & 0x7F;
        tia_word_n_i = (reg0 >> 8) & 0x7F;
        tia_word_p_q = (reg0 >> 16) & 0x7F;
        tia_word_n_q = (reg0 >> 24) & 0x7F;
        tia_word_p_2nd_i = reg1 & 0x7F;
        tia_word_n_2nd_i = (reg1 >> 8) & 0x7F;
        tia_word_p_2nd_q = (reg1 >> 16) & 0x7F;
        tia_word_n_2nd_q = (reg1 >> 24) & 0x7F;

        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, 0xCC000010); // set TIA overwrite
        reg0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1028);
        mask = 0xFF0000FF;
        reg0 &= mask;
        mask = (tia_word_n_i << 16) + (tia_word_p_i << 8);
        reg0 ^= mask;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, reg0);
        reg0 = (tia_word_n_2nd_i << 24) + (tia_word_p_2nd_i << 16) + (tia_word_n_q << 8) + tia_word_p_q;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x102C, reg0);
        reg0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1030);
        mask = 0xFFFF0000;
        reg0 &= mask;
        mask = (tia_word_n_2nd_q << 8) + tia_word_p_2nd_q;
        reg0 ^= mask;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, reg0);

        // RX0: LNA ON
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01704780); // turn on calibration clock
        reg_tmp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1028);
        reg_tmp = (reg_tmp & 0xFFFFFF00) + 0xFF;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, reg_tmp); // TIA - VGA calibration time setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); // VGA calibration current settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273804); // VGA vcm settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, 0x60731304); // VGA vcm settings and RX0 LNA stage2 ON

        for(int32_t i = 0; i < 15; i++)
        {
            reg_i = 0;
            reg_q = 0;
            dc_i_tmp = 15;
            dc_q_tmp = 15;
            addr_i = UWB_BASE_ADDR + 0x0C00 + (i << 2);
            addr_q = UWB_BASE_ADDR + 0x0C3C + (i << 2);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x01000000 + (power_table[i] << 8) );

            for(int32_t j = 0; j < 30 ; j++)
            {
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000000); // auto calibration set to 0
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000000); // code overwrite
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000011); // auto calibration on

                dc_i = zn_dc_estimate(0);
                dc_i += zn_dc_estimate(0);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(1);
                dc_q += zn_dc_estimate(1);
                dc_q = dc_q >> 1;

                if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
                {
                    dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                    reg_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0624);
                }

                if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
                {
                    dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                    reg_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0628);
                }

                if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
                {
                    break;
                }
            }
            // write into auto reg
            zn_write_32bit_reg(addr_i, reg_i);
            cal_data[i] = reg_i;
            zn_write_32bit_reg(addr_q, reg_q);
            cal_data[i + 15] = reg_q;
            printf("RX0: i = %d, dc_i = %d, dc_q = %d, reg_i = %X, reg_q = %X\n", i, dc_i_tmp, dc_q_tmp, reg_i, reg_q);
            s_lna_on_cal_rx0[i] = cal_data[i];
            s_lna_on_cal_rx0[i + 15] = cal_data[i + 15];
        }
        // turn off calibration clock
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); //turn off cal clk
        printf("===========================================\n");
        printf("=========RX0 VGA calbration Done===========\n");

        // RX0: LNA OFF
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01004780); // turn on calibration clock
        reg_tmp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1028);
        reg_tmp = (reg_tmp & 0xFFFFFF00) + 0xFF;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, reg_tmp);    // TIA - VGA calibration time setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); // VGA calibration current settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273804); // VGA vcm settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, 0x00731304); // VGA vcm settings and RX0 LNA stage2 OFF

        for(int32_t i = 0; i < 15; i++)
        {
            reg_i = 0;
            reg_q = 0;
            dc_i_tmp = 15;
            dc_q_tmp = 15;
            addr_i = UWB_BASE_ADDR + 0x0C00 + (i << 2);
            addr_q = UWB_BASE_ADDR + 0x0C3C + (i << 2);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x01000000 + (power_table[i] << 8) );

            for(int32_t j = 0; j < 30 ; j++)
            {
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000000); // auto calibration set to 0
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000000); // code overwrite
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000011); // auto calibration on

                dc_i = zn_dc_estimate(0);
                dc_i += zn_dc_estimate(0);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(1);
                dc_q += zn_dc_estimate(1);
                dc_q = dc_q >> 1;

                if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
                {
                    dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                    reg_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0624);
                }

                if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
                {
                    dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                    reg_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0628);
                }

                if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
                {
                    break;
                }
            }
            // write into auto reg
            s_lna_off_cal_rx0[i] = reg_i;
            s_lna_off_cal_rx0[i + 15] = reg_q;
            printf("RX0: i = %d, dc_i = %d, dc_q = %d, reg_i = %X, reg_q = %X\n", i, dc_i_tmp, dc_q_tmp, reg_i, reg_q);
        }
        // turn off calibration clock
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); //turn off cal clk
        printf("===========================================\n");
        printf("=========RX0 VGA calbration Done===========\n");

        #ifdef _AOA_EN
        printf("start RX1 VGA auto gain calibration\n");

        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01704780); // turn on calibration clock
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); 
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, reg_tmp);    // TIA-VGA calibration time setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x50000000); // VGA calibration current settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000030);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273f3c); // VGA vcm settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, 0xa4a4e204); // RX1 LNA stage2 ON

        // RX1 LNA ON:
        for(int32_t i = 0; i < 15; i++)
        {
            reg_i = 0;
            reg_q = 0;
            dc_i_tmp = 15;
            dc_q_tmp = 15;
            addr_i = UWB_BASE_ADDR + 0x0D00 + (i << 2);
            addr_q = UWB_BASE_ADDR + 0x0D3C + (i << 2);

            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x02000000 + (power_table[i] << 16));

            for(int32_t j = 0 ; j < 30 ; j++)
            {
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000011);  // auto calibration set to 0
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000000);  // code overwrite
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00001111);  // auto calibration on

                dc_i = zn_dc_estimate(2);
                dc_i += zn_dc_estimate(2);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(3);
                dc_q += zn_dc_estimate(3);
                dc_q = dc_q >> 1;

                if(dc_i < dc_i_tmp && dc_i > -dc_i_tmp)
                {
                    dc_i_tmp = dc_i >= 0 ? dc_i : -dc_i;
                    reg_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x062C);
                }

                if(dc_q < dc_q_tmp && dc_q > -dc_q_tmp)
                {
                    dc_q_tmp = dc_q >= 0 ? dc_q : -dc_q;
                    reg_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0630);
                }

                if(dc_i_tmp <= 1 && dc_i_tmp >= -1 && dc_q_tmp <= 1 && dc_q_tmp >= -1)
                {
                    break;
                }
            }
            // write into auto reg
            zn_write_32bit_reg(addr_i, reg_i);
            cal_data[30 + i] = reg_i;
            zn_write_32bit_reg(addr_q, reg_q);
            cal_data[45 + i] = reg_q;
            printf("i = %d, dc_i = %d, dc_q = %d, reg_i = %X, reg_q = %X\n", i ,dc_i_tmp, dc_q_tmp, reg_i, reg_q);
            s_lna_on_cal_rx1[i] = reg_i;
            s_lna_on_cal_rx1[i + 15] = reg_q;
        }

        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090010);  //RX1 LNA stage1 OFF
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1028, reg_tmp);     // TIA-VGA calibration time setting
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x50000000);  // VGA calibration current settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000030);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273f3c);  // VGA vcm settings
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, 0xa4a42204);  //RX1 LNA stage2 OFF

        // RX1 LNA OFF:
        for(int32_t i = 0; i < 15; i++)
        {
            reg_i = 0;
            reg_q = 0;
            dc_i_tmp = 15;
            dc_q_tmp = 15;
            addr_i = UWB_BASE_ADDR + 0x0D00 + (i << 2);
            addr_q = UWB_BASE_ADDR + 0x0D3C + (i << 2);

            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x02000000 + (power_table[i] << 16));

            for(int32_t j = 0 ; j < 30 ; j++)
            {
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00000011);  // auto calibration set to 0
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x00000000);  // code overwrite
                mdelay(1);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x00001111);  // auto calibration on

                dc_i = zn_dc_estimate(2);
                dc_i += zn_dc_estimate(2);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(3);
                dc_q += zn_dc_estimate(3);
                dc_q = dc_q >> 1;

                if(dc_i < dc_i_tmp && dc_i > -dc_i_tmp)
                {
                    dc_i_tmp = dc_i >= 0 ? dc_i : -dc_i;
                    reg_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x062C);
                }

                if(dc_q < dc_q_tmp && dc_q > -dc_q_tmp)
                {
                    dc_q_tmp = dc_q >= 0 ? dc_q : -dc_q;
                    reg_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0630);
                }

                if(dc_i_tmp <= 1 && dc_i_tmp >= -1 && dc_q_tmp <= 1 && dc_q_tmp >= -1)
                {
                    break;
                }
            }

            // write into auto reg
            zn_write_32bit_reg(addr_i, reg_i);
            zn_write_32bit_reg(addr_q, reg_q);
            printf("i = %d, dc_i = %d, dc_q = %d, reg_i = %X, reg_q = %X\n", i, dc_i_tmp, dc_q_tmp, reg_i, reg_q);
            s_lna_off_cal_rx1[i] = reg_i;
            s_lna_off_cal_rx1[i + 15] = reg_q;
        }
        printf("=========RX1 VGA calbration Done===========\n");
        #endif

        // store TIA
        cal_data[60] = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1028);
        cal_data[61] = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102C);
        cal_data[62] = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1030);
        printf("TIA store: reg1028 =%x, reg102C=%x, reg1030=%x\n", cal_data[60], cal_data[61], cal_data[62]);
        zn_vga_fix_gain_cal(&cal_data[64], &cal_data[65], &cal_data[66], &cal_data[67]);
        zn_pll_clk_cal(&cal_data[68]);

        g_tx_ant_delta = zn_inside_loop();
        cal_data[69] = (uint32_t)g_tx_ant_delta;

        change_32_to_8_len = change_32_to_8(cal_data, flash_data, 63);
        qspi_erase_sector(flash_addr);
        qspi_write_buffer(flash_data, flash_addr, change_32_to_8_len, QSPI_DATASIZE_BYTE);
        change_32_to_8_len = change_32_to_8(&cal_data[64], flash_data, 6);
        qspi_write_buffer(flash_data, flash_addr + QSPI_PAGE_SIZE, change_32_to_8_len, QSPI_DATASIZE_BYTE);
    }
    else
    {
        zn_read_vga_cal_from_flash(channel_num);
    }
    __enable_irq();

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); // turn on calibration clock
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); // VGA calibration current settings
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1060, 0x50000000); // VGA calibration current settings
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x082c, 0x07273f3c); // VGA vcm settings
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0600, 0x000000F0);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x0C000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x02080504);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1858, 0x00000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1860, 0x00280028);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1864, 0x00280808);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x184c, 0x001501ff);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1928, 0x02080504);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1938, 0x00000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1940, 0x00280028);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1944, 0x00280808);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x192c, 0x001501ff);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, 0x60731304); //RX0 LNA stage2 ON
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, 0xa4a4e204); //RX1 LNA stage2 ON

    #ifdef _APPLE_TAG
    sys_ctrl_write(SOC_CLK_CFG, 0x20); //sys clk div 3 31.2M
    #endif

    #ifdef _POWER_SAVE
    zn_to_idle_state();
    #endif
}
#endif

#if ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
void zn_vga_all_gain_cal(uint8_t channel_num)
{
    //TODO
}
#endif

/*************************************************************************************************************
* Description: This function is waiting until pll is locked or timeout.
* Input param:
* mask - register mask value
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_wait_pll_stable(uint8_t mask)
{
    uint32_t wait_count = 6250;
    uint8_t  reg_value  = 0;

    do
    {
        reg_value = zn_read_32bit_reg(PLL_STATUS_ID) & mask;
        wait_count--;
    }while((reg_value != mask) && (wait_count > 0));
}

/*************************************************************************************************************
* Description: This function is start tx on.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_tx_on(void)
{
#if (ZN_SOC == ZN2014)
    uint32_t val = 0;

    #ifdef _POWER_SAVE
    write_mreg32(UWB_BASE_ADDR + 0x0828, 0x01410900);
    write_mreg32(UWB_BASE_ADDR + 0x084C, 0x82010000);
    zn_wait_pll_stable(0x97);
    write_mreg32(UWB_BASE_ADDR + 0x1874, 0x1);
    write_mreg32(UWB_BASE_ADDR + 0x0540, 0xfff00aa5);
    #else
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x084C, 0x82010000); //TX ON
    #endif
#elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
#endif
}

/*************************************************************************************************************
* Description: This function is to the tx state.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_to_tx_state(void)
{
    zn_tx_on();
    zn_write_32bit_reg(ANT_SWITCH_ID, ANT_SWITCH_0_BIT_MASK | ANT_SWITCH_0_OVRD_BIT_MASK);
}

/*************************************************************************************************************
* Description: This function is off the rx.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_to_idle_state(void)
{
#ifdef _POWER_SAVE
    #if (ZN_SOC == ZN2014)
    uint32_t val = 0;

    write_mreg32(UWB_BASE_ADDR + 0x084C, 0x02000000); //RFPLL OFF
    write_mreg32(UWB_BASE_ADDR + 0x1874, 0x1);
    write_mreg32(UWB_BASE_ADDR + 0x0540, 0xfff00aa5);
    // val = read_mreg32(UWB_BASE_ADDR + 0x0830);
    // val |= 0x1;
    // write_mreg32(UWB_BASE_ADDR + 0x0830, val);
    #elif (ZN_SOC == ZN3005)
    #endif
#else
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x084C, 0x82000000); //RX OFF
#endif
}

/*************************************************************************************************************
* Description: This function is on the rx.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_rx_on(void)
{
#if (ZN_SOC == ZN2014)
    #ifdef _POWER_SAVE
    uint32_t val = 0;

    #ifdef _AOA_EN
    write_mreg32(UWB_BASE_ADDR + 0x0828, 0x01412800);
    #else
    write_mreg32(UWB_BASE_ADDR + 0x0828, 0x01410800);
    #endif

    write_mreg32(UWB_BASE_ADDR + 0x084C, 0x82200000);
    zn_wait_pll_stable(0xa7);
    write_mreg32(UWB_BASE_ADDR + 0x1874, 0x3);
    write_mreg32(UWB_BASE_ADDR + 0x0540, 0xfc000aa5);
    val = read_mreg32(UWB_BASE_ADDR + 0x0830);
    val &= 0xfe;
    write_mreg32(UWB_BASE_ADDR + 0x0830, val);
    #else
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x084C, 0x82200000); //RX ON
    #endif
#elif(ZN_SOC == ZN3005)
#endif
}

/*************************************************************************************************************
* Description: This function is to the rx state.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_to_rx_state(void)
{
    zn_rx_on();
    zn_write_32bit_reg(ANT_SWITCH_ID, ANT_SWITCH_0_OVRD_BIT_MASK);
}

/*************************************************************************************************************
* Description: This function is on pa.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_pa_on(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76392000); //PA ON 3:0x76112000 7:0x76392000 32'h
    #elif (ZN_SOC == ZN3005)
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76392000); //PA ON 3:0x76112000 7:0x76392000 32'h
    #endif
}

/*************************************************************************************************************
* Description: This function is off pa.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_pa_off(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76012000); //PA OFF
    #elif (ZN_SOC == ZN3005)
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76012000); //PA OFF
    #endif
}

/*************************************************************************************************************
* Description: This function is pa set.
* Input param: 
* value - power level
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_pa_set(uint16_t value)
{
    uint32_t pa_gain_sel[MAX_PA_SET_NUM]  = {0, 1, 1, 3, 2, 5, 3, 7, 4, 5, 6, 7};
    uint32_t pa_gain_half[MAX_PA_SET_NUM] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0};
    uint32_t reg_value                    = 0;

    if(value >= MAX_PA_SET_NUM)
    {
        return;
    }
    #if(ZNSOC == ZN2014)
    reg_value = zn_read_32bit_reg(TX_PA_GAIN_ID);
    reg_value |= (pa_gain_half[value] << TX_CTRL_PA_GAIN_HALF_BIT_OFFSET);
    zn_write_32bit_reg(TX_PA_GAIN_ID, reg_value);

    reg_value = zn_read_32bit_reg(TX_CTRL_ID);
    reg_value &= (~(TX_CTRL_PA_GAIN_SEL_BIT_MASK));
    reg_value |= ((pa_gain_sel[value] << TX_CTRL_PA_GAIN_SEL_BIT_OFFSET) & TX_CTRL_PA_GAIN_SEL_BIT_MASK);
    #endif
    
    #if (ZN_SOC == ZN3005 || ZN_SOC == ZN3006)
    reg_value &= (~TX_CTRL_PA_GAIN_SEL2_BIT_MASK);
    reg_value |= ((value << TX_CTRL_PA_GAIN_SEL2_BIT_OFFSET) & TX_CTRL_PA_GAIN_SEL2_BIT_MASK);
    #endif

    zn_write_32bit_reg(TX_CTRL_ID, reg_value);
}

/*************************************************************************************************************
* Description: This function is on the lna.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_lna_on(void)
{
    uint32_t reg_value = 0;
    uint32_t lna_stage2_mask = 0;

    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); //RX0 LNA stage1 on

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0818);
    lna_stage2_mask = 0x60000000;
    reg_value &= (~lna_stage2_mask);
    reg_value |= ((3 << 29) & lna_stage2_mask);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, reg_value);  //RX0 LNA stage2 on

    #ifdef _AOA_EN
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090e10); //RX1 LNA stage1 on

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0854);
    lna_stage2_mask = 0xC000;
    reg_value &= (~lna_stage2_mask);
    reg_value |= ((3 << 14) & lna_stage2_mask);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, reg_value);  //RX1 LNA stage2 on
    #endif

    #elif (ZN_SOC == ZN3005)
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01700780); //LNA ON
    #endif
}

/*************************************************************************************************************
* Description: This function is on the lna cal.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_lna_cal_on(void)
{
    uint32_t addr_i = 0;
    uint32_t addr_q = 0;
    uint32_t index  = 0;

    for(index = 0; index < 15; index++)
    {
        addr_i = UWB_BASE_ADDR + 0x0C00 + (index << 2);
        addr_q = UWB_BASE_ADDR + 0x0C3C + (index << 2);
        zn_write_32bit_reg(addr_i, s_lna_on_cal_rx0[index]);
        zn_write_32bit_reg(addr_q, s_lna_on_cal_rx0[index + 15]);
    }

    #ifdef _AOA_EN
    for(index = 0; index < 15; index++)
    {
        addr_i = UWB_BASE_ADDR + 0x0D00 + (index << 2);
        addr_q = UWB_BASE_ADDR + 0x0D3C + (index << 2);
        zn_write_32bit_reg(addr_i, s_lna_on_cal_rx1[index]);
        zn_write_32bit_reg(addr_q, s_lna_on_cal_rx1[index + 15]);
    }
    #endif
}

/*************************************************************************************************************
* Description: This function is off the lna.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_lna_off(void)
{
    uint32_t reg_value       = 0;
    uint32_t lna_stage2_mask = 0;

    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01000780); //RX0 LNA stage1 off

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0818);
    lna_stage2_mask = 0x60000000;
    reg_value &= (~lna_stage2_mask);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0818, reg_value);  //RX0 LNA stage2 off

    #ifdef _AOA_EN
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x50090010); //RX1 LNA stage1 off

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0854);
    lna_stage2_mask = 0xC000;
    reg_value &= (~lna_stage2_mask);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0854, reg_value);  //RX1 LNA stage2 off
    #endif

    #elif (ZN_SOC == ZN3005)
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1054, 0x01000780); //LNA OFF
    #endif
}

/*************************************************************************************************************
* Description: This function is off the lna cal.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_lna_cal_off(void)
{
    uint32_t addr_i = 0;
    uint32_t addr_q = 0;
    uint32_t index  = 0;

    for(index = 0; index < 15; index++)
    {
        addr_i = UWB_BASE_ADDR + 0x0C00 + (index << 2);
        addr_q = UWB_BASE_ADDR + 0x0C3C + (index << 2);
        zn_write_32bit_reg(addr_i, s_lna_off_cal_rx0[index]);
        zn_write_32bit_reg(addr_q, s_lna_off_cal_rx0[index + 15]);
    }

    #ifdef _AOA_EN
    for(index = 0; index < 15; index++)
    {
        addr_i = UWB_BASE_ADDR + 0x0D00 + (index << 2);
        addr_q = UWB_BASE_ADDR + 0x0D3C + (index << 2);
        zn_write_32bit_reg(addr_i, s_lna_off_cal_rx1[index]);
        zn_write_32bit_reg(addr_q, s_lna_off_cal_rx1[index + 15]);
    }
    #endif
}

/*************************************************************************************************************
* Description: This function is modify the channel num.
* Input param:
* channel_num - channel num
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_channel(uint32_t channel_num)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(SYSTEM_PARAM_ID); // clear bit to 0
    reg_value &= (~SYSTEM_PARAM_RF_CHAN_BIT_MASK);
    reg_value |= (channel_num & SYSTEM_PARAM_RF_CHAN_BIT_MASK);
    zn_write_32bit_reg(SYSTEM_PARAM_ID, reg_value);

    #if (ZN_SOC == ZN2014)
    if(CHANNEL_5 == channel_num)
    {
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0810, 0x298c8092);
    }
    else if(CHANNEL_9 == channel_num)
    {
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x0810, 0x508c8092);
    }

    if(s_last_channel != channel_num)
    {
        // zn_read_vga_cal_from_flash(channel_num);
        // last_channel = channel_num;
    }
    #elif (ZN_SOC == ZN3005)
    if(CHANNEL_5 == channel_num)
    {
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x0810, 0x298c8092);
    }
    else if(CHANNEL_9 == channel_num)
    {
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x0810, 0x508c8092);
    }
    #endif

    return;
}

/*************************************************************************************************************
* Description: This function is modify the sfd id.
* Input param:
* sfd_id - sfd id
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_sfd_id(uint32_t sfd_id)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(SYSTEM_PARAM_ID); // clear bit to 0
    reg_value &= (~SYSTEM_PARAM_SFD_TYPE_BIT_MASK);
    reg_value |= ((sfd_id << SYSTEM_PARAM_SFD_TYPE_BIT_OFFSET) & SYSTEM_PARAM_SFD_TYPE_BIT_MASK);
    zn_write_32bit_reg(SYSTEM_PARAM_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the preamble code idx.
* Input param:
* preamble_code_idx - preamble code index
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_preamble_code_idx(uint32_t preamble_code_idx)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(SYSTEM_PARAM_ID); // clear bit to 0
    reg_value &= (~SYSTEM_PARAM_TX_PCODE_BIT_MASK);
    reg_value |= ((preamble_code_idx << SYSTEM_PARAM_TX_PCODE_BIT_OFFSET) & SYSTEM_PARAM_TX_PCODE_BIT_MASK);
    reg_value &= (~SYSTEM_PARAM_RX_PCODE_BIT_MASK);
    reg_value |= ((preamble_code_idx << SYSTEM_PARAM_RX_PCODE_BIT_OFFSET) & SYSTEM_PARAM_RX_PCODE_BIT_MASK);
    zn_write_32bit_reg(SYSTEM_PARAM_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the rframe config.
* Input param:
* rframe_cfg - rframe config
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_rframe_cfg(uint32_t rframe_cfg)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(SYS_CFG_ID); // clear bit to 0
    reg_value &= (~SYS_CFG_STS_MODE_BIT_MASK);
    reg_value |= ((rframe_cfg << SYS_CFG_STS_MODE_BIT_OFFSET) & SYS_CFG_STS_MODE_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, reg_value);

    #if(ZN_SOC == ZN2014)
    if(RFRAME_SP0 != rframe_cfg)
    {
        reg_value = zn_read_32bit_reg(STS_CFG_ID);
        reg_value |= (1 << STS_DELTA_BIT_OFFSET);
        zn_write_32bit_reg(STS_CFG_ID, reg_value);
    }
    #endif

    return;
}

/*************************************************************************************************************
* Description: This function is modify the bprf phr data rate.
* Input param:
* bprf_phr_data_rate - bprf phr data rate
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_bprf_phr_data_rate(uint32_t bprf_phr_data_rate)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(SYS_CFG_ID); // clear bit to 0
    reg_value &= (~SYS_CFG_PHR_6M8_BIT_MASK);
    reg_value |= ((bprf_phr_data_rate << SYS_CFG_PHR_6M8_BIT_OFFSET) & SYS_CFG_PHR_6M8_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the sts length.
* Input param:
* sts_length - sts length
* Output param: None
* Return value: None
*************************************************************************************************************/
//void zn_modify_sts_length(uint32_t sts_length)  -->>zn_modify_sts_seg_len()
//{
//    uint32_t reg_value = 0;

//    reg_value = zn_read_32bit_reg(STS_CFG_ID);
//    reg_value &= (~STS_CPS_LEN_BIT_MASK);
//    reg_value |= ((sts_length << STS_CPS_LEN_BIT_OFFSET) & STS_CPS_LEN_BIT_MASK);
//    zn_write_32bit_reg(STS_CFG_ID, reg_value);

//    return;
//}

/*************************************************************************************************************
* Description: This function is modify the sts segment number.
* Input param:
* sts_seg_num - sts segment num
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_sts_seg_num(uint32_t sts_seg_num)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(STS_CFG_ID);
    reg_value &= (~STS_STS_SEG_NUM_BIT_MASK);
    reg_value |= ((sts_seg_num << STS_STS_SEG_NUM_BIT_OFFSET) & STS_STS_SEG_NUM_BIT_MASK);
    zn_write_32bit_reg(STS_CFG_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the sts segement length.
* Input param:
* sts_seg_len - sts segment length
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_sts_seg_len(uint32_t sts_seg_len)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(STS_CFG_ID);
    reg_value &= (~STS_CPS_LEN_BIT_MASK);
    reg_value |= ((sts_seg_len << STS_CPS_LEN_BIT_OFFSET) & STS_CPS_LEN_BIT_MASK);
    zn_write_32bit_reg(STS_CFG_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the preamble duration.
* Input param:
* preamble_duration - preamble duration
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_preamble_duration(uint32_t preamble_duration)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(TX_PHR_CTRL_ID);
    reg_value &= (~TX_PHR_CTRL_TXPSR_PE_BIT_MASK);
    reg_value |= ((preamble_duration << TX_PHR_CTRL_TXPSR_PE_BIT_OFFSET) & TX_PHR_CTRL_TXPSR_PE_BIT_MASK);
    zn_write_32bit_reg(TX_PHR_CTRL_ID, reg_value);

    return;
}

/*************************************************************************************************************
* Description: This function is modify the psdu data rate.
* Input param:
* psdu_data_rate - psdu data rete
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_psdu_data_rate(uint32_t psdu_data_rate)
{
    uint32_t reg_value = 0;

    reg_value = zn_read_32bit_reg(TX_PHR_CTRL_ID);
    reg_value &= (~TX_PHR_CTRL_TXBR_BIT_MASK);
    reg_value |= ((psdu_data_rate << TX_PHR_CTRL_TXBR_BIT_OFFSET) & TX_PHR_CTRL_TXBR_BIT_MASK);
    zn_write_32bit_reg(TX_PHR_CTRL_ID, reg_value);

    return;
}
/*************************************************************************************************************
* Description: This function is modify the prf mode.
* Input param:
* prf - prf mode 0:BPRF 62.4M 1:HPRF 124.8M 2:249.6M 3:499.2M
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_prf_mode(uint8_t prf)
{
    uint32_t reg_value = 0;
    uint32_t reg_value1 = 0; 
#if(ZN_SOC == ZN2014)
    reg_value = zn_read_32bit_reg(PRF_MODE_ID);
    if(prf == PRF_624)
    {
        reg_value &= (~PRF_MODE_HPRF_EN_BIT_MASK); 
    }
    else if(prf == PRF_1248)
    {
        reg_value |= PRF_MODE_HPRF_EN_BIT_MASK; 
        reg_value &= (~PRF_MODE_HPRF_MODE_BIT_MASK); 
    }
    else if(prf == PRF_2496)
    {
        reg_value |= PRF_MODE_HPRF_EN_BIT_MASK; 
        reg_value &= (~PRF_MODE_HPRF_MODE_BIT_MASK);
        reg_value |= (1 << PRF_MODE_HPRF_MODE_BIT_OFFSET); 
    }
    zn_write_32bit_reg(PRF_MODE_ID, reg_value);
    #endif
#if(ZN_SOC == ZN3006)
    reg_value = zn_read_32bit_reg(PRF_MODE_ID);
    if(prf == PRF_624)
    {
        zn_modify_HPRF_en(0);
    }
    else
    {
        zn_modify_HPRF_en(1);
        zn_write_32bit_reg(PRF_MODE_ID, prf-1);
    }

 #endif   
    return;
}
/*************************************************************************************************************
* Description: This function is to config Start and win, see fira.
* Input param:
* config - config value pointer address
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_cfg_t_start_and_win(void *config)
{
    zn_tstart_twin_t *start_win = (zn_tstart_twin_t *)config;
    uint32_t         reg_value  = 0;

    if(!config)
    {
        return;
    }

    reg_value = zn_read_32bit_reg(DRX_SFDTOC_ID); // clear bit to 0
    reg_value &= (~DRX_SFDTOC_BIT_MASK);
    reg_value |= ((start_win->start << DRX_SFDTOC_BIT_OFFSET) & DRX_SFDTOC_BIT_MASK);
    zn_write_32bit_reg(DRX_SFDTOC_ID, reg_value);
    zn_write_32bit_reg(RX_FWTO_ID, start_win->win);
}

/*************************************************************************************************************
* Description: This function is to modify the zn config.
* Input param:
* config - config value pointer address
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_config(void *config)
{
    zn_config_t *zn_config = NULL;

    if(!config)
    {
        return;
    }

    NVIC_DisableIRQ(UWB_IRQn);
    zn_config = (zn_config_t *)config;
    zn_modify_rframe_cfg(zn_config->rf_cfg);
    zn_modify_channel(zn_config->channel_num);
    zn_modify_sfd_id(zn_config->sfd_id);
    zn_modify_preamble_code_idx(zn_config->preamble_code_idx);
    zn_modify_bprf_phr_data_rate(zn_config->bprf_phr_data_rate);
    zn_modify_preamble_duration(zn_config->preamble_duration);
    zn_modify_psdu_data_rate(zn_config->psdu_data_rate);
    zn_modify_sts_seg_num(zn_config->sts_seg_num);
    zn_modify_sts_seg_len(zn_config->sts_seg_len);
    zn_modify_prf_mode(zn_config->prf_mode);
    zn_set_rx_preamble_config(zn_config->rf_cfg, zn_config->preamble_duration);
    zn_set_rx_sfd_config(zn_config->sfd_id);

    NVIC_EnableIRQ(UWB_IRQn);
}

/*************************************************************************************************************
* Description: This function is to open loopback.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_loopback_open(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1814, 0x00030000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1844, 0x9c000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x10000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1900, 0x2aff8028);
    #endif
}

/*************************************************************************************************************
* Description: This function is to close loopback.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_loopback_close(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1844, 0x1c000000);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x00000000);
    #endif
}

/*************************************************************************************************************
* Description: This function is to send cw.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_start_send_cw(void)
{
    zn_or_32bit_reg(TX_CTRL_ID, TX_CTRL_PA_CW_OUT_BIT_MASK);
}

/*************************************************************************************************************
* Description: This function is to stop send cw.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_stop_send_cw(void)
{
    zn_and_32bit_reg(TX_CTRL_ID, ~TX_CTRL_PA_CW_OUT_BIT_MASK);
}

/*************************************************************************************************************
* Description: This function is to modify the swc.
* Input param:
* value - swc value
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_swc(uint16_t value)
{
    uint32_t reg_value = zn_read_32bit_reg(CLK_XO_ID);

    reg_value &= ((~D_XO_SWC_O_BIT_MASK) & (~D_XO_SWC_I_BIT_MASK));
    reg_value |= (((value << D_XO_SWC_I_BIT_OFFSET) & D_XO_SWC_I_BIT_MASK) | ((value << D_XO_SWC_O_BIT_OFFSET) & D_XO_SWC_O_BIT_MASK));
    zn_write_32bit_reg(CLK_XO_ID, reg_value);
}

/*************************************************************************************************************
* Description: This function is to modify the swf.
* Input param:
* value - swf value
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_modify_swf(uint16_t value)
{
    uint32_t reg_value = zn_read_32bit_reg(CLK_XO_ID);

    reg_value &= (~D_XO_SWF_BIT_MASK) ;
    reg_value |= ((value << D_XO_SWF_BIT_OFFSET) & D_XO_SWF_BIT_MASK);
    zn_write_32bit_reg(CLK_XO_ID, reg_value);
}

/*************************************************************************************************************
* Description: This function is to enable AOA.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_aoa_en(void)
{
    #if (ZN_SOC == ZN2014)
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007); //pdoa digital config

    #ifdef _AOA_TYPE_2D
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x01c481c4); //pdoa digital en
    #else
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x0bc489c4); //pdoa digital en
    #endif

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, 0x01482800); //pdoa rf en
    #endif

    #if (ZN_SOC == ZN3005)
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x191c, 0x00000007); //pdoa digital config
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1810, 0x01c481c4); //pdoa digital en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, 0x01482800); //pdoa rf en
    #endif
}

/*************************************************************************************************************
* Description: This function is to disable AOA.
* Input param: None
* Output param: None
* Return value: None
*************************************************************************************************************/
void zn_aoa_dis(void)
{
    zn_and_32bit_reg(SYS_CFG_ID, ~(SYS_CFG_PDOA_EN_BIT_MASK));
}

/***************************************************************************************************************
* Description: This is used to read the dif_phase and convert to the aoa angle
* Input param:
* channel - channel id
* Output param:
* aoa - aoa result
* valid_flag - indicate the valid flag of the pdoa result
* Return value: None
****************************************************************************************************************/
void zn_read_aoa_result(double *aoa, uint8_t *valid_flag, uint8_t channel)
{
    uint16_t a0         = ((zn_read_32bit_reg(IP_NACC0_ID) >> IP_NACC0_BIT_OFFSET) & (IP_NACC0_BIT_MASK >> IP_NACC0_BIT_OFFSET));
    uint16_t a1         = ((zn_read_32bit_reg(IP_NACC1_ID) >> IP_NACC1_BIT_OFFSET) & (IP_NACC1_BIT_MASK >> IP_NACC1_BIT_OFFSET));
    uint32_t pdoa_phase = zn_read_32bit_reg(DIF_PHASE_ID) & DIF_PHASE_BIT_MASK;
    int32_t  a          = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
    double   p          = (double)a / 8388608;
    double   f          = p * (channel == CHANNEL_5 ? 1.1557 : 1.0043);

    *valid_flag = (a0 == a1);
    if(!(*valid_flag))
    {
        return;
    }

    #ifdef _PDOA_EN
    *aoa = (p + PHASE_COMPEN_RX0_RX1) * 180;
    #else
    if(f >= 1)
    {
        *aoa = 90 + 57.1158 * asin(f - 1);
    }
    else if(f <= -1)
    {
        *aoa = -90 + 57.1158 * asin(f + 1);
    }
    else
    {
        *aoa = 57.1158 * asin(f);
    }
    #endif
}

/***************************************************************************************************************
* Description: This is used to read the 3d dif_phase and convert to the aoa angle
* Input param:
* channel - channel id
* Output param:
* aoa1 - aoa1 result
* aoa2 - aoa2 result
* valid_flag - indicate the valid flag of the pdoa result
* Return value: None
****************************************************************************************************************/
void zn_read_3d_aoa_result(double *aoa1, double *aoa2, uint8_t *valid_flag, uint8_t channel)
{
    uint16_t a0         = ((zn_read_32bit_reg(IP_NACC0_ID) >> IP_NACC0_BIT_OFFSET) & (IP_NACC0_BIT_MASK >> IP_NACC0_BIT_OFFSET));
    uint16_t a1         = ((zn_read_32bit_reg(IP_NACC1_ID) >> IP_NACC1_BIT_OFFSET) & (IP_NACC1_BIT_MASK >> IP_NACC1_BIT_OFFSET));
    uint32_t pdoa_phase = zn_read_32bit_reg(DIF_PHASE_ID) & DIF_PHASE_BIT_MASK;
    int32_t  a          = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
    double   p          = (double)a / 8388608;
    double   f          = 0.0;

    *valid_flag = (a0 == a1);
    if(!(*valid_flag))
    {
        return;
    }

    #ifdef _PDOA_EN
    *aoa1 = (p - PHASE_COMPEN_RX0_RX1) * 180;
    #else
    f = (p - PHASE_COMPEN_RX0_RX1) * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
    if(f >= 1)
    {
        *aoa1 = 90 + 57.1158 * asin(f - 1);
    }
    else if(f <= -1)
    {
        *aoa1 = -90 + 57.1158 * asin(f + 1);
    }
    else
    {
        *aoa1 = 57.1158 * asin(f);
    }
    #endif

    pdoa_phase = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1d5c) & 0x1FFFFFF; 
    a = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
    p = (double)a / 8388608;

    #ifdef _PDOA_EN
    *aoa2 = (p - PHASE_COMPEN_RX0_RX2) * 180;
    #else
    f = (p - PHASE_COMPEN_RX0_RX2) * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
    if(f >= 1)
    {
        *aoa2 = 90 + 57.1158 * asin(f - 1);
    }
    else if(f <= -1)
    {
        *aoa2 = -90 + 57.1158 * asin(f + 1);
    }
    else
    {
        *aoa2 = 57.1158 * asin(f);
    }
    #endif
}

double phase_anti_tan_calc(double compe_i, double compe_q)
{
    if(compe_i >= 0 && compe_q >=0)
    {
        return atan((double)compe_q / compe_i);
    }
    else if(compe_i < 0 && compe_q >= 0)
    {
        return PI + atan((double)compe_q / compe_i);
    }
    else if(compe_i < 0 && compe_q < 0)
    {
        return PI + atan((double)compe_q / compe_i);
    }
    else
    {
        return 2 * PI + atan((double)compe_q / compe_i);
    }
}

double phase_compen_calc_ip(uint8_t rx0_rx1_flag, uint8_t *valid_flag)
{
    double   phase   = 0.0;
    uint32_t temp_i  = 0; 
    uint32_t temp_q  = 0;
    int32_t  compe_i = 0; 
    int32_t  compe_q = 0;

    if(rx0_rx1_flag == 0)
    {
        // compentation phase calculation
        temp_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D44) & 0x3FFFFF;
        compe_i = temp_i >= 2097152 ? (signed)(temp_i - 4194304) : (signed)temp_i;
        temp_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D48) & 0x3FFFFF;
        compe_q = temp_q >= 2097152 ? (signed)(temp_q - 4194304) : (signed)temp_q;
    }
    else
    {
        temp_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D4C) & 0x3FFFFF;
        compe_i = temp_i >= 2097152 ? (signed)(temp_i - 4194304) : (signed)temp_i;
        temp_q = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D50) & 0x3FFFFF;
        compe_q = temp_q >= 2097152 ? (signed)(temp_q - 4194304) : (signed)temp_q;
    }

    *valid_flag = (compe_i == 0 ? 0 : 1);

    if(*valid_flag == 0)
    {
        return 0.0;
    }

    phase = phase_anti_tan_calc(compe_i, compe_q);

    return phase;
}

/*******************************************************************************************************************
* Description: This is used to calculate the IP_pdoa result in case of NLOS senario
* Input para: None
* Output para:
* valid_flag - the valid flag for current calculation.
* phase_out_rx0 - rx0 phase information.
* phase_out_rx1 - rx1 phase information.
* return value: None
*******************************************************************************************************************/
void pdoa_calc_nlos_ip(uint8_t *valid_flag, double *phase_out_rx0, double *phase_out_rx1)
{
    double   phase0_rx0, phase1_rx0, phase_rx0;
    double   phase0_rx1, phase1_rx1, phase_rx1;
    uint64_t sqrt_iq_rx0 = 0,sqrt_iq_rx1 = 0, sqrt_iq_tmp_rx0, sqrt_iq_tmp_rx1;
    uint32_t i_rx0 = 0, q_rx0 = 0, i_rx1 = 0, q_rx1 = 0, i1_rx0, q1_rx0, i1_rx1, q1_rx1;
    uint32_t i_tmp, q_tmp;
    uint32_t temp_i, temp_q;
    uint32_t addr = 0;
    int32_t  compe_i, compe_q;
    uint16_t f_idx_rx0, f_idx_rx1;
    uint16_t start_idx;
    uint8_t  rx0_done = 0, rx1_done = 0;
    uint8_t  row_idx, col_idx;
    uint8_t  tmp_read_mem_buf[8];

    // phase calculation of the compenstation phase
    phase1_rx0 = phase_compen_calc_ip(0, valid_flag);
    if(*valid_flag == 0)
    {
        return;
    }

    phase1_rx1 = phase_compen_calc_ip(1, valid_flag);
    if(*valid_flag == 0)
    {
        return;
    }

    f_idx_rx0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a38) >> 22;
    f_idx_rx0++;
    f_idx_rx1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b14) >> 22;
    f_idx_rx1++;

    if(DELAY_RX0_RX1 >= 0)
    {
        uint16_t t = f_idx_rx1 + DELAY_RX0_RX1 <= 1015 ? f_idx_rx1 + DELAY_RX0_RX1 : f_idx_rx1 + DELAY_RX0_RX1 - 1016;
        if(t >= f_idx_rx0)
        {
            f_idx_rx0 = t;
        }
        else
        {
            f_idx_rx1 = f_idx_rx0 - DELAY_RX0_RX1 >= 0 ? f_idx_rx0 - DELAY_RX0_RX1 : f_idx_rx0 - DELAY_RX0_RX1 + 1016;
        }
    }
    else
    {
        uint16_t t = f_idx_rx0 - DELAY_RX0_RX1 <= 1015 ? f_idx_rx0 - DELAY_RX0_RX1 : f_idx_rx0 - DELAY_RX0_RX1 - 1016;
        if(t >= f_idx_rx1)
        {
            f_idx_rx1 = t;
        }
        else
        {
            f_idx_rx0 = f_idx_rx1 + DELAY_RX0_RX1 >= 0 ? f_idx_rx1 + DELAY_RX0_RX1 : f_idx_rx1 + DELAY_RX0_RX1 + 1016;
        }
    }

    for(uint32_t i = 0; i < 4; i++)
    {
        if(f_idx_rx0 > 1015)
        {
            f_idx_rx0 -= 1016;
        }
        addr = UWB_BASE_ADDR + 0x2000;
        start_idx = f_idx_rx0 * 8;
        zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
        i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
        q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

        if(i_tmp > 8388608)
        {
            i1_rx0 = (signed)i_tmp - 16777216;
        }
        else
        {
            i1_rx0 = (signed)i_tmp;
        }

        if(q_tmp > 8388608)
        {
            q1_rx0 = (signed)q_tmp - 16777216;
        }
        else
        {
            q1_rx0 = (signed)q_tmp;
        }

        sqrt_iq_tmp_rx0 = (uint64_t)i1_rx0 * i1_rx0 + (uint64_t)q1_rx0 * q1_rx0;

        if(f_idx_rx1 > 1015)
        {
            f_idx_rx1 -= 1016;
        }

        start_idx = f_idx_rx1 * 8;
        addr = UWB_BASE_ADDR + 0x4000;
        zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
        i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
        q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

        if(i_tmp > 8388608)
        {
            i1_rx1 = (signed)i_tmp - 16777216;
        }
        else
        {
            i1_rx1 = (signed)i_tmp;
        }

        if(q_tmp > 8388608)
        {
            q1_rx1 = (signed)q_tmp - 16777216;
        }
        else
        {
            q1_rx1 = (signed)q_tmp;
        }

        sqrt_iq_tmp_rx1 = (uint64_t)i1_rx1 * i1_rx1 + (uint64_t)q1_rx1 * q1_rx1;

        if(rx0_done == 0 && sqrt_iq_tmp_rx0 < sqrt_iq_rx0)
        {
            rx0_done = 1;
        }

        if(rx1_done == 0 && sqrt_iq_tmp_rx1 < sqrt_iq_rx1)
        {
            rx1_done = 1;
        }

        //if((rx0_done == 1 && rx1_done == 1) || (rx0_done == 1 && f_idx_rx0 <= f_idx_rx1) || (rx1_done == 1 && f_idx_rx1 <= f_idx_rx0)){
        //if((rx0_done == 1 || rx1_done == 1)){		
        if((rx0_done == 1 && rx1_done == 1))
        {
            break;
        }
        else
        {
            if(rx0_done == 0)
            {
                i_rx0 = i1_rx0;
                q_rx0 = q1_rx0;
                sqrt_iq_rx0 = sqrt_iq_tmp_rx0;
                f_idx_rx0++;
            }

            if(rx1_done == 0)
            {
                i_rx1 = i1_rx1;
                q_rx1 = q1_rx1;
                sqrt_iq_rx1 = sqrt_iq_tmp_rx1;
                f_idx_rx1++;
            }
        }

//      if(rx0_done == 0 && sqrt_iq_tmp_rx0 > 400000000 && i_rx0 > 0)
//      {
//          rx0_done = 1;
//      }
//      if(rx1_done == 0 && sqrt_iq_tmp_rx1 > 400000000 && i_rx1 > 0)
//      {
//          rx1_done = 1;
//      } 
    }

//  printf("rx0_done=%d, rx1_done=%d, f_idx_rx0=%d, f_idx_rx1=%d, i_rx0=%d, q_rx0=%d, i_rx1=%d, q_rx1=%d\n", rx0_done, rx1_done, f_idx_rx0, f_idx_rx1, i_rx0,q_rx0, i_rx1, q_rx1);

    // phase calculation,  phase0 : [0,2*PI)
    phase0_rx0 = phase_anti_tan_calc(i_rx0, q_rx0);
    // phase calculation
    phase_rx0 = phase0_rx0 - phase1_rx0;
    if(phase_rx0 < 0)
    {
        phase_rx0 += 2 * PI;
    }

    *phase_out_rx0 = phase_rx0 / PI;

    phase0_rx1 = phase_anti_tan_calc(i_rx1, q_rx1);
    // phase calculation
    phase_rx1 = phase0_rx1 - phase1_rx1;
    if(phase_rx1 < 0)
    {
        phase_rx1 += 2 * PI;
    }

    *phase_out_rx1 = phase_rx1 / PI;
}

/********************************************************************************************************************
* Description: This is used to calculate the IP_pdoa result in case of NLOS senario
* Input para: 
* rx0_rx1_flag - indicate the RX Chain. 0: RX0  1:RX1
* pulse_idx - pulse index
* Output para: 
* valid_flag - the valid flag for current calculation. 
* phase - phase information. 
* return value: None
*******************************************************************************************************************/
void pdoa_calc_nlos_ip_single(uint8_t *valid_flag, double *phase, uint8_t rx0_rx1_flag, uint16_t pulse_idx)
{
    double   phase0_rx0, phase1_rx0, phase_rx0;
    double   phase0_rx1, phase1_rx1, phase_rx1;
    uint64_t sqrt_iq_rx0 = 0, sqrt_iq_rx1 = 0, sqrt_iq_tmp_rx0, sqrt_iq_tmp_rx1;
    int32_t  i_rx0 = 0, q_rx0 = 0, i_rx1 = 0, q_rx1 = 0, i1_rx0, q1_rx0, i1_rx1, q1_rx1;
    uint32_t i_tmp, q_tmp;
    uint32_t temp_i, temp_q;
    uint32_t addr = 0;
    int32_t  compe_i, compe_q;
    uint16_t f_idx_rx0, f_idx_rx1;
    uint16_t start_idx;
    uint8_t  rx0_done = 0, rx1_done = 0;
    uint8_t  row_idx, col_idx;
    uint8_t  tmp_read_mem_buf[8];

    if(rx0_rx1_flag == 0)
    {
        phase1_rx0 = phase_compen_calc_ip(0, valid_flag);
        if(*valid_flag == 0)
        {
            return;
        }

        f_idx_rx0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a38) >> 22; 
        f_idx_rx0++;

        for(uint32_t i = 0; i < 4; i++)
        {
            if(f_idx_rx0 > 1015)
            {
                f_idx_rx0 -= 1016;
            }

            addr = UWB_BASE_ADDR + 0x2000;
            start_idx = f_idx_rx0 * 8;
            zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
            i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
            q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

            if(i_tmp > 8388608)
            {
                i1_rx0 = (signed)i_tmp - 16777216;
            }
            else
            {
                i1_rx0 = (signed)i_tmp;
            }

            if(q_tmp > 8388608)
            {
                q1_rx0 = (signed)q_tmp - 16777216;
            }
            else
            {
                q1_rx0 = (signed)q_tmp;
            }

            sqrt_iq_tmp_rx0 = (uint64_t)i1_rx0 * i1_rx0 + (uint64_t)q1_rx0 * q1_rx0;

            if(sqrt_iq_tmp_rx0 < sqrt_iq_rx0)
            {
                rx0_done = 1;
            }

            if(rx0_done == 0)
            {
                i_rx0 = i1_rx0;
                q_rx0 = q1_rx0;
                sqrt_iq_rx0 = sqrt_iq_tmp_rx0;
            }

//          int16_t t = f_idx_rx0 - pulseIdx < -900 ? 1016 + (f_idx_rx0 - pulseIdx) : (f_idx_rx0 - pulseIdx > 900 ? 1016 - (f_idx_rx0 - pulseIdx) : f_idx_rx0 - pulseIdx);
//          printf("f_idx_rx0=%d, pulseIdx=%d, i_rx0=%d, q_rx0=%d\n", f_idx_rx0, pulseIdx, i_rx0, q_rx0);
            if(rx0_done == 1)
            {
//              if(t < DELAY_RX0_RX1 || t > DELAY_RX0_RX1 + 1)
//              {
//                  *validFlag = 0;
//              }
                break;
            }
            f_idx_rx0++;
        }

        if(rx0_done == 0 || *valid_flag == 0)
        {
            *valid_flag = 0;
            return;
        }

//      printf("IP: RX0, f_idx_rx0=%d, i1_rx0=%d, q1_rx0=%d\n", f_idx_rx0, i_rx0, q_rx0);
        // phase calculation,  phase0 : [0,2*PI)
        phase0_rx0 = phase_anti_tan_calc(i_rx0, q_rx0);

        // phase calculation
        phase_rx0 = phase0_rx0 - phase1_rx0;
        if(phase_rx0 < 0)
        {
            phase_rx0 += 2 * PI;
        }
        *phase = phase_rx0 / PI;
    }
    else
    {
        // rx1
        phase1_rx1 = phase_compen_calc_ip(1, valid_flag);
        if(*valid_flag == 0)
        {
            return;
        }

        f_idx_rx1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b14) >> 22;
        f_idx_rx1++;

        for(uint32_t i = 0; i < 4; i++)
        {
            if(f_idx_rx1 > 1015)
            {
                f_idx_rx1 -= 1016;
            }
            start_idx = f_idx_rx1 * 8;
            addr = UWB_BASE_ADDR + 0x4000;
            zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
            i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
            q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

            if(i_tmp > 8388608)
            {
                i1_rx1 = (signed)i_tmp - 16777216;
            }
            else
            {
                i1_rx1 = (signed)i_tmp;
            }

            if(q_tmp > 8388608)
            {
                q1_rx1 = (signed)q_tmp - 16777216;
            }
            else
            {
                q1_rx1 = (signed)q_tmp;
            }

            sqrt_iq_tmp_rx1 = (uint64_t)i1_rx1 * i1_rx1 + (uint64_t)q1_rx1 * q1_rx1;
            if(sqrt_iq_tmp_rx1 < sqrt_iq_rx1)
            {
                rx1_done = 1;
            }

            if(rx1_done == 0)
            {
                i_rx1 = i1_rx1;
                q_rx1 = q1_rx1;
                sqrt_iq_rx1 = sqrt_iq_tmp_rx1;
            }
//          int16_t t = pulseIdx - f_idx_rx1 < -900 ? 1016 + (pulseIdx - f_idx_rx1) : (pulseIdx - f_idx_rx1 > 900 ? 1016 - (pulseIdx - f_idx_rx1) : pulseIdx - f_idx_rx1);
//          printf("f_idx_rx1=%d, pulseIdx=%d, i_rx1=%d, q_rx1=%d\n", f_idx_rx1, pulseIdx, i_rx1, q_rx1);
            if(rx1_done == 1)
            {
//              if(t < DELAY_RX0_RX1 || t > DELAY_RX0_RX1 + 1)
//              {
//                  *validFlag = 0;
//              }
                break;
            }
            f_idx_rx1++;
        }

        if(rx1_done == 0 || *valid_flag == 0)
        {
            *valid_flag = 0;
            return;
        }

//      printf("IP: RX1, f_idx_rx1=%d, i1_rx1=%d, q1_rx0=%d\n", f_idx_rx1, i_rx1, q_rx1);

        phase0_rx1 = phase_anti_tan_calc(i_rx1, q_rx1);
        // phase calculation
        phase_rx1 = phase0_rx1 - phase1_rx1;
        if(phase_rx1 < 0)
        {
            phase_rx1 += 2 * PI;
        }
        *phase = phase_rx1 / PI;
    }
}

double phase_compen_calc_cp(uint8_t rx0_rx1_flag, uint8_t *valid_flag)
{
    double   phase;
    uint32_t temp_i, temp_q;
    int32_t  compe_i, compe_q;

    if(rx0_rx1_flag == 0)
    {
        // compentation phase calculation
        temp_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D54) & 0xFFFF;
        compe_i = temp_i >= 32768 ? (signed)(temp_i - 65536) : (signed)temp_i;
        temp_q = ( zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D54) >> 16) & 0xFFFF;
        compe_q = temp_q >= 32768 ? (signed)(temp_q - 65536) : (signed)temp_q;
    }
    else
    {
        temp_i = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D58) & 0xFFFF;
        compe_i = temp_i >= 32768 ? (signed)(temp_i - 65536) : (signed)temp_i;
        temp_q = ( zn_read_32bit_reg(UWB_BASE_ADDR + 0x1D58) >> 16) & 0xFFFF;
        compe_q = temp_q >= 32768 ? (signed)(temp_q - 65536) : (signed)temp_q;
    }

    *valid_flag = (compe_i == 0 ? 0 : 1);
    if(*valid_flag == 0)
    {
        return 0.0;
    }

    phase = phase_anti_tan_calc(compe_i, compe_q);

    return phase;
}

/********************************************************************************************************************
 * Description: This is used to calculate the pdoa result in case of NLOS senario
 * Input para: None         
 * Output para: 
 * valid_flag - the valid flag for current calculation. 
 * phase_out_rx0 - rx0 phase information. 
 * phase_out_rx1 - rx1 phase information. 
 * return value: None
 *******************************************************************************************************************/
void pdoa_calc_nlos_cp(uint8_t *valid_flag, double *phase_out_rx0, double *phase_out_rx1)
{
    double   phase0_rx0, phase1_rx0, phase_rx0;
    double   phase0_rx1, phase1_rx1, phase_rx1;
    uint64_t sqrt_iq_rx0 = 0, sqrt_iq_rx1 = 0, sqrt_iq_tmp_rx0, sqrt_iq_tmp_rx1;
    int32_t  i_rx0 = 0, q_rx0 = 0, i_rx1 = 0, q_rx1 = 0, i1_rx0, q1_rx0, i1_rx1, q1_rx1;
    uint32_t i_tmp, q_tmp;
    uint32_t temp_i, temp_q;
    int32_t  compe_i, compe_q;
    uint32_t addr = 0;
    int32_t  dt = 0, c2;
    uint16_t f_idx_rx0, f_idx_rx1;
    uint16_t start_idx;
    uint8_t  rx0_done = 0, rx1_done = 0;
    uint8_t  win_len = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1634) >> 23;
    uint16_t win_l = (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1638) >> 14) & 0x1FF;
    uint16_t win_r = (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1638) >> 23) & 0x1FF;
    uint16_t pulse_idx_rx0 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a74) >> 22;
    uint16_t pulse_idx_rx1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b1c) >> 22;
    uint8_t  pdoa_type = zn_read_32bit_reg(UWB_BASE_ADDR + 0x191c) & 0x1;
    int16_t  pdoa_adj  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1920) & 0x3FF;
    uint8_t  row_idx, col_idx;
    uint8_t  c0,c1;
    uint8_t  tmp_read_mem_buf[8];

    if(pdoa_adj >= 512)
    {
        pdoa_adj -= 1024;
    }

    if(pdoa_type == 0)
    {
        pulse_idx_rx1 = (pulse_idx_rx0 + pdoa_adj > 1015) ? pulse_idx_rx0 + pdoa_adj - 1016 : (pulse_idx_rx0 + pdoa_adj < 0 ? pulse_idx_rx0 + pdoa_adj + 1016 : pulse_idx_rx0 + pdoa_adj);
    }

    // compentation phase calculation
    phase1_rx0 = phase_compen_calc_cp(0, valid_flag);
    if(*valid_flag == 0)
    {
        return;
    }

    // rx1
    phase1_rx1 = phase_compen_calc_cp(1, valid_flag);
    if(*valid_flag == 0)
    {
        return;
    }

    uint16_t peak_idx0 =  zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a48) >> 22;
    uint16_t peak_idx1 =  zn_read_32bit_reg(UWB_BASE_ADDR + 0x1c48) >> 22;	
    f_idx_rx0 = peak_idx0 - (win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a58) >> 22));
    f_idx_rx0+=2;
    f_idx_rx1 = peak_idx1 - (win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1C58) >> 22));
    f_idx_rx1+=2;

//  printf("f_idx_rx0=%d, f_idx_rx1=%d\n", f_idx_rx0, f_idx_rx1);
    if(pulse_idx_rx0 < win_l - 1)
    {
        c0 = 8 - ((win_l - 1) - pulse_idx_rx0) & 0x7;
    }
    else
    {
        c0 = (pulse_idx_rx0 - (win_l - 1)) & 0x7;
    }

    if(pulse_idx_rx1 < win_l - 1)
    {
        c1 = 8 - ((win_l - 1) - pulse_idx_rx1 ) & 0x7;
    }
    else
    {
        c1 = (pulse_idx_rx1 - (win_l - 1)) & 0x7;
    }

    if(pulse_idx_rx0 + win_r >= 1024)
    {
        c0 += ((((pulse_idx_rx0 + win_r - 1024) >> 3) + 1) << 3);
    }

    if(pulse_idx_rx1 + win_r >= 1024)
    {
        c1 += ((((pulse_idx_rx1 + win_r - 1024) >> 3) + 1) << 3);
    }

    if(pulse_idx_rx0 - pulse_idx_rx1 < -900)
    {
        c2 = pulse_idx_rx0 - pulse_idx_rx1 + 1016;
    }
    else if(pulse_idx_rx0 - pulse_idx_rx1 > 900)
    {
        c2 = pulse_idx_rx0 - pulse_idx_rx1 - 1016;
    }
    else
    {
        c2 = pulse_idx_rx0 - pulse_idx_rx1;
    }

    dt = c0 - c1 - (c2 - DELAY_RX0_RX1);
    if(f_idx_rx0 - f_idx_rx1 >= dt)
    {
        f_idx_rx1 += (f_idx_rx0 - f_idx_rx1 - dt);
    }
    else
    {
        f_idx_rx0 -= (f_idx_rx0 - f_idx_rx1 - dt);
    }

//	printf("pulseIdx_rx0=%d, pulseIdx_rx1=%d, c0=%d, c1=%d, f_idx_rx0=%d, f_idx_rx1=%d\n", pulseIdx_rx0, pulseIdx_rx1, c0, c1, f_idx_rx0, f_idx_rx1);
    
    for(int32_t i = 0; i < 5; i++)
    {
        addr = UWB_BASE_ADDR + 0x6000;
        start_idx = f_idx_rx0 * 8;
        zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
        i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
        q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

        if(i_tmp > 131072)
        {
            i1_rx0 = (signed)i_tmp - 262144;
        }
        else
        {
            i1_rx0 = (signed)i_tmp;
        }

        if(q_tmp > 131072)
        {
            q1_rx0 = (signed)q_tmp - 262144;
        }
        else
        {
            q1_rx0 = (signed)q_tmp;
        }

        sqrt_iq_tmp_rx0 = (uint64_t)i1_rx0 * i1_rx0 + (uint64_t)q1_rx0 * q1_rx0;

        addr = UWB_BASE_ADDR + 0x8000;
        start_idx = f_idx_rx1 * 8;
        zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
        i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
        q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

        if(i_tmp > 131072)
        {
            i1_rx1 = (signed)i_tmp - 262144;
        }
        else
        {
            i1_rx1 = (signed)i_tmp;
        }

        if(q_tmp > 131072)
        {
            q1_rx1 = (signed)q_tmp - 262144;
        }
        else
        {
            q1_rx1 = (signed)q_tmp;
        }

        sqrt_iq_tmp_rx1 = (uint64_t)i1_rx1 * i1_rx1 + (uint64_t)q1_rx1 * q1_rx1;

        if(rx0_done == 0 && sqrt_iq_tmp_rx0 < sqrt_iq_rx0)
        {
            rx0_done = 1;
        }

        if(rx1_done == 0 && sqrt_iq_tmp_rx1 < sqrt_iq_rx1)
        {
            rx1_done = 1;
        }

        //if((rx0_done == 1 && rx1_done == 1) || (rx0_done == 1 && f_idx_rx0 <= f_idx_rx1) || (rx1_done == 1 && f_idx_rx1 <= f_idx_rx0)){
        //if((rx0_done == 1 || rx1_done == 1)){	
        if((rx0_done == 1 && rx1_done == 1))
        {
            break;
        }
        else
        {
            if(rx0_done == 0)
            {
                i_rx0 = i1_rx0;
                q_rx0 = q1_rx0;
                sqrt_iq_rx0 = sqrt_iq_tmp_rx0;
                f_idx_rx0++;
            }

            if(rx1_done == 0)
            {
                i_rx1 = i1_rx1;
                q_rx1 = q1_rx1;
                sqrt_iq_rx1 = sqrt_iq_tmp_rx1;
                f_idx_rx1++;
            }
        }

//      if(rx0_done == 0 && sqrt_iq_tmp_rx0 > 400000000 && i_rx0 > 0)
//      {
//          rx0_done = 1;
//      }
//      if(rx1_done == 0 && sqrt_iq_tmp_rx1 > 400000000 && i_rx1 > 0)
//      {
//          rx1_done = 1;
//      }
    }
    printf("rx0_done=%d, rx1_done=%d, f_idx_rx0=%d, f_idx_rx1=%d, i_rx0=%d, q_rx0=%d, i_rx1=%d, q_rx1=%d\n", rx0_done, rx1_done, f_idx_rx0, f_idx_rx1, i_rx0, q_rx0, i_rx1, q_rx1);

    // phase calculation,  phase0 : [0,2*PI)
    phase0_rx0 = phase_anti_tan_calc(i_rx0, q_rx0);

    // phase calculation
    phase_rx0 = phase0_rx0 - phase1_rx0;
    if(phase_rx0 < 0)
    {
        phase_rx0 += 2*PI;
    }

    *phase_out_rx0 = phase_rx0 / PI;

    phase0_rx1 = phase_anti_tan_calc(i_rx1, q_rx1);
    // phase calculation
    phase_rx1 = phase0_rx1 - phase1_rx1;
    if(phase_rx1 < 0)
    {
        phase_rx1 += 2 * PI;
    }

    *phase_out_rx1 = phase_rx1 / PI;
}

/*******************************************************************************************************************
* Description: This is used to calculate the pdoa result in case of NLOS senario
* Input para:
* rx0_rx1_flg - flag
* Output para: 
* valid_flag - the valid flag for current calculation. 
* phase - phase information. 
* return value: None
*******************************************************************************************************************/
void pdoa_calc_nlos_cp_single(uint8_t *valid_flag, double *phase, uint8_t rx0_rx1_flag)
{
    double   phase0_rx0, phase1_rx0, phase_rx0;
    double   phase0_rx1, phase1_rx1, phase_rx1;
    uint64_t sqrt_iq_rx0 = 0, sqrt_iq_rx1 = 0, sqrt_iq_tmp_rx0, sqrt_iq_tmp_rx1;
    int32_t  i_rx0 = 0, q_rx0 = 0, i_rx1 = 0, q_rx1 = 0, i1_rx0, q1_rx0, i1_rx1, q1_rx1;
    uint32_t i_tmp, q_tmp;
    uint32_t temp_i, temp_q;
    int32_t  compe_i, compe_q;
    uint32_t addr = 0;
    uint16_t f_idx_rx0, f_idx_rx1;
    uint16_t start_idx;
    uint8_t  rx0_done = 0, rx1_done = 0;
    uint8_t  win_len = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1634) >> 23;
    uint8_t  row_idx, col_idx;
    uint8_t  tmp_read_mem_buf[8];

    if(rx0_rx1_flag == 0)
    {
        // compentation phase calculation
        phase1_rx0 = phase_compen_calc_cp(0, valid_flag);
        if(*valid_flag == 0)
        {
            return;
        }

        uint16_t peak_idx0 =  zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a48) >> 22;
        f_idx_rx0 = peak_idx0 - (win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a58) >> 22));
        f_idx_rx0 += 2;

        for(int32_t i = 0; i < 5; i++)
        {
            addr = UWB_BASE_ADDR+0x6000;
            start_idx = f_idx_rx0 * 8;
            zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
            i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
            q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

            if(i_tmp > 131072)
            {
                i1_rx0 = (signed)i_tmp - 262144;
            }
            else
            {
                i1_rx0 = (signed)i_tmp;
            }

            if(q_tmp > 131072)
            {
                q1_rx0 = (signed)q_tmp - 262144;
            }
            else
            {
                q1_rx0 = (signed)q_tmp;
            }

            sqrt_iq_tmp_rx0 = (uint64_t)i1_rx0 * i1_rx0 + (uint64_t)q1_rx0 * q1_rx0;
            if(sqrt_iq_tmp_rx0 < sqrt_iq_rx0)
            {
                rx0_done = 1;
                break;
            }

            printf("CP_RX0: Idx_rx0=%d, pulseIdx=%d, i_rx0=%d, q_rx0=%d\n", f_idx_rx0, peak_idx0, i_rx0, q_rx0);

            i_rx0 = i1_rx0;
            q_rx0 = q1_rx0;
            sqrt_iq_rx0 = sqrt_iq_tmp_rx0;
            f_idx_rx0++;
        }

        if(rx0_done == 0)
        {
            *valid_flag = 0;
            return;			
        }

//      printf("CP: RX0, f_idx_rx0=%d, i1_rx0=%d, q1_rx0=%d\n", f_idx_rx0, i_rx0, q_rx0);

        // phase calculation,  phase0 : [0,2*PI)
        phase0_rx0 = phase_anti_tan_calc(i_rx0, q_rx0);

        // phase calculation
        phase_rx0 = phase0_rx0 - phase1_rx0;
        if(phase_rx0 < 0)
        {
            phase_rx0 += 2 * PI;
        }
        *phase = phase_rx0 / PI;
    }
    else
    {
        // rx1
        phase1_rx1 = phase_compen_calc_cp(1, valid_flag);
        if(*valid_flag == 0)
        {
            return;
        }

        uint16_t peak_idx1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1c48) >> 22;	
        f_idx_rx1 = peak_idx1 - (win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1C58) >> 22));
        f_idx_rx1 += 2;

        for(int32_t i = 0; i < 7; i++)
        {
            addr = UWB_BASE_ADDR + 0x8000;
            start_idx = f_idx_rx1 * 8;
            zn_read_from_device(addr + start_idx, 8, tmp_read_mem_buf);
            i_tmp = (tmp_read_mem_buf[2]<<16) + (tmp_read_mem_buf[1]<<8) + tmp_read_mem_buf[0];
            q_tmp = (tmp_read_mem_buf[6]<<16) + (tmp_read_mem_buf[5]<<8) + tmp_read_mem_buf[4];

            if(i_tmp > 131072)
            {
                i1_rx1 = (signed)i_tmp - 262144;
            }
            else
            {
                i1_rx1 = (signed)i_tmp;
            }

            if(q_tmp > 131072)
            {
                q1_rx1 = (signed)q_tmp - 262144;
            }
            else
            {
                q1_rx1 = (signed)q_tmp;
            }

            sqrt_iq_tmp_rx1 = (uint64_t)i1_rx1 * i1_rx1 + (uint64_t)q1_rx1 * q1_rx1;
            if(sqrt_iq_tmp_rx1 < sqrt_iq_rx1)
            {
                rx1_done = 1;
                break;
            }
//          printf("CP_RX1: Idx_rx1=%d, pulseIdx=%d, i_rx1=%d, q_rx1=%d\n", f_idx_rx1, peak_idx1, i_rx1, q_rx1);
            i_rx1 = i1_rx1;
            q_rx1 = q1_rx1;
            sqrt_iq_rx1 = sqrt_iq_tmp_rx1;
            f_idx_rx1++;
        }

        if(rx1_done == 0)
        {
            *valid_flag = 0;
            return;
        }

//      printf("CP: RX1, f_idx_rx1=%d, i1_rx1=%d, q1_rx1=%d\n", f_idx_rx1, i_rx1, q_rx1);

        phase0_rx1 = phase_anti_tan_calc(i_rx1, q_rx1);

        // phase calculation
        phase_rx1 = phase0_rx1 - phase1_rx1;
        if(phase_rx1 < 0)
        {
            phase_rx1 += 2 * PI;
        }

        *phase = phase_rx1 / PI;
    }
}

/***************************************************************************************************************
* Description: This is used to read the 3d diff_phase and convert to the aoa angle
* Input param:
* channel - channel id
* Output param:
* aoa1 - aoa1 result
* aoa2 - aoa2 result
* valid_flag - indicate the valid flag of the pdoa result
* Return value: None
****************************************************************************************************************/
void zn_read_3d_aoa_nlos_result(double *aoa1, double *aoa2, uint8_t *valid_flag, uint8_t channel)
{
    uint16_t a0         = ((zn_read_32bit_reg(IP_NACC0_ID) >> IP_NACC0_BIT_OFFSET) & (IP_NACC0_BIT_MASK >> IP_NACC0_BIT_OFFSET));
    uint16_t a1         = ((zn_read_32bit_reg(IP_NACC1_ID) >> IP_NACC1_BIT_OFFSET) & (IP_NACC1_BIT_MASK >> IP_NACC1_BIT_OFFSET));
    uint32_t pdoa_phase = zn_read_32bit_reg(DIF_PHASE_ID) & DIF_PHASE_BIT_MASK;
    int32_t  a          = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
    double   p          = (double)a / 8388608;
    double   f          = 0.0;
    double 	 phase0     = 0.0;
    double   phase1     = 0.0;
    double   pdoa       = 0.0;
    uint16_t b0         = (zn_read_32bit_reg(IP_NACC0_ID) >> 20) & 0xFFF;
    uint16_t b1         = (zn_read_32bit_reg(IP_NACC1_ID) >> 20) & 0xFFF;
    uint16_t f_idx_rx0  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a38) >> 22;  
    uint16_t f_idx_rx1  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b14) >> 22; 
    uint16_t pulse_rx0  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a74) >> 22;
    uint16_t pulse_rx1  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b1c) >> 22;
    uint8_t  win_len    = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1634) >> 23;
    uint8_t  pdoa_type  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x191c) & 0x1;
    int16_t  pdoa_adj   = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1920) & 0x3FF;
    uint8_t  gap_rx0    = 0;
    uint8_t  gap_rx1    = 0;
    uint8_t  valid_flag_tmp;

    if(pdoa_adj >= 512)
    {
        pdoa_adj -= 1024;
    }

    if(pdoa_type == 0)
    {
        pulse_rx1 = (pulse_rx0 + pdoa_adj > 1015) ? pulse_rx0 + pdoa_adj - 1016 : (pulse_rx0 + pdoa_adj < 0 ? pulse_rx0 + pdoa_adj + 1016 : pulse_rx0 + pdoa_adj);
    }

    gap_rx0 = f_idx_rx0 > pulse_rx0 ? 1016 + pulse_rx0 - f_idx_rx0 : pulse_rx0 - f_idx_rx0;
    gap_rx1 = f_idx_rx1 > pulse_rx1 ? 1016 + pulse_rx1 - f_idx_rx1 : pulse_rx1 - f_idx_rx1;

    if(g_aoa_threshold == 0)
    {
        *valid_flag = abs(a0 - a1) <= 2;
    }
    else
    {
        *valid_flag = abs(a0 - a1) <= 16;
    }

    if(pdoa_type == 0 && pulse_rx0 < abs(pdoa_adj))
    {
        *valid_flag = 0;
    }

    if(gap_rx0 <= 5 && gap_rx1 <= 5)
    {
        pdoa = p - PHASE_COMPEN_RX0_RX1;
        if(pdoa > 1)
        {
            pdoa -= 2;
        }

        #ifdef _PDOA_EN
        *aoa1 = pdoa * 180;
        #else
        f = pdoa  * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
        if(f >= 1)
        {
            *aoa1 = 90 + 57.1158 * asin(f - 1);
        }
        else if(f <= -1)
        {
            *aoa1 = -90 + 57.1158 * asin(f + 1);
        }
        else
        {
            *aoa1 = 57.1158 * asin(f);
        }
        #endif
    }
    else
    {
        if(gap_rx0 > 4 && gap_rx1 <= 4)
        {
            pdoa_calc_nlos_ip_single(&valid_flag_tmp, &phase0, 0, pulse_rx1);		
            phase1 = (double)((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b3c) & 0x1FFFFFF)) / 8388608;
        }
        else if(gap_rx0 <= 4 && gap_rx1 > 4)
        {
            pdoa_calc_nlos_ip_single(&valid_flag_tmp, &phase1, 1, pulse_rx0);		
            phase0 = (double)((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b34) & 0x1FFFFFF)) / 8388608;
        }
        else
        {
            pdoa_calc_nlos_ip(&valid_flag_tmp, &phase0, &phase1);		
        }
        *valid_flag = *valid_flag & valid_flag_tmp;
        phase1 += PHASE_COMPEN_RX0_RX1;

        if(phase1 < 0)
        {
            phase1 += 2;
        }

        if(phase0 >= phase1)
        {
            double diff_phase = phase0 - phase1;
            pdoa = diff_phase > 1 ? diff_phase - 2 : diff_phase;
        }
        else
        {
            double diff_phase = phase1 - phase0;
            pdoa = diff_phase > 1 ? 2 - diff_phase : -diff_phase;
        }

        #ifdef _PDOA_EN
        *aoa1 = pdoa * 180;
        #else
        f = pdoa * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
        if(f >= 1)
        {
            *aoa1 = 90 + 57.1158 * asin(f - 1);
        }
        else if(f <= -1)
        {
            *aoa1 = -90 + 57.1158 * asin(f + 1);
        }
        else
        {
            *aoa1 = 57.1158 * asin(f);
        }
        #endif
    }

    gap_rx0 = win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a58) >> 22);
    gap_rx1 = win_len - (zn_read_32bit_reg(UWB_BASE_ADDR + 0x1C58) >> 22);
    if(gap_rx0 <= 5 && gap_rx1 <= 5)
    {
        pdoa_phase = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1d5c) & 0x1FFFFFF; 
        a = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
        p = (double)a / 8388608;

        pdoa = p - PHASE_COMPEN_RX0_RX2;
        if(pdoa > 1)
        {
            pdoa -= 2;
        }

        #ifdef _PDOA_EN
        *aoa2 = pdoa * 180;
        #else
        f = pdoa * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
        if(f >= 1)
        {
            *aoa2 = 90 + 57.1158 * asin(f - 1);
        }
        else if(f <= -1)
        {
            *aoa2 = -90 + 57.1158 * asin(f + 1);
        }
        else
        {
            *aoa2 = 57.1158 * asin(f);
        }
        #endif
    }
    else
    {
        if(gap_rx0 > 5 && gap_rx1 <= 5)
        {
            pdoa_calc_nlos_cp_single(&valid_flag_tmp, &phase0, 0);
            phase1 = (double)((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b40) & 0x1FFFFFF)) / 8388608;
        }
        else if(gap_rx0 <= 5 && gap_rx1 > 5)
        {
            pdoa_calc_nlos_cp_single(&valid_flag_tmp, &phase1, 1);
            phase0 = (double)((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b38) & 0x1FFFFFF)) / 8388608;
        }
        else
        {
            pdoa_calc_nlos_cp(&valid_flag_tmp, &phase0, &phase1);
        }

        *valid_flag = *valid_flag & valid_flag_tmp;
        phase1 += PHASE_COMPEN_RX0_RX2;
        if(phase1 < 0)
        {
            phase1 += 2;
        }

        if(phase0 >= phase1)
        {
            double diff_phase = phase0 - phase1;
            pdoa = diff_phase > 1 ? diff_phase - 2 : diff_phase;
        }
        else
        {
            double diff_phase = phase1 - phase0;
            pdoa = diff_phase > 1 ? 2 - diff_phase : -diff_phase;
        }

        #ifdef _PDOA_EN
        *aoa2 = pdoa * 180;
        #else
        f = pdoa * (channel == CHANNEL_5 ? 1.1557 : 1.0043);
        if(f >= 1)
        {
            *aoa2 = 90 + 57.1158 * asin(f - 1);
        }
        else if(f <= -1)
        {
            *aoa2 = -90 + 57.1158 * asin(f + 1);
        }
        else
        {
            *aoa2 = 57.1158 * asin(f);
        }
        #endif
    }
}

/*******************************************************************************************************************
* Description: This is used to read PDOA Result
* Input param: None
* Output param:
* pdoa - pdoa result
* Return value: None
* *******************************************************************************************************************/
void zn_read_pdoa_result(double *pdoa)
{
    uint32_t pdoa_phase = zn_read_32bit_reg(DIF_PHASE_ID) & DIF_PHASE_BIT_MASK;
    int32_t  a          = (pdoa_phase >= 16777216 ? pdoa_phase - 33554432 : pdoa_phase);
    double   P          = (double)a / 8388608;

    *pdoa = P * 180;
}

/*******************************************************************************************************************
* Description: This is used to calculate estimated clock frequency offset
* Input param: None
* Output param: None
* Return value: frequency offset value
********************************************************************************************************************/
double zn_read_frequency_offset_result(void)
{
    uint16_t a = (zn_read_32bit_reg(COE_PPM_ID) >> COE_PPM_BIT_OFFSET) & (COE_PPM_BIT_MASK >> COE_PPM_BIT_OFFSET);
    int16_t  f = (a >= 32768 ? a - 65536 : a);

    return (double)f / 542;
}

#if (ZN_SOC == ZN2014)
/********************************************************************************************************************
* Description: function to read the OTP memory.
* Input Param:
* address - address to read at
* Output Param: None
* Return value: the 32bit of read data
********************************************************************************************************************/
static uint32_t _zn_otp_read(uint16_t address)
{
    uint32_t ret_data = 0;
    uint16_t reg_value = 0;

    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value | OTP_CFG_OPT_RDM_MASK);
    zn_write_16bit_reg(OTP_ADDR_ID, address);
    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value | OTP_CFG_OPT_RD_MASK);
    mdelay(10);
    ret_data = zn_read_32bit_reg(OTP_RDATA_ID);

    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value & (~(OTP_CFG_OPT_RDM_MASK|OTP_CFG_OPT_RD_MASK)));

    return ret_data;
}

/********************************************************************************************************************
* Description: function to write the OTP memory.
* Input Param:
* address - address to write at
* value - value to write
* Output Param: None
* Return value: None
********************************************************************************************************************/
static void _zn_otp_write(uint16_t address, uint32_t value)
{
    uint32_t ret_data = 0;
    uint16_t reg_value = 0;

    zn_write_16bit_reg(OTP_CFG_ID, 0);
    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value | OTP_CFG_OPT_WRM_MASK);
    zn_write_16bit_reg(OTP_ADDR_ID, address);
    zn_write_32bit_reg(OTP_WDATA_ID, value);
    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value | OTP_CFG_OPT_WR_MASK);

    mdelay(10);
    while(!((reg_value = zn_read_16bit_reg(OTP_STATUS_ID))&OTP_STATUS_OTP_PROG_DONE_BIT_MASK));
    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value & (~(OTP_CFG_OPT_WRM_MASK|OTP_CFG_OPT_WR_MASK)));

    return;
}

/********************************************************************************************************************
* Description: This is used to read the OTP data from given address into provided array
* Input Param:
* address - this is the OTP address to read from
* length - this is the number of 32 bit words to read (array needs to be at least this length)
* Output Param:
* array - this is the pointer to the array into which to read the data
* Return value: None
********************************************************************************************************************/
void zn_otp_read(uint16_t address, uint32_t *array, uint8_t length)
{
    uint32_t index = 0;

    for(index = 0; index < length; index++)
    {
        array[index] = _zn_otp_read(address + index);
    }

    return ;
}

/********************************************************************************************************************
* Description: This is used to write the OTP data from provided array into given address
* Input Param:
* address - this is the OTP address to write to
* array - this is the pointer to the array from which to write the data
* length - this is the number of 32 bit words to write
* Output Param: None
* Return value: None
********************************************************************************************************************/
void zn_otp_write(uint16_t address, uint32_t *array, uint8_t length)
{
    int index = 0;

    for(index = 0; index < length; index++)
    {
        _zn_otp_write(address + index, array[index]);
    }

    return;
}

/********************************************************************************************************************
* Description: This is used to load the otp value to register
* Input Param: None
* Output Param: None
* Return value: None
********************************************************************************************************************/
void zn_otp_load(void)
{
    uint16_t reg_value = 0;

    zn_write_16bit_reg(OTP_CFG_ID, 0);
    reg_value = zn_read_16bit_reg(OTP_CFG_ID);
    zn_write_16bit_reg(OTP_CFG_ID, reg_value | OTP_CFG_OTP_AUTO_READ_MASK);
}
#endif

/*******************************************************************************************************************
* Description: This is used to switch LNA
* Input param: None
* Output param: None
* Return value: the flag of LNA
********************************************************************************************************************/
uint8_t zn_lna_switch(void)
{
#ifdef _LNA_SWITCH_ON
    #if (ZN_SOC == ZN2014)
    uint32_t max_peak        = 0;
    uint32_t min_peak        = 0;
    uint32_t agc_out         = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1868) & 0x3F;
    uint32_t cir_peak_value  = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a24) >> 8;
    uint32_t lna_config      = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1054);
    uint8_t  lna_switch_flag = 0xff;
    uint8_t  sync_config     = zn_read_32bit_reg(TX_PHR_CTRL_ID) & TX_PHR_CTRL_TXPSR_PE_BIT_MASK;
    uint32_t sync_len        = 0;

    switch(sync_config)
    {
        case PREAMBLE_16:
        {
            sync_len = 16;
            break;
        }
        case PREAMBLE_64:
        {
            sync_len = 64;
            break;
        }
        case PREAMBLE_1024:
        {
            sync_len = 1024;
            break;
        }
        case PREAMBLE_4096:
        {
            sync_len = 4096;
            break;
        }
        case PREAMBLE_32:
        {
            sync_len = 32;
            break;
        }
        case PREAMBLE_128:
        {
            sync_len = 128;
            break;
        }
        case PREAMBLE_256:
        {
            sync_len = 256;
            break;
        }
        case PREAMBLE_512:
        {
            sync_len = 512;
            break;
        }
        case PREAMBLE_2048:
        {
            sync_len = 2048;
            break;
        }
        default:
        {
            sync_len = 64;
            break;
        }
    }

    max_peak = 469 * sync_len + 9984;
    min_peak = 312 * sync_len - 1968;

    if((lna_config == 0x01700780) && (cir_peak_value > max_peak) && ((agc_out) < 0x1))
    {
         lna_switch_flag = false;   //close lna
    }
    else if((lna_config == 0x01000780) && (cir_peak_value < min_peak) /*&& ((agc_out) > 0x02)*/)
    {
         lna_switch_flag = true;    //open lna
    }

    return lna_switch_flag;
    #elif (ZN_SOC == ZN3005)
    #endif
#else
    return 1;
#endif
}

/*******************************************************************************************************************
* Description: This is set STS_IV and STS_Key value from configured not from the AON
* Input param: None
* Output param: None
* Return value: None
********************************************************************************************************************/
void zn_sts_load_cfg(void)
{
    #if (ZN_SOC == ZN2014)
    zn_and_32bit_reg(STS_LOAD_ID, ~(STS_CP_LOADIV_BIT_MASK | STS_CP_LOADKEY_BIT_MASK));
    #elif (ZN_SOC == ZN3006)
    zn_or_32bit_reg(STS_CFG_ID, (STS_CP_LOADIV_BIT_MASK | STS_CP_LOADKEY_BIT_MASK));
    #endif
}

/*******************************************************************************************************************
* Description: This is set the STS_IV
* Input param:
* sts_iv - sts iv value
* Output param: None
* return value: None
********************************************************************************************************************/
void zn_sts_iv_set(uint32_t *sts_iv)
{
    zn_write_32bit_reg(STS_IV_P0, sts_iv[0]);
    zn_write_32bit_reg(STS_IV_P1, sts_iv[1]);
    zn_write_32bit_reg(STS_IV_P2, sts_iv[2]);
    zn_write_32bit_reg(STS_IV_P3, sts_iv[3]);
}

/*******************************************************************************************************************
* Description: This is set the STS_Key
* Input param:
* sts_key - sts key value
* Output param: None
* return value: None
********************************************************************************************************************/
void zn_sts_key_set(uint32_t *sts_key)
{
    zn_write_32bit_reg(STS_KEY_P0, sts_key[0]);
    zn_write_32bit_reg(STS_KEY_P1, sts_key[1]);
    zn_write_32bit_reg(STS_KEY_P2, sts_key[2]);
    zn_write_32bit_reg(STS_KEY_P3, sts_key[3]);
}

/*******************************************************************************************************************
* Description: This is set the sts index
* Input param:
* crypto_sts_index - crypto sts index
* Output param: None
* return value: None
********************************************************************************************************************/
void zn_crypto_sts_idx_set(uint32_t crypto_sts_index)
{
    zn_write_32bit_reg(STS_IV_P1, crypto_sts_index);
}

/*******************************************************************************************************************
* Description: This is used to read the data from the CIR_Memery.Preamble CIR Memory contains 8182 valid data and
* STS Cir Memory contains 4224 valid data
* Input param: None
* Output param: 
* buffer_i - read the I data in the Cir memory
* buffer_q - read the Q data in the Cir memory
* Returns value: None
*******************************************************************************************************************/
void zn_read_cir_memory_iprx0(int32_t *buffer_i, int32_t *buffer_q)
{
    #if (ZN_SOC == ZN2014)
    uint32_t addr = UWB_BASE_ADDR + 0x2000;
    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    uint32_t addr = UWB_BASE_ADDR + 0x2000;
    #endif
    uint32_t n         = 0xFFFFFF;
    uint32_t n_1       = 0x7FFFFF;
    uint32_t value_tmp = 0;
    uint8_t  iq_flag   = 0;
    uint16_t read_idx  = 0;

    zn_read_from_device(addr, 8128, s_cir_memory);

    for(uint32_t i = 0; i < 8128; i++)
    {
        if(((i + 1) & 0x3) == 0)
        {
            if(iq_flag == 0)
            {
                buffer_i[read_idx] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 1;
            }
            else
            {
                buffer_q[read_idx++] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 0;
            }
            value_tmp = 0;
        }
        else
        {
            value_tmp += (s_cir_memory[i] << ((i & 0x3) << 3));
        }
    }

    return;
}

/*******************************************************************************************************************
* Description: preamble accumulated Cir memory of RX1 chain
* Input param: None
* Output param:
* buffer_i - read the I data in the Cir memory
* buffer_q - read the Q data in the Cir memory
* Returns value: None
*******************************************************************************************************************/
void zn_read_cir_memory_iprx1(int32_t *buffer_i, int32_t *buffer_q)
{
    #if (ZN_SOC == ZN2014)
    uint32_t addr = UWB_BASE_ADDR + 0x4000;
    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    uint32_t addr = UWB_BASE_ADDR + 0x4000;
    #endif
    uint32_t n         = 0xFFFFFF;
    uint32_t n_1       = 0x7FFFFF;
    uint32_t value_tmp = 0;
    uint8_t  iq_flag   = 0;
    uint16_t read_idx  = 0;

    zn_read_from_device(addr, 8128, s_cir_memory);

    for(uint32_t i = 0; i < 8128; i++)
    {
        if(((i + 1) & 0x3) == 0)
        {
            if(iq_flag == 0)
            {
                buffer_i[read_idx] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 1;
            }
            else
            {
                buffer_q[read_idx++] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 0;
            }
            value_tmp = 0;
        }
        else
        {
            value_tmp += (s_cir_memory[i] << ((i & 0x3) << 3));
        }
    }

    return;
}

/*******************************************************************************************************************
* Description: STS accumulated Cir memory of RX0 chain
* Input param: None
* Output param:
* buffer_i - read the I data in the Cir memory
* buffer_q - read the Q data in the Cir memory
* Returns value: None
*******************************************************************************************************************/
void zn_read_cir_memory_cprx0(int32_t *buffer_i, int32_t *buffer_q)
{
    #if (ZN_SOC == ZN2014)
    uint32_t addr = UWB_BASE_ADDR + 0x6000;
    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    uint32_t addr = UWB_BASE_ADDR + 0x6000;
    #endif
    uint32_t n         = 0x3FFFF;
    uint32_t n_1       = 0x1FFFF;
    uint32_t value_tmp = 0;
    uint8_t  iq_flag   = 0;
    uint16_t read_idx  = 0;

    zn_read_from_device(addr, 4224, s_cir_memory);

    for(uint32_t i = 0; i < 4224; i++)
    {
        if(((i + 1) & 0x3) == 0)
        {
            if(iq_flag == 0)
            {
                buffer_i[read_idx] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 1;
            }
            else
            {
                buffer_q[read_idx++] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 0;
            }
            value_tmp = 0;
        }
        else
        {
            value_tmp += (s_cir_memory[i] << ((i & 0x3) << 3));
        }
    }

    return;
}

/*******************************************************************************************************************
* Description: STS accumulated Cir memory of RX1 chain
* Input param: None
* Output param:
* buffer_i - read the I data in the Cir memory
* buffer_q - read the Q data in the Cir memory
* Returns value: None
*******************************************************************************************************************/
void zn_read_cir_memory_cprx1(int32_t *buffer_i, int32_t *buffer_q)
{
    #if (ZN_SOC == ZN2014)
    uint32_t addr = UWB_BASE_ADDR + 0x8000;
    #elif ((ZN_SOC == ZN3005) || (ZN_SOC == ZN3006))
    uint32_t addr = UWB_BASE_ADDR + 0x8000;
    #endif
    uint32_t n         = 0x3FFFF;
    uint32_t n_1       = 0x1FFFF;
    uint32_t value_tmp = 0;
    uint8_t  iq_flag   = 0;
    uint16_t read_idx  = 0;

    zn_read_from_device(addr, 4224, s_cir_memory);

    for(uint32_t i = 0 ; i < 4224; i++)
    {
        if(((i + 1) & 0x3) == 0)
        {
            if(iq_flag == 0)
            {
                buffer_i[read_idx] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 1;
            }
            else
            {
                buffer_q[read_idx++] = (value_tmp > n_1 ? value_tmp - n : value_tmp);
                iq_flag = 0;
            }
            value_tmp = 0;
        }
        else
        {
            value_tmp += (s_cir_memory[i] << ((i & 0x3) << 3));
        }
    }

    return;
}

/*******************************************************************************************************************
* Description: this function is change the uci channel_num to zn config
* Input param:
* channel_no - uci channel_num
* Output param: None
* Returns value: zn channel_num
*******************************************************************************************************************/
uint8_t zn_uci_channel_num_to_zn_cfg(uint8_t channel_no)
{
    channel_num_t chanel = CHANNEL_9;

    switch(channel_no)
    {
        case 5:
        {
            chanel = CHANNEL_5;
            break;
        }
        case 9:
        {
            chanel = CHANNEL_9;
            break;
        }
        default:
        {
            chanel = CHANNEL_9;
            break;
        }
    }
    return chanel;
}

/*******************************************************************************************************************
* Description: this function is change the uci SFD id to zn config
* Input param:
* sfd_id - uci SFD id
* Output param: None
* Returns value: zn SFD id
*******************************************************************************************************************/
uint8_t zn_uci_sfd_id_to_zn_cfg(uint8_t sfd_id)
{
    sfd_id_t sfd = SFD_ID_SFD8;

    switch(sfd_id)
    {
        case 0:
        {
            sfd = SFD_ID_LEGACY_SFD8;
            break;
        }
        case 1:
        {
            sfd = SFD_ID_SFD4;
            break;
        }
        case 2:
        {
            sfd = SFD_ID_SFD8;
            break;
        }
        case 3:
        {
            sfd = SFD_ID_SFD16;
            break;
        }
        default:
        {
            sfd = SFD_ID_SFD8;
            break;
        }
    }

    return sfd;
}

/*******************************************************************************************************************
* Description: this function is change the uci preamble code idx to zn config
* Input param:
* index - uci preamble code index
* Output param: None
* Returns value: zn preamble code index
*******************************************************************************************************************/
uint8_t zn_preamble_code_idx_to_zn_cfg(uint8_t index)
{
    preamble_code_index_t preamble_code_idx = PREAMBLE_CODE_INDEX_10;

    switch(index)
    {
        case 9:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_9;
            break;
        }
        case 10:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_10;
            break;
        }
        case 11:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_11;
            break;
        }
        case 12:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_12;
            break;
        }
        case 25:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_25;
            break;
        }
        case 26:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_26;
            break;
        }
        case 27:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_27;
            break;
        }
        case 28:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_28;
            break;
        }
        case 29:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_29;
            break;
        }
        case 30:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_30;
            break;
        }
        case 31:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_31;
            break;
        }
        case 32:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_32;
            break;
        }
        default:
        {
            preamble_code_idx = PREAMBLE_CODE_INDEX_10;
            break;
        }
    }

    return preamble_code_idx;
}

/*******************************************************************************************************************
* Description: this function is change the uci seg num to zn config
* Input param:
* seg - uci seg num
* Output param: None
* Returns value: zn seg num
*******************************************************************************************************************/
uint8_t zn_sts_seg_num_to_zn_cfg(uint8_t seg)
{
    sts_seg_num_t sts_seg_num = STS_SEG_1;

    switch(seg)
    {
        case 1:
        {
            sts_seg_num = STS_SEG_1;
            break;
        }
        case 2:
        {
            sts_seg_num = STS_SEG_2;
            break;
        }
        case 3:
        {
            sts_seg_num = STS_SEG_3;
            break;
        }
        case 4:
        {
            sts_seg_num = STS_SEG_4;
            break;
        }
        default:
        {
            sts_seg_num = STS_SEG_1;
            break;
        }
    }

    return sts_seg_num;
}

/*******************************************************************************************************************
* Description: this function is change the uci data rate to zn config
* Input param:
* rate - uci data rate
* Output param: None
* Returns value: zn data rate
*******************************************************************************************************************/
uint8_t zn_psdu_data_rate_to_zn_cfg(uint8_t rate)
{
    psdu_data_rate_t psdu_data_rate = PSDU_RATE_681;

    switch(rate)
    {
        case 0:
        {
            psdu_data_rate = PSDU_RATE_681;
            break;
        }
        case 2:
        {
            psdu_data_rate = PSDU_RATE_272;
            break;
        }
        case 4:
        {
            psdu_data_rate = PSDU_RATE_850;
            break;
        }
        default:
        {
            psdu_data_rate = PSDU_RATE_681;
            break;
        }
    }

    return psdu_data_rate;
}

/*******************************************************************************************************************
* Description: this function is change the uci preamble duration to zn config
* Input param:
* duration - uci preamble duration
* Output param: None
* Returns value: zn preamble duration
*******************************************************************************************************************/
uint8_t zn_preamble_duration_to_zn_cfg(uint8_t duration)
{
    switch(duration)
    {
        case 0:
        {
            duration = PREAMBLE_32;
            break;
        }
        case 1:
        {
            duration = PREAMBLE_64;
            break;
        }
        default:
        {
            duration = 5;
            break;
        }
    }
    return duration;
}

/*******************************************************************************************************************
* Description: this function is change the uci phr data rate to zn config
* Input param:
* rate - uci phr data rate
* Output param: None
* Returns value: zn phr data rate
*******************************************************************************************************************/
uint8_t zn_phr_data_rate_to_zn_cfg(uint8_t rate)
{
    bprf_phr_data_rate_t phr_data_rate = BPRF_PHR_RATE_850;

    switch(rate)
    {
        case 0:
        {
            phr_data_rate = BPRF_PHR_RATE_850;
            break;
        }
        default:
        {
            phr_data_rate = BPRF_PHR_RATE_850;
            break;
        }
    }
    return phr_data_rate;
}

/*******************************************************************************************************************
* Description: this function is change the uci rframe cfg to zn config
* Input param:
* cfg - uci rframe cfg
* Output param: None
* Returns value: zn rframe cfg
*******************************************************************************************************************/
uint8_t zn_rframe_cfg_to_zn_cfg(uint8_t cfg)
{
    rframe_config_t rframe_cfg = RFRAME_SP0;

    switch(cfg)
    {
        case 0:
        {
            rframe_cfg = RFRAME_SP0;
            break;
        }
        case 1:
        {
            rframe_cfg = RFRAME_SP1;
            break;
        }
        case 3:
        {
            rframe_cfg = RFRAME_SP3;
            break;
        }
        default:
        {
            rframe_cfg = RFRAME_SP0;
            break;
        }
    }

    return rframe_cfg;
}

/*******************************************************************************************************************
* Description: this function is change the uci prf mode to zn config
* Input param:
* cfg - uci prf mode
* Output param: None
* Returns value: zn prf mode
*******************************************************************************************************************/
uint8_t zn_prf_mode_to_zn_cfg(uint8_t prf)
{
    uint8_t prf_mode = PRF_624;

    switch(prf)
    {
        case 0:
        {
            prf_mode = PRF_624;
            break;
        }
        case 1:
        {
            prf_mode = PRF_1248;
            break;
        }
        case 2:
        {
            prf_mode = PRF_2496;
            break;
        }
        default:
        {
            prf_mode = PRF_624;
            break;
        }
    }

    return prf_mode;
}

/*******************************************************************************************************************
* Description: this function is change the uci sts seg len to zn config
* Input param:
* seg_len - uci sts seg len
* Output param: None 
* Returns value: zn sts seg len
*******************************************************************************************************************/
uint16_t zn_sts_segments_len_to_zn_cfg(uint8_t seg_len)
{
    uint16_t sts_seg_len = STS_SEG_LEN_64;

    switch(seg_len)
    {
        case 0:
        {
            sts_seg_len = STS_SEG_LEN_32;
            break;
        }
        case 1:
        {
            sts_seg_len = STS_SEG_LEN_64;
            break;
        }
        case 2:
        {
            sts_seg_len = STS_SEG_LEN_128;
            break;
        }
        default:
        {
            sts_seg_len = STS_SEG_LEN_64;
            break;
        }
    }

    return sts_seg_len;
}

/*******************************************************************************************************************
* Description: this function is change the zn psdu data rate to uci config
* Input param:
* rate - zn psdu data rate
* Output param: None
* Returns value: uci psdu data rate
*******************************************************************************************************************/
uint8_t zn_psdu_data_rate_to_uci_cfg(uint8_t rate)
{
    psdu_data_rate_t psdu_data_rate = PSDU_RATE_681;

    switch(rate)
    {
        case PSDU_RATE_681:
        {
            psdu_data_rate = 0;
            break;
        }
        case PSDU_RATE_272:
        {
            psdu_data_rate = 2;
            break;
        }
        case PSDU_RATE_850:
        {
            psdu_data_rate = 4;
            break;
        }
        default:
        {
            psdu_data_rate = 0;
            break;
        }
    }

    return psdu_data_rate;
}

/*******************************************************************************************************************
* Description: this function is change the zn preamble duration to uci config
* Input param: 
* duration - zn preamble duration
* Output param: None
* Returns value: uci preamble duration
*******************************************************************************************************************/
uint8_t zn_preamble_duration_to_uci_cfg(uint8_t duration)
{
    switch(duration)
    {
        case PREAMBLE_32:
        {
            duration = 0;
            break;
        }
        case PREAMBLE_64:
        {
            duration = 1;
            break;
        }
        default:
        {
            duration = 1;
            break;
        }
    }

    return duration;
}

/*******************************************************************************************************************
* Description: this function is cfg rx preamble config
* Input param: 
* sts_cfg  - zn sts config
* duration - zn preamble duration
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_set_rx_preamble_config(uint8_t sts_cfg, uint8_t preamble_duration)
{
    if(RFRAME_SP1 == sts_cfg || RFRAME_SP3 == sts_cfg || preamble_duration >= PREAMBLE_128 
        || preamble_duration == PREAMBLE_1024 || preamble_duration == PREAMBLE_4096)
    {
        zn_write_32bit_reg(PREAMBLE_CFG_ID, 0x2a3fc7d0);
    }
    else if(preamble_duration == PREAMBLE_16 || preamble_duration == PREAMBLE_32)
    {
        zn_write_32bit_reg(PREAMBLE_CFG_ID, 0x2a7fcfc0);
    }
    else
    {
        zn_write_32bit_reg(PREAMBLE_CFG_ID, 0x2affcfc8);
    }

    return;
}
/*******************************************************************************************************************
* Description: this function is cfg rx sfd config
* Input param: 
* sfd_id - zn sfd index
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_set_rx_sfd_config(uint8_t sfd_id)
{
    if(SFD_ID_LEGACY_SFD8 == sfd_id) 
    {
        zn_write_32bit_reg(SFD_CFG_ID, 0x76500024);
    }
    else if(SFD_ID_SFD4 == sfd_id) 
    {
        zn_write_32bit_reg(SFD_CFG_ID, 0x574b10e0);
    }
    else
    {
        zn_write_32bit_reg(SFD_CFG_ID, 0x57ffd4e4);
    }

    return;
}

/*******************************************************************************************************************
* Description: this function is process the inside loop to calibrate the timestamp.
* Input para: None
* 
* Output para: None
* Returns Value: None
*******************************************************************************************************************/
int32_t zn_inside_loop(void)
{
    uint32_t reg_0828            = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0828);
    uint32_t reg_084c            = zn_read_32bit_reg(UWB_BASE_ADDR + 0x084C);
    uint32_t reg_0824            = zn_read_32bit_reg(UWB_BASE_ADDR + 0x0824);
    uint32_t reg_1848            = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1848);
    uint32_t average_time        = 0;
    int32_t  delat_time          = 0;
    uint16_t test_num            = 10;
    uint32_t delta_time[100 + 1] = {0};
    uint8_t  tx_msg[1]         = {0};
    uint64_t tx_timestamp      = 0;
    uint64_t rx_timestamp      = 0;

    // TXRX Enable
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, 0x01408000); //2014 add
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x084C, 0x82210000);
    mdelay(10);

    // TX Message
    zn_write_tx_data(1, tx_msg, 0); //Zero offset in TX buffer.
    zn_write_tx_frame_ctrl(3, 0, 0);     //Zero offset in TX buffer, no ranging.

    // TX Power
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, 0x76092000);

    // RX Timeout
    zn_set_rx_timeout(200000);

    for(int32_t i = 0; i < test_num; i++)
    {
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x12080504);
        udelay(50);

        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x52080504);
        mdelay(10);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, 0x02080504);
        zn_read_tx_timestamp(&tx_timestamp);
        zn_read_rx_timestamp(&rx_timestamp);
        delta_time[i] = rx_timestamp - tx_timestamp;
    }

    for(int32_t i = 0; i < test_num; i++)
    {
        average_time += delta_time[i];
    }

    average_time /= test_num;
    delat_time = average_time - 26160;

    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0828, reg_0828);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x084C, reg_084c);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x0824, reg_0824);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1848, reg_1848);

    printf("insideloop average time is %d, delat_time is %d \r\n", average_time, delat_time);

    return delat_time;
}

#ifdef _PHY_DEBUG
/*******************************************************************************************************************
* Description: This is used to calculate receiver power for the first path
* Input param:
* lna_flag - whether enable LNA or not 1: LNA enabled  0: LNA disabled
* Output param: None
* Returns value: receive power
*******************************************************************************************************************/
double zn_recv_power_for_first_path(uint8_t lna_flag)
{
    double  power_adj = 2.06;
    uint8_t lna_tia   = (lna_flag ? 63 : 45);

    // CI Len
    int32_t ci_len    = (((zn_read_32bit_reg(UWB_BASE_ADDR + 0x828) >> 16) & 0x1F) >= 24 ? 91 : 127);
    int32_t pulse_len = (ci_len == 127 ? 64 : 80);

    // NACC
    uint16_t nacc = (zn_read_32bit_reg(IP_NACC0_ID) >> 20);

    // read maglitude of the first path
    uint16_t fpa1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a2c) >> 14; // *0.25/16
    uint16_t fpa2 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a30) >> 14;
    uint16_t fpa3 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1a34) >> 14;
    double fpa1_f = (double)fpa1 / nacc / pulse_len;
    double fpa2_f = (double)fpa2 / nacc / pulse_len;
    double fpa3_f = (double)fpa3 / nacc / pulse_len;
    double p = (fpa1_f * fpa1_f + fpa2_f * fpa2_f + fpa3_f * fpa3_f) / 50;
    double pdb = 10 * log10(p);

    // AGC output
    uint8_t agc_out  = AGCLUTstep2dB((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1868) & 0x3F)) * 2 ;
    double rec_power = pdb + 30 - lna_tia - agc_out + power_adj;
    printf("power estimation of RX1: recP = %.2f\n", rec_power);

    return rec_power;
}

/*******************************************************************************************************************
* Description: This is used to calculate receiver power for the second path
* Input param:
* lna_flag - whether enable LNA or not 1: LNA enabled  0: LNA disabled
* Output param: None
* Returns value: receive power
*******************************************************************************************************************/
double zn_recv_power_for_second_path(uint8_t lna_flag)
{
    double  power_adj = 2.06;
    uint8_t lna_tia   = (lna_flag ? 63 : 45);

    // CI Len
    int32_t ci_len    = (((zn_read_32bit_reg(UWB_BASE_ADDR + 0x828) >> 16) & 0x1F) >= 24 ? 91 : 127);
    int32_t pulse_len = (ci_len == 127 ? 64 : 80);

    // NACC
    uint16_t nacc = (zn_read_32bit_reg(IP_NACC1_ID) >> 20);

    // read maglitude of the first path
    uint16_t fpa1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b08) >> 14; // *0.25/16
    uint16_t fpa2 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b0c) >> 14;
    uint16_t fpa3 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1b10) >> 14;
    double fpa1_f = (double)fpa1 / nacc / pulse_len;
    double fpa2_f = (double)fpa2 / nacc / pulse_len;
    double fpa3_f = (double)fpa3 / nacc / pulse_len;
    double p = (fpa1_f * fpa1_f + fpa2_f * fpa2_f + fpa3_f * fpa3_f) / 50;
    double pdb = 10 * log10(p);

    // AGC output
    uint8_t agc_out  = AGCLUTstep2dB((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1948) & 0x3F)) * 2 ;
    double rec_power = pdb + 30 - lna_tia - agc_out + power_adj;
    printf("power estimation of RX1: recP = %.2f\n", rec_power);

    return rec_power;
}

/*******************************************************************************************************************
* Description: This is used to calculate receiver power for the third path
* Input param:
* lna_flag - whether enable LNA or not 1: LNA enabled  0: LNA disabled
* Output param: None
* Returns value: receive power
*******************************************************************************************************************/
double zn_recv_power_for_third_path(uint8_t lna_flag)
{
    double  power_adj = 7.06;
    uint8_t lna_tia   = (lna_flag ? 63 : 45);

    // read maglitude of the first path
    uint16_t fpa1 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1c4c) >> 18; // *0.25/16
    uint16_t fpa2 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1c50) >> 18;
    uint16_t fpa3 = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1c54) >> 18;
    double fpa1_f = (double)fpa1 / 64 / 64;
    double fpa2_f = (double)fpa2 / 64 / 64;
    double fpa3_f = (double)fpa3 / 64 / 64;
    double p = (fpa1_f * fpa1_f + fpa2_f * fpa2_f + fpa3_f * fpa3_f) / 50;
    double pdb = 10 * log10(p);

    // AGC output
    uint8_t agc_out  = AGCLUTstep2dB((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1948) & 0x3F)) * 2;
    double rec_power = pdb + 30 - lna_tia - agc_out + power_adj;
    printf("power estimation of RX1: recP = %.2f\n", rec_power);

    return rec_power;
}

/*******************************************************************************************************************
* Description: This is used to set double RX buffer
* Input param:
* status - the status is enbale or disable double rx buffer
* Output param: None
* Returns value: None
*******************************************************************************************************************/
void zn_set_double_buff(uint8_t status)
{
    uint8_t reg_value = 0; 
    reg_value = zn_read_32bit_reg(SYS_CFG_ID);
    if(status == ENABLE)
    {
        reg_value |= SYS_CFG_EN_DRXB_BIT_MASK;
    }
    else
    {
        reg_value |=(~SYS_CFG_EN_DRXB_BIT_MASK);
    }
    zn_write_32bit_reg(SYS_CFG_ID,reg_value);    
}

void zn_modify_PDOA_RX1_TIME_en(uint32_t rx_en)
{
    uint32_t  regValue = 0;
    regValue = zn_read_32bit_reg(SYS_CFG_ID);// clear bit to 0
    regValue &= (~SYS_CFG_PDOA_RX1_TIME_EN_BIT_MASK);
    regValue |= ((rx_en << SYS_CFG_PDOA_RX1_TIME_EN_BIT_OFFSET) & SYS_CFG_PDOA_RX1_TIME_EN_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, regValue);
    return;
}
//isHPRFen 0:HPRF disable 1:enable HPRF
void zn_modify_HPRF_en(uint32_t isHPRFen)
{
    uint32_t  regValue = 0;
    regValue = zn_read_32bit_reg(SYS_CFG_ID);// clear bit to 0
    regValue &= (~SYS_CFG_HPRF_EN_BIT_MASK);
    regValue |= ((isHPRFen << SYS_CFG_HPRF_EN_BIT_OFFSET) & SYS_CFG_HPRF_EN_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, regValue);
    return;
}
//ccMode 0:K3 1:K7
void zn_modify_CC_mode(uint32_t ccMode)
{
    uint32_t  regValue = 0;
    regValue = zn_read_32bit_reg(SYS_CFG_ID);// clear bit to 0
    regValue &= (~SYS_CFG_CC_MODE_BIT_MASK);
    regValue |= ((ccMode << SYS_CFG_CC_MODE_BIT_OFFSET) & SYS_CFG_CC_MODE_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, regValue);
    return;
}
//disRS 1:disable RS 0:enable RS
void zn_modify_RS_dis(uint32_t disRS)
{
    uint32_t  regValue = 0;
    regValue = zn_read_32bit_reg(SYS_CFG_ID);// clear bit to 0
    regValue &= (~SYS_CFG_DIS_RS_BIT_MASK);
    regValue |= ((disRS << SYS_CFG_DIS_RS_BIT_OFFSET) & SYS_CFG_DIS_RS_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, regValue);
    return;
}

//extendMode 0:<=1023byte 1:<=2047byte 2:<=4095byte
void zn_modify_HPRF_phr_extend_mode(uint32_t extendMode)
{
    uint32_t  regValue = 0;
    regValue = zn_read_32bit_reg(SYS_CFG_ID);// clear bit to 0
    regValue &= (~SYS_CFG_HPRF_PHR_EXTEND_BIT_MASK);
    regValue |= ((extendMode << SYS_CFG_HPRF_PHR_EXTEND_BIT_OFFSET) & SYS_CFG_HPRF_PHR_EXTEND_BIT_MASK);
    zn_write_32bit_reg(SYS_CFG_ID, regValue);
    return;
}
//stsMode 0:BPRF 1:HPRF
void zn_modify_sts_mode(uint32_t stsMode)
{
    uint32_t  regValue = zn_read_32bit_reg(STS_CFG_ID);
    regValue &= (~STS_DELTA_BIT_MASK);
    regValue |= ((stsMode << STS_DELTA_BIT_OFFSET) & STS_DELTA_BIT_MASK);
    zn_write_32bit_reg(STS_CFG_ID, regValue);
    return;
}

int AGCLUTstep2dB(int i)
{
    if(i<=0)
        return 0;
    else if(i==0x01)
        return 1;
    else if(i==0x02)
        return 2;
    else if(i==0x03)
        return 3;
    else if(i==0x04)
        return 4;
    else if(i==0x05)
        return 5;
    else if(i==0x06)
        return 6;
    else if(i==0x07)
        return 7;
    else if(i==0x0f)
        return 8;
    else if(i==0x17)
        return 9;
    else if(i==0x1f)
        return 10;
    else if(i==0x27)
        return 11;
    else if(i==0x2f)
        return 12;
    else if(i==0x37)
        return 13;
    else if(i==0x3f)
        return 14;
    else
        return 14;
}
#define RX_CH1_LNA_ID                           (UWB_BASE_ADDR + 0x102C)

#define D_LNA_STAGE2_AGC_BIT_OFFSET             (19U)
#define D_LNA_STAGE2_AGC_BIT_LEN                (2U)
#define D_LNA_STAGE2_AGC_BIT_MASK               0x180000U

#define D_LNA_STAGE2_VCM_SEL_BIT_OFFSET         (16U)
#define D_LNA_STAGE2_VCM_SEL_BIT_LEN            (3U)
#define D_LNA_STAGE2_VCM_SEL_BIT_MASK           0x70000U

#define D_LNA_STAGE2_BIAS_SEL_BIT_OFFSET        (12U)
#define D_LNA_STAGE2_BIAS_SEL_BIT_LEN           (4U)
#define D_LNA_STAGE2_BIAS_SEL_BIT_MASK          0xf000U

#define D_LNA_STAGE1_BIAS_SEL_BIT_OFFSET        (9U)
#define D_LNA_STAGE1_BIAS_SEL_BIT_LEN           (3U)
#define D_LNA_STAGE1_BIAS_SEL_BIT_MASK          0xe00U


#define D_LNA_STAGE1_AGC_OVRD_BIT_OFFSET        (8U)
#define D_LNA_STAGE1_AGC_OVRD_BIT_LEN           (1U)
#define D_LNA_STAGE1_AGC_OVRD_BIT_MASK          0x100U

#define D_LNA_STAGE2_AGC_OVRD_BIT_OFFSET        (7U)
#define D_LNA_STAGE2_AGC_OVRD_BIT_LEN           (1U)
#define D_LNA_STAGE2_AGC_OVRD_BIT_MASK          0x80U

#define D_TIA_AGC_OVRD_BIT_OFFSET               (6U)
#define D_TIA_AGC_OVRD_BIT_LEN                  (1U)
#define D_TIA_AGC_OVRD_BIT_MASK                 0x40U

#define RX_CH1_MIXER_ID                         (UWB_BASE_ADDR + 0x1030)

#define D_LNA_STAGE1_VCT_BIT_OFFSET             (0U)
#define D_LNA_STAGE1_VCT_BIT_LEN                (7U)
#define D_LNA_STAGE1_VCT_BIT_MASK               0x7fU


#define RX_CH1_TIA_I_ID                         (UWB_BASE_ADDR + 0x1034)

#define D_I_TIA_FB_RES_BIT_OFFSET               (15U)
#define D_I_TIA_FB_RES_BIT_LEN                  (6U)
#define D_I_TIA_FB_RES_BIT_MASK                 0x1f8000U

#define D_I_TIA_STAG1_ISEL_BIT_OFFSET          (13U)
#define D_I_TIA_STAG1_ISEL_BIT_LEN             (2U)
#define D_I_TIA_STAG1_ISEL_BIT_MASK            0x6000U

#define D_I_TIA_STG1_ITRIM_BIT_OFFSET           (10U)
#define D_I_TIA_STG1_ITRIM_BIT_LEN              (3U)
#define D_I_TIA_STG1_ITRIM_BIT_MASK             0x1c00U

#define D_I_TIA_DCOC_2UA_EN_BIT_OFFSET          (9U)
#define D_I_TIA_DCOC_2UA_EN_BIT_LEN             (1U)
#define D_I_TIA_DCOC_2UA_EN_BIT_MASK            0x200U

#define D_I_TIA_DCOC_EN_BIT_OFFSET              (8U)
#define D_I_TIA_DCOC_EN_BIT_LEN                 (1U)
#define D_I_TIA_DCOC_EN_BIT_MASK                0x100U

#define D_I_TIA_KEEP_DCOC_BIT_OFFSET            (7U)
#define D_I_TIA_KEEP_DCOC_BIT_LEN               (1U)
#define D_I_TIA_KEEP_DCOC_BIT_MASK              0x80U


#define RX_CH1_TIA_Q_ID                         (UWB_BASE_ADDR + 0x1038)

#define D_Q_TIA_DCOC_EN_BIT_OFFSET              (26U)
#define D_Q_TIA_DCOC_EN_BIT_LEN                 (1U)
#define D_Q_TIA_DCOC_EN_BIT_MASK                0x4000000U

#define D_Q_TIA_KEEP_DCOC_BIT_OFFSET            (25U)
#define D_Q_TIA_KEEP_DCOC_BIT_LEN               (1U)
#define D_Q_TIA_KEEP_DCOC_BIT_MASK              0x2000000U

#define D_Q_TIA_FB_RES_BIT_OFFSET               (6U)
#define D_Q_TIA_FB_RES_BIT_LEN                  (6U)
#define D_Q_TIA_FB_RES_BIT_MASK                 0xfc0U

#define D_Q_TIA_STAG1_ISEL_BIT_OFFSET          (4U)
#define D_Q_TIA_STAG1_ISEL_BIT_LEN             (2U)
#define D_Q_TIA_STAG1_ISEL_BIT_MASK            0x30U

#define D_Q_TIA_STG1_ITRIM_BIT_OFFSET           (1U)
#define D_Q_TIA_STG1_ITRIM_BIT_LEN              (3U)
#define D_Q_TIA_STG1_ITRIM_BIT_MASK             0xeU

#define D_Q_TIA_DCOC_2UA_EN_BIT_OFFSET          (0U)
#define D_Q_TIA_DCOC_2UA_EN_BIT_LEN             (1U)
#define D_Q_TIA_DCOC_2UA_EN_BIT_MASK            0x1U

#define RX_CH1_ATEST_ID                         (UWB_BASE_ADDR + 0x1040)

#define D_Q_TIA_ATEST_EN_BIT_OFFSET             (12U)
#define D_Q_TIA_ATEST_EN_BIT_LEN                (1U)
#define D_Q_TIA_ATEST_EN_BIT_MASK               0x1000U

#define D_I_TIA_ATEST_EN_BIT_OFFSET             (8U)
#define D_I_TIA_ATEST_EN_BIT_LEN                (1U)
#define D_I_TIA_ATEST_EN_BIT_MASK               0x100U

#define D_RX_CLK38P4_DIV_EN_BIT_OFFSET          (1U)
#define D_RX_CLK38P4_DIV_EN_BIT_LEN             (1U)
#define D_RX_CLK38P4_DIV_EN_BIT_MASK            0x2U

#define RX_CH2_LNA_ID                           (UWB_BASE_ADDR + 0x1044)

#define D_TIA_AGC_VORD_2ND_BIT_OFFSET           (6U)
#define D_TIA_AGC_VORD_2ND_BIT_LEN              (1U)
#define D_TIA_AGC_VORD_2ND_BIT_MASK             0x40U

#define D_LNA_STAGE2_AGC_VORD_2ND_BIT_OFFSET    (7U)
#define D_LNA_STAGE2_AGC_VORD_2ND_BIT_LEN       (1U)
#define D_LNA_STAGE2_AGC_VORD_2ND_BIT_MASK      0x80U

#define D_LNA_STAGE1_AGC_VORD_2ND_BIT_OFFSET    (8U)
#define D_LNA_STAGE1_AGC_VORD_2ND_BIT_LEN       (1U)
#define D_LNA_STAGE1_AGC_VORD_2ND_BIT_MASK      0x100U

#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_OFFSET    (9U)
#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_LEN       (3U)
#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_MASK      0xe00U

#define D_LNA_STAGE2_BIAS_SEL_2ND_BIT_OFFSET    (12U)
#define D_LNA_STAGE2_BIAS_SEL_2ND_BIT_LEN       (4U)
#define D_LNA_STAGE2_BIAS_SEL_2ND_BIT_MASK      0xf000U

#define D_LNA_STAGE2_VCM_SEL_2ND_BIT_OFFSET     (16U)
#define D_LNA_STAGE2_VCM_SEL_2ND_BIT_LEN        (3U)
#define D_LNA_STAGE2_VCM_SEL_2ND_BIT_MASK       0x70000U

#define D_LNA_STAGE2_AGC_2ND_BIT_OFFSET         (19U)
#define D_LNA_STAGE2_AGC_2ND_BIT_LEN            (2U)
#define D_LNA_STAGE2_AGC_2ND_BIT_MASK           0x180000U

#define RX_CH2_MIXER_ID                         (UWB_BASE_ADDR + 0x1048)

#define D_LNA_STAGE1_VCT_2ND_BIT_OFFSET         (0U)
#define D_LNA_STAGE1_VCT_2ND_BIT_LEN            (7U)
#define D_LNA_STAGE1_VCT_2ND_BIT_MASK           0x7fU

#define RX_CH2_TIA_I_ID                         (UWB_BASE_ADDR + 0x104c)

#define D_I_TIA_KEEP_DCOC_2ND_BIT_OFFSET        (7U)
#define D_I_TIA_KEEP_DCOC_2ND_BIT_LEN           (1U)
#define D_I_TIA_KEEP_DCOC_2ND_BIT_MASK          0x80U

#define D_I_TIA_DCOC_EN_2ND_BIT_OFFSET          (8U)
#define D_I_TIA_DCOC_EN_2ND_BIT_LEN             (1U)
#define D_I_TIA_DCOC_EN_2ND_BIT_MASK            0x100U

#define D_I_TIA_DCOC_2UA_EN_2ND_BIT_OFFSET      (9U)
#define D_I_TIA_DCOC_2UA_EN_2ND_BIT_LEN         (1U)
#define D_I_TIA_DCOC_2UA_EN_2ND_BIT_MASK        0x200U

#define D_I_TIA_STG1_ITRIM_2ND_BIT_OFFSET       (10U)
#define D_I_TIA_STG1_ITRIM_2ND_BIT_LEN          (3U)
#define D_I_TIA_STG1_ITRIM_2ND_BIT_MASK         0x1c00U

#define D_I_TIA_STAG1_ISEL_2ND_BIT_OFFSET       (13U)
#define D_I_TIA_STAG1_ISEL_2ND_BIT_LEN          (2U)
#define D_I_TIA_STAG1_ISEL_2ND_BIT_MASK         0x6000U

#define D_I_TIA_FB_RES_2ND_BIT_OFFSET           (15U)
#define D_I_TIA_FB_RES_2ND_BIT_LEN              (6U)
#define D_I_TIA_FB_RES_2ND_BIT_MASK             0x1f8000U

#define RX_CH2_TIA_Q_ID                         (UWB_BASE_ADDR + 0x1050)

#define D_Q_TIA_KEEP_DCOC_2ND_BIT_OFFSET        (0U)
#define D_Q_TIA_KEEP_DCOC_2ND_BIT_LEN           (1U)
#define D_Q_TIA_KEEP_DCOC_2ND_BIT_MASK          0x1U

#define D_Q_TIA_DCOC_EN_2ND_BIT_OFFSET          (1U)
#define D_Q_TIA_DCOC_EN_2ND_BIT_LEN             (1U)
#define D_Q_TIA_DCOC_EN_2ND_BIT_MASK            0x2U

#define D_Q_TIA_DCOC_2UA_EN_2ND_BIT_OFFSET      (2U)
#define D_Q_TIA_DCOC_2UA_EN_2ND_BIT_LEN         (1U)
#define D_Q_TIA_DCOC_2UA_EN_2ND_BIT_MASK        0x4U

#define D_Q_TIA_STG1_ITRIM_2ND_BIT_OFFSET       (3U)
#define D_Q_TIA_STG1_ITRIM_2ND_BIT_LEN          (3U)
#define D_Q_TIA_STG1_ITRIM_2ND_BIT_MASK         0x38U

#define D_Q_TIA_STAG1_ISEL_2ND_BIT_OFFSET       (6U)
#define D_Q_TIA_STAG1_ISEL_2ND_BIT_LEN          (2U)
#define D_Q_TIA_STAG1_ISEL_2ND_BIT_MASK         0xc0U

#define D_Q_TIA_FB_RES_2ND_BIT_OFFSET           (8U)
#define D_Q_TIA_FB_RES_2ND_BIT_LEN              (6U)
#define D_Q_TIA_FB_RES_2ND_BIT_MASK             0x3f00U

#define RX_CH2_ATEST_ID                         (UWB_BASE_ADDR + 0x1058)

#define D_I_TIA_ATEST_EN_2ND_BIT_OFFSET         (8U)
#define D_I_TIA_ATEST_EN_2ND_BIT_LEN            (1U)
#define D_I_TIA_ATEST_EN_2ND_BIT_MASK           0x100U

#define D_Q_TIA_ATEST_EN_2ND_BIT_OFFSET         (12U)
#define D_Q_TIA_ATEST_EN_2ND_BIT_LEN            (1U)
#define D_Q_TIA_ATEST_EN_2ND_BIT_MASK           0x1000U

#define D_I_ADC_CMP_DLY_CTRL_2ND_BIT_OFFSET     (28U)
#define D_I_ADC_CMP_DLY_CTRL_2ND_BIT_LEN        (4U)
#define D_I_ADC_CMP_DLY_CTRL_2ND_BIT_MASK       0xf0000000U

#define RX_REDU_ID                              (UWB_BASE_ADDR + 0x105C)

#define D_AOA_MODE_SEL_BIT_OFFSET               (29U)
#define D_AOA_MODE_SEL_BIT_LEN                  (1U)
#define D_AOA_MODE_SEL_BIT_MASK                 0x20000000U

#define ADDR_0X1120                             (UWB_BASE_ADDR + 0x1120)

#define Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_OFFSET  (21U)
#define Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_LEN     (1U)
#define Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_MASK    0x200000U

#define I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_OFFSET  (20U)
#define I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_LEN     (1U)
#define I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_MASK    0x100000U

#define Q_AGC_SEL_TIA_WORD_OVRD_BIT_OFFSET      (19U)
#define Q_AGC_SEL_TIA_WORD_OVRD_BIT_LEN         (1U)
#define Q_AGC_SEL_TIA_WORD_OVRD_BIT_MASK        0x80000U

#define I_AGC_SEL_TIA_WORD_OVRD_BIT_OFFSET      (18U)
#define I_AGC_SEL_TIA_WORD_OVRD_BIT_LEN         (1U)
#define I_AGC_SEL_TIA_WORD_OVRD_BIT_MASK        0x40000U

#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET    (17U)
#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN       (1U)
#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK      0x20000U

#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET    (16U)
#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN       (1U)
#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK      0x10000U

#define Q_TIA_CALIB_LOAD_OVRD_BIT_OFFSET        (15U)
#define Q_TIA_CALIB_LOAD_OVRD_BIT_LEN           (1U)
#define Q_TIA_CALIB_LOAD_OVRD_BIT_MASK          0x8000U

#define I_TIA_CALIB_LOAD_OVRD_BIT_OFFSET        (14U)
#define I_TIA_CALIB_LOAD_OVRD_BIT_LEN           (1U)
#define I_TIA_CALIB_LOAD_OVRD_BIT_MASK          0x4000U

#define TIA_CAL_WAIT_TIME_BIT_OFFSET            (4U)
#define TIA_CAL_WAIT_TIME_BIT_LEN               (8U)
#define TIA_CAL_WAIT_TIME_BIT_MASK              0xff0U

#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (3U)
#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_MASK        0x8U

#define I_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (2U)
#define I_TIA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define I_TIA_AUTO_CALIB_EN_2ND_BIT_MASK        0x4U

#define Q_TIA_AUTO_CALIB_EN_BIT_OFFSET          (1U)
#define Q_TIA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define Q_TIA_AUTO_CALIB_EN_BIT_MASK            0x2U

#define I_TIA_AUTO_CALIB_EN_BIT_OFFSET          (0U)
#define I_TIA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define I_TIA_AUTO_CALIB_EN_BIT_MASK            0x1U

#define ADDR_0X1140                             (UWB_BASE_ADDR + 0x1140)

#define I_VGA_AUTO_CALIB_EN_BIT_OFFSET          (0U)
#define I_VGA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define I_VGA_AUTO_CALIB_EN_BIT_MASK            0x1U

#define I_VGA_DCOC_COMP_EN_BIT_OFFSET           (1U)
#define I_VGA_DCOC_COMP_EN_BIT_LEN              (1U)
#define I_VGA_DCOC_COMP_EN_BIT_MASK             0x2U

#define I_VGA_DCOC_KEEP_EN_BIT_OFFSET           (2U)
#define I_VGA_DCOC_KEEP_EN_BIT_LEN              (1U)
#define I_VGA_DCOC_KEEP_EN_BIT_MASK             0x4U

#define I_VGA_VCM_COMP_EN_BIT_OFFSET            (3U)
#define I_VGA_VCM_COMP_EN_BIT_LEN               (1U)
#define I_VGA_VCM_COMP_EN_BIT_MASK              0x8U

#define Q_VGA_AUTO_CALIB_EN_BIT_OFFSET          (4U)
#define Q_VGA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define Q_VGA_AUTO_CALIB_EN_BIT_MASK            0x10U

#define Q_VGA_DCOC_COMP_EN_BIT_OFFSET           (5U)
#define Q_VGA_DCOC_COMP_EN_BIT_LEN              (1U)
#define Q_VGA_DCOC_COMP_EN_BIT_MASK             0x20U

#define Q_VGA_DCOC_KEEP_EN_BIT_OFFSET           (6U)
#define Q_VGA_DCOC_KEEP_EN_BIT_LEN              (1U)
#define Q_VGA_DCOC_KEEP_EN_BIT_MASK             0x40U

#define Q_VGA_VCM_COMP_EN_BIT_OFFSET            (7U)
#define Q_VGA_VCM_COMP_EN_BIT_LEN               (1U)
#define Q_VGA_VCM_COMP_EN_BIT_MASK              0x80U

#define I_VGA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (8U)
#define I_VGA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define I_VGA_AUTO_CALIB_EN_2ND_BIT_MASK        0x100U

#define I_VGA_DCOC_COMP_EN_2ND_BIT_OFFSET       (9U)
#define I_VGA_DCOC_COMP_EN_2ND_BIT_LEN          (1U)
#define I_VGA_DCOC_COMP_EN_2ND_BIT_MASK         0x200U

#define I_VGA_DCOC_KEEP_EN_2ND_BIT_OFFSET       (10U)
#define I_VGA_DCOC_KEEP_EN_2ND_BIT_LEN          (1U)
#define I_VGA_DCOC_KEEP_EN_2ND_BIT_MASK         0x400U

#define I_VGA_VCM_COMP_EN_2ND_BIT_OFFSET        (11U)
#define I_VGA_VCM_COMP_EN_2ND_BIT_LEN           (1U)
#define I_VGA_VCM_COMP_EN_2ND_BIT_MASK          0x800U

#define Q_VGA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (12U)
#define Q_VGA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define Q_VGA_AUTO_CALIB_EN_2ND_BIT_MASK        0x1000U

#define Q_VGA_DCOC_COMP_EN_2ND_BIT_OFFSET       (13U)
#define Q_VGA_DCOC_COMP_EN_2ND_BIT_LEN          (1U)
#define Q_VGA_DCOC_COMP_EN_2ND_BIT_MASK         0x2000U

#define Q_VGA_DCOC_KEEP_EN_2ND_BIT_OFFSET       (14U)
#define Q_VGA_DCOC_KEEP_EN_2ND_BIT_LEN          (1U)
#define Q_VGA_DCOC_KEEP_EN_2ND_BIT_MASK         0x4000U

#define Q_VGA_VCM_COMP_EN_2ND_BIT_OFFSET        (15U)
#define Q_VGA_VCM_COMP_EN_2ND_BIT_LEN           (1U)
#define Q_VGA_VCM_COMP_EN_2ND_BIT_MASK          0x8000U

#define I_VGA_AUX_BIT_OFFSET                    (16U)
#define I_VGA_AUX_BIT_LEN                       (3U)
#define I_VGA_AUX_BIT_MASK                      0x70000U

#define I_VGA_AUX_OVRD_BIT_OFFSET               (19U)
#define I_VGA_AUX_OVRD_BIT_LEN                  (1U)
#define I_VGA_AUX_OVRD_BIT_MASK                 0x80000U

#define Q_VGA_AUX_BIT_OFFSET                    (20U)
#define Q_VGA_AUX_BIT_LEN                       (3U)
#define Q_VGA_AUX_BIT_MASK                      0x700000U

#define Q_VGA_AUX_OVRD_BIT_OFFSET               (23U)
#define Q_VGA_AUX_OVRD_BIT_LEN                  (1U)
#define Q_VGA_AUX_OVRD_BIT_MASK                 0x800000U

#define I_VGA_AUX_2ND_BIT_OFFSET                (24U)
#define I_VGA_AUX_2ND_BIT_LEN                   (3U)
#define I_VGA_AUX_2ND_BIT_MASK                  0x7000000U

#define I_VGA_AUX_OVRD_2ND_BIT_OFFSET           (27U)
#define I_VGA_AUX_OVRD_2ND_BIT_LEN              (1U)
#define I_VGA_AUX_OVRD_2ND_BIT_MASK             0x8000000U

#define Q_VGA_AUX_2ND_BIT_OFFSET                (28U)
#define Q_VGA_AUX_2ND_BIT_LEN                   (3U)
#define Q_VGA_AUX_2ND_BIT_MASK                  0x70000000U

#define Q_VGA_AUX_OVRD_2ND_BIT_OFFSET           (31U)
#define Q_VGA_AUX_OVRD_2ND_BIT_LEN              (1U)
#define Q_VGA_AUX_OVRD_2ND_BIT_MASK             0x80000000U


#define ADDR_0X1144                             (UWB_BASE_ADDR + 0x1144)

#define VGA_CAL_WAIT_TIME_BIT_OFFSET            (20U)
#define VGA_CAL_WAIT_TIME_BIT_LEN               (8U)
#define VGA_CAL_WAIT_TIME_BIT_MASK              0xff00000U

#define AGC_GAIN_OVRD_2ND_BIT_OFFSET            (19U)
#define AGC_GAIN_OVRD_2ND_BIT_LEN               (1U)
#define AGC_GAIN_OVRD_2ND_BIT_MASK              0x80000U

#define AGC_GAIN_OVRD_BIT_OFFSET                (18U)
#define AGC_GAIN_OVRD_BIT_LEN                   (1U)
#define AGC_GAIN_OVRD_BIT_MASK                  0x40000U

#define VGA_GAIN_SEL_OVRD_2ND_BIT_OFFSET        (17U)
#define VGA_GAIN_SEL_OVRD_2ND_BIT_LEN           (1U)
#define VGA_GAIN_SEL_OVRD_2ND_BIT_MASK          0x20000U

#define VGA_GAIN_SEL_OVRD_BIT_OFFSET            (16U)
#define VGA_GAIN_SEL_OVRD_BIT_LEN               (1U)
#define VGA_GAIN_SEL_OVRD_BIT_MASK              0x10000U


#define VGA_GAIN_SEL_2ND_BIT_OFFSET             (8U)
#define VGA_GAIN_SEL_2ND_BIT_LEN                (6U)
#define VGA_GAIN_SEL_2ND_BIT_MASK               0x3f00U

#define VGA_GAIN_SEL_BIT_OFFSET                 (0U)
#define VGA_GAIN_SEL_BIT_LEN                    (6U)
#define VGA_GAIN_SEL_BIT_MASK                   0x3fU

#define ADDR_0X114c                             (UWB_BASE_ADDR + 0x114c)

#define I_VGA_WORD_OVRD_BIT_OFFSET              (0U)
#define I_VGA_WORD_OVRD_BIT_LEN                 (1U)
#define I_VGA_WORD_OVRD_BIT_MASK                0x1U

#define Q_VGA_WORD_OVRD_BIT_OFFSET              (1U)
#define Q_VGA_WORD_OVRD_BIT_LEN                 (1U)
#define Q_VGA_WORD_OVRD_BIT_MASK                0x2U

#define I_VGA_WORD_OVRD_2ND_BIT_OFFSET          (2U)
#define I_VGA_WORD_OVRD_2ND_BIT_LEN             (1U)
#define I_VGA_WORD_OVRD_2ND_BIT_MASK            0x4U

#define Q_VGA_WORD_OVRD_2ND_BIT_OFFSET          (3U)
#define Q_VGA_WORD_OVRD_2ND_BIT_LEN             (1U)
#define Q_VGA_WORD_OVRD_2ND_BIT_MASK            0x8U

#define I_AGC_WORD_OVRD_BIT_OFFSET              (4U)
#define I_AGC_WORD_OVRD_BIT_LEN                 (1U)
#define I_AGC_WORD_OVRD_BIT_MASK                0x10U

#define Q_AGC_WORD_OVRD_BIT_OFFSET              (5U)
#define Q_AGC_WORD_OVRD_BIT_LEN                 (1U)
#define Q_AGC_WORD_OVRD_BIT_MASK                0x20U

#define I_AGC_WORD_OVRD_2ND_BIT_OFFSET          (6U)
#define I_AGC_WORD_OVRD_2ND_BIT_LEN             (1U)
#define I_AGC_WORD_OVRD_2ND_BIT_MASK            0x40U

#define Q_AGC_WORD_OVRD_2ND_BIT_OFFSET          (7U)
#define Q_AGC_WORD_OVRD_2ND_BIT_LEN             (1U)
#define Q_AGC_WORD_OVRD_2ND_BIT_MASK            0x80U


void zn_set_reg_field_wr(uint32_t addr , uint32_t field_mask, uint32_t field_len, uint32_t field_offset, uint32_t field_value)
{
    uint32_t reg_value = 0;
    reg_value = zn_read_32bit_reg(addr);
    reg_value &= (~field_mask);
    reg_value |= ((field_value<<field_offset) & field_mask);
    zn_write_32bit_reg(addr, reg_value);
}

uint32_t tia_gain_table[]={0x0FC2,0x0FC4,0x0FC8,0x0FD0,0x0FE0,0x0F20,0x1A20,0x7A20,0x1860,0x1820,0x5820,0x7820,0x7420};
///gain_db               ={55,    50,    44,    39,    33,    27,    20,    16,    12,    8 ,    1 ,    -4,    -9};
void zn_set_tia_lna_gain(uint32_t gain_table)
{
    uint32_t lna_stage1;
    uint32_t lna_stage2;
    uint32_t tia_gain;
    lna_stage1 = (gain_table>>8) & 0x7f;
    lna_stage2 = (gain_table>>6) & 0x3;
    tia_gain   = (gain_table & 0x3f);
    uint32_t reg_value = 0;
    //lna stage1
    //reg_value = zn_read_32bit_reg(RX_CH1_MIXER_ID);
    //reg_value &= (~D_LNA_STAGE1_VCT_BIT_MASK);
    //reg_value |= ((lna_stage1<<D_LNA_STAGE1_VCT_BIT_OFFSET) & D_LNA_STAGE1_VCT_BIT_MASK);
    //zn_write_32bit_reg(RX_CH1_MIXER_ID, reg_value);
    zn_set_reg_field_wr(RX_CH1_MIXER_ID,
                        D_LNA_STAGE1_VCT_BIT_MASK,
                        D_LNA_STAGE1_VCT_BIT_LEN,
                        D_LNA_STAGE1_VCT_BIT_OFFSET,
                        lna_stage1);

    //lna stage2
    //reg_value = 0;
    //reg_value = zn_read_32bit_reg(RX_CH1_LNA_ID);
    //reg_value &= (~D_LNA_STAGE2_AGC_BIT_MASK);
    //reg_value |= ((lna_stage2<<D_LNA_STAGE2_AGC_BIT_OFFSET) & D_LNA_STAGE2_AGC_BIT_MASK);
    //zn_write_32bit_reg(RX_CH1_LNA_ID, reg_value);
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE2_AGC_BIT_MASK,
                        D_LNA_STAGE2_AGC_BIT_LEN,
                        D_LNA_STAGE2_AGC_BIT_OFFSET,
                        lna_stage2);
    //tia i
    //reg_value = 0;
    //reg_value = zn_read_32bit_reg(RX_CH1_TIA_I_ID);
    //reg_value &= (~D_I_TIA_FB_RES_BIT_MASK);
    //reg_value |= ((tia_gain<<D_I_TIA_FB_RES_BIT_OFFSET) & D_I_TIA_FB_RES_BIT_MASK);
    //zn_write_32bit_reg(RX_CH1_TIA_I_ID, reg_value);
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_FB_RES_BIT_MASK,
                        D_I_TIA_FB_RES_BIT_LEN,
                        D_I_TIA_FB_RES_BIT_OFFSET,
                        tia_gain);
    //tia q
    //reg_value = 0;
    //reg_value = zn_read_32bit_reg(RX_CH1_TIA_Q_ID);
    //reg_value &= (~D_Q_TIA_FB_RES_BIT_MASK);
    //reg_value |= ((tia_gain<<D_Q_TIA_FB_RES_BIT_OFFSET) & D_Q_TIA_FB_RES_BIT_MASK);
    //zn_write_32bit_reg(RX_CH1_TIA_Q_ID, reg_value);
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_FB_RES_BIT_MASK,
                        D_Q_TIA_FB_RES_BIT_LEN,
                        D_Q_TIA_FB_RES_BIT_OFFSET,
                        tia_gain);
}
void zn_set_tia_lna_gain_2nd(uint32_t gain_table)
{
    uint32_t lna_stage1;
    uint32_t lna_stage2;
    uint32_t tia_gain;
    lna_stage1 = (gain_table>>8) & 0x7f;
    lna_stage2 = (gain_table>>6) & 0x3;
    tia_gain   = (gain_table & 0x3f);
    uint32_t reg_value = 0;

    zn_set_reg_field_wr(RX_CH2_MIXER_ID,
                        D_LNA_STAGE1_VCT_2ND_BIT_MASK,
                        D_LNA_STAGE1_VCT_2ND_BIT_LEN,
                        D_LNA_STAGE1_VCT_2ND_BIT_OFFSET,
                        lna_stage1);


    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE2_AGC_2ND_BIT_MASK,
                        D_LNA_STAGE2_AGC_2ND_BIT_LEN,
                        D_LNA_STAGE2_AGC_2ND_BIT_OFFSET,
                        lna_stage2);

    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_FB_RES_2ND_BIT_MASK,
                        D_I_TIA_FB_RES_2ND_BIT_LEN,
                        D_I_TIA_FB_RES_2ND_BIT_OFFSET,
                        tia_gain);

    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_FB_RES_2ND_BIT_MASK,
                        D_Q_TIA_FB_RES_2ND_BIT_LEN,
                        D_Q_TIA_FB_RES_2ND_BIT_OFFSET,
                        tia_gain);
}

void zn_set_lna_stage1_agc_vord(uint32_t lna_stage1_agc_vord)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE1_AGC_OVRD_BIT_MASK,
                        D_LNA_STAGE1_AGC_OVRD_BIT_LEN,
                        D_LNA_STAGE1_AGC_OVRD_BIT_OFFSET,
                        lna_stage1_agc_vord);
}

void zn_set_lna_stage2_agc_vord(uint32_t lna_stage2_agc_vord)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE2_AGC_OVRD_BIT_MASK,
                        D_LNA_STAGE2_AGC_OVRD_BIT_LEN,
                        D_LNA_STAGE2_AGC_OVRD_BIT_OFFSET,
                        lna_stage2_agc_vord);
}

void zn_set_tia_agc_vord(uint32_t tia_agc_vord)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_TIA_AGC_OVRD_BIT_MASK,
                        D_TIA_AGC_OVRD_BIT_LEN,
                        D_TIA_AGC_OVRD_BIT_OFFSET,
                        tia_agc_vord);
}

void zn_set_lna_stage2_vcm_sel(uint32_t lna_stage2_vcm_sel)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE2_VCM_SEL_BIT_MASK,
                        D_LNA_STAGE2_VCM_SEL_BIT_LEN,
                        D_LNA_STAGE2_VCM_SEL_BIT_OFFSET,
                        lna_stage2_vcm_sel);
}

void zn_set_lna_stage2_bias_sel(uint32_t lna_stage2_bias_sel)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE2_BIAS_SEL_BIT_MASK,
                        D_LNA_STAGE2_BIAS_SEL_BIT_LEN,
                        D_LNA_STAGE2_BIAS_SEL_BIT_OFFSET,
                        lna_stage2_bias_sel);
}

void zn_set_lna_stage1_bias_sel(uint32_t lna_stage1_bias_sel)
{
    zn_set_reg_field_wr(RX_CH1_LNA_ID,
                        D_LNA_STAGE1_BIAS_SEL_BIT_MASK,
                        D_LNA_STAGE1_BIAS_SEL_BIT_LEN,
                        D_LNA_STAGE1_BIAS_SEL_BIT_OFFSET,
                        lna_stage1_bias_sel);
}

void zn_set_i_tia_stag1_isel(uint32_t i_tia_stag1_isel)
{
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_STAG1_ISEL_BIT_MASK,
                        D_I_TIA_STAG1_ISEL_BIT_LEN,
                        D_I_TIA_STAG1_ISEL_BIT_OFFSET,
                        i_tia_stag1_isel);
}

void zn_set_i_tia_stg1_itrim(uint32_t i_tia_stg1_itrim)
{
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_STG1_ITRIM_BIT_MASK,
                        D_I_TIA_STG1_ITRIM_BIT_LEN,
                        D_I_TIA_STG1_ITRIM_BIT_OFFSET,
                        i_tia_stg1_itrim);
}

void zn_set_i_tia_dcoc_2ua_en(uint32_t i_tia_dcoc_2ua_en)
{
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_DCOC_2UA_EN_BIT_MASK,
                        D_I_TIA_DCOC_2UA_EN_BIT_LEN,
                        D_I_TIA_DCOC_2UA_EN_BIT_OFFSET,
                        i_tia_dcoc_2ua_en);
}

void zn_set_q_tia_stag1_isel(uint32_t q_tia_stag1_isel)
{
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_STAG1_ISEL_BIT_MASK,
                        D_Q_TIA_STAG1_ISEL_BIT_LEN,
                        D_Q_TIA_STAG1_ISEL_BIT_OFFSET,
                        q_tia_stag1_isel);
}

void zn_set_q_tia_stg1_itrim(uint32_t q_tia_stg1_itrim)
{
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_STG1_ITRIM_BIT_MASK,
                        D_Q_TIA_STG1_ITRIM_BIT_LEN,
                        D_Q_TIA_STG1_ITRIM_BIT_OFFSET,
                        q_tia_stg1_itrim);
}

void zn_set_q_tia_dcoc_2ua_en(uint32_t q_tia_dcoc_2ua_en)
{
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_DCOC_2UA_EN_BIT_MASK,
                        D_Q_TIA_DCOC_2UA_EN_BIT_LEN,
                        D_Q_TIA_DCOC_2UA_EN_BIT_OFFSET,
                        q_tia_dcoc_2ua_en);
}


void zn_set_q_tia_dcoc_en(uint32_t q_tia_dcoc_en)
{
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_DCOC_EN_BIT_MASK,
                        D_Q_TIA_DCOC_EN_BIT_LEN,
                        D_Q_TIA_DCOC_EN_BIT_OFFSET,
                        q_tia_dcoc_en);
}

void zn_set_q_tia_keep_dcoc(uint32_t q_tia_keep_dcoc)
{
    zn_set_reg_field_wr(RX_CH1_TIA_Q_ID,
                        D_Q_TIA_KEEP_DCOC_BIT_MASK,
                        D_Q_TIA_KEEP_DCOC_BIT_LEN,
                        D_Q_TIA_KEEP_DCOC_BIT_OFFSET,
                        q_tia_keep_dcoc);
}

void zn_set_i_tia_atest_en(uint32_t i_tia_atest_en)
{
    zn_set_reg_field_wr(RX_CH1_ATEST_ID,
                        D_I_TIA_ATEST_EN_BIT_MASK,
                        D_I_TIA_ATEST_EN_BIT_LEN,
                        D_I_TIA_ATEST_EN_BIT_OFFSET,
                        i_tia_atest_en);
}

void zn_set_q_tia_atest_en(uint32_t q_tia_atest_en)
{
    zn_set_reg_field_wr(RX_CH1_ATEST_ID,
                        D_Q_TIA_ATEST_EN_BIT_MASK,
                        D_Q_TIA_ATEST_EN_BIT_LEN,
                        D_Q_TIA_ATEST_EN_BIT_OFFSET,
                        q_tia_atest_en);
}

void zn_set_rx_clk38p4_div_en(uint32_t rx_clk38p4_div_en)
{
    zn_set_reg_field_wr(RX_CH1_ATEST_ID,
                        D_RX_CLK38P4_DIV_EN_BIT_MASK,
                        D_RX_CLK38P4_DIV_EN_BIT_LEN,
                        D_RX_CLK38P4_DIV_EN_BIT_OFFSET,
                        rx_clk38p4_div_en);
}

void zn_set_i_tia_dcoc_en(uint32_t i_tia_dcoc_en)
{
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_DCOC_EN_BIT_MASK,
                        D_I_TIA_DCOC_EN_BIT_LEN,
                        D_I_TIA_DCOC_EN_BIT_OFFSET,
                        i_tia_dcoc_en);
}

void zn_set_i_tia_keep_dcoc(uint32_t i_tia_keep_dcoc)
{
    zn_set_reg_field_wr(RX_CH1_TIA_I_ID,
                        D_I_TIA_KEEP_DCOC_BIT_MASK,
                        D_I_TIA_KEEP_DCOC_BIT_LEN,
                        D_I_TIA_KEEP_DCOC_BIT_OFFSET,
                        i_tia_keep_dcoc);
}

void zn_set_aoa_mode_sel(uint32_t aoa_mode_sel)
{
    zn_set_reg_field_wr(RX_REDU_ID,
                        D_AOA_MODE_SEL_BIT_MASK,
                        D_AOA_MODE_SEL_BIT_LEN,
                        D_AOA_MODE_SEL_BIT_OFFSET,
                        aoa_mode_sel);
}

void zn_set_i_adc_cmp_dly_ctrl_2nd(uint32_t i_adc_cmp_dly_ctrl_2nd)
{
    zn_set_reg_field_wr(RX_CH2_ATEST_ID,
                        D_I_ADC_CMP_DLY_CTRL_2ND_BIT_MASK,
                        D_I_ADC_CMP_DLY_CTRL_2ND_BIT_LEN,
                        D_I_ADC_CMP_DLY_CTRL_2ND_BIT_OFFSET,
                        i_adc_cmp_dly_ctrl_2nd);
}
void zn_set_i_tia_atest_en_2nd(uint32_t i_tia_atest_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_ATEST_ID,
                        D_I_TIA_ATEST_EN_2ND_BIT_MASK,
                        D_I_TIA_ATEST_EN_2ND_BIT_LEN,
                        D_I_TIA_ATEST_EN_2ND_BIT_OFFSET,
                        i_tia_atest_en_2nd);
}
void zn_set_q_tia_atest_en_2nd(uint32_t q_tia_atest_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_ATEST_ID,
                        D_Q_TIA_ATEST_EN_2ND_BIT_MASK,
                        D_Q_TIA_ATEST_EN_2ND_BIT_LEN,
                        D_Q_TIA_ATEST_EN_2ND_BIT_OFFSET,
                        q_tia_atest_en_2nd);
}


void zn_set_vga_cal_wait_time(uint32_t vga_cal_wait_time)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        VGA_CAL_WAIT_TIME_BIT_MASK,
                        VGA_CAL_WAIT_TIME_BIT_LEN,
                        VGA_CAL_WAIT_TIME_BIT_OFFSET,
                        vga_cal_wait_time);
}
void zn_set_agc_gain_ovrd_2nd(uint32_t agc_gain_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        AGC_GAIN_OVRD_2ND_BIT_MASK,
                        AGC_GAIN_OVRD_2ND_BIT_LEN,
                        AGC_GAIN_OVRD_2ND_BIT_OFFSET,
                        agc_gain_ovrd_2nd);
}
void zn_set_agc_gain_ovrd(uint32_t agc_gain_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        AGC_GAIN_OVRD_BIT_MASK,
                        AGC_GAIN_OVRD_BIT_LEN,
                        AGC_GAIN_OVRD_BIT_OFFSET,
                        agc_gain_ovrd);
}
void zn_set_vga_gain_sel_ovrd_2nd(uint32_t vga_gain_sel_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        VGA_GAIN_SEL_OVRD_2ND_BIT_MASK,
                        VGA_GAIN_SEL_OVRD_2ND_BIT_LEN,
                        VGA_GAIN_SEL_OVRD_2ND_BIT_OFFSET,
                        vga_gain_sel_ovrd_2nd);
}
void zn_set_vga_gain_sel_ovrd(uint32_t vga_gain_sel_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        VGA_GAIN_SEL_OVRD_BIT_MASK,
                        VGA_GAIN_SEL_OVRD_BIT_LEN,
                        VGA_GAIN_SEL_OVRD_BIT_OFFSET,
                        vga_gain_sel_ovrd);
}
void zn_set_vga_gain_sel_2nd(uint32_t vga_gain_sel_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        VGA_GAIN_SEL_2ND_BIT_MASK,
                        VGA_GAIN_SEL_2ND_BIT_LEN,
                        VGA_GAIN_SEL_2ND_BIT_OFFSET,
                        vga_gain_sel_2nd);
}
void zn_set_vga_gain_sel(uint32_t vga_gain_sel)
{
    zn_set_reg_field_wr(ADDR_0X1144,
                        VGA_GAIN_SEL_BIT_MASK,
                        VGA_GAIN_SEL_BIT_LEN,
                        VGA_GAIN_SEL_BIT_OFFSET,
                        vga_gain_sel);
}

void zn_set_i_tia_auto_calib_en(uint32_t i_tia_auto_calib_en)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_TIA_AUTO_CALIB_EN_BIT_MASK,
                        I_TIA_AUTO_CALIB_EN_BIT_LEN,
                        I_TIA_AUTO_CALIB_EN_BIT_OFFSET,
                        i_tia_auto_calib_en);
}
void zn_set_q_tia_auto_calib_en(uint32_t q_tia_auto_calib_en)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_TIA_AUTO_CALIB_EN_BIT_MASK,
                        Q_TIA_AUTO_CALIB_EN_BIT_LEN,
                        Q_TIA_AUTO_CALIB_EN_BIT_OFFSET,
                        q_tia_auto_calib_en);
}
void zn_set_i_tia_auto_calib_en_2nd(uint32_t i_tia_auto_calib_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_TIA_AUTO_CALIB_EN_2ND_BIT_MASK,
                        I_TIA_AUTO_CALIB_EN_2ND_BIT_LEN,
                        I_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET,
                        i_tia_auto_calib_en_2nd);
}
void zn_set_q_tia_auto_calib_en_2nd(uint32_t q_tia_auto_calib_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_TIA_AUTO_CALIB_EN_2ND_BIT_MASK,
                        Q_TIA_AUTO_CALIB_EN_2ND_BIT_LEN,
                        Q_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET,
                        q_tia_auto_calib_en_2nd);
}
void zn_set_tia_cal_wait_time(uint32_t tia_cal_wait_time)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        TIA_CAL_WAIT_TIME_BIT_MASK,
                        TIA_CAL_WAIT_TIME_BIT_LEN,
                        TIA_CAL_WAIT_TIME_BIT_OFFSET,
                        tia_cal_wait_time);
}


void zn_set_i_tia_calib_load_ovrd(uint32_t i_tia_calib_load_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_TIA_CALIB_LOAD_OVRD_BIT_MASK,
                        I_TIA_CALIB_LOAD_OVRD_BIT_LEN,
                        I_TIA_CALIB_LOAD_OVRD_BIT_OFFSET,
                        i_tia_calib_load_ovrd);
}
void zn_set_q_tia_calib_load_ovrd(uint32_t q_tia_calib_load_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_TIA_CALIB_LOAD_OVRD_BIT_MASK,
                        Q_TIA_CALIB_LOAD_OVRD_BIT_LEN,
                        Q_TIA_CALIB_LOAD_OVRD_BIT_OFFSET,
                        q_tia_calib_load_ovrd);
}
void zn_set_i_tia_calib_load_ovrd_2nd(uint32_t i_tia_calib_load_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK,
                        I_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN,
                        I_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET,
                        i_tia_calib_load_ovrd_2nd);
}
void zn_set_q_tia_calib_load_ovrd_2nd(uint32_t q_tia_calib_load_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK,
                        Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN,
                        Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET,
                        q_tia_calib_load_ovrd_2nd);
}
void zn_set_i_agc_sel_tia_word_ovrd(uint32_t i_agc_sel_tia_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_AGC_SEL_TIA_WORD_OVRD_BIT_MASK,
                        I_AGC_SEL_TIA_WORD_OVRD_BIT_LEN,
                        I_AGC_SEL_TIA_WORD_OVRD_BIT_OFFSET,
                        i_agc_sel_tia_word_ovrd);
}
void zn_set_q_agc_sel_tia_word_ovrd(uint32_t q_agc_sel_tia_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_AGC_SEL_TIA_WORD_OVRD_BIT_MASK,
                        Q_AGC_SEL_TIA_WORD_OVRD_BIT_LEN,
                        Q_AGC_SEL_TIA_WORD_OVRD_BIT_OFFSET,
                        q_agc_sel_tia_word_ovrd);
}
void zn_set_i_agc_sel_tia_word_ovrd_2nd(uint32_t i_agc_sel_tia_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_MASK,
                        I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_LEN,
                        I_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_OFFSET,
                        i_agc_sel_tia_word_ovrd_2nd);
}
void zn_set_q_agc_sel_tia_word_ovrd_2nd(uint32_t q_agc_sel_tia_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1120,
                        Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_MASK,
                        Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_LEN,
                        Q_AGC_SEL_TIA_WORD_OVRD_2ND_BIT_OFFSET,
                        q_agc_sel_tia_word_ovrd_2nd);
}

void zn_set_i_vga_word_ovrd(uint32_t i_vga_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        I_VGA_WORD_OVRD_BIT_MASK,
                        I_VGA_WORD_OVRD_BIT_LEN,
                        I_VGA_WORD_OVRD_BIT_OFFSET,
                        i_vga_word_ovrd);
}
void zn_set_q_vga_word_ovrd(uint32_t q_vga_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        Q_VGA_WORD_OVRD_BIT_MASK,
                        Q_VGA_WORD_OVRD_BIT_LEN,
                        Q_VGA_WORD_OVRD_BIT_OFFSET,
                        q_vga_word_ovrd);
}
void zn_set_i_vga_word_ovrd_2nd(uint32_t i_vga_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        I_VGA_WORD_OVRD_2ND_BIT_MASK,
                        I_VGA_WORD_OVRD_2ND_BIT_LEN,
                        I_VGA_WORD_OVRD_2ND_BIT_OFFSET,
                        i_vga_word_ovrd_2nd);
}
void zn_set_q_vga_word_ovrd_2nd(uint32_t q_vga_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        Q_VGA_WORD_OVRD_2ND_BIT_MASK,
                        Q_VGA_WORD_OVRD_2ND_BIT_LEN,
                        Q_VGA_WORD_OVRD_2ND_BIT_OFFSET,
                        q_vga_word_ovrd_2nd);
}
void zn_set_i_agc_word_ovrd(uint32_t i_agc_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        I_AGC_WORD_OVRD_BIT_MASK,
                        I_AGC_WORD_OVRD_BIT_LEN,
                        I_AGC_WORD_OVRD_BIT_OFFSET,
                        i_agc_word_ovrd);
}
void zn_set_q_agc_word_ovrd(uint32_t q_agc_word_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        Q_AGC_WORD_OVRD_BIT_MASK,
                        Q_AGC_WORD_OVRD_BIT_LEN,
                        Q_AGC_WORD_OVRD_BIT_OFFSET,
                        q_agc_word_ovrd);
}
void zn_set_i_agc_word_ovrd_2nd(uint32_t i_agc_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        I_AGC_WORD_OVRD_2ND_BIT_MASK,
                        I_AGC_WORD_OVRD_2ND_BIT_LEN,
                        I_AGC_WORD_OVRD_2ND_BIT_OFFSET,
                        i_agc_word_ovrd_2nd);
}
void zn_set_q_agc_word_ovrd_2nd(uint32_t q_agc_word_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X114c,
                        Q_AGC_WORD_OVRD_2ND_BIT_MASK,
                        Q_AGC_WORD_OVRD_2ND_BIT_LEN,
                        Q_AGC_WORD_OVRD_2ND_BIT_OFFSET,
                        q_agc_word_ovrd_2nd);
}

void zn_set_i_vga_auto_calib_en(uint32_t i_vga_auto_calib_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUTO_CALIB_EN_BIT_MASK,
                        I_VGA_AUTO_CALIB_EN_BIT_LEN,
                        I_VGA_AUTO_CALIB_EN_BIT_OFFSET,
                        i_vga_auto_calib_en);
}
void zn_set_i_vga_dcoc_comp_en(uint32_t i_vga_dcoc_comp_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_DCOC_COMP_EN_BIT_MASK,
                        I_VGA_DCOC_COMP_EN_BIT_LEN,
                        I_VGA_DCOC_COMP_EN_BIT_OFFSET,
                        i_vga_dcoc_comp_en);
}
void zn_set_i_vga_dcoc_keep_en(uint32_t i_vga_dcoc_keep_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_DCOC_KEEP_EN_BIT_MASK,
                        I_VGA_DCOC_KEEP_EN_BIT_LEN,
                        I_VGA_DCOC_KEEP_EN_BIT_OFFSET,
                        i_vga_dcoc_keep_en);
}
void zn_set_i_vga_vcm_comp_en(uint32_t i_vga_vcm_comp_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_VCM_COMP_EN_BIT_MASK,
                        I_VGA_VCM_COMP_EN_BIT_LEN,
                        I_VGA_VCM_COMP_EN_BIT_OFFSET,
                        i_vga_vcm_comp_en);
}
void zn_set_q_vga_auto_calib_en(uint32_t q_vga_auto_calib_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUTO_CALIB_EN_BIT_MASK,
                        Q_VGA_AUTO_CALIB_EN_BIT_LEN,
                        Q_VGA_AUTO_CALIB_EN_BIT_OFFSET,
                        q_vga_auto_calib_en);
}
void zn_set_q_vga_dcoc_comp_en(uint32_t q_vga_dcoc_comp_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_DCOC_COMP_EN_BIT_MASK,
                        Q_VGA_DCOC_COMP_EN_BIT_LEN,
                        Q_VGA_DCOC_COMP_EN_BIT_OFFSET,
                        q_vga_dcoc_comp_en);
}
void zn_set_q_vga_dcoc_keep_en(uint32_t q_vga_dcoc_keep_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_DCOC_KEEP_EN_BIT_MASK,
                        Q_VGA_DCOC_KEEP_EN_BIT_LEN,
                        Q_VGA_DCOC_KEEP_EN_BIT_OFFSET,
                        q_vga_dcoc_keep_en);
}
void zn_set_q_vga_vcm_comp_en(uint32_t q_vga_vcm_comp_en)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_VCM_COMP_EN_BIT_MASK,
                        Q_VGA_VCM_COMP_EN_BIT_LEN,
                        Q_VGA_VCM_COMP_EN_BIT_OFFSET,
                        q_vga_vcm_comp_en);
}
void zn_set_i_vga_auto_calib_en_2nd(uint32_t i_vga_auto_calib_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUTO_CALIB_EN_2ND_BIT_MASK,
                        I_VGA_AUTO_CALIB_EN_2ND_BIT_LEN,
                        I_VGA_AUTO_CALIB_EN_2ND_BIT_OFFSET,
                        i_vga_auto_calib_en_2nd);
}
void zn_set_i_vga_dcoc_comp_en_2nd(uint32_t i_vga_dcoc_comp_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_DCOC_COMP_EN_2ND_BIT_MASK,
                        I_VGA_DCOC_COMP_EN_2ND_BIT_LEN,
                        I_VGA_DCOC_COMP_EN_2ND_BIT_OFFSET,
                        i_vga_dcoc_comp_en_2nd);
}
void zn_set_i_vga_dcoc_keep_en_2nd(uint32_t i_vga_dcoc_keep_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_DCOC_KEEP_EN_2ND_BIT_MASK,
                        I_VGA_DCOC_KEEP_EN_2ND_BIT_LEN,
                        I_VGA_DCOC_KEEP_EN_2ND_BIT_OFFSET,
                        i_vga_dcoc_keep_en_2nd);
}
void zn_set_i_vga_vcm_comp_en_2nd(uint32_t i_vga_vcm_comp_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_VCM_COMP_EN_2ND_BIT_MASK,
                        I_VGA_VCM_COMP_EN_2ND_BIT_LEN,
                        I_VGA_VCM_COMP_EN_2ND_BIT_OFFSET,
                        i_vga_vcm_comp_en_2nd);
}
void zn_set_q_vga_auto_calib_en_2nd(uint32_t q_vga_auto_calib_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUTO_CALIB_EN_2ND_BIT_MASK,
                        Q_VGA_AUTO_CALIB_EN_2ND_BIT_LEN,
                        Q_VGA_AUTO_CALIB_EN_2ND_BIT_OFFSET,
                        q_vga_auto_calib_en_2nd);
}
void zn_set_q_vga_dcoc_comp_en_2nd(uint32_t q_vga_dcoc_comp_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_DCOC_COMP_EN_2ND_BIT_MASK,
                        Q_VGA_DCOC_COMP_EN_2ND_BIT_LEN,
                        Q_VGA_DCOC_COMP_EN_2ND_BIT_OFFSET,
                        q_vga_dcoc_comp_en_2nd);
}
void zn_set_q_vga_dcoc_keep_en_2nd(uint32_t q_vga_dcoc_keep_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_DCOC_KEEP_EN_2ND_BIT_MASK,
                        Q_VGA_DCOC_KEEP_EN_2ND_BIT_LEN,
                        Q_VGA_DCOC_KEEP_EN_2ND_BIT_OFFSET,
                        q_vga_dcoc_keep_en_2nd);
}
void zn_set_q_vga_vcm_comp_en_2nd(uint32_t q_vga_vcm_comp_en_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_VCM_COMP_EN_2ND_BIT_MASK,
                        Q_VGA_VCM_COMP_EN_2ND_BIT_LEN,
                        Q_VGA_VCM_COMP_EN_2ND_BIT_OFFSET,
                        q_vga_vcm_comp_en_2nd);
}
void zn_set_i_vga_aux(uint32_t i_vga_aux)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUX_BIT_MASK,
                        I_VGA_AUX_BIT_LEN,
                        I_VGA_AUX_BIT_OFFSET,
                        i_vga_aux);
}
void zn_set_q_vga_aux(uint32_t q_vga_aux)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUX_BIT_MASK,
                        Q_VGA_AUX_BIT_LEN,
                        Q_VGA_AUX_BIT_OFFSET,
                        q_vga_aux);
}
void zn_set_i_vga_aux_2nd(uint32_t i_vga_aux_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUX_2ND_BIT_MASK,
                        I_VGA_AUX_2ND_BIT_LEN,
                        I_VGA_AUX_2ND_BIT_OFFSET,
                        i_vga_aux_2nd);
}
void zn_set_q_vga_aux_2nd(uint32_t q_vga_aux_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUX_2ND_BIT_MASK,
                        Q_VGA_AUX_2ND_BIT_LEN,
                        Q_VGA_AUX_2ND_BIT_OFFSET,
                        q_vga_aux_2nd);
}
void zn_set_i_vga_aux_ovrd(uint32_t i_vga_aux_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUX_OVRD_BIT_MASK,
                        I_VGA_AUX_OVRD_BIT_LEN,
                        I_VGA_AUX_OVRD_BIT_OFFSET,
                        i_vga_aux_ovrd);
}
void zn_set_q_vga_aux_ovrd(uint32_t q_vga_aux_ovrd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUX_OVRD_BIT_MASK,
                        Q_VGA_AUX_OVRD_BIT_LEN,
                        Q_VGA_AUX_OVRD_BIT_OFFSET,
                        q_vga_aux_ovrd);
}
void zn_set_i_vga_aux_ovrd_2nd(uint32_t i_vga_aux_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        I_VGA_AUX_OVRD_2ND_BIT_MASK,
                        I_VGA_AUX_OVRD_2ND_BIT_LEN,
                        I_VGA_AUX_OVRD_2ND_BIT_OFFSET,
                        i_vga_aux_ovrd_2nd);
}
void zn_set_q_vga_aux_ovrd_2nd(uint32_t q_vga_aux_ovrd_2nd)
{
    zn_set_reg_field_wr(ADDR_0X1140,
                        Q_VGA_AUX_OVRD_2ND_BIT_MASK,
                        Q_VGA_AUX_OVRD_2ND_BIT_LEN,
                        Q_VGA_AUX_OVRD_2ND_BIT_OFFSET,
                        q_vga_aux_ovrd_2nd);
}

void zn_set_i_tia_keep_dcoc_2nd(uint32_t i_tia_keep_dcoc_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_KEEP_DCOC_2ND_BIT_MASK,
                        D_I_TIA_KEEP_DCOC_2ND_BIT_LEN,
                        D_I_TIA_KEEP_DCOC_2ND_BIT_OFFSET,
                        i_tia_keep_dcoc_2nd);
}
void zn_set_i_tia_dcoc_en_2nd(uint32_t i_tia_dcoc_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_DCOC_EN_2ND_BIT_MASK,
                        D_I_TIA_DCOC_EN_2ND_BIT_LEN,
                        D_I_TIA_DCOC_EN_2ND_BIT_OFFSET,
                        i_tia_dcoc_en_2nd);
}
void zn_set_i_tia_dcoc_2ua_en_2nd(uint32_t i_tia_dcoc_2ua_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_DCOC_2UA_EN_2ND_BIT_MASK,
                        D_I_TIA_DCOC_2UA_EN_2ND_BIT_LEN,
                        D_I_TIA_DCOC_2UA_EN_2ND_BIT_OFFSET,
                        i_tia_dcoc_2ua_en_2nd);
}
void zn_set_i_tia_stg1_itrim_2nd(uint32_t i_tia_stg1_itrim_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_STG1_ITRIM_2ND_BIT_MASK,
                        D_I_TIA_STG1_ITRIM_2ND_BIT_LEN,
                        D_I_TIA_STG1_ITRIM_2ND_BIT_OFFSET,
                        i_tia_stg1_itrim_2nd);
}
void zn_set_i_tia_stag1_isel_2nd(uint32_t i_tia_stag1_isel_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_I_ID,
                        D_I_TIA_STAG1_ISEL_2ND_BIT_MASK,
                        D_I_TIA_STAG1_ISEL_2ND_BIT_LEN,
                        D_I_TIA_STAG1_ISEL_2ND_BIT_OFFSET,
                        i_tia_stag1_isel_2nd);
}

void zn_set_q_tia_keep_dcoc_2nd(uint32_t q_tia_keep_dcoc_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_KEEP_DCOC_2ND_BIT_MASK,
                        D_Q_TIA_KEEP_DCOC_2ND_BIT_LEN,
                        D_Q_TIA_KEEP_DCOC_2ND_BIT_OFFSET,
                        q_tia_keep_dcoc_2nd);
}
void zn_set_q_tia_dcoc_en_2nd(uint32_t q_tia_dcoc_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_DCOC_EN_2ND_BIT_MASK,
                        D_Q_TIA_DCOC_EN_2ND_BIT_LEN,
                        D_Q_TIA_DCOC_EN_2ND_BIT_OFFSET,
                        q_tia_dcoc_en_2nd);
}
void zn_set_q_tia_dcoc_2ua_en_2nd(uint32_t q_tia_dcoc_2ua_en_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_DCOC_2UA_EN_2ND_BIT_MASK,
                        D_Q_TIA_DCOC_2UA_EN_2ND_BIT_LEN,
                        D_Q_TIA_DCOC_2UA_EN_2ND_BIT_OFFSET,
                        q_tia_dcoc_2ua_en_2nd);
}
void zn_set_q_tia_stg1_itrim_2nd(uint32_t q_tia_stg1_itrim_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_STG1_ITRIM_2ND_BIT_MASK,
                        D_Q_TIA_STG1_ITRIM_2ND_BIT_LEN,
                        D_Q_TIA_STG1_ITRIM_2ND_BIT_OFFSET,
                        q_tia_stg1_itrim_2nd);
}
void zn_set_q_tia_stag1_isel_2nd(uint32_t q_tia_stag1_isel_2nd)
{
    zn_set_reg_field_wr(RX_CH2_TIA_Q_ID,
                        D_Q_TIA_STAG1_ISEL_2ND_BIT_MASK,
                        D_Q_TIA_STAG1_ISEL_2ND_BIT_LEN,
                        D_Q_TIA_STAG1_ISEL_2ND_BIT_OFFSET,
                        q_tia_stag1_isel_2nd);
}


void zn_set_tia_agc_vord_2nd(uint32_t tia_agc_vord_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_TIA_AGC_VORD_2ND_BIT_MASK,
                        D_TIA_AGC_VORD_2ND_BIT_LEN,
                        D_TIA_AGC_VORD_2ND_BIT_OFFSET,
                        tia_agc_vord_2nd);
}
void zn_set_lna_stage2_agc_vord_2nd(uint32_t lna_stage2_agc_vord_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE2_AGC_VORD_2ND_BIT_MASK,
                        D_LNA_STAGE2_AGC_VORD_2ND_BIT_LEN,
                        D_LNA_STAGE2_AGC_VORD_2ND_BIT_OFFSET,
                        lna_stage2_agc_vord_2nd);
}
void zn_set_lna_stage1_agc_vord_2nd(uint32_t lna_stage1_agc_vord_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE1_AGC_VORD_2ND_BIT_MASK,
                        D_LNA_STAGE1_AGC_VORD_2ND_BIT_LEN,
                        D_LNA_STAGE1_AGC_VORD_2ND_BIT_OFFSET,
                        lna_stage1_agc_vord_2nd);
}
void zn_set_lna_stage1_bias_sel_2nd(uint32_t lna_stage1_bias_sel_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE1_BIAS_SEL_2ND_BIT_MASK,
                        D_LNA_STAGE1_BIAS_SEL_2ND_BIT_LEN,
                        D_LNA_STAGE1_BIAS_SEL_2ND_BIT_OFFSET,
                        lna_stage1_bias_sel_2nd);
}
void zn_set_lna_stage2_bias_sel_2nd(uint32_t lna_stage2_bias_sel_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE2_BIAS_SEL_2ND_BIT_MASK,
                        D_LNA_STAGE2_BIAS_SEL_2ND_BIT_LEN,
                        D_LNA_STAGE2_BIAS_SEL_2ND_BIT_OFFSET,
                        lna_stage2_bias_sel_2nd);
}
void zn_set_lna_stage2_vcm_sel_2nd(uint32_t lna_stage2_vcm_sel_2nd)
{
    zn_set_reg_field_wr(RX_CH2_LNA_ID,
                        D_LNA_STAGE2_VCM_SEL_2ND_BIT_MASK,
                        D_LNA_STAGE2_VCM_SEL_2ND_BIT_LEN,
                        D_LNA_STAGE2_VCM_SEL_2ND_BIT_OFFSET,
                        lna_stage2_vcm_sel_2nd);
}

void zn_calibration_pre(void)
{
    uint32_t reg_value;
    uint32_t reg_value_temp;
    int32_t power_table[15] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37, 0x3F};
    int32_t dc_i     = 0;
    int32_t dc_q     = 0;
    int32_t dc_i_tmp = 0;
    int32_t dc_q_tmp = 0;

    printf("calibration begin\n");

    //cfg RX0
    ////cfg ovrd vga gain min
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x00030000);//gain sel ovrd vga & 2nd


    //cfg RX0
    //cfg ovrd vga gain max
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x0001003f);//gain sel ovrd vga & 2nd
    zn_set_vga_gain_sel(0x3f);
    zn_set_vga_gain_sel_ovrd(1);

    //tia band width cfg RX0
    zn_set_i_tia_stag1_isel(3);     // When to enable and When to disable
    zn_set_i_tia_stg1_itrim(7);
    zn_set_i_tia_dcoc_2ua_en(1);    // When to enable and When to disable
    zn_set_q_tia_stag1_isel(3);
    zn_set_q_tia_stg1_itrim(7);
    zn_set_q_tia_dcoc_2ua_en(1);    // When to enable and When to disable

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x0004ffc0);//lna & tia gain ovrd
    zn_set_lna_stage2_vcm_sel(4);   // LNA
    zn_set_lna_stage2_bias_sel(15);
    zn_set_lna_stage1_bias_sel(7);
    zn_set_lna_stage1_agc_vord(1);
    zn_set_lna_stage2_agc_vord(1);
    zn_set_tia_agc_vord(1);

    //cfg RX1
    //cfg ovrd vga gain max
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x0001003f);//gain sel ovrd vga & 2nd
    zn_set_vga_gain_sel_2nd(0x3f);
    zn_set_vga_gain_sel_ovrd_2nd(1);

    //tia band width cfg RX0
    zn_set_i_tia_stag1_isel_2nd(3);     // When to enable and When to disable
    zn_set_i_tia_stg1_itrim_2nd(7);
    zn_set_i_tia_dcoc_2ua_en_2nd(1);    // When to enable and When to disable
    zn_set_q_tia_stag1_isel_2nd(3);
    zn_set_q_tia_stg1_itrim_2nd(7);
    zn_set_q_tia_dcoc_2ua_en_2nd(1);    // When to enable and When to disable

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x0004ffc0);//lna, tia gain ovrd 2nd
    zn_set_lna_stage2_vcm_sel_2nd(4);   // LNA
    zn_set_lna_stage2_bias_sel_2nd(15);
    zn_set_lna_stage1_bias_sel_2nd(7);
    zn_set_lna_stage1_agc_vord_2nd(1);
    zn_set_lna_stage2_agc_vord_2nd(1);
    zn_set_tia_agc_vord_2nd(1);


    // sw to rx0
    zn_set_aoa_mode_sel(0);         // How to use this function

    //// set tia gain max 55 db max
    //zn_set_tia_lna_gain(tia_gain_table[0]);
    // set tia gain min -9  db max
    zn_set_tia_lna_gain(tia_gain_table[12]);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x0000000f);//lna stage1 gain setup max_db_set
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, reg_value & 0xffe7ffff);
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, reg_value | 0x00180000);//lna stage2 gain setup
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, 0x00017f80);//I_tia gain setup
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, 0x000000bf);//Q_tia gain setup

    //cfg tia cali wait time
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);//calibration time setup & cali en close
    zn_set_tia_cal_wait_time(0x7f);

    //cfg analog top I channel
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, 0x00000102);//I_tia atest_en, cali clk en
    zn_set_i_tia_atest_en(1);                       //  When to enable and when to disable
    zn_set_rx_clk38p4_div_en(1);                    //  When to enable

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1034);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, reg_value | 0x00000380);//I_tia_dcoc_en, keep_dococ_en, I_cali_2u
    zn_set_i_tia_dcoc_en(1);                        //  When to enable
    zn_set_i_tia_keep_dcoc(1);                      //  When to enable
    ////zn_set_i_tia_dcoc_2ua_en(1);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);//cali en close
    //cfg I channel cali_en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f1);// digital cali en
    zn_set_i_tia_auto_calib_en(0);
    zn_set_i_tia_auto_calib_en(1);

    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124) & 0x00000080) != 0x00000080){};
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
    if((reg_value & 0x00000080) == 0x00000080)
    {
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
        //reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1270, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
        reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12A0);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x12A0, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
    }

    //close I channel cali_en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);// digital cali en
    zn_set_i_tia_auto_calib_en(0);
    //close I channel dcoc en at ana if regfile					// need it ?
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X1034);
    ////zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, reg_value &0xfffffc7f);
    //zn_set_i_tia_dcoc_en(0);                    //  When to enable
    //zn_set_i_tia_keep_dcoc(0);                  //  When to enable
    //zn_set_i_tia_dcoc_2ua_en(0);                //  When to enable
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, reg_value &0xfffffeff); while at cali_pre is 0xfffffc7f
    zn_set_i_tia_dcoc_en(0);                    //  When to enable

    //open Q channel analog top cfg
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1040, 0x00001002);//Q_tia atest_en, cali clk en
    zn_set_i_tia_atest_en(0);                   //  When to enable
    zn_set_q_tia_atest_en(1);                   //  When to enable and When to disable
    ////zn_set_rx_clk38p4_div_en(1);

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1038);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, reg_value|0x06000001);//Q_tia_dcoc_en, keep_dococ_en;q_2u
    zn_set_q_tia_dcoc_en(1);
    zn_set_q_tia_keep_dcoc(1);
    ////zn_set_q_tia_dcoc_2ua_en(1);

    //cfg Q channel cali off
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f0);
    //cfg Q channel cali en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f2);
    zn_set_q_tia_auto_calib_en(0);
    zn_set_q_tia_auto_calib_en(1);

    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124) & 0x00800000) != 0x00800000){};
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
    if((reg_value & 0x00800000) == 0x00800000)
    {
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
        //reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1270, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
        reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12A0);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x12A0, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
    }

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f0);
    zn_set_q_tia_auto_calib_en(0);
    ////close Q channel dcoc en at ana if regfile
    ////reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X1038);
    ////zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, reg_value &0xf9fffffe);
    //zn_set_q_tia_dcoc_en(0);                    //  When to enable
    //zn_set_q_tia_keep_dcoc(0);                  //  When to enable
    //zn_set_q_tia_dcoc_2ua_en(0);                //  When to enable

    //close Q channel dcoc en at ana if regfile
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X1038);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, reg_value &0xfbffffff); while at cali_pre is 0xf9fffffe
    zn_set_q_tia_dcoc_en(0);

    //cfg RX1
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx1
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value | 0x20000000);
    zn_set_aoa_mode_sel(1);

    // set tia gain min -9  db max
    zn_set_tia_lna_gain_2nd(tia_gain_table[12]);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x0000000f);
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1044);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, reg_value & 0xffe7ffff);
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1044);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, reg_value | 0x00180000);//lna stage2 gain setup
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0x00017f80); //d_i_tia_stag1_isel d_i_tia_stg1_itrim fb_res
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1050, 0x000002f8);	//d_q_tia_stag1_isel d_q_tia_stg1_itrim fb_res

    //cfg tia cali wait time
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);//calibration time setup
    ////zn_set_tia_cal_wait_time(0x7f);

    //cfg analog top I channel
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1040);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, reg_value | 0x00000002);//cali clk en
    ////zn_set_rx_clk38p4_div_en(1);

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x00000100);  //I_tia atest_en, cali clk en should en?
    zn_set_i_tia_atest_en_2nd(1);

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x104c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, reg_value | 0x00000380);//I_tia_dcoc_en, keep_dococ_en, I_cali_2u
    zn_set_i_tia_dcoc_en_2nd(1);                        //  When to enable
    zn_set_i_tia_keep_dcoc_2nd(1);                      //  When to enable
    ////zn_set_i_tia_dcoc_2ua_en_2nd(1);

    //cfg I channel cali_en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f4);
    zn_set_i_tia_auto_calib_en_2nd(0);
    zn_set_i_tia_auto_calib_en_2nd(1);
    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128) & 0x00000080) != 0x00000080){};

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
    if((reg_value & 0x00000080) == 0x00000080)
    {
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        //reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x12b0, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
        reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12e0);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x12e0, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
    }
    //close I channel cali_en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);
    zn_set_i_tia_auto_calib_en_2nd(0);
    //close I channel dcoc en at ana if regfile
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X104c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, reg_value &0xfffffc7f);
    zn_set_i_tia_dcoc_en_2nd(0);

    //open Q channel analog top cfg
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1040);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, reg_value | 0x00000002);//cali clk en
    ////zn_set_rx_clk38p4_div_en(1);

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1058, 0x00001000);//Q_tia atest_en,
    zn_set_i_tia_atest_en_2nd(0);                   //  When to enable
    zn_set_q_tia_atest_en_2nd(1);

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1050);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1050, reg_value|0x00000007);//Q_tia_dcoc_en, keep_dococ_en;q_2u
    zn_set_q_tia_dcoc_en_2nd(1);
    zn_set_q_tia_keep_dcoc_2nd(1);
    ////zn_set_q_tia_dcoc_2ua_en_2nd(1);

    //cfg Q channel cali en
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f8);
    zn_set_q_tia_auto_calib_en_2nd(0);
    zn_set_q_tia_auto_calib_en_2nd(1);

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128) & 0x00800000) != 0x00800000);

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
    if((reg_value & 0x00800000) == 0x00800000)
    {
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        //reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x12b0, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
        reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12e0);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x12e0, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
    }

    //close tia agc ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c,reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord(0);
    zn_set_lna_stage2_agc_vord(0);
    zn_set_tia_agc_vord(0);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1044);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord_2nd(0);
    zn_set_lna_stage2_agc_vord_2nd(0);
    zn_set_tia_agc_vord_2nd(0);

    //en agc sel tia word ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x003c0100);
    zn_set_i_agc_sel_tia_word_ovrd(1);
    zn_set_q_agc_sel_tia_word_ovrd(1);
    zn_set_i_agc_sel_tia_word_ovrd_2nd(1);
    zn_set_q_agc_sel_tia_word_ovrd_2nd(1);
    //close vga ovrd max
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1144);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, reg_value & 0xfffcffff);//gain sel ovrd vga
    zn_set_vga_gain_sel_ovrd(0);
    zn_set_vga_gain_sel_ovrd_2nd(0);
    //VGA
    //ovrd tia max gain & cali_value
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x0000000f);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x001cffc0); //ovrd ana en & stage2
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, 0x00017f80);//I_tia gain setup
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, 0x060000bf);//Q_tia gain setup


    ////zn_set_lna_stage2_vcm_sel(4);   // LNA
    ////zn_set_lna_stage2_bias_sel(15);
    ////zn_set_lna_stage1_bias_sel(7);
    zn_set_lna_stage1_agc_vord(1);
    zn_set_lna_stage2_agc_vord(1);
    zn_set_tia_agc_vord(1);
    //tia band width cfg
    ////zn_set_i_tia_stag1_isel(3);
    ////zn_set_i_tia_stg1_itrim(7);
    //zn_set_i_tia_dcoc_2ua_en(1);
    ////zn_set_q_tia_stag1_isel(3);
    ////zn_set_q_tia_stg1_itrim(7);
    //zn_set_q_tia_dcoc_2ua_en(1);
    //zn_set_i_tia_dcoc_en(0);
    //zn_set_i_tia_keep_dcoc(1);
    //zn_set_q_tia_dcoc_en(0);
    //zn_set_q_tia_keep_dcoc(1);
    zn_set_tia_lna_gain(tia_gain_table[12]);



//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x0000000f);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x001cffc0); //ovrd ana en & stage2
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0x00017f80);//I_tia gain setup
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1050, 0x000002ff);//Q_tia gain setup
    zn_set_lna_stage1_agc_vord_2nd(1);
    zn_set_lna_stage2_agc_vord_2nd(1);
    zn_set_tia_agc_vord_2nd(1);
    zn_set_tia_lna_gain_2nd(tia_gain_table[12]);

    //ovrd cfg
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12A0);
    reg_value_temp = reg_value>>16;
    reg_value = reg_value<<16;
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12e0);
    reg_value_temp = reg_value>>16;
    reg_value = reg_value<<16;
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
    //en tia_calib_load_ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, reg_value | 0x0003c000);
    zn_set_i_tia_calib_load_ovrd(1);
    zn_set_q_tia_calib_load_ovrd(1);
    zn_set_i_tia_calib_load_ovrd_2nd(1);
    zn_set_q_tia_calib_load_ovrd_2nd(1);

    //close vga_word_ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x00000000);
    zn_set_i_vga_word_ovrd(0);
    zn_set_q_vga_word_ovrd(0);
    zn_set_i_vga_word_ovrd_2nd(0);
    zn_set_q_vga_word_ovrd_2nd(0);
    zn_set_i_agc_word_ovrd(0);
    zn_set_q_agc_word_ovrd(0);
    zn_set_i_agc_word_ovrd_2nd(0);
    zn_set_q_agc_word_ovrd_2nd(0);

    //vga cal wait time
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x07f00000);//0x01000000, calibration time setup
    zn_set_vga_cal_wait_time(0x7f);

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1040, 0x00000002);//cali clk en, where is the bb clk 125M setup?
    ////zn_set_i_tia_atest_en(0);       // When to enable
    zn_set_q_tia_atest_en(0);           // When to enable
    ////zn_set_rx_clk38p4_div_en(1);    // When to enable and When to disable

    dc_i_tmp = 15;
    dc_q_tmp = 15;
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx0
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value & 0xdfffffff);
    zn_set_aoa_mode_sel(0);         // How to use this function

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000000);
    zn_set_q_vga_aux_ovrd_2nd(0);
    zn_set_q_vga_aux_2nd(0);
    zn_set_i_vga_aux_ovrd_2nd(0);
    zn_set_i_vga_aux_2nd(0);
    zn_set_q_vga_aux_ovrd(0);
    zn_set_q_vga_aux(0);
    zn_set_i_vga_aux_ovrd(0);
    zn_set_i_vga_aux(0);
    zn_set_q_vga_vcm_comp_en_2nd(0);
    zn_set_q_vga_dcoc_keep_en_2nd(0);
    zn_set_q_vga_dcoc_comp_en_2nd(0);
    zn_set_q_vga_auto_calib_en_2nd(0);
    zn_set_i_vga_vcm_comp_en_2nd(0);
    zn_set_i_vga_dcoc_keep_en_2nd(0);
    zn_set_i_vga_dcoc_comp_en_2nd(0);
    zn_set_i_vga_auto_calib_en_2nd(0);
    zn_set_q_vga_vcm_comp_en(0);
    zn_set_q_vga_dcoc_keep_en(0);
    zn_set_q_vga_dcoc_comp_en(0);
    zn_set_q_vga_auto_calib_en(0);
    zn_set_i_vga_vcm_comp_en(0);
    zn_set_i_vga_dcoc_keep_en(0);
    zn_set_i_vga_dcoc_comp_en(0);
    zn_set_i_vga_auto_calib_en(0);

    //set vga gain
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x07f10000);//gain sel ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x07f1003f);//gain sel ovrd
    zn_set_vga_gain_sel(0x3f);
    zn_set_vga_gain_sel_ovrd(1);

    for(int32_t j = 0; j < 30 ; j++)
    {
        //iq_keep_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1140, 0x00000000);//keep dcoc, auto cali en,0x00000044 keep dcoc how to use?
        zn_set_i_vga_auto_calib_en(0);
        zn_set_q_vga_auto_calib_en(0);

        //iq vag auto calib en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000011);//keep dcoc, auto cali en,0x00000055
        zn_set_i_vga_auto_calib_en(1);
        zn_set_q_vga_auto_calib_en(1);
        //wait RX finished
        mdelay(10);
        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148) & 0x30) != 0x30);
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148);
        if((reg_value & 0x30) == 0x30)
        {
            dc_i = zn_dc_estimate(0);
            dc_i += zn_dc_estimate(0);
            dc_i = dc_i >> 1;
            dc_q = zn_dc_estimate(1);
            dc_q += zn_dc_estimate(1);
            dc_q = dc_q >> 1;

            if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
            {
                dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1170);
                //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1180, reg_value);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x11b8, reg_value);
            }

            if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
            {
                dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1174);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x11f4,reg_value);
            }

            if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
            {
                break;
            }
        }
    }
    printf("RX0 cali pre ,absolute value dc_i_tmp : %d , value dc_q_tmp : %d ,\n",dc_i_tmp,dc_q_tmp);
    dc_i_tmp = 15;
    dc_q_tmp = 15;
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx1
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value | 0x20000000);
    zn_set_aoa_mode_sel(1);

    //close iq auto cali
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000000);
    ////zn_set_q_vga_aux_ovrd_2nd(0);
    ////zn_set_q_vga_aux_2nd(0);
    ////zn_set_i_vga_aux_ovrd_2nd(0);
    ////zn_set_i_vga_aux_2nd(0);
    ////zn_set_q_vga_aux_ovrd(0);
    ////zn_set_q_vga_aux(0);
    ////zn_set_i_vga_aux_ovrd(0);
    ////zn_set_i_vga_aux(0);
    ////zn_set_q_vga_vcm_comp_en_2nd(0);
    ////zn_set_q_vga_dcoc_keep_en_2nd(0);
    ////zn_set_q_vga_dcoc_comp_en_2nd(0);
    ////zn_set_q_vga_auto_calib_en_2nd(0);
    ////zn_set_i_vga_vcm_comp_en_2nd(0);
    ////zn_set_i_vga_dcoc_keep_en_2nd(0);
    ////zn_set_i_vga_dcoc_comp_en_2nd(0);
    ////zn_set_i_vga_auto_calib_en_2nd(0);
    ////zn_set_q_vga_vcm_comp_en(0);
    ////zn_set_q_vga_dcoc_keep_en(0);
    ////zn_set_q_vga_dcoc_comp_en(0);
    zn_set_q_vga_auto_calib_en(0);
    ////zn_set_i_vga_vcm_comp_en(0);
    ////zn_set_i_vga_dcoc_keep_en(0);
    ////zn_set_i_vga_dcoc_comp_en(0);
    zn_set_i_vga_auto_calib_en(0);
    //set vga gain
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x0ff20000);
    zn_set_vga_gain_sel_2nd(0x3f);
    zn_set_vga_gain_sel_ovrd_2nd(1);
    for(int32_t j = 0; j < 30 ; j++)
    {
        //iq_keep_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1140, 0x00000000);//keep dcoc, auto cali en,0x00004400
        zn_set_i_vga_auto_calib_en_2nd(0);
        zn_set_q_vga_auto_calib_en_2nd(0);
        //iq vag auto calib en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00001100);//keep dcoc, auto cali en,0x00005500
        zn_set_i_vga_auto_calib_en_2nd(1);
        zn_set_q_vga_auto_calib_en_2nd(1);

        //wait RX finished
        mdelay(10);
        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148) & 0xc0) != 0xc0){};

        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148);
        if((reg_value & 0xc0) == 0xc0)
        {
            dc_i = zn_dc_estimate(2);
            dc_i += zn_dc_estimate(2);
            dc_i = dc_i >> 1;
            dc_q = zn_dc_estimate(3);
            dc_q += zn_dc_estimate(3);
            dc_q = dc_q >> 1;

            if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
            {
                dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1178);
                //zn_write_32bit_reg(UWB_BASE_ADDR + 0x11f8, reg_value);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1230, reg_value);
            }

            if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
            {
                dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x117c);
                //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1234,reg_value);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x126c, reg_value);
            }

            if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
            {
                break;
            }
        }
    }

    printf("RX1 cali pre ,absolute value dc_i_tmp : %d , value dc_q_tmp : %d ,\n",dc_i_tmp,dc_q_tmp);
//    //cfg vga_word_ovrd
//    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1180);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11b8);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1150 , reg_value);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11f4);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1154 , reg_value);
//    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x00000003);
//    zn_set_i_vga_word_ovrd(1);
//    zn_set_q_vga_word_ovrd(1);
//
//    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11f8);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1230);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1158 , reg_value);
//    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1234);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x126c);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x115c , reg_value);
//    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x0000000f);
//    zn_set_i_vga_word_ovrd_2nd(1);
//    zn_set_q_vga_word_ovrd_2nd(1);
//
//    //ovrd vga=0 vga_cali_value
//    //cfg ovrd vga gain max
//    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x00033f3f);//gain sel ovrd vga & 2nd
//    zn_set_vga_gain_sel(0x3f);
//    zn_set_vga_gain_sel_ovrd(1);
//    zn_set_vga_gain_sel_2nd(0x3f);
//    zn_set_vga_gain_sel_ovrd_2nd(1);
    //close tia_word_ovrd
    zn_set_i_agc_sel_tia_word_ovrd(0);
    zn_set_q_agc_sel_tia_word_ovrd(0);
    zn_set_i_agc_sel_tia_word_ovrd_2nd(0);
    zn_set_q_agc_sel_tia_word_ovrd_2nd(0);
    zn_set_i_tia_calib_load_ovrd(0);
    zn_set_q_tia_calib_load_ovrd(0);
    zn_set_i_tia_calib_load_ovrd_2nd(0);
    zn_set_q_tia_calib_load_ovrd_2nd(0);
}

void zn_tia_calibration(void)
{
    uint32_t reg_value;
    uint32_t reg_value_temp;
    uint32_t tia_cali_tmp_value[32]={0};
    uint32_t times;
    int32_t dc_i     = 0;
    int32_t dc_q     = 0;
    int32_t dc_i_tmp = 0;
    int32_t dc_q_tmp = 0;

    uint32_t i_p_value;
    uint32_t i_n_value;
    uint32_t q_p_value;
    uint32_t q_n_value;
    uint32_t i_cali_value;
    uint32_t q_cali_value;

    zn_calibration_pre();
    printf("rx0 tia cali begin:\n");

    //cfg vga_word_ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1180);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11b8);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1150 , reg_value);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11f4);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1154 , reg_value);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x00000003);
    zn_set_i_vga_word_ovrd(1);
    zn_set_q_vga_word_ovrd(1);

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x11f8);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1230);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1158 , reg_value);
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1234);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x126c);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x115c , reg_value);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x0000000f);
    zn_set_i_vga_word_ovrd_2nd(1);
    zn_set_q_vga_word_ovrd_2nd(1);

    //cfg ovrd vga gain max
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x00033f3f);//gain sel ovrd vga & 2nd
    zn_set_vga_gain_sel(0x3f);
    zn_set_vga_gain_sel_ovrd(1);
    zn_set_vga_gain_sel_2nd(0x3f);
    zn_set_vga_gain_sel_ovrd_2nd(1);


    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx0
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value & 0xdfffffff);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x0004ffc0);//lna, tia gain ovrd
    // sw to rx0
    zn_set_aoa_mode_sel(0);         // How to use this function
    //tia band width cfg
    ////zn_set_i_tia_stag1_isel(3);     // When to enable and When to disable
    ////zn_set_i_tia_stg1_itrim(7);
    ////zn_set_i_tia_dcoc_2ua_en(1);    // When to enable and When to disable
    ////zn_set_q_tia_stag1_isel(3);
    ////zn_set_q_tia_stg1_itrim(7);
    ////zn_set_q_tia_dcoc_2ua_en(1);    // When to enable and When to disable

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x0004ffc0);//lna & tia gain ovrd
    ////zn_set_lna_stage2_vcm_sel(4);   // LNA
    ////zn_set_lna_stage2_bias_sel(15);
    ////zn_set_lna_stage1_bias_sel(7);
    ////zn_set_lna_stage1_agc_vord(1);
    ////zn_set_lna_stage2_agc_vord(1);
    ////zn_set_tia_agc_vord(1);
    //////zn_set_rx_clk38p4_div_en(1);

    for(int cali_case =0; cali_case<13; cali_case++)//13
    {
        zn_set_tia_lna_gain(tia_gain_table[cali_case]);
        //cfg tia cali wait time
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);//calibration time setup & cali en close
        ////zn_set_tia_cal_wait_time(0x7f);
        //cfg analog top I channel
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, 0x00000102);//I_tia atest_en, cali clk en
        zn_set_i_tia_atest_en(1);                       //  When to enable and when to disable
        ////zn_set_rx_clk38p4_div_en(1);                    //  When to enable

        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1034);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, reg_value | 0x00000380);//I_tia_dcoc_en, keep_dococ_en, I_cali_2u
        zn_set_i_tia_dcoc_en(1);                        //  When to enable
        ////zn_set_i_tia_keep_dcoc(1);                      //  When to enable , always enable?
        ////zn_set_i_tia_dcoc_2ua_en(1);
        //close I channel cali_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);// digital cali en & close tia word ovrd
        //cfg I channel cali_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f1);// digital cali en
        ////zn_set_i_tia_auto_calib_en(0);
        zn_set_i_tia_auto_calib_en(1);

        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124) & 0x00000080) != 0x00000080){};
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);


        if((reg_value & 0x00000080) == 0x00000080)
        {
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
            //printf("####0x1124 value :%x\n",reg_value);
            reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case);
            //printf("####0x%x value :%x\n",0x1270 + 4*cali_case,reg_value_temp);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
            //printf("####I:reg0x%x=0x%x.",0x1270 + 4*cali_case,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case));
        }
        //close I channel cali_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);// digital cali en
        zn_set_i_tia_auto_calib_en(0);
        //close I channel dcoc en at ana if regfile					// need it ?
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X1034);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, reg_value &0xfffffeff); while at cali_pre is 0xfffffc7f
        zn_set_i_tia_dcoc_en(0);                    //  When to enable

        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1034);
        //printf("0x1034 value :%x\n ",reg_value);

        //open Q channel analog top cfg
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1040, 0x00001002);//Q_tia atest_en, cali clk en
        zn_set_i_tia_atest_en(0);                   //  When to enable
        zn_set_q_tia_atest_en(1);                   //  When to enable and When to disable
        ////zn_set_rx_clk38p4_div_en(1);


        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1038);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, reg_value|0x06000001);//Q_tia_dcoc_en, keep_dococ_en;q_2u
        zn_set_q_tia_dcoc_en(1);
        zn_set_q_tia_keep_dcoc(1);
        ////zn_set_q_tia_dcoc_2ua_en(1);

        //cfg Q channel cali off
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f0);
        //cfg Q channel cali en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f2);
        zn_set_q_tia_auto_calib_en(0);
        zn_set_q_tia_auto_calib_en(1);

        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124) & 0x00800000) != 0x00800000){};
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
        if((reg_value & 0x00800000) == 0x00800000)
        {
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1124);
            reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case);
            //printf("####reg0x%x value :%x\n ",0x1270 + 4*cali_case,reg_value_temp);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff)); 
            printf("####:reg0x%x=0x%x.\n",0x1270 + 4*cali_case,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case));
        }
        //cfg Q channel cali off
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f0);
        zn_set_q_tia_auto_calib_en(0);
        //close Q channel dcoc en at ana if regfile
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X1038);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, reg_value &0xfbffffff); while at cali_pre is 0xf9fffffe
        zn_set_q_tia_dcoc_en(0);

        //printf("reg0x%x=0x%x.\n",0x1120,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120));

        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case);
        reg_value_temp = reg_value>>16;
        reg_value = reg_value<<16;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
        //en tia_calib_load_ovrd
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, reg_value | 0x0003c000);
        zn_set_i_tia_calib_load_ovrd(1);
        zn_set_q_tia_calib_load_ovrd(1);

        for(int32_t j = 0; j < 128 ; j++)
        {
                reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x112c);
                i_n_value = (0xff000000 & reg_value)>>24;
                i_p_value = (0x00ff0000 & reg_value)>>16;
                dc_i = zn_dc_estimate(0);
                dc_i += zn_dc_estimate(0);
                dc_i = dc_i >> 1;
                if(dc_i < -1)
                {
                      if(i_p_value == 0x7f || i_n_value == 0x00)
                      {
                          break;
                      }
                      else
                      {
                          i_p_value = i_p_value + 1;
                          i_n_value = i_n_value - 1;
                          reg_value_temp = (i_p_value <<16) + (i_n_value <<24);
                          zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
                      }
                }
                if(dc_i > 1)
                {
                      if(i_p_value == 0x00 || i_n_value == 0x7f)
                      {
                            break;
                      }
                      i_p_value = i_p_value - 1;
                      i_n_value = i_n_value + 1;
                      reg_value_temp = (i_p_value <<16) + (i_n_value <<24);
                      zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
                }

                if((dc_i <= 1) && (dc_i >= -1) )
                {
                      break;
                }
        }
        for(int32_t j = 0; j < 128 ; j++)
        {
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x112c);
            q_n_value = (0x0000ff00 & reg_value)>>8;
            q_p_value = (0x000000ff & reg_value);
            dc_q = zn_dc_estimate(1);
            dc_q += zn_dc_estimate(1);
            dc_q = dc_q >> 1;
            if(dc_q < -1)
            {
                if(q_p_value == 0x7f || q_n_value == 0x00)
                {
                    break;
                }
                else
                {
                    q_p_value = q_p_value + 1;
                    q_n_value = q_n_value - 1;
                    reg_value_temp = (q_p_value) + (i_n_value << 8);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value_temp &0x0000ffff)|(reg_value &0xffff0000));
                }
            }
            if(dc_q > 1)
            {
                  if(q_p_value == 0x00 || q_n_value == 0x7f)
                    {
                        break;
                    }
                    else
                    {
                        q_p_value = q_p_value - 1;
                        q_n_value = q_n_value + 1;
                        reg_value_temp = (q_p_value) + (i_n_value << 8);
                        zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value_temp &0x0000ffff)|(reg_value &0xffff0000));
                    }
            }


            if((dc_q <= 1) && (dc_q >= -1) )
            {
                  break;
            }
        }
        printf("RX0 tia case :%d,absolute value dc_i : %d , value dc_q : %d \n", cali_case , dc_i,dc_q);
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x112c);
        q_cali_value = (reg_value & 0x0000ffff)<<16;
        i_cali_value = (reg_value & 0xffff0000)>>16;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case, (q_cali_value&0xffff0000)|(i_cali_value&0x0000ffff));
        printf("####:reg0x%x=0x%x.\n",0x1270 + 4*cali_case,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270 + 4*cali_case));
        zn_set_i_tia_calib_load_ovrd(0);
        zn_set_q_tia_calib_load_ovrd(0);

    }
//#ifdef _AOA_EN
    //cali tia 2nd
    printf("rx1 tia cali begin:\n");
    for(int cali_case =0; cali_case<13; cali_case++)//13
    {
        zn_set_tia_lna_gain_2nd(tia_gain_table[cali_case]);
        mdelay(10);
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx1
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value | 0x20000000);
        zn_set_aoa_mode_sel(1);

        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x0004ffc0);//lna, tia gain ovrd 2nd
        ////zn_set_lna_stage2_vcm_sel_2nd(4);   // LNA
        ////zn_set_lna_stage2_bias_sel_2nd(15);
        ////zn_set_lna_stage1_bias_sel_2nd(7);
        zn_set_lna_stage1_agc_vord_2nd(1);
        zn_set_lna_stage2_agc_vord_2nd(1);
        zn_set_tia_agc_vord_2nd(1);

        //cfg tia cali wait time
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);//calibration time setup
        ////zn_set_tia_cal_wait_time(0x7f);
        //cfg analog top I channel
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1040);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, reg_value | 0x00000002);//cali clk en
        ////zn_set_rx_clk38p4_div_en(1);

        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x00000100);  //I_tia atest_en, cali clk en should en?
        zn_set_i_tia_atest_en_2nd(1);

        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x104c);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, reg_value | 0x00000380);//I_tia_dcoc_en, keep_dococ_en, I_cali_2u
        zn_set_i_tia_dcoc_en_2nd(1);                        //  When to enable
        zn_set_i_tia_keep_dcoc_2nd(1);                      //  When to enable
        ////zn_set_i_tia_dcoc_2ua_en_2nd(1);

        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        //printf("####before cali_en 0x1128 value :%x\n",reg_value);

        //cfg I channel cali_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f4);
        zn_set_i_tia_auto_calib_en_2nd(0);
        zn_set_i_tia_auto_calib_en_2nd(1);

        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128) & 0x00000080) != 0x00000080);

        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        //printf("####I after 0x1128 value :%x\n",reg_value);
        if((reg_value & 0x00000080) == 0x00000080)
        {
            //printf("rx1 I channel case:%d pass.\n",cali_case);
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
            //printf("####0x1128 to 12b0 value :%x\n",reg_value);
            reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));

        }
        //close I channel cali_en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x000007f0);
        zn_set_i_tia_auto_calib_en_2nd(0);
        //close I channel dcoc en at ana if regfile
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0X104c);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, reg_value &0xfffffc7f);
        zn_set_i_tia_dcoc_en_2nd(0);

        //open Q channel analog top cfg
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1040);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1040, reg_value | 0x00000002);//cali clk en
        ////zn_set_rx_clk38p4_div_en(1);

        //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1058, 0x00001000);//Q_tia atest_en,
        zn_set_i_tia_atest_en_2nd(0);                   //  When to enable should close???
        zn_set_q_tia_atest_en_2nd(1);

        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1050);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1050, reg_value|0x00000007);//Q_tia_dcoc_en, keep_dococ_en;q_2u
        zn_set_q_tia_dcoc_en_2nd(1);
        zn_set_q_tia_keep_dcoc_2nd(1);
        ////zn_set_q_tia_dcoc_2ua_en_2nd(1);

        //cfg Q channel cali en
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120,0x000007f8);
        zn_set_q_tia_auto_calib_en_2nd(0);
        zn_set_q_tia_auto_calib_en_2nd(1);
        // wait cal done
        //mdelay(10);
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128) & 0x00800000) != 0x00800000);

        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
        //printf("####Q after 0x1128 value :%x\n",reg_value);
        if((reg_value & 0x00800000) == 0x00800000)
        {
            //printf("rx1 Q channel case:%d pass.\n",cali_case);
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1128);
            reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case);
            zn_write_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
            printf("Q:reg0x%x=0x%x.\n",0x12b0 + 4*cali_case,zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case));
        }

        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case);
        reg_value_temp = reg_value>>16;
        reg_value = reg_value<<16;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
        //en tia_calib_load_ovrd
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120);
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, reg_value | 0x0003c000);
        zn_set_i_tia_calib_load_ovrd_2nd(1);
        zn_set_q_tia_calib_load_ovrd_2nd(1);

        for(int32_t j = 0; j < 128 ; j++)
        {
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1130);
            i_n_value = (0xff000000 & reg_value)>>24;
            i_p_value = (0x00ff0000 & reg_value)>>16;
            dc_i = zn_dc_estimate(2);
            dc_i += zn_dc_estimate(2);
            dc_i = dc_i >> 1;
            if(dc_i < -1)
            {
                if(i_p_value == 0x7f || i_n_value == 0x00)
                {
                    break;
                }
                else
                {
                    i_p_value = i_p_value + 1;
                    i_n_value = i_n_value - 1;
                    reg_value_temp = (i_p_value <<16) + (i_n_value <<24);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
                }
            }
            if(dc_i > 1)
            {
                if(i_p_value == 0x00 || i_n_value == 0x7f)
                {
                    break;
                }
                i_p_value = i_p_value - 1;
                i_n_value = i_n_value + 1;
                reg_value_temp = (i_p_value <<16) + (i_n_value <<24);
                zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value&0x0000ffff)|(reg_value_temp&0xffff0000));
            }

            if((dc_i <= 1) && (dc_i >= -1) )
            {
                  break;
            }
        }
        for(int32_t j = 0; j < 128 ; j++)
        {
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1130);
            q_n_value = (0x0000ff00 & reg_value)>>8;
            q_p_value = (0x000000ff & reg_value);
            dc_q = zn_dc_estimate(3);
            dc_q += zn_dc_estimate(3);
            dc_q = dc_q >> 1;
            if(dc_q < -1)
            {
                if(q_p_value == 0x7f || q_n_value == 0x00)
                {
                    break;
                }
                else
                {
                    q_p_value = q_p_value + 1;
                    q_n_value = q_n_value - 1;
                    reg_value_temp = (q_p_value) + (i_n_value << 8);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value_temp &0x0000ffff)|(reg_value &0xffff0000));
                }
            }
            if(dc_q > 1)
            {
                if(q_p_value == 0x00 || q_n_value == 0x7f)
                {
                    break;
                }
                else
                {
                    q_p_value = q_p_value - 1;
                    q_n_value = q_n_value + 1;
                    reg_value_temp = (q_p_value) + (i_n_value << 8);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value_temp &0x0000ffff)|(reg_value &0xffff0000));
                }
            }

            if((dc_q <= 1) && (dc_q >= -1) )
            {
                  break;
            }
        }
        printf("RX1 tia case :%d,absolute value dc_i : %d , value dc_q : %d \n", cali_case , dc_i,dc_q);
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1130);
        q_cali_value = (reg_value & 0x0000ffff)<<16;
        i_cali_value = (reg_value & 0xffff0000)>>16;
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case, (q_cali_value&0xffff0000)|(i_cali_value&0x0000ffff));
        printf("####:reg0x%x=0x%x.\n",0x12b0 + 4*cali_case,zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0 + 4*cali_case));
        zn_set_i_tia_calib_load_ovrd_2nd(0);
        zn_set_q_tia_calib_load_ovrd_2nd(0);
    }
//#endif
    //close tia agc ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c,reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord(0);
    zn_set_lna_stage2_agc_vord(0);
    zn_set_tia_agc_vord(0);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1044);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord_2nd(0);
    zn_set_lna_stage2_agc_vord_2nd(0);
    zn_set_tia_agc_vord_2nd(0);

    //en agc sel tia word ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, 0x003c0100);
    ////zn_set_i_tia_calib_load_ovrd(0);
    ////zn_set_q_tia_calib_load_ovrd(0);
    ////zn_set_i_tia_calib_load_ovrd_2nd(0);
    ////zn_set_q_tia_calib_load_ovrd_2nd(0);
    zn_set_i_agc_sel_tia_word_ovrd(1);
    zn_set_q_agc_sel_tia_word_ovrd(1);
    zn_set_i_agc_sel_tia_word_ovrd_2nd(1);
    zn_set_q_agc_sel_tia_word_ovrd_2nd(1);


    //close vga ovrd min
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1144);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, reg_value & 0xfffcffff);//gain sel ovrd vga
    zn_set_vga_gain_sel_ovrd(0);
    //zn_set_vga_gain_sel_ovrd_2nd(0);
    printf("tia calibration end!\n");
}

void zn_vga_calibration(void)
{
    uint32_t reg_value;
    uint32_t reg_value_temp;
    uint32_t times;
    uint32_t vga_i_cali_value_tmp[32] = {0};
    uint32_t vga_q_cali_value_tmp[32] = {0};
    int32_t power_table[15] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37, 0x3F};
    
    int32_t dc_i     = 0;
    int32_t dc_q     = 0;
    int32_t dc_i_tmp = 0;
    int32_t dc_q_tmp = 0;

    printf("rx0 vga calibration begin\n");


    //ovrd tia max gain & cali_value
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1030, 0x0000000f);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c, 0x001cffc0); //ovrd ana en & stage2
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1034, 0x00017f80);//I_tia gain setup
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1038, 0x060000bf);//Q_tia gain setup

    ////zn_set_lna_stage2_vcm_sel(4);   // LNA
    ////zn_set_lna_stage2_bias_sel(15);
    ////zn_set_lna_stage1_bias_sel(7);
    zn_set_lna_stage1_agc_vord(1);
    zn_set_lna_stage2_agc_vord(1);
    zn_set_tia_agc_vord(1);
    //tia band width cfg
    ////zn_set_i_tia_stag1_isel(3);
    ////zn_set_i_tia_stg1_itrim(7);
    ////zn_set_i_tia_dcoc_2ua_en(1);
    ////zn_set_q_tia_stag1_isel(3);
    ////zn_set_q_tia_stg1_itrim(7);
    ////zn_set_q_tia_dcoc_2ua_en(1);
    //zn_set_i_tia_dcoc_en(0);            // When to enable
    ////zn_set_i_tia_keep_dcoc(1);
    //zn_set_q_tia_dcoc_en(0);
    ////zn_set_q_tia_keep_dcoc(1);
    zn_set_tia_lna_gain(tia_gain_table[12]);

//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1048, 0x0000000f);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, 0x001cffc0); //ovrd ana en & stage2
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x104c, 0x00017f80);//I_tia gain setup
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1050, 0x000002ff);//Q_tia gain setup
    zn_set_lna_stage1_agc_vord_2nd(1);
    zn_set_lna_stage2_agc_vord_2nd(1);
    zn_set_tia_agc_vord_2nd(1);
    zn_set_tia_lna_gain_2nd(tia_gain_table[12]);

    //ovrd cfg
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1270);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12A0);
    reg_value_temp = reg_value>>16;
    reg_value = reg_value<<16;
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x112c, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12b0);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x12e0);
    reg_value_temp = reg_value>>16;
    reg_value = reg_value<<16;
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1130, (reg_value&0xffff0000)|(reg_value_temp&0x0000ffff));
    //en tia_calib_load_ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, reg_value | 0x0003c000);
    //printf("reg0x%x=0x%x.\n",0x1120,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120));
    zn_set_i_tia_calib_load_ovrd(1);
    zn_set_q_tia_calib_load_ovrd(1);
    zn_set_i_tia_calib_load_ovrd_2nd(1);
    zn_set_q_tia_calib_load_ovrd_2nd(1);

    //close vga_word_ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x00000000);
    zn_set_i_vga_word_ovrd(0);
    zn_set_q_vga_word_ovrd(0);
    zn_set_i_vga_word_ovrd_2nd(0);
    zn_set_q_vga_word_ovrd_2nd(0);
    ////zn_set_i_agc_word_ovrd(0);
    ////zn_set_q_agc_word_ovrd(0);
    ////zn_set_i_agc_word_ovrd_2nd(0);
    ////zn_set_q_agc_word_ovrd_2nd(0);

    //vga cal wait time
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x07f00000);//0x01000000, calibration time setup
    ////zn_set_vga_cal_wait_time(0x7f);

    //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1040, 0x00000002);//cali clk en, where is the bb clk 125M setup?
    ////zn_set_i_tia_atest_en(0);       // When to enable
    zn_set_q_tia_atest_en(0);           // When to enable , at last should be enable ?
    ////zn_set_rx_clk38p4_div_en(1);    // When to enable and When to disable


    for(int cali_idx=0; cali_idx<15; cali_idx++)//15
    {
        dc_i_tmp = 15;
        dc_q_tmp = 15;
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx0
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value & 0xdfffffff);
        zn_set_aoa_mode_sel(0);         // How to use this function

        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000000);
        ////zn_set_q_vga_aux_ovrd_2nd(0);
        ////zn_set_q_vga_aux_2nd(0);
        ////zn_set_i_vga_aux_ovrd_2nd(0);
        ////zn_set_i_vga_aux_2nd(0);
        ////zn_set_q_vga_aux_ovrd(0);
        ////zn_set_q_vga_aux(0);
        ////zn_set_i_vga_aux_ovrd(0);
        ////zn_set_i_vga_aux(0);
        ////zn_set_q_vga_vcm_comp_en_2nd(0);
        //zn_set_q_vga_dcoc_keep_en_2nd(0);
        ////zn_set_q_vga_dcoc_comp_en_2nd(0);
        //zn_set_q_vga_auto_calib_en_2nd(0);
        ////zn_set_i_vga_vcm_comp_en_2nd(0);
        //zn_set_i_vga_dcoc_keep_en_2nd(0);
        ////zn_set_i_vga_dcoc_comp_en_2nd(0);
        //zn_set_i_vga_auto_calib_en_2nd(0);
        ////zn_set_q_vga_vcm_comp_en(0);
        zn_set_q_vga_dcoc_keep_en(0);
        ////zn_set_q_vga_dcoc_comp_en(0);
        zn_set_q_vga_auto_calib_en(0);
        ////zn_set_i_vga_vcm_comp_en(0);
        zn_set_i_vga_dcoc_keep_en(0);           // When to enable , at model sim it should be enable?
        ////zn_set_i_vga_dcoc_comp_en(0);
        zn_set_i_vga_auto_calib_en(0);
        //set vga gain
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x07f10000 + power_table[cali_idx]);//gain sel ovrd
        zn_set_vga_gain_sel(power_table[cali_idx]);
        zn_set_vga_gain_sel_ovrd(1);

        for(int32_t j = 0; j < 30 ; j++)
        {
            //iq_keep_en
            //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1140, 0x00000000);//keep dcoc, auto cali en,0x00000044
            zn_set_i_vga_auto_calib_en(0);
            zn_set_q_vga_auto_calib_en(0);
            //iq vag auto calib en
            //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000011);//keep dcoc, auto cali en,0x00000055
            zn_set_i_vga_auto_calib_en(1);
            zn_set_q_vga_auto_calib_en(1);
            //wait RX finished
            mdelay(10);
            while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148) & 0x30) != 0x30);
            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148);
            if((reg_value & 0x30) == 0x30)
            {
                dc_i = zn_dc_estimate(0);
                dc_i += zn_dc_estimate(0);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(1);
                dc_q += zn_dc_estimate(1);
                dc_q = dc_q >> 1;

                if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
                {
                    dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1170);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1180 + 4*cali_idx, reg_value);
                }

                if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
                {
                    dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1174);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x11bc + 4*cali_idx,reg_value);
                }

                if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
                {
                    break;
                }
            }
        }

        printf("vga case:%d pass.\n",cali_idx);
        printf("RX0 vga case:%d ,absolute value dc_i_tmp : %d , value dc_q_tmp : %d ,",cali_idx,dc_i_tmp,dc_q_tmp);
        printf("I:reg0x%x=0x%x.   ",0x1180 + 4*cali_idx,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1180 + 4*cali_idx));
        printf("Q:reg0x%x=0x%x.\n",0x11bc + 4*cali_idx,zn_read_32bit_reg(UWB_BASE_ADDR + 0x11bc + 4*cali_idx));
    }   
    //vga cali rx1 begin
    printf("rx1 vga calibration begin\n");
    for(int cali_idx=0; cali_idx<15; cali_idx++)//15
    {
        dc_i_tmp = 15;
        dc_q_tmp = 15;
        //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c); // sw to rx1
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value | 0x20000000);
        zn_set_aoa_mode_sel(1);         // How to use this function

        //close iq auto cali
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00000000);
        ////zn_set_q_vga_aux_ovrd_2nd(0);
        ////zn_set_q_vga_aux_2nd(0);
        ////zn_set_i_vga_aux_ovrd_2nd(0);
        ////zn_set_i_vga_aux_2nd(0);
        ////zn_set_q_vga_aux_ovrd(0);
        ////zn_set_q_vga_aux(0);
        ////zn_set_i_vga_aux_ovrd(0);
        ////zn_set_i_vga_aux(0);
        ////zn_set_q_vga_vcm_comp_en_2nd(0);
        zn_set_q_vga_dcoc_keep_en_2nd(0);
        ////zn_set_q_vga_dcoc_comp_en_2nd(0);
        zn_set_q_vga_auto_calib_en_2nd(0);
        ////zn_set_i_vga_vcm_comp_en_2nd(0);
        zn_set_i_vga_dcoc_keep_en_2nd(0);
        ////zn_set_i_vga_dcoc_comp_en_2nd(0);
        zn_set_i_vga_auto_calib_en_2nd(0);
        ////zn_set_q_vga_vcm_comp_en(0);
        //zn_set_q_vga_dcoc_keep_en(0);
        ////zn_set_q_vga_dcoc_comp_en(0);
        //zn_set_q_vga_auto_calib_en(0);
        ////zn_set_i_vga_vcm_comp_en(0);
        //zn_set_i_vga_dcoc_keep_en(0);           // When to enable , at model sim it should be enable?
        ////zn_set_i_vga_dcoc_comp_en(0);
        //zn_set_i_vga_auto_calib_en(0);
        //set vga gain
        //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x0ff20000 + (power_table[cali_idx]<<8));
        zn_set_vga_gain_sel_2nd(power_table[cali_idx]<<8);
        zn_set_vga_gain_sel_ovrd_2nd(1);

        for(int32_t j = 0; j < 30 ; j++)
        {
            //iq_keep_en
            //zn_write_32bit_reg(UWB_BASE_ADDR + 0X1140, 0x00000000);//keep dcoc, auto cali en,0x00004400
            zn_set_i_vga_auto_calib_en_2nd(0);
            zn_set_q_vga_auto_calib_en_2nd(0);
            //iq vag auto calib en
            //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1140, 0x00001100);//keep dcoc, auto cali en,0x00005500
            zn_set_i_vga_auto_calib_en_2nd(1);
            zn_set_q_vga_auto_calib_en_2nd(1);

            //wait RX finished
            mdelay(10);
            while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148) & 0xc0) != 0xc0){};

            reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1148);
            if((reg_value & 0xc0) == 0xc0)
            {
                dc_i = zn_dc_estimate(2);
                dc_i += zn_dc_estimate(2);
                dc_i = dc_i >> 1;
                dc_q = zn_dc_estimate(3);
                dc_q += zn_dc_estimate(3);
                dc_q = dc_q >> 1;

                if((dc_i < dc_i_tmp) && (dc_i > - dc_i_tmp))
                {
                    dc_i_tmp = (dc_i >= 0 ? dc_i : - dc_i);
                    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1178);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x11f8 + 4*cali_idx, reg_value);
                }

                if((dc_q < dc_q_tmp) && (dc_q > - dc_q_tmp))
                {
                    dc_q_tmp = (dc_q >= 0 ? dc_q : - dc_q);
                    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x117c);
                    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1234 + 4*cali_idx,reg_value);
                }

                if((dc_i_tmp <= 1) && (dc_i_tmp >= -1) && (dc_q_tmp <= 1) && (dc_q_tmp >= -1))
                {
                    break;
                }
            }
        }

        printf("rx1 vga case:%d pass.\n",cali_idx);
        printf("RX1 vga case:%d ,absolute value dc_i_tmp : %d , value dc_q_tmp : %d ,",cali_idx,dc_i_tmp,dc_q_tmp);
        printf("I:reg0x%x=0x%x. ",0x11f8 + 4*cali_idx,zn_read_32bit_reg(UWB_BASE_ADDR + 0x11f8 + 4*cali_idx));
        printf("Q:reg0x%x=0x%x.\n",0x1234 + 4*cali_idx,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1234 + 4*cali_idx));
    }

    //close tia agc ovrd
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x102c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x102c,reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord(0);
    zn_set_lna_stage2_agc_vord(0);
    zn_set_tia_agc_vord(0);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1044);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1044, reg_value&0xfffffe3f);
    zn_set_lna_stage1_agc_vord_2nd(0);
    zn_set_lna_stage2_agc_vord_2nd(0);
    zn_set_tia_agc_vord_2nd(0);

    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1120, reg_value & 0xfffc3fff);
    //printf("reg0x%x=0x%x.\n",0x1120,zn_read_32bit_reg(UWB_BASE_ADDR + 0x1120));
    zn_set_i_tia_calib_load_ovrd(0);
    zn_set_q_tia_calib_load_ovrd(0);
    zn_set_i_tia_calib_load_ovrd_2nd(0);
    zn_set_q_tia_calib_load_ovrd_2nd(0);

    //cfg agc_gain_ovrd and close vga_gain_sel
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1144, 0x010c0000);
    zn_set_vga_gain_sel_ovrd(0);
    zn_set_vga_gain_sel_ovrd_2nd(0);
    zn_set_agc_gain_ovrd(1);
    zn_set_agc_gain_ovrd_2nd(1);

    //cfg agc_word_ovrd
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x114c, 0x000000f0);
    zn_set_i_agc_word_ovrd(1);
    zn_set_q_agc_word_ovrd(1);
    //zn_set_i_agc_word_ovrd_2nd(1);
    //zn_set_q_agc_word_ovrd_2nd(1);
    ////zn_set_i_vga_word_ovrd(0);
    ////zn_set_q_vga_word_ovrd(0);
    ////zn_set_i_vga_word_ovrd_2nd(0);
    ////zn_set_q_vga_word_ovrd_2nd(0);

    //sw to rx0
    //reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x105c);
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x105c, reg_value & 0xdfffffff);
    zn_set_aoa_mode_sel(0);
    //cfg adc_tune_ch2 to reduce the generation of -16
    //zn_write_32bit_reg(UWB_BASE_ADDR + 0x1058, 0x70000000);
    zn_set_i_adc_cmp_dly_ctrl_2nd(7);

    printf("vga calibration end!\n");
}

void zn_vco_calibration(void)
{
    uint32_t reg_value;
    uint32_t reg_value_temp;
    printf("ana vco calibration begin.\n");
    //cfg cal lock mode 0
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value&0xfffffffb);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value&0xfffffffb);

    // cfg cal en
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value|0x00000001);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value|0x00000001);

    //wait cal done
    mdelay(10);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c) & 0x0000000c) != 0x0000000c);
    
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    if((reg_value&0x4) == 0x4)
    {
        if((reg_value_temp&0x8) == 0x8)
        {
            printf("0:cali mode 1,vco_7b TEST case PASS.\n");
        }
    }
    else
    {
        if((reg_value_temp&0x28) == 0x28)
        {
            printf("0:cali mode 0,vco_7b TEST case PASS.\n");
        }
        else
        {
            printf("0:cali mode 0,vco_7b TEST case FAILED.\n");
            return;//finish_fail;
        }
    }

    
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    if((reg_value&0x4) == 0x4)
    {
        if((reg_value_temp&0x4) == 0x4)
        {
            printf("1:cali mode 1,vco_5b TEST case PASS.\n");
        }
    }
    else
    {
        if((reg_value_temp&0x14) == 0x14)
        {
            printf("1:cali mode 0,vco_5b TEST case PASS.\n");
        }
        else
        {
            printf("1:cali mode 0,vco_5b TEST case FAILED.\n");
            return;//finish_fail;
        }
    }
    
    // close cal en
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value&0xfffffffe);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value&0xfffffffe);

    // cfg cal lock mode 1
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value|0x4);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value|0x4);
    
    mdelay(10);
    // cfg cal en
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value|0x1);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value|0x1);
       
    //wait cal done
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c) & 0x0000000c) != 0x0000000c);
    
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    if((reg_value&0x4) == 0x4)
    {
        if((reg_value_temp&0x8) == 0x8)
        {
            printf("2:cali mode 1,vco_7b TEST case PASS.\n");
        }
    }
    else
    {
        if((reg_value_temp&0x28) == 0x28)
        {
            printf("2:cali mode 0,vco_7b TEST case PASS.\n");
        }
        else
        {
            printf("2:cali mode 0,vco_7b TEST case FAILED.\n");
            return;//finish_fail;
        }
    }

    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
    if((reg_value&0x4) == 0x4)
    {
        if((reg_value_temp&0x4) == 0x4)
        {
            printf("3:cali mode 1,vco_5b TEST case PASS.\n");
        }
    }
    else
    {
        if((reg_value_temp&0x14) == 0x14)
        {
            printf("3:cali mode 0,vco_5b TEST case PASS.\n");
        }
        else
        {
            printf("3:cali mode 0,vco_5b TEST case FAILED.\n");
            return;//finish_fail;
        }
    }


    //cfg cal lock mode 0
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value&0xfffffffb);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value&0xfffffffb);

    // close cal_en at cali_reg
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value&0xfffffffe);
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value&0xfffffffe);
//		//cfg ana_if_reg is debug mode, need one by one to cfg related reg
//    //cfg cal_en at ana_if_reg
//		//cfg enable
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1024);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1024, reg_value|0x100);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x106c);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x106c, reg_value|0x40);
//printf("c\n");
//    //enable sel
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x101c);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x101c, reg_value|0x80);
//printf("d\n");
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1018);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1018, reg_value|0x08000000);
//printf("e\n");


//    // wait cal done
//    mdelay(10);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c) & 0x0000000c) != 0x0000000c);
//    
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
//    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    if((reg_value&0x4) == 0x4)
//    {
//        if((reg_value_temp&0x8) == 0x8)
//        {
//            printf("4:ana_if cfg, cali mode 1,vco_7b TEST case PASS.\n");
//        }
//    }
//    else
//    {
//        if((reg_value_temp&0x28) == 0x28)
//        {
//            printf("4:ana_if cfg, cali mode 0,vco_7b TEST case PASS.\n");
//        }
//        else
//        {
//            printf("4:ana_if cfg, cali mode 0,vco_7b TEST case FAILED.\n");
//            return;//finish_fail;
//        }
//    }

//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
//    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    if((reg_value&0x4) == 0x4)
//    {
//        if((reg_value_temp&0x4) == 0x4)
//        {
//            printf("5:ana_if cfg,cali mode 1,vco_5b TEST case PASS.\n");
//        }
//    }
//    else
//    {
//        if((reg_value_temp&0x14) == 0x14)
//        {
//            printf("5:ana_if cfg, cali mode 0,vco_5b TEST case PASS.\n");
//        }
//        else
//        {
//            printf("5:ana_if cfg, cali mode 0,vco_5b TEST case FAILED.\n");
//            return;//finish_fail;
//        }
//    }
//    
//    // close cal en at ana if
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1024);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1024, reg_value&0xfffffeff);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x106c);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x106c, reg_value&0xffffffbf);

//    //cfg cal lock mode 1
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1104, reg_value&0x4);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1108, reg_value&0x4);
//    
//    // cfg cal en
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1024);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1024, reg_value|0x100);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x106c);
//    zn_write_32bit_reg(UWB_BASE_ADDR + 0x106c, reg_value|0x40);
//    
//    // wait cal done
//    mdelay(10);
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c) & 0x0000000c) != 0x0000000c);
//    
//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1104);
//    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    if((reg_value&0x4) == 0x4)
//    {
//        if((reg_value_temp&0x8) == 0x8)
//        {
//            printf("6:ana_if cfg, cali mode 1,vco_7b TEST case PASS.\n");
//        }
//    }
//    else
//    {
//        if((reg_value_temp&0x28) == 0x28)
//        {
//            printf("6:ana_if cfg, cali mode 0,vco_7b TEST case PASS.\n");
//        }
//        else
//        {
//            printf("6:ana_if cfg, cali mode 0,vco_7b TEST case FAILED.\n");
//            return;//finish_fail;
//        }
//    }

//    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1108);
//    reg_value_temp = zn_read_32bit_reg(UWB_BASE_ADDR + 0x110c);
//    if((reg_value&0x4) == 0x4)
//    {
//        if((reg_value_temp&0x4) == 0x4)
//        {
//            printf("7:ana_if cfg,cali mode 1,vco_5b TEST case PASS.\n");
//        }
//    }
//    else
//    {
//        if((reg_value_temp&0x14) == 0x14)
//        {
//            printf("7:ana_if cfg, cali mode 0,vco_5b TEST case PASS.\n");
//        }
//        else
//        {
//            printf("7:ana_if cfg, cali mode 0,vco_5b TEST case FAILED.\n");
//            return;//finish_fail;
//        }
//    }
        printf("vco calibration end!\n");
}


void zn_32k_calibration(void)
{
    uint32_t reg_value;
    uint32_t reg_value_temp;

    for(int32_t j = 0; j < 32 ; j++)
    {
        //cfg 32k ovrd_en & ovrd word
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110,0x60 + j);
        //close 32k cali_en
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1110);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110, reg_value & 0xffffffdf);
        //cfg 32k cali_en
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1110);
        zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110,reg_value | 0x20);
        mdelay(10);
        while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1114)&0x00200000) != 0x00200000);
    
        reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1114);
        reg_value_temp = reg_value&0xffff;
        printf("%f \n",19200.0/reg_value_temp);
        //printf("cali_ovrd_word: %d,clk_freq %f.\n",j,19200.0/reg_value_temp);
        mdelay(1000);
    }
    //close 32k ovrd
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1110);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110, reg_value & 0xffffffbf);
    //close 32k cali_en
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1110);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110, reg_value & 0xffffffdf);
    //cfg 32k cali_en
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1110);
    zn_write_32bit_reg(UWB_BASE_ADDR + 0x1110,reg_value | 0x20);
    mdelay(10);
    while((zn_read_32bit_reg(UWB_BASE_ADDR + 0x1114)&0x00200000) != 0x00200000);
    
    reg_value = zn_read_32bit_reg(UWB_BASE_ADDR + 0x1114);
    reg_value_temp = reg_value&0xffff;
    printf("%f \n",19200.0/reg_value_temp);
//	  printf("AUTO_CALI clk_freq %f.\n",19200.0/reg_value_temp);
    if((545<=(reg_value&0xffff)) && ((reg_value&0xffff)<=667))
    {
        printf("32K AUTO cali case PASS.\n");
    }
    else
    {
        printf("32K AUTO cali case FAILED.\n");
    }
        mdelay(1000);
        printf("32k calibration end!\n");
}

#endif
