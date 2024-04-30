/*****************************************************************************
* File: zn_regs_zn2014.h
*
* Descirption: this file contains the functions support driver regs.
*
* Copyright (C) : 2023 ultraception.com
*
* Author: ultraception FW team
* Mail:   ultraception@ultraception.com
* Date:   2023 - 5
*****************************************************************************/
#ifndef __ZN_REGS_ZN2014_H
#define __ZN_REGS_ZN2014_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zn_macro.h"
#include "base_addr.h"

//******************************************************************************
//* Description: Bit definitions for register SYS_MASK
//******************************************************************************
#define SYS_MASK_ID                         (UWB_BASE_ADDR + 0x00)             
#define SYS_MASK_LEN                        (4U)                
#define SYS_MASK_MASK                       0xFFFFFFFFUL

#define SYS_MASK_ARFE_ENABLE_BIT_OFFSET     (29U)               
#define SYS_MASK_ARFE_ENABLE_BIT_LEN        (1U)                
#define SYS_MASK_ARFE_ENABLE_BIT_MASK       0x20000000UL

#define SYS_MASK_RXSTSE_ENABLE_BIT_OFFSET   (28U)               
#define SYS_MASK_RXSTSE_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RXSTSE_ENABLE_BIT_MASK     0x10000000UL

#define SYS_MASK_HPDWARN_ENABLE_BIT_OFFSET  (27U)               
#define SYS_MASK_HPDWARN_ENABLE_BIT_LEN     (1U)                
#define SYS_MASK_HPDWARN_ENABLE_BIT_MASK    0x8000000UL

#define SYS_MASK_RXSTO_ENABLE_BIT_OFFSET    (26U)               
#define SYS_MASK_RXSTO_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXSTO_ENABLE_BIT_MASK      0x4000000UL

#define SYS_MASK_PLL_HILO_ENABLE_BIT_OFFSET (25U)               
#define SYS_MASK_PLL_HILO_ENABLE_BIT_LEN    (1U)                
#define SYS_MASK_PLL_HILO_ENABLE_BIT_MASK   0x2000000UL

#define SYS_MASK_RCINIT_ENABLE_BIT_OFFSET   (24U)               
#define SYS_MASK_RCINIT_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RCINIT_ENABLE_BIT_MASK     0x1000000UL

#define SYS_MASK_SPIRDY_ENABLE_BIT_OFFSET   (23U)               
#define SYS_MASK_SPIRDY_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_SPIRDY_ENABLE_BIT_MASK     0x800000UL

#define SYS_MASK_LCSSERR_ENABLE_BIT_OFFSET  (22U)               
#define SYS_MASK_LCSSERR_ENABLE_BIT_LEN     (1U)                
#define SYS_MASK_LCSSERR_ENABLE_BIT_MASK    0x400000UL

#define SYS_MASK_RXPTO_ENABLE_BIT_OFFSET    (21U)               
#define SYS_MASK_RXPTO_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXPTO_ENABLE_BIT_MASK      0x200000UL

#define SYS_MASK_RXOVRR_ENABLE_BIT_OFFSET   (20U)               
#define SYS_MASK_RXOVRR_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RXOVRR_ENABLE_BIT_MASK     0x100000UL

#define SYS_MASK_VWARN_ENABLE_BIT_OFFSET    (19U)               
#define SYS_MASK_VWARN_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_VWARN_ENABLE_BIT_MASK      0x80000UL

#define SYS_MASK_RXCIAFAIL_ENABLE_BIT_OFFSET (18U)               
#define SYS_MASK_RXCIAFAIL_ENABLE_BIT_LEN   (1U)                
#define SYS_MASK_RXCIAFAIL_ENABLE_BIT_MASK  0x40000UL

#define SYS_MASK_RXRFTO_ENABLE_BIT_OFFSET   (17U)               
#define SYS_MASK_RXRFTO_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RXRFTO_ENABLE_BIT_MASK     0x20000UL

#define SYS_MASK_RXFSL_ENABLE_BIT_OFFSET    (16U)               
#define SYS_MASK_RXFSL_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXFSL_ENABLE_BIT_MASK      0x10000UL

#define SYS_MASK_RXFCE_ENABLE_BIT_OFFSET    (15U)               
#define SYS_MASK_RXFCE_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXFCE_ENABLE_BIT_MASK      0x8000U

#define SYS_MASK_RXFCG_ENABLE_BIT_OFFSET    (14U)               
#define SYS_MASK_RXFCG_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXFCG_ENABLE_BIT_MASK      0x4000U

#define SYS_MASK_RXDONE_ENABLE_BIT_OFFSET   (13U)               
#define SYS_MASK_RXDONE_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RXDONE_ENABLE_BIT_MASK     0x2000U

#define SYS_MASK_RXPHE_ENABLE_BIT_OFFSET    (12U)               
#define SYS_MASK_RXPHE_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXPHE_ENABLE_BIT_MASK      0x1000U

#define SYS_MASK_RXPHD_ENABLE_BIT_OFFSET    (11U)               
#define SYS_MASK_RXPHD_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXPHD_ENABLE_BIT_MASK      0x800U

#define SYS_MASK_CIADONE_ENABLE_BIT_OFFSET  (10U)               
#define SYS_MASK_CIADONE_ENABLE_BIT_LEN     (1U)                
#define SYS_MASK_CIADONE_ENABLE_BIT_MASK    0x400U

#define SYS_MASK_RXSFDD_ENABLE_BIT_OFFSET   (9U)                
#define SYS_MASK_RXSFDD_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_RXSFDD_ENABLE_BIT_MASK     0x200U

#define SYS_MASK_RXPRD_ENABLE_BIT_OFFSET    (8U)                
#define SYS_MASK_RXPRD_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_RXPRD_ENABLE_BIT_MASK      0x100U

#define SYS_MASK_TXDONE_ENABLE_BIT_OFFSET   (7U)                
#define SYS_MASK_TXDONE_ENABLE_BIT_LEN      (1U)                
#define SYS_MASK_TXDONE_ENABLE_BIT_MASK     0x80U

#define SYS_MASK_TXPHS_ENABLE_BIT_OFFSET    (6U)                
#define SYS_MASK_TXPHS_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_TXPHS_ENABLE_BIT_MASK      0x40U

#define SYS_MASK_TXPRS_ENABLE_BIT_OFFSET    (5U)                
#define SYS_MASK_TXPRS_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_TXPRS_ENABLE_BIT_MASK      0x20U

#define SYS_MASK_TXFRB_ENABLE_BIT_OFFSET    (4U)                
#define SYS_MASK_TXFRB_ENABLE_BIT_LEN       (1U)                
#define SYS_MASK_TXFRB_ENABLE_BIT_MASK      0x10U

#define SYS_MASK_AAT_ENABLE_BIT_OFFSET      (3U)                
#define SYS_MASK_AAT_ENABLE_BIT_LEN         (1U)                
#define SYS_MASK_AAT_ENABLE_BIT_MASK        0x8U 

#define SYS_MASK_SPICRCE_ENABLE_BIT_OFFSET  (2U)                
#define SYS_MASK_SPICRCE_ENABLE_BIT_LEN     (1U)                
#define SYS_MASK_SPICRCE_ENABLE_BIT_MASK    0x4U

#define SYS_MASK_CP_LOCK_ENABLE_BIT_OFFSET  (1U)                
#define SYS_MASK_CP_LOCK_ENABLE_BIT_LEN     (1U)                
#define SYS_MASK_CP_LOCK_ENABLE_BIT_MASK    0x2U   
           

//******************************************************************************
//* Description: Bit definitions for register SYS_STATUS
//******************************************************************************
#define SYS_STATUS_ID                        (UWB_BASE_ADDR + 0x08)                 
#define SYS_STATUS_LEN                       (4U)                
#define SYS_STATUS_MASK                      0xFFFFFFFFUL

#define SYS_STATUS_AFFREJ_BIT_OFFSET         (29U)               
#define SYS_STATUS_AFFREJ_BIT_LEN            (1U)                
#define SYS_STATUS_AFFREJ_BIT_MASK           0x20000000UL

#define SYS_STATUS_RXSTSE_BIT_OFFSET         (28U)               
#define SYS_STATUS_RXSTSE_BIT_LEN            (1U)                
#define SYS_STATUS_RXSTSE_BIT_MASK           0x10000000UL

#define SYS_STATUS_RXSFDTO_BIT_OFFSET        (26U)               
#define SYS_STATUS_RXSFDTO_BIT_LEN           (1U)                
#define SYS_STATUS_RXSFDTO_BIT_MASK          0x4000000UL

#define SYS_STATUS_RXPTO_BIT_OFFSET          (21U)               
#define SYS_STATUS_RXPTO_BIT_LEN             (1U)                
#define SYS_STATUS_RXPTO_BIT_MASK            0x200000UL

#define SYS_STATUS_RXOVRR_BIT_OFFSET         (20U)               
#define SYS_STATUS_RXOVRR_BIT_LEN            (1U)                
#define SYS_STATUS_RXOVRR_BIT_MASK           0x100000UL

