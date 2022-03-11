/**
 * @file jump_function.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief  Jump function to User application
 * @version 0.1
 * @date 2020-09-04
 * @copyright Copyright (c) 2020
 */

#ifndef JUMP_FUNCTION_H
#define JUMP_FUNCTION_H

#include "stdint.h"
#include "stdio.h"
#include "stm32f0xx_hal.h"

/* Flash Sector Size is 4K */
#define FLASH_SECTOR_SIZE           ((uint32_t)(4 * FLASH_PAGE_SIZE))

/**
 * @brief Select MCU and make sure the address selected match the Linker File Addresses for BL and User app.
 * 
 */
#ifdef STM32F030xC
#define FLASH_USER_APP_START_PAGE        (13)   //-> 0x0800_0000 + 13*0x800  = 0x0800_6800
#define FLASH_USER_APP_END_PAGE          (125)  //-> 0x0800_0000 + 125*0x800 = 0x0803_E800
#define FLASH_BOOTLOADER_START_PAGE      (0)    //-> 0x0800_0000 + 0*0x800   = 0x0800_0000
#define FLASH_BOOTLOADER_END_PAGE        (12)   //-> 0x0800_0000 + 12*0x800  = 0x0800_6000
#define FLASH_BL_USER_APP_INFO_PAGE      (126)  //-> 0x0800_0000 + 126*0x800 = 0x0803_F000
#define FLASH_BL_BOOT_REQ_FLG_PAGE       (127)  //-> 0x0800_0000 + 127*0x800 = 0x0803_F800

#else
#define FLASH_USER_APP_START_PAGE        (25)   //-> 0x0800_0000 + 25*0x400  = 0x0800_6400      
#define FLASH_USER_APP_END_PAGE          (61)   //-> 0x0800_0000 + 61*0x400  = 0x0800_F400
#define FLASH_BOOTLOADER_START_PAGE      (0)    //-> 0x0800_0000 + 0*0x400   = 0x0800_0000
#define FLASH_BOOTLOADER_END_PAGE        (23)   //-> 0x0800_0000 + 23*0x400  = 0x0800_5C00  
#define FLASH_BL_USER_APP_INFO_PAGE      (62)   //-> 0x0800_0000 + 62*0x400  = 0x0800_F800  
#define FLASH_BL_BOOT_REQ_FLG_PAGE       (63)   //-> 0x0800_0000 + 63*0x400  = 0x0800_FC00   

#endif

/* Bootloader reserves sectors [0 - 3] */
#define FLASH_USER_APP_START_ADDR    ((uint32_t)(FLASH_BASE + FLASH_USER_APP_START_PAGE * FLASH_PAGE_SIZE)) 
#define FLASH_USER_APP_END_ADDR      ((uint32_t)(FLASH_BASE + FLASH_USER_APP_END_PAGE * FLASH_PAGE_SIZE))  

/* Bootloader starts at 0x08000000 */
#define FLASH_BL_START_ADDR          ((uint32_t)(FLASH_BASE + FLASH_BOOTLOADER_START_PAGE * FLASH_PAGE_SIZE))
#define FLASH_BL_END_ADDR            ((uint32_t)(FLASH_BASE + FLASH_BOOTLOADER_END_PAGE * FLASH_PAGE_SIZE))

/* Reserve last page to store CRC and data Len Page 63*/
#define FLASH_BL_CRC_ADDR            ((uint32_t)FLASH_BASE + FLASH_BL_USER_APP_INFO_PAGE * FLASH_PAGE_SIZE)
#define FLASH_BL_DATA_LEN_ADDR       (FLASH_BL_CRC_ADDR + 4)

/* Reserve last page to store CRC and data Len Page 64*/
#define FLASH_BL_BOOT_REQ_FLG_ADDR   ((uint32_t)FLASH_BASE + FLASH_BL_BOOT_REQ_FLG_PAGE * FLASH_PAGE_SIZE)


/* Exported Functions */
uint8_t jump_to_user_app(void);
void jump_to_bootloader(void);
void jump_to_new_context(uint32_t start_address);
void remap_vector_table_to_flash(void);
uint8_t check_user_app_integrity(void);




#endif