#define SYS_STATUS_VWARN_BIT_OFFSET          (19U)               
#define SYS_STATUS_VWARN_BIT_LEN             (1U)                
#define SYS_STATUS_VWARN_BIT_MASK            0x80000UL

#define SYS_STATUS_CIAERR_BIT_OFFSET         (18U)               
#define SYS_STATUS_CIAERR_BIT_LEN            (1U)                
#define SYS_STATUS_CIAERR_BIT_MASK           0x40000UL

#define SYS_STATUS_RXRFTO_BIT_OFFSET         (17U)               
#define SYS_STATUS_RXRFTO_BIT_LEN            (1U)                
#define SYS_STATUS_RXRFTO_BIT_MASK           0x20000UL

#define SYS_STATUS_RXRFSL_BIT_OFFSET         (16U)               
#define SYS_STATUS_RXRFSL_BIT_LEN            (1U)                
#define SYS_STATUS_RXRFSL_BIT_MASK           0x10000UL

#define SYS_STATUS_RXFCE_BIT_OFFSET          (15U)               
#define SYS_STATUS_RXFCE_BIT_LEN             (1U)                
#define SYS_STATUS_RXFCE_BIT_MASK            0x8000U

#define SYS_STATUS_RXFCG_BIT_OFFSET          (14U)               
#define SYS_STATUS_RXFCG_BIT_LEN             (1U)                
#define SYS_STATUS_RXFCG_BIT_MASK            0x4000U

#define SYS_STATUS_RXDONE_BIT_OFFSET         (13U)               
#define SYS_STATUS_RXDONE_BIT_LEN            (1U)                
#define SYS_STATUS_RXDONE_BIT_MASK           0x2000U

#define SYS_STATUS_RXPHE_BIT_OFFSET          (12U)               
#define SYS_STATUS_RXPHE_BIT_LEN             (1U)                
#define SYS_STATUS_RXPHE_BIT_MASK            0x1000U

#define SYS_STATUS_RXPHD_BIT_OFFSET          (11U)               
#define SYS_STATUS_RXPHD_BIT_LEN             (1U)                
#define SYS_STATUS_RXPHD_BIT_MASK            0x800U

#define SYS_STATUS_RXCIADONE_BIT_OFFSET      (10U)               
#define SYS_STATUS_RXCIADONE_BIT_LEN         (1U)                
#define SYS_STATUS_RXCIADONE_BIT_MASK        0x400U

#define SYS_STATUS_RXSFDD_BIT_OFFSET         (9U)                
#define SYS_STATUS_RXSFDD_BIT_LEN            (1U)                
#define SYS_STATUS_RXSFDD_BIT_MASK           0x200U

#define SYS_STATUS_RXPRD_BIT_OFFSET          (8U)                
#define SYS_STATUS_RXPRD_BIT_LEN             (1U)                
#define SYS_STATUS_RXPRD_BIT_MASK            0x100U

#define SYS_STATUS_TXDONE_BIT_OFFSET         (7U)                
#define SYS_STATUS_TXDONE_BIT_LEN            (1U)                
#define SYS_STATUS_TXDONE_BIT_MASK           0x80U 

#define SYS_STATUS_TXPHS_BIT_OFFSET          (6U)                
#define SYS_STATUS_TXPHS_BIT_LEN             (1U)                
#define SYS_STATUS_TXPHS_BIT_MASK            0x40U

#define SYS_STATUS_TXPRS_BIT_OFFSET          (5U)                
#define SYS_STATUS_TXPRS_BIT_LEN             (1U)                
#define SYS_STATUS_TXPRS_BIT_MASK            0x20U

#define SYS_STATUS_TXFRB_BIT_OFFSET          (4U)                
#define SYS_STATUS_TXFRB_BIT_LEN             (1U)                
#define SYS_STATUS_TXFRB_BIT_MASK            0x10U

#define SYS_STATUS_AAT_BIT_OFFSET            (3U)                
#define SYS_STATUS_AAT_BIT_LEN               (1U)                
#define SYS_STATUS_AAT_BIT_MASK              0x8U

#define SYS_STATUS_SPICRCE_BIT_OFFSET       (2U)                
#define SYS_STATUS_SPICRCE_BIT_LEN          (1U)                
#define SYS_STATUS_SPICRCE_BIT_MASK         0x4U

#define SYS_STATUS_CP_LOCK_BIT_OFFSET       (1U)                
#define SYS_STATUS_CP_LOCK_BIT_LEN          (1U)                
#define SYS_STATUS_CP_LOCK_BIT_MASK         0x2U

#define SYS_STATUS_IRQS_BIT_OFFSET          (0U)                
#define SYS_STATUS_IRQS_BIT_LEN             (1U)                
#define SYS_STATUS_IRQS_BIT_MASK            0x1U                 

//******************************************************************************
//* Description: Bit definitions for register SYS_TIME
//******************************************************************************
#define SYS_TIME_ID                          (UWB_BASE_ADDR + 0x14)
#define SYS_TIME_LEN                         (4U)                
#define SYS_TIME_MASK                        0xFFFFFFFFUL

#define SYS_TIME_SYS_TIME_BIT_OFFSET         (1U)                
#define SYS_TIME_SYS_TIME_BIT_LEN            (32U)               
#define SYS_TIME_SYS_TIME_BIT_MASK           0xffffffffUL    

//******************************************************************************
//* Description: Bit definitions for register DX_TIME
//******************************************************************************
#define DX_TIME_ID                           (UWB_BASE_ADDR + 0x1C)                 
#define DX_TIME_LEN                          (4U)                
#define DX_TIME_MASK                         0xFFFFFFFFUL

#define DX_TIME_DX_TIME_BIT_OFFSET           (0U)                
#define DX_TIME_DX_TIME_BIT_LEN              (32U)               
#define DX_TIME_DX_TIME_BIT_MASK             0xffffffffUL         

//******************************************************************************
//* Description: Bit definitions for register ACK_RESP
//******************************************************************************
#define ACK_RESP_ID                          (UWB_BASE_ADDR + 0x30)       
#define ACK_RESP_LEN                         (4U)                
#define ACK_RESP_MASK                        0xFFFFFFFFUL

#define ACK_RESP_ACK_TIM_BIT_OFFSET          (24U)               
#define ACK_RESP_ACK_TIM_BIT_LEN             (8U)                
#define ACK_RESP_ACK_TIM_BIT_MASK            0xff000000UL

#define ACK_RESP_WAIT4RESP_TIM_BIT_OFFSET    (0U)                
#define ACK_RESP_WAIT4RESP_TIM_BIT_LEN       (20U)               
#define ACK_RESP_WAIT4RESP_TIM_BIT_MASK      0xfffffUL 

//******************************************************************************
//* Description: Bit definitions for register RDB_DIAG_MODE
//******************************************************************************
#define FAST_CMD_ID                         (UWB_BASE_ADDR + 0x3C)     
#define FAST_CMD_LEN                        (4U)                
#define FAST_CMD_MASK                       0xFFFFFFFFUL              

#define FAST_CMD_BIT_OFFSET                 (8U)                
#define FAST_CMD_BIT_LEN                    (5U)                
#define FAST_CMD_BIT_MASK                   0x1F00U    

//******************************************************************************
//* Description: Bit definitions for register FAST_CMD_CTL
//******************************************************************************
#define FAST_CMD_VLD_ID                     (UWB_BASE_ADDR + 0x40)         
#define FAST_CMD_VLD_LEN                    (4U)                
#define FAST_CMD_VLD_MASK                   0xFFFFFFFFUL

#define FAST_CMD_VLD_BIT_OFFSET             (0U)                
#define FAST_CMD_VLD_BIT_LEN                (1U)                
#define FAST_CMD_VLD_BIT_MASK               0x1U 
 
//******************************************************************************
//* Description: Bit definitions for register XTAL_SW_CTL
//******************************************************************************
#define CLK_XO_ID                           (UWB_BASE_ADDR + 0x0800)         
#define CLK_XO_LEN                          (4U)                
#define CLK_XO_MASK                         0xFFFFFFFFUL

#define D_XO_SWC_O_BIT_OFFSET               (16U)                
#define D_XO_SWC_O_BIT_LEN                  (6U)                
#define D_XO_SWC_O_BIT_MASK                 0x3f0000U 

#define D_XO_SWF_BIT_OFFSET                 (8U)                
#define D_XO_SWF_BIT_LEN                    (6U)                
#define D_XO_SWF_BIT_MASK                   0x3f00U 

#define D_XO_SWC_I_BIT_OFFSET               (0U)                
#define D_XO_SWC_I_BIT_LEN                  (6U)                
#define D_XO_SWC_I_BIT_MASK                 0x3fU 

//******************************************************************************
//* Description: Bit definitions for register TX_PA_GAIN_SET
//******************************************************************************
#define TX_PA_GAIN_ID                       (UWB_BASE_ADDR + 0x080C)
#define TX_PA_GAIN_LEN                      (4U)
#define TX_PA_GAIN_MASK                     0xFFFFFFFFUL

#define TX_CTRL_PA_GAIN_HALF_BIT_OFFSET     (5U)
#define TX_CTRL_PA_GAIN_HALF_BIT_LEN        (1U)
#define TX_CTRL_PA_GAIN_HALF_BIT_MASK       0x20U

//******************************************************************************
//* Description: Bit definitions for register PLL_STATUS
//******************************************************************************
#define PLL_STATUS_ID                       (UWB_BASE_ADDR + 0x081c)         
#define PLL_STATUS_LEN                      (4U)                
#define PLL_STATUS_MASK                     0xFFUL

//******************************************************************************
//* Description: Bit definitions for register TX_CTRL
//******************************************************************************
#define TX_CTRL_ID                           (UWB_BASE_ADDR + 0x0824)         
#define TX_CTRL_LEN                          (4U)                
#define TX_CTRL_MASK                         0xFFFFFFFFUL

#define TX_CTRL_PA_CW_OUT_BIT_OFFSET         (23U)                
#define TX_CTRL_PA_CW_OUT_BIT_LEN            (1U)                
#define TX_CTRL_PA_CW_OUT_BIT_MASK           0x800000U 

#define TX_CTRL_PA_GAIN_SEL_BIT_OFFSET       (19U)                
#define TX_CTRL_PA_GAIN_SEL_BIT_LEN          (3U)                
#define TX_CTRL_PA_GAIN_SEL_BIT_MASK         0x380000U 

//******************************************************************************
//* Description: Bit definitions for register OTP_WDATA
//******************************************************************************
#define OTP_WDATA_ID                              (UWB_BASE_ADDR + 0xb00)              
#define OTP_WDATA_LEN                             (4U)                
#define OTP_WDATA_MASK                            0xFFFFFFFFUL        
#define OTP_WDATA_OTP_WDATA_BIT_OFFSET            (0U)                
#define OTP_WDATA_OTP_WDATA_BIT_LEN               (32U)               
#define OTP_WDATA_OTP_WDATA_BIT_MASK              0xffffffffUL         

//******************************************************************************
//* Description: Bit definitions for register OTP_ADDR
//******************************************************************************
#define OTP_ADDR_ID                              (UWB_BASE_ADDR + 0xb04)              
#define OTP_ADDR_LEN                             (4U)                
#define OTP_ADDR_MASK                            0xFFFFFFFFUL        
#define OTP_ADDR_OTP_ADDR_BIT_OFFSET             (0U)                
#define OTP_ADDR_OTP_ADDR_BIT_LEN                (11U)               
#define OTP_ADDR_OTP_ADDR_BIT_MASK               0x7ffU     

//******************************************************************************
//* Description: Bit definitions for register OTP_CFG
//******************************************************************************
#define OTP_CFG_ID                               (UWB_BASE_ADDR + 0xb08)              
#define OTP_CFG_LEN                              (4U)                
#define OTP_CFG_MASK                             0xFFFFFFFFUL        
               

#define OTP_CFG_OTP_MAIN_CTR_MASK                (0x01U<<0)
#define OTP_CFG_OPT_RD_MASK                      (0x01U<<1)
#define OTP_CFG_OPT_WR_MASK                      (0x01U<<2)
#define OTP_CFG_OPT_WRM_MASK                     (0x01U<<3)
#define OTP_CFG_OPT_RDM_MASK                     (0x01U<<6)
#define OTP_CFG_LDO_KICK_MASK  		               (0x01U<<7)
#define OTP_CFG_OTP_AUTO_READ_MASK               (0x01U<<8)
#define OTP_CFG_OPS_KICK_MASK                    (0x01U<<10)
#define OTP_CFG_OPS_SEL_MASK                     (0x03U<<11)

//******************************************************************************
//* Description: Bit definitions for register OTP_STATUS
//******************************************************************************
#define OTP_STATUS_ID                            (UWB_BASE_ADDR + 0xb0c)
#define OTP_STATUS_LEN                           (4U)                
#define OTP_STATUS_MASK                          0xFFFFFFFFUL        
#define OTP_STATUS_OTP_VPP_OK_BIT_OFFSET         (1U)                
#define OTP_STATUS_OTP_VPP_OK_BIT_LEN            (1U)                
#define OTP_STATUS_OTP_VPP_OK_BIT_MASK           0x2U                 
#define OTP_STATUS_OTP_PROG_DONE_BIT_OFFSET      (0U)                
#define OTP_STATUS_OTP_PROG_DONE_BIT_LEN         (1U)                
#define OTP_STATUS_OTP_PROG_DONE_BIT_MASK        0x1U                 

//******************************************************************************
//* Description: Bit definitions for register OTP_RDATA
//******************************************************************************
#define OTP_RDATA_ID                             (UWB_BASE_ADDR + 0xb10)
#define OTP_RDATA_LEN                            (4U)                
#define OTP_RDATA_MASK                           0xFFFFFFFFUL        
#define OTP_RDATA_OTP_RDATA_BIT_OFFSET           (0U)                
#define OTP_RDATA_OTP_RDATA_BIT_LEN              (32U)               
#define OTP_RDATA_OTP_RDATA_BIT_MASK             0xffffffffUL

#define OTP_BASE_ADDR                            0xb000

//******************************************************************************
//* Description: Bit definitions for register FAST_CMD_CTL
//******************************************************************************
#define SYS_STATE_ID                          (UWB_BASE_ADDR + 0x0F30)         
#define SYS_STATE_LEN                         (4U)                
#define SYS_STATE_MASK                        0xFFFFFFFFUL

#define SYS_STATE_PMSC_BIT_OFFSET             (16U)                
#define SYS_STATE_PMSC_BIT_LEN                (5U)                
#define SYS_STATE_PMSC_BIT_MASK               0x1F0000U 

#define SYS_STATE_RX_BIT_OFFSET               (8U)                
#define SYS_STATE_RX_BIT_LEN                  (6U)                
#define SYS_STATE_RX_BIT_MASK                 0x3F00U  

#define SYS_STATE_TX_BIT_OFFSET               (0U)                
#define SYS_STATE_TX_BIT_LEN                  (4U)                
#define SYS_STATE_TX_BIT_MASK                 0xFU  


#define ANALOG_ANTENNA_SWITCH_ID                         (UWB_BASE_ADDR + 0x105C)
#define ANALOG_ANTENNA_SWITCH_LEN                        (4U)                
#define ANALOG_ANTENNA_SWITCH_MASK                       0xF00UL

#define PDOA_SW_FORCE_RX1RX2_INPUT_SWITCH_LEN            (2U)                
#define PDOA_SW_FORCE_RX1RX2_INPUT_SWITCH_MASK           0xC00U

#define PDOA_SW_FORCE_TO_RX1_INPUT_BIT_OFFSET            (11U)                
#define PDOA_SW_FORCE_TO_RX1_INPUT_BIT_LEN               (1U)                
#define PDOA_SW_FORCE_TO_RX1_INPUT_BIT_MASK              0x800U

#define PDOA_SW_FORCE_TO_RX2_INPUT_BIT_OFFSET            (10U)                
#define PDOA_SW_FORCE_TO_RX2_INPUT_BIT_LEN               (1U)                
#define PDOA_SW_FORCE_TO_RX2_INPUT_BIT_MASK              0x400U

#define TRX_SW_FORCE_TO_RX0_INPUT_BIT_OFFSET             (9U)                
#define TRX_SW_FORCE_TO_RX0_INPUT_BIT_LEN                (1U)                
#define TRX_SW_FORCE_TO_RX0_INPUT_BIT_MASK               0x200U

#define TRX_SW_FORCE_TO_TX_OUTPUT_BIT_OFFSET             (8U)                
#define TRX_SW_FORCE_TO_TX_OUTPUT_BIT_LEN                (1U)                
#define TRX_SW_FORCE_TO_TX_OUTPUT_BIT_MASK               0x100U

//******************************************************************************/
// Description: Bit definitions for register IP CIA
//******************************************************************************/
#define IP_CIA_ID                             (UWB_BASE_ADDR + 0x1600)           
#define IP_CIA_LEN                            (4U)                
#define IP_CIA_MASK                           0xFFFFFFFFUL

#define IP_CIA_FW_BIT_OFFSET                  (23U)                
#define IP_CIA_FW_BIT_LEN                     (9U)                
#define IP_CIA_FW_BIT_MASK                    0xFF800000U

#define IP_CIA_NTM1_BIT_OFFSET                (8U)                
#define IP_CIA_NTM1_BIT_LEN                   (8U)                
#define IP_CIA_NTM1_BIT_MASK                  0xFF00U
                
#define IP_CIA_NTM0_NUM_BIT_OFFSET            (0U)                
#define IP_CIA_NTM0_NUM_BIT_LEN               (8U)                
#define IP_CIA_NTM0_NUM_BIT_MASK              0xFFU

//******************************************************************************/
// Description: Bit definitions for register sts configuration
//******************************************************************************/
#define STS_CFG_ID                              (UWB_BASE_ADDR + 0x1700)           
#define STS_CFG_LEN                             (4U)                
#define STS_CFG_MASK                            0xFFFFFFFFUL

#define STS_CPS_LEN_BIT_OFFSET                  (23U)                
#define STS_CPS_LEN_BIT_LEN                     (9U)                
#define STS_CPS_LEN_BIT_MASK                    0xFF800000U

#define STS_DELTA_BIT_OFFSET                    (22U)                
#define STS_DELTA_BIT_LEN                       (1U)                
#define STS_DELTA_BIT_MASK                      0x400000U
		
#define STS_STS_SEG_NUM_BIT_OFFSET              (20U)                
#define STS_STS_SEG_NUM_BIT_LEN                 (2U)                
#define STS_STS_SEG_NUM_BIT_MASK                0x300000U

//******************************************************************************/
// Description: Bit definitions for register sts_iv_p0
//******************************************************************************/
#define STS_IV_P0                               (UWB_BASE_ADDR + 0x1704)                 
#define STS_IV_P0_LEN                           (4U)                
#define STS_IV_P0_MASK                          0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_iv_p1
//******************************************************************************/
#define STS_IV_P1                               (UWB_BASE_ADDR + 0x1708)                 
#define STS_IV_P1_LEN                           (4U)                
#define STS_IV_P1_MASK                          0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_iv_p2
//******************************************************************************/
#define STS_IV_P2                               (UWB_BASE_ADDR + 0x170c)                 
#define STS_IV_P2_LEN                           (4U)                
#define STS_IV_P2_MASK                          0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_iv_p3
//******************************************************************************/
#define STS_IV_P3                               (UWB_BASE_ADDR + 0x1710)                 
#define STS_IV_P3_LEN                           (4U)                
#define STS_IV_P3_MASK                          0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_key_p0
//******************************************************************************/
#define STS_KEY_P0                              (UWB_BASE_ADDR + 0x1714)                 
#define STS_KEY_P0_LEN                          (4U)                
#define STS_KEY_P0_MASK                         0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_key_p1
//******************************************************************************/
#define STS_KEY_P1                              (UWB_BASE_ADDR + 0x1718)                 
#define STS_KEY_P1_LEN                          (4U)                
#define STS_KEY_P1_MASK                         0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_key_p2
//******************************************************************************/
#define STS_KEY_P2                              (UWB_BASE_ADDR + 0x171c)                 
#define STS_KEY_P2_LEN                          (4U)                
#define STS_KEY_P2_MASK                         0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts_key_p3
//******************************************************************************/
#define STS_KEY_P3                              (UWB_BASE_ADDR + 0x1720)                 
#define STS_KEY_P3_LEN                          (4U)                
#define STS_KEY_P3_MASK                         0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register sts key iv load configuration 
//******************************************************************************/
#define STS_LOAD_ID                             (UWB_BASE_ADDR + 0x1724)                 
#define STS_LOAD_LEN                            (4U)                
#define STS_LOAD_MASK                           0xFFFFFFFFUL

#define STS_CP_LOADIV_BIT_OFFSET                (31U)
#define STS_CP_LOADIV_BIT_LEN                   (1U)
#define STS_CP_LOADIV_BIT_MASK                  0x80000000U 

#define STS_CP_LOADKEY_BIT_OFFSET               (30U)
#define STS_CP_LOADKEY_BIT_LEN                  (1U)
#define STS_CP_LOADKEY_BIT_MASK                 0x40000000U 

//******************************************************************************/
// Description: Bit definitions for register IP_STS_TX_STAMP_H32 
//******************************************************************************/
#define IP_STS_TX_STAMP_H32                     (UWB_BASE_ADDR + 0x172C)                 
#define IP_STS_TX_STAMP_H32_LEN                 (4U)                
#define IP_STS_TX_STAMP_H32_MASK                0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register load IP_STS_TX_STAMP_L8 
//******************************************************************************/
#define IP_STS_TX_STAMP_L8                      (UWB_BASE_ADDR + 0x1730)                 
#define IP_STS_TX_STAMP_L8_LEN                  (1U)                
#define IP_STS_TX_STAMP_L8_MASK                 0xffU

//******************************************************************************/
// Description: Bit definitions for register SYS_CFG
//******************************************************************************/
#define SYS_CFG_ID                              (UWB_BASE_ADDR + 0x1810)                 
#define SYS_CFG_LEN                             (4U)                
#define SYS_CFG_MASK                            0xFFFFFFFFUL  

#define SYS_CFG_EXT_ANT_FLG_RX0_BIT_OFFSET      (26U)
#define SYS_CFG_EXT_ANT_FLG_RX0_BIT_LEN         (1U)
#define SYS_CFG_EXT_ANT_FLG_RX0_BIT_MASK        0x4000000UL

#define SYS_CFG_EXT_ANT_FLG_RX1_BIT_OFFSET      (25U)
#define SYS_CFG_EXT_ANT_FLG_RX1_BIT_LEN         (1U)
#define SYS_CFG_EXT_ANT_FLG_RX1_BIT_MASK        0x2000000UL

#define SYS_CFG_CP_RAKE_EN_BIT_OFFSET           (24U)
#define SYS_CFG_CP_RAKE_EN_BIT_LEN              (1U)
#define SYS_CFG_CP_RAKE_EN_BIT_MASK             0x1000000UL

#define SYS_CFG_IP_RAKE_EN_BIT_OFFSET           (23U)
#define SYS_CFG_IP_RAKE_EN_BIT_LEN              (1U)
#define SYS_CFG_IP_RAKE_EN_BIT_MASK             0x800000UL

#define SYS_CFG_RESAMPLE_FLG_BIT_OFFSET         (22U)
#define SYS_CFG_RESAMPLE_FLG_BIT_LEN            (1U)
#define SYS_CFG_RESAMPLE_FLG_BIT_MASK           0x400000UL

#define SYS_CFG_PHASE_FLG_BIT_OFFSET            (20U)
#define SYS_CFG_PHASE_FLG_BIT_LEN               (2U)
#define SYS_CFG_PHASE_FLG_BIT_MASK              0x300000UL

#define SYS_CFG_DIS_DCEN_BIT_OFFSET             (19U)
#define SYS_CFG_DIS_DCEN_BIT_LEN                (1U)
#define SYS_CFG_DIS_DCEN_BIT_MASK               0x80000UL

#define SYS_CFG_FOC_INIT_BIT_OFFSET             (18U)
#define SYS_CFG_FOC_INIT_BIT_LEN                (1U)
#define SYS_CFG_FOC_INIT_BIT_MASK               0x40000UL

#define SYS_CFG_DIS_RAKE_BIT_OFFSET             (17U)
#define SYS_CFG_DIS_RAKE_BIT_LEN                (1U)
#define SYS_CFG_DIS_RAKE_BIT_MASK               0x20000UL

#define SYS_CFG_DIS_TRACK_BIT_OFFSET            (16U)               
#define SYS_CFG_DIS_TRACK_BIT_LEN               (1U)                
#define SYS_CFG_DIS_TRACK_BIT_MASK              0x10000UL

#define SYS_CFG_PDOA_EN_BIT_OFFSET              (15U)               
#define SYS_CFG_PDOA_EN_BIT_LEN                 (1U)                
#define SYS_CFG_PDOA_EN_BIT_MASK                0x8000UL

#define SYS_CFG_FOE_DIS_EN_BIT_OFFSET           (14U)               
#define SYS_CFG_FOE_DIS_EN_BIT_LEN              (1U)                
#define SYS_CFG_FOE_DIS_EN_BIT_MASK             0x4000UL

#define SYS_CFG_FAST_AAT_EN_BIT_OFFSET          (13U)               
#define SYS_CFG_FAST_AAT_EN_BIT_LEN             (1U)                
#define SYS_CFG_FAST_AAT_EN_BIT_MASK            0x2000UL

#define SYS_CFG_STS_MODE_BIT_OFFSET             (11U)               
#define SYS_CFG_STS_MODE_BIT_LEN                (2U)                
#define SYS_CFG_STS_MODE_BIT_MASK               0x1800UL
            
#define SYS_CFG_AUTO_ACK_BIT_OFFSET             (10U)               
#define SYS_CFG_AUTO_ACK_BIT_LEN                (1U)                
#define SYS_CFG_AUTO_ACK_BIT_MASK               0x400U

#define SYS_CFG_RXAUTR_BIT_OFFSET               (9U)               
#define SYS_CFG_RXAUTR_BIT_LEN                  (1U)                
#define SYS_CFG_RXAUTR_BIT_MASK                 0x200U

#define SYS_CFG_RXWTOE_BIT_OFFSET               (8U)                
#define SYS_CFG_RXWTOE_BIT_LEN                  (1U)                
#define SYS_CFG_RXWTOE_BIT_MASK                 0x100U  

#define SYS_CFG_CIA_STS_BIT_OFFSET              (7U)                
#define SYS_CFG_CIA_STS_BIT_LEN                 (1U)                
#define SYS_CFG_CIA_STS_BIT_MASK                0x80U

#define SYS_CFG_CIA_IPATOV_BIT_OFFSET           (6U)                
#define SYS_CFG_CIA_IPATOV_BIT_LEN              (1U)                
#define SYS_CFG_CIA_IPATOV_BIT_MASK             0x40U

#define SYS_CFG_SPI_CRC_BIT_OFFSET              (5U)                
#define SYS_CFG_SPI_CRC_BIT_LEN                 (1U)                
#define SYS_CFG_SPI_CRC_BIT_MASK                0x20U

#define SYS_CFG_PHR_6M8_BIT_OFFSET              (4U)                
#define SYS_CFG_PHR_6M8_BIT_LEN                 (1U)                
#define SYS_CFG_PHR_6M8_BIT_MASK                0x10U

#define SYS_CFG_PHR_MODE_BIT_OFFSET             (3U)                
#define SYS_CFG_PHR_MODE_BIT_LEN                (1U)                
#define SYS_CFG_PHR_MODE_BIT_MASK               0x8U

#define SYS_CFG_EN_DRXB_BIT_OFFSET              (2U)                
#define SYS_CFG_EN_DRXB_BIT_LEN                 (1U)                
#define SYS_CFG_EN_DRXB_BIT_MASK                0x4U

#define SYS_CFG_DIS_FCS_TX_BIT_OFFSET           (1U)                
#define SYS_CFG_DIS_FCS_TX_BIT_LEN              (1U)                
#define SYS_CFG_DIS_FCS_TX_BIT_MASK             0x2U

#define SYS_CFG_FFEN_BIT_OFFSET                 (0U)                
#define SYS_CFG_FFEN_BIT_LEN                    (1U)                
#define SYS_CFG_FFEN_BIT_MASK                   0x1U                 

//******************************************************************************/
// Description: Bit definitions for register TX_FCTRL0
//******************************************************************************/
#define TX_DATA_CTRL_ID                          (UWB_BASE_ADDR + 0x1818)                 
#define TX_DATA_CTRL_LEN                         (4U)                
#define TX_DATA_CTRL_MASK                        0xFFFFFFFFUL

#define TX_DATA_CTRL_TXB_OFFSET_BIT_OFFSET       (16U)               
#define TX_DATA_CTRL_TXB_OFFSET_BIT_LEN          (10U)               
#define TX_DATA_CTRL_TXB_OFFSET_BIT_MASK         0x3ff0000UL

#define TX_DATA_CTRL_TXTR_BIT_OFFSET             (10U)               
#define TX_DATA_CTRL_TXTR_BIT_LEN                (1U)               
#define TX_DATA_CTRL_TXTR_BIT_MASK               0x400UL

#define TX_DATA_CTRL_TXFLEN_BIT_OFFSET           (0U)               
#define TX_DATA_CTRL_TXFLEN_BIT_LEN              (10U)               
#define TX_DATA_CTRL_TXFLEN_BIT_MASK             0x3ffUL

//******************************************************************************/
// Description: Bit definitions for register TX_FCTRL0
//******************************************************************************/
#define TX_PHR_CTRL_ID                          (UWB_BASE_ADDR + 0x181c)               
#define TX_PHR_CTRL_LEN                         (4U)                
#define TX_PHR_CTRL_MASK                        0xFFFFFFFFUL           
              
#define TX_PHR_CTRL_TXBR_BIT_OFFSET             (8U)               
#define TX_PHR_CTRL_TXBR_BIT_LEN                (2U)                
#define TX_PHR_CTRL_TXBR_BIT_MASK               0x300U

#define TX_PHR_CTRL_TXPSR_PE_BIT_OFFSET         (0U)               
#define TX_PHR_CTRL_TXPSR_PE_BIT_LEN            (4U)                
#define TX_PHR_CTRL_TXPSR_PE_BIT_MASK           0xfU     

//******************************************************************************/
// Description: Bit definitions for register CHAN_CTRL
//******************************************************************************/
#define SYSTEM_PARAM_ID                         (UWB_BASE_ADDR + 0x1828)              
#define SYSTEM_PARAM_LEN                        (4U)                
#define SYSTEM_PARAM_MASK                       0xFFFFFFFFUL

#define SYSTEM_PARAM_RX_PCODE_BIT_OFFSET        (16U)                
#define SYSTEM_PARAM_RX_PCODE_BIT_LEN           (5U)                
#define SYSTEM_PARAM_RX_PCODE_BIT_MASK          0x1f0000U

#define SYSTEM_PARAM_TX_PCODE_BIT_OFFSET        (8U)                
#define SYSTEM_PARAM_TX_PCODE_BIT_LEN           (5U)                
#define SYSTEM_PARAM_TX_PCODE_BIT_MASK          0x1f00U

#define SYSTEM_PARAM_SFD_TYPE_BIT_OFFSET        (3U)                
#define SYSTEM_PARAM_SFD_TYPE_BIT_LEN           (2U)                
#define SYSTEM_PARAM_SFD_TYPE_BIT_MASK          0x18U

#define SYSTEM_PARAM_RF_CHAN_BIT_OFFSET         (0U)                
#define SYSTEM_PARAM_RF_CHAN_BIT_LEN            (3U)                
#define SYSTEM_PARAM_RF_CHAN_BIT_MASK           0x7U           //2,3,5,6,7,8,9    

//******************************************************************************/
// Description: Bit definitions for register High 32 bit of Transmit Time Stamp
//******************************************************************************/
#define TX_STAMP_H32_ID                         (UWB_BASE_ADDR + 0x182c)
#define TX_STAMP_H32_LEN                        (4U)                
#define TX_STAMP_H32_MASK                       0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register Low 8 bit of Transmit Time Stamp
//******************************************************************************/
#define TX_STAMP_L8_ID                          (UWB_BASE_ADDR + 0x1830)                
#define TX_STAMP_L8_LEN                         (1U)                
#define TX_STAMP_L8_MASK                        0xFFUL

#define TX_STAMP_L8_BIT_OFFSET                  (0U)                
#define TX_STAMP_L8_BIT_LEN                     (8U)                
#define TX_STAMP_L8_BIT_MASK                    0xffU   

//******************************************************************************/
// Description: Bit definitions for register Low 8 bit of Transmit Time ANT Delay
//******************************************************************************/
#define ANT_DELAY_ID                            (UWB_BASE_ADDR + 0x1838)                
#define ANT_DELAY_LEN                           (4U)                
#define ANT_DELAY_MASK                          0xFFFFFFFFUL

#define ANT_DELAY_RX_BIT_OFFSET                 (16U)                
#define ANT_DELAY_RX_BIT_LEN                    (16U)                
#define ANT_DELAY_RX_BIT_MASK                   0xffff0000U  

#define ANT_DELAY_TX_BIT_OFFSET                 (0U)                
#define ANT_DELAY_TX_BIT_LEN                    (16U)                
#define ANT_DELAY_TX_BIT_MASK                   0xffffU   

//******************************************************************************/
// Description: Bit definitions for register Transmit Time Stamp raw
//******************************************************************************/
#define TX_STAMP_RAW_ID                                     (UWB_BASE_ADDR + 0x1834)                 
#define TX_STAMP_RAW_LEN                                    (4U)                
#define TX_STAMP_RAW_MASK                                   0xFFFFFFFFUL

#define TX_STAMP_RAW_BIT_OFFSET                             (0U)                
#define TX_STAMP_RAW_BIT_LEN                                (32U)               
#define TX_STAMP_RAW_BIT_MASK                               0xffffffffUL 

//******************************************************************************/
// Description: Bit definitions for register TX_RX_TEST
//******************************************************************************/
#define ADC_SYNC_ID                     (UWB_BASE_ADDR + 0x1844)              
#define ADC_SYNC_LEN                    (4U)                
#define ADC_SYNC_MASK                   0xFFFFFFFFUL

#define ADC_SYNC_TEST_EN_BIT_OFFSET             (31U)                
#define ADC_SYNC_TEST_EN_BIT_LEN                (1U)               
#define ADC_SYNC_TEST_EN_BIT_MASK               0x80000000UL

#define ADC_SYNC_RD_ONCE_BIT_OFFSET             (30U)                
#define ADC_SYNC_RD_ONCE_BIT_LEN                (1U)               
#define ADC_SYNC_RD_ONCE_BIT_MASK               0x40000000UL

#define ADC_SYNC_RX_COBR_BIT_OFFSET             (29U)                
#define ADC_SYNC_RX_COBR_BIT_LEN                (1U)               
#define ADC_SYNC_RX_COBR_BIT_MASK               0x20000000UL

#define ADC_SYNC_RX_CIBR_BIT_OFFSET             (28U)                
#define ADC_SYNC_RX_CIBR_BIT_LEN                (1U)               
#define ADC_SYNC_RX_CIBR_BIT_MASK               0x10000000UL

#define ADC_SYNC_TX_COBR_BIT_OFFSET             (27U)                
#define ADC_SYNC_TX_COBR_BIT_LEN                (1U)               
#define ADC_SYNC_TX_COBR_BIT_MASK               0x8000000UL

#define ADC_SYNC_TX_CIBR_BIT_OFFSET             (26U)                
#define ADC_SYNC_TX_CIBR_BIT_LEN                (1U)               
#define ADC_SYNC_TX_CIBR_BIT_MASK               0x4000000UL

#define ADC_SYNC_RX_WR_MODE_BIT_OFFSET          (23U)                
#define ADC_SYNC_RX_WR_MODE_BIT_LEN             (3U)               
#define ADC_SYNC_RX_WR_MODE_BIT_MASK            0x3800000UL

#define ADC_SYNC_WR_MODE_BIT_OFFSET             (21U)                
#define ADC_SYNC_WR_MODE_BIT_LEN                (2U)               
#define ADC_SYNC_WR_MODE_BIT_MASK               0x600000UL

#define ADC_SYNC_RX0RX1_SEL_BIT_OFFSET          (20U)                
#define ADC_SYNC_RX0RX1_SEL_BIT_LEN             (1U)               
#define ADC_SYNC_RX0RX1_SEL_BIT_MASK            0x100000UL

#define ADC_SYNC_RAM_WR_EN_BIT_OFFSET           (19U)                
#define ADC_SYNC_RAM_WR_EN_BIT_LEN              (1U)               
#define ADC_SYNC_RAM_WR_EN_BIT_MASK             0x80000UL

#define ADC_SYNC_TX_RD_EN_BIT_OFFSET            (18U)                
#define ADC_SYNC_TX_RD_EN_BIT_LEN               (1U)               
#define ADC_SYNC_TX_RD_EN_BIT_MASK              0x40000UL

#define ADC_SYNC_TX_DATA_SEL_BIT_OFFSET         (17U)                
#define ADC_SYNC_TX_DATA_SEL_BIT_LEN            (1U)               
#define ADC_SYNC_TX_DATA_SEL_BIT_MASK           0x20000UL

#define ADC_SYNC_ENDIAN_SEL_BIT_OFFSET          (16U)                
#define ADC_SYNC_ENDIAN_SEL_BIT_LEN             (1U)               
#define ADC_SYNC_ENDIAN_SEL_BIT_MASK            0x10000UL

//******************************************************************************/
// Description: Bit definitions for register prf mode
//******************************************************************************/
#define PRF_MODE_ID                              (UWB_BASE_ADDR + 0x1878)                 
#define PRF_MODE_LEN                             (4U)                
#define PRF_MODE_MASK                            0xFFFFFFFFUL

#define PRF_MODE_HPRF_MODE_BIT_OFFSET            (1U)                
#define PRF_MODE_HPRF_MODE_BIT_LEN               (2U)                
#define PRF_MODE_HPRF_MODE_BIT_MASK              0x6U

#define PRF_MODE_HPRF_EN_BIT_OFFSET              (0U)                
#define PRF_MODE_HPRF_EN_BIT_LEN                 (1U)                
#define PRF_MODE_HPRF_EN_BIT_MASK                0x1U


//******************************************************************************/
// Description: Bit definitions for register DRX_SFDTOC
//******************************************************************************/
#define PREAMBLE_CFG_ID                           (UWB_BASE_ADDR + 0x1900)              
#define PREAMBLE_CFG_LEN                          (4U)                
#define PREAMBLE_CFG_MASK                         0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register DRX_SFDTOC
//******************************************************************************/
#define SFD_CFG_ID                              (UWB_BASE_ADDR + 0x1904)              
#define SFD_CFG_LEN                             (4U)                
#define SFD_CFG_MASK                            0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register DRX_PSDU_CFG
//******************************************************************************/
#define DRX_PSDU_CFG_ID                         (UWB_BASE_ADDR + 0x1908)              
#define DRX_PSDU_CFG_LEN                        (4U)                
#define DRX_PSDU_CFG_MASK                       0xFFFFFFFFUL

//******************************************************************************/
// Description: Bit definitions for register DRX_SFDTOC
//******************************************************************************/
#define DRX_SFDTOC_ID                           (UWB_BASE_ADDR + 0x1910)              
#define DRX_SFDTOC_LEN                          (4U)                
#define DRX_SFDTOC_MASK                         0xFFFFFFFFUL

#define DRX_SFDTOC_BIT_OFFSET                   (16U)                
#define DRX_SFDTOC_BIT_LEN                      (16U)                
#define DRX_SFDTOC_BIT_MASK                     0xffff0000U

//******************************************************************************/
// Description: Bit definitions for register DRX_PRETOC
//******************************************************************************/
#define DRX_PRETOC_ID                           (UWB_BASE_ADDR + 0x1914)            
#define DRX_PRETOC_LEN                          (4U)                
#define DRX_PRETOC_MASK                         0xFFFFFFFFUL

#define DRX_PRETOC_BIT_OFFSET                   (16U)                
#define DRX_PRETOC_BIT_LEN                      (16U)                
#define DRX_PRETOC_BIT_MASK                     0xffff0000U

//******************************************************************************/
// Description: Bit definitions for register RX_FWRO
//******************************************************************************/
#define RX_FWTO_ID                              (UWB_BASE_ADDR + 0x1918)              
#define RX_FWTO_LEN                             (4U)                
#define RX_FWTO_MASK                            0xFFFFFFFFUL

#define RX_FWTO_BIT_OFFSET                      (0U)                
#define RX_FWTO_BIT_LEN                         (32U)                
#define RX_FWTO_BIT_MASK                        0xffffffffU

//******************************************************************************/
// Description: Bit definitions for register Preamble configration
//******************************************************************************/
#define PREAMBLE_CFG_ID                         (UWB_BASE_ADDR + 0x1900)              
#define PREAMBLE_CFG_LEN                        (4U)                
#define PREAMBLE_CFG_MASK                       0xFFFFFFFFUL

#define PREAMBLE_CFG_PACW_BIT_OFFSET            (28U)                
#define PREAMBLE_CFG_PACW_BIT_LEN               (4U)                
#define PREAMBLE_CFG_PACW_BIT_MASK              0xf0000000U 

#define PREAMBLE_CFG_PADT_BIT_OFFSET            (27U)                
#define PREAMBLE_CFG_PADT_BIT_LEN               (1U)                
#define PREAMBLE_CFG_PADT_BIT_MASK              0x8000000U 

#define PREAMBLE_CFG_PADW_BIT_OFFSET            (24U)                
#define PREAMBLE_CFG_PADW_BIT_LEN               (3U)                
#define PREAMBLE_CFG_PADW_BIT_MASK              0x7000000U 

#define PREAMBLE_CFG_PATH1_BIT_OFFSET           (14U)                
#define PREAMBLE_CFG_PATH1_BIT_LEN              (10U)                
#define PREAMBLE_CFG_PATH1_BIT_MASK             0xffc000U 

#define PREAMBLE_CFG_PATH_BIT_OFFSET            (6U)                
#define PREAMBLE_CFG_PATH_BIT_LEN               (8U)                
#define PREAMBLE_CFG_PATH_BIT_MASK              0x3fc0U 

#define PREAMBLE_CFG_PAC_BIT_OFFSET             (3U)                
#define PREAMBLE_CFG_PAC_BIT_LEN                (3U)                
#define PREAMBLE_CFG_PAC_BIT_MASK               0x38U 

#define PREAMBLE_CFG_PACF_BIT_OFFSET            (0U)                
#define PREAMBLE_CFG_PACF_BIT_LEN               (3U)                
#define PREAMBLE_CFG_PACF_BIT_MASK              0x07U 

//******************************************************************************/
// Description: Bit definitions for register PDOA_EN
//******************************************************************************/
#define PDOA_CONFIG_ID                           (UWB_BASE_ADDR + 0x191C)              
#define PDOA_CONFIG_LEN                          (4U)                
#define PDOA_CONFIG_MASK                         0xFFFFFFFFUL

#define PDOA_FILTER_MODE_BIT_OFFSET              (2U)                
#define PDOA_FILTER_MODE_BIT_LEN                 (1U)                
#define PDOA_FILTER_MODE_BIT_MASK                0x4U

#define PDOA_DET_TYPE_BIT_OFFSET                 (1U)                
#define PDOA_DET_TYPE_BIT_LEN                    (1U)                
#define PDOA_DET_TYPE_BIT_MASK                   0x2U

#define PDOA_TYPE_BIT_OFFSET                     (0U)                
#define PDOA_TYPE_BIT_LEN                        (1U)                
#define PDOA_TYPE_BIT_MASK                       0x1U

//******************************************************************************/
// Description: Bit definitions for register ant switch
//******************************************************************************/
#define ANT_SWITCH_ID                            (UWB_BASE_ADDR + 0x1964)                
#define ANT_SWITCH_LEN                           (4U)                
#define ANT_SWITCH_MASK                          0xFFFFFFFFUL

#define ANT_SWITCH_1_OVRD_BIT_OFFSET             (3U)                
#define ANT_SWITCH_1_OVRD_BIT_LEN                (1U)                
#define ANT_SWITCH_1_OVRD_BIT_MASK               0x08U  

#define ANT_SWITCH_1_BIT_OFFSET                  (2U)                
#define ANT_SWITCH_1_BIT_LEN                     (1U)                
#define ANT_SWITCH_1_BIT_MASK                    0x04U  

#define ANT_SWITCH_0_OVRD_BIT_OFFSET             (1U)                
#define ANT_SWITCH_0_OVRD_BIT_LEN                (1U)                
#define ANT_SWITCH_0_OVRD_BIT_MASK               0x02U  

#define ANT_SWITCH_0_BIT_OFFSET                  (0U)                
#define ANT_SWITCH_0_BIT_LEN                     (1U)                
#define ANT_SWITCH_0_BIT_MASK                    0x01U

//******************************************************************************/
// Description: Bit definitions for register High 32bits in the TimeStamp 
//******************************************************************************/
#define IP_TOA_H32_ID                           (UWB_BASE_ADDR + 0x1a00)            
#define IP_TOA_H32_LEN                          (4U)                
#define IP_TOA_H32_MASK                         0xFFFFFFFFUL

#define IP_TOA_H32_BIT_OFFSET                   (0U)                
#define IP_TOA_H32_BIT_LEN                      (32U)                
#define IP_TOA_H32_BIT_MASK                     0xffffffffU

//******************************************************************************/
// Description: Bit definitions for register Low 32bits in the TimeStamp 
//******************************************************************************/
#define IP_TOA_L8_ID                            (UWB_BASE_ADDR + 0x1a04)            
#define IP_TOA_L8_LEN                           (1U)                
#define IP_TOA_L8_MASK                          0xFF000000UL

#define IP_TOA_L8_BIT_OFFSET                    (24U)                
#define IP_TOA_L8_BIT_LEN                       (8U)                
#define IP_TOA_L8_BIT_MASK                      0xff000000U

//******************************************************************************************/
// Description: Bit definitions for register High 32bits in the TimeStamp  in STS CIA 
//******************************************************************************************/
#define CP_TOA_H32_ID                           (UWB_BASE_ADDR + 0x1a0c)            
#define CP_TOA_H32_LEN                          (4U)                
#define CP_TOA_H32_MASK                         0xFFFFFFFFUL

#define CP_TOA_H32_BIT_OFFSET                   (0)                
#define CP_TOA_H32_BIT_LEN                      (32U)                
#define CP_TOA_H32_BIT_MASK                     0xffffffffU

//******************************************************************************************/
// Description: Bit definitions for register Low 8bit s in the TimeStamp in STS CIA
//******************************************************************************************/
#define CP_TOA_L8_ID                           (UWB_BASE_ADDR + 0x1a10)            
#define CP_TOA_L8_LEN                          (1U)                
#define CP_TOA_L8_MASK                         0xFFFFFFFFUL

#define CP_TOA_L8_BIT_OFFSET                   (24)                
#define CP_TOA_L8_BIT_LEN                      (8U)                
#define CP_TOA_L8_BIT_MASK                     0xff000000U

//******************************************************************************************/
// Description: Bit definitions for register Clock Offset Estimate.
//******************************************************************************************/
#define COE_PPM_ID                           (UWB_BASE_ADDR + 0x1a20)            
#define COE_PPM_LEN                          (4U)                
#define COE_PPM_MASK                         0xFFFFFFFFUL

#define COE_PPM_BIT_OFFSET                   (16)                
#define COE_PPM_BIT_LEN                      (16U)                
#define COE_PPM_BIT_MASK                     0xffff0000U

#define COE_DP_BIT_OFFSET                    (0)                
#define COE_DP_BIT_LEN                       (16U)                
#define COE_DP_BIT_MASK                      0x0000ffffU

//******************************************************************************************/
// Description: Bit definitions for register The number of preamble sequence symbols accumulated 
//******************************************************************************************/
#define IP_NACC0_ID                           (UWB_BASE_ADDR + 0x1a3c)            
#define IP_NACC0_LEN                          (4U)                
#define IP_NACC0_MASK                         0xFFFFFFFFUL

#define IP_NACC0_BIT_OFFSET                   (20)                
#define IP_NACC0_BIT_LEN                      (12U)                
#define IP_NACC0_BIT_MASK                     0xfff00000U

//******************************************************************************************/
// Description: Bit definitions for register Noise Estimation Result
//******************************************************************************************/
#define IP_DELTA_NOISE_ID                     (UWB_BASE_ADDR + 0x1a40)            
#define IP_DELTA_NOISE_LEN                    (4U)                
#define IP_DELTA_NOISE_MASK                   0xFFFFFFFFUL

#define IP_DELTA_NOISE_BIT_OFFSET             (16)                
#define IP_DELTA_NOISE_BIT_LEN                (16U)                
#define IP_DELTA_NOISE_BIT_MASK               0xffff0000U

//******************************************************************************/
// Description: Bit definitions for register Receive buffer
//******************************************************************************/
#define RX_INFO_BUF0_ID                           (UWB_BASE_ADDR + 0x1a60)            
#define RX_INFO_BUF0_LEN                          (4U)                
#define RX_INFO_BUF0_MASK                         0xFFFFFFFFUL

#define RX_INFO_BUF0_RNG_BIT_OFFSET               (31U)                
#define RX_INFO_BUF0_RNG_BIT_LEN                  (1U)                
#define RX_INFO_BUF0_RNG_BIT_MASK                 0x80000000U

#define RX_INFO_BUF0_RXRB_BIT_OFFSET              (29U)                
#define RX_INFO_BUF0_RXRB_BIT_LEN                 (2U)                
#define RX_INFO_BUF0_RXRB_BIT_MASK                0x60000000U

#define RX_INFO_BUF0_RXPSR_BIT_OFFSET             (25U)                
#define RX_INFO_BUF0_RXPSR_BIT_LEN                (4U)                
#define RX_INFO_BUF0_RXPSR_BIT_MASK               0x1e000000U

#define RX_INFO_BUF0_RXFLEN_BIT_OFFSET             (14U)                
#define RX_INFO_BUF0_RXFLEN_BIT_LEN                (10U)                
#define RX_INFO_BUF0_RXFLEN_BIT_MASK               0x00ffc000U

//******************************************************************************/
// Description: Bit definitions for register Receive Time Stamp of No Adjust
//******************************************************************************/
#define RX_STAMP_RAW_ID                           (UWB_BASE_ADDR + 0x1a6c)            
#define RX_STAMP_RAW_LEN                          (4U)                
#define RX_STAMP_RAW_MASK                         0xFFFFFFFFUL

#define RX_STAMP_RAW_BIT_OFFSET                   (0U)                
#define RX_STAMP_RAW_BIT_LEN                      (32U)                
#define RX_STAMP_RAW_BIT_MASK                     0xffffffffU

//******************************************************************************/
// Description: Bit definitions for register Phase Difference Between Two RX
//******************************************************************************/
#define DIF_PHASE_ID                                (UWB_BASE_ADDR + 0x1a70)            
#define DIF_PHASE_LEN                               (4U)                
#define DIF_PHASE_MASK                              0xFFFFFFFFUL

#define DIF_PHASE_BIT_OFFSET                        (0U)                
#define DIF_PHASE_BIT_LEN                           (25U)                
#define DIF_PHASE_BIT_MASK                          0x1ffffffU

//******************************************************************************/
// Description: Bit definitions for register Chunk number in Preamble Detection
//******************************************************************************/
#define CHUNK_NUM0_ID                               (UWB_BASE_ADDR + 0x1a78)            
#define CHUNK_NUM0_LEN                              (4U)                
#define CHUNK_NUM0_MASK                             0xFFFFFFFFUL

#define CHUNK_NUM0_BIT_OFFSET                       (16U)                
#define CHUNK_NUM0_BIT_LEN                          (16U)                
#define CHUNK_NUM0_BIT_MASK                         0xffff0000U

//*******************************************************************************************************************/
// Description: Bit definitions for register The number of preamble sequence symbols accumulated 
//*******************************************************************************************************************/
#define IP_NACC1_ID                                 (UWB_BASE_ADDR + 0x1b18)            
#define IP_NACC1_LEN                                (4U)                
#define IP_NACC1_MASK                               0xFFFFFFFFUL

#define IP_NACC1_BIT_OFFSET                         (20U)                
#define IP_NACC1_BIT_LEN                            (12U)                
#define IP_NACC1_BIT_MASK                           0xfff00000U


//******************************************************************************************************************/
// Description: Bit definitions for register CALI

#define ADDR_0X0818                             (UWB_BASE_ADDR + 0x0818)

#define D_LNA_STAGE2_AGC_BIT_OFFSET             (29U)
#define D_LNA_STAGE2_AGC_BIT_LEN                (2U)
#define D_LNA_STAGE2_AGC_BIT_MASK               0x60000000U

#define ADDR_0X0854                             (UWB_BASE_ADDR + 0x0854)


#define D_LNA_STAGE2_AGC_2ND_BIT_OFFSET         (14U)
#define D_LNA_STAGE2_AGC_2ND_BIT_LEN            (2U)
#define D_LNA_STAGE2_AGC_2ND_BIT_MASK           0xc000U

#define ADDR_0X1028                             (UWB_BASE_ADDR + 0x1028)

#define CAL_WAIT_TIME_BIT_OFFSET                (0U)
#define CAL_WAIT_TIME_BIT_LEN                   (8U)
#define CAL_WAIT_TIME_BIT_MASK                  0xffU

#define I_TIA_AUTO_CALIB_EN_BIT_OFFSET          (24U)
#define I_TIA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define I_TIA_AUTO_CALIB_EN_BIT_MASK            0x1000000U

#define Q_TIA_AUTO_CALIB_EN_BIT_OFFSET          (25U)
#define Q_TIA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define Q_TIA_AUTO_CALIB_EN_BIT_MASK            0x2000000U

#define I_TIA_CALIB_LOAD_OVRD_BIT_OFFSET        (26U)
#define I_TIA_CALIB_LOAD_OVRD_BIT_LEN           (1U)
#define I_TIA_CALIB_LOAD_OVRD_BIT_MASK          0x4000000U

#define Q_TIA_CALIB_LOAD_OVRD_BIT_OFFSET        (27U)
#define Q_TIA_CALIB_LOAD_OVRD_BIT_LEN           (1U)
#define Q_TIA_CALIB_LOAD_OVRD_BIT_MASK          0x8000000U

#define I_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (28U)
#define I_TIA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define I_TIA_AUTO_CALIB_EN_2ND_BIT_MASK        0x10000000U

#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_OFFSET      (29U)
#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_LEN         (1U)
#define Q_TIA_AUTO_CALIB_EN_2ND_BIT_MASK        0x20000000U

#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET    (30U)
#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN       (1U)
#define I_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK      0x40000000U

#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_OFFSET    (31U)
#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_LEN       (1U)
#define Q_TIA_CALIB_LOAD_OVRD_2ND_BIT_MASK      0x80000000U

#define ADDR_0X1030                             (UWB_BASE_ADDR + 0x1030)

#define I_TIA_ATEST_EN_BIT_OFFSET              (16U)
#define I_TIA_ATEST_EN_BIT_LEN                 (1U)
#define I_TIA_ATEST_EN_BIT_MASK                0x10000U

#define I_TIA_KEEP_DCOC_BIT_OFFSET              (17U)
#define I_TIA_KEEP_DCOC_BIT_LEN                 (1U)
#define I_TIA_KEEP_DCOC_BIT_MASK                0x20000U

#define I_TIA_DCOC_EN_BIT_OFFSET                (18U)
#define I_TIA_DCOC_EN_BIT_LEN                   (1U)
#define I_TIA_DCOC_EN_BIT_MASK                  0x40000U

#define Q_TIA_ATEST_EN_BIT_OFFSET              (19U)
#define Q_TIA_ATEST_EN_BIT_LEN                 (1U)
#define Q_TIA_ATEST_EN_BIT_MASK                0x80000U

#define Q_TIA_KEEP_DCOC_BIT_OFFSET              (20U)
#define Q_TIA_KEEP_DCOC_BIT_LEN                 (1U)
#define Q_TIA_KEEP_DCOC_BIT_MASK                0x100000U

#define Q_TIA_DCOC_EN_BIT_OFFSET                (21U)
#define Q_TIA_DCOC_EN_BIT_LEN                   (1U)
#define Q_TIA_DCOC_EN_BIT_MASK                  0x200000U

#define I_TIA_ATEST_EN_2ND_BIT_OFFSET          (22U)
#define I_TIA_ATEST_EN_2ND_BIT_LEN             (1U)
#define I_TIA_ATEST_EN_2ND_BIT_MASK            0x400000U

#define I_TIA_KEEP_DCOC_2ND_BIT_OFFSET          (23U)
#define I_TIA_KEEP_DCOC_2ND_BIT_LEN             (1U)
#define I_TIA_KEEP_DCOC_2ND_BIT_MASK            0x800000U

#define I_TIA_DCOC_EN_2ND_BIT_OFFSET            (24U)
#define I_TIA_DCOC_EN_2ND_BIT_LEN               (1U)
#define I_TIA_DCOC_EN_2ND_BIT_MASK              0x1000000U

#define Q_TIA_ATEST_EN_2ND_BIT_OFFSET          (25U)
#define Q_TIA_ATEST_EN_2ND_BIT_LEN             (1U)
#define Q_TIA_ATEST_EN_2ND_BIT_MASK            0x2000000U

#define Q_TIA_KEEP_DCOC_2ND_BIT_OFFSET          (26U)
#define Q_TIA_KEEP_DCOC_2ND_BIT_LEN             (1U)
#define Q_TIA_KEEP_DCOC_2ND_BIT_MASK            0x4000000U

#define Q_TIA_DCOC_EN_2ND_BIT_OFFSET            (27U)
#define Q_TIA_DCOC_EN_2ND_BIT_LEN               (1U)
#define Q_TIA_DCOC_EN_2ND_BIT_MASK              0x8000000U



#define ADDR_0X1048                             (UWB_BASE_ADDR + 0x1048)

#define I_VGA_AUTO_CALIB_EN_BIT_OFFSET          (0U)
#define I_VGA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define I_VGA_AUTO_CALIB_EN_BIT_MASK            0x1U

#define Q_VGA_AUTO_CALIB_EN_BIT_OFFSET          (4U)
#define Q_VGA_AUTO_CALIB_EN_BIT_LEN             (1U)
#define Q_VGA_AUTO_CALIB_EN_BIT_MASK            0x10U

#define ADDR_0X1054                             (UWB_BASE_ADDR + 0x1054)

#define D_Q_TIA_DCOC_2UA_en_BIT_OFFSET          (7U)
#define D_Q_TIA_DCOC_2UA_en_BIT_LEN             (1U)
#define D_Q_TIA_DCOC_2UA_en_BIT_MASK            0x80U

#define D_I_TIA_DCOC_2UA_en_BIT_OFFSET          (8U)
#define D_I_TIA_DCOC_2UA_en_BIT_LEN             (1U)
#define D_I_TIA_DCOC_2UA_en_BIT_MASK            0x100U

#define D_Q_TIA_DCOC_2UA_en_2nd_BIT_OFFSET      (9U)
#define D_Q_TIA_DCOC_2UA_en_2nd_BIT_LEN         (1U)
#define D_Q_TIA_DCOC_2UA_en_2nd_BIT_MASK        0x200U

#define D_I_TIA_DCOC_2UA_en_2nd_BIT_OFFSET      (10U)
#define D_I_TIA_DCOC_2UA_en_2nd_BIT_LEN         (1U)
#define D_I_TIA_DCOC_2UA_en_2nd_BIT_MASK        0x400U

#define D_RX_CLK38P4_DIV_EN_BIT_OFFSET          (14U)
#define D_RX_CLK38P4_DIV_EN_BIT_LEN             (1U)
#define D_RX_CLK38P4_DIV_EN_BIT_MASK            0x4000U

#define D_LNA_STAGE1_BIAS_SEL_BIT_OFFSET        (20U)
#define D_LNA_STAGE1_BIAS_SEL_BIT_LEN           (3U)
#define D_LNA_STAGE1_BIAS_SEL_BIT_MASK          0x700000U

#define D_LNA_STAGE1_FEED_R_SEL_BIT_OFFSET      (24U)
#define D_LNA_STAGE1_FEED_R_SEL_BIT_LEN         (2U)
#define D_LNA_STAGE1_FEED_R_SEL_BIT_MASK        0x3000000U

#define ADDR_0X1058                             (UWB_BASE_ADDR + 0x1058)

#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_OFFSET    (9U)
#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_LEN       (3U)
#define D_LNA_STAGE1_BIAS_SEL_2ND_BIT_MASK      0xe00U

#define D_Q_VGA_CAL_SEL_BIT_OFFSET              (28U)
#define D_Q_VGA_CAL_SEL_BIT_LEN                 (2U)
#define D_Q_VGA_CAL_SEL_BIT_MASK                0x30000000U

#define D_I_VGA_CAL_SEL_BIT_OFFSET              (30U)
#define D_I_VGA_CAL_SEL_BIT_LEN                 (2U)
#define D_I_VGA_CAL_SEL_BIT_MASK                0xc0000000U




#ifdef __cplusplus
}
#endif

#endif
