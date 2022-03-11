/**
 * @brief Jump Function Implementation 
 * 
 */
#include "jump_function.h"

#include "peripherals_init.h"
#include "intel_hex_parser.h"

/**@brief Enable/Disable debug messages */
#define JUMP_FUN_DBG 1
#define JUMP_FUN_DBG_TAG "bl jump : "

/**@brief uart debug function for server comm operations  */
#if JUMP_FUN_DBG
#define print_dbg_message(format, ...) printf(JUMP_FUN_DBG_TAG format, ##__VA_ARGS__)
#else
#define print_dbg_message(format, ...) \
    do                                 \
    { /* Do nothing */                 \
    } while (0)
#endif

void jump_to_new_context(uint32_t start_address)
{
    /* Just a function pointer to hold the address of the reset handler of the applocation. */
    void (*app_reset_handler)(void);
    print_dbg_message("bl debug msg : jump to new context \n");

    /* 1. configure the MSP by reading the value from the base addr of the sector */
    uint32_t msp_value = *(volatile uint32_t *)start_address;
    print_dbg_message("bl debug msg : MSP value : %#x\n", msp_value);

    /* Set Main Stack pointer */
    __set_MSP(msp_value);

    /* 2. Now fetch the reset handler addr of the new context from the location (start_address + 4) */
    uint32_t app_reset_handler_addr = *(volatile uint32_t *)(start_address + 4);
    app_reset_handler = (void *)app_reset_handler_addr;
    print_dbg_message("bl debug msg : app reset handler addr : %#x\n", app_reset_handler);

    /* 3. Jump to Reset Handler of the Application */
    app_reset_handler();
}


void jump_to_bootloader(void)
{
    /* Just a function pointer to hold the address of the reset handler of the bootloader. */
    void (*app_reset_handler)(void);
    print_dbg_message("bl debug msg :  jump to bootloader\n");

    /* 1. configure the MSP by reading the value from the base addr of the sector */
    uint32_t msp_value = *(volatile uint32_t *)FLASH_BL_START_ADDR;
    print_dbg_message("bl debug msg : MSP value : %#x\n", msp_value);

    SysTick->CTRL = 0x0; //Disables SysTick timer and its related interrupt
    HAL_DeInit();

    RCC->CIR = 0x00000000; //Disable all interrupts related to clock

    uint32_t *pulSRAMBase = (uint32_t *)SRAM_BASE;
    uint32_t *pulFlashBase = (uint32_t *)FLASH_BL_START_ADDR;
    uint16_t i = 0;

    do
    {
        if (pulFlashBase[i] == 0xAABBCCDD)
            break;
        pulSRAMBase[i] = pulFlashBase[i];
    } while (++i);

    /* Set Main Stack pointer */
    __set_MSP(msp_value);

    SYSCFG->CFGR1 |= 0x3; /* __HAL_RCC_SYSCFG_CLK_ENABLE() already called from HAL_MspInit() */

    /* 2. Now fetch the reset handler addr of the bootloader from the location (FLASH_USER_APP_START_ADDR + 4) */
    uint32_t app_reset_handler_addr = *(volatile uint32_t *)(FLASH_BL_START_ADDR + 4);
    app_reset_handler = (void *)app_reset_handler_addr;
    print_dbg_message("bl debug msg : app reset handler addr : %#x\n", app_reset_handler);

    /* 3. Jump to Reset Handler of the Application */
    app_reset_handler();
}

uint8_t check_user_app_integrity(void)
{
    uint32_t start_addr = FLASH_USER_APP_START_ADDR;
    uint32_t user_app_len = flash_read_user_app_len();
    uint32_t user_app_crc = flash_read_user_app_crc();

    print_dbg_message("user app -> len [0x%X] crc [0x%X]\r\n", user_app_len, user_app_crc);

    if (user_app_len == 0xFFFFFFFF || (user_app_len + start_addr) > FLASH_BANK1_END)
    {
        print_dbg_message("invalid len [0x%X]\r\n", user_app_len, user_app_crc);
        return 0;
    }

    print_dbg_message("calculating crc from [0x%X] to [0x%X]\r\n", start_addr, start_addr + user_app_len);
    uint32_t crc_calculated = flash_get_crc32b(start_addr, start_addr + user_app_len);
    if (user_app_crc == crc_calculated)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief Verify the integrity of the User app and jump if succeed
 * @return return 0 if fail.
 * 
 */
uint8_t jump_to_user_app(void)
{
	uint32_t start_addr = FLASH_USER_APP_START_ADDR;
    uint32_t user_app_len = flash_read_user_app_len();
    uint32_t user_app_crc = flash_read_user_app_crc();

    print_dbg_message("user app -> len [0x%X] crc [0x%X]\r\n", user_app_len, user_app_crc);

    if (user_app_len == 0xFFFFFFFF || (user_app_len + start_addr) > FLASH_BANK1_END)
    {
        print_dbg_message("invalid len [0x%X]\r\n", user_app_len, user_app_crc);
        return 0;
    }

    print_dbg_message("calculating crc from [0x%X] to [0x%X]\r\n", start_addr, start_addr + user_app_len);
    uint32_t crc_calculated = flash_get_crc32b(start_addr, start_addr + user_app_len);
    if (user_app_crc == crc_calculated)
    {

        print_dbg_message("crc in flash and crc calculated match [0x%X]\r\n", crc_calculated);

        /* 1. configure the MSP by reading the value from the base addr of the sector */
        uint32_t msp_value = *(volatile uint32_t *)FLASH_USER_APP_START_ADDR;
        print_dbg_message("bl debug msg : MSP value : %#x\r\n", msp_value);

        void (*app_reset_handler)(void) = NULL;

        /* 2. Now fetch the reset handler addr of the user app from the location (FLASH_USER_APP_START_ADDR + 4) */
        uint32_t app_reset_handler_addr = *(volatile uint32_t *)(FLASH_USER_APP_START_ADDR + 4);
        app_reset_handler = (void *)app_reset_handler_addr;
        print_dbg_message("bl debug msg : app reset handler addr : %#x\r\n", app_reset_handler);
        print_dbg_message("bl debug msg : jump to user app\r\n");

        HAL_Delay(500);
        peripherals_deinit();
        /* Set Main Stack pointer */
        __set_MSP(msp_value);
        /* 3. Jump to Reset Handler of the Application */
        app_reset_handler();
    }
    else
    {
        print_dbg_message("crc mismatch : flash -> [0x%X] != calculated [0x%X]\r\n", user_app_crc, crc_calculated);
    }

    return 0 ;
}

void remap_vector_table_to_flash(void)
{
	uint32_t *pulFlashOrigin = (uint32_t *)FLASH_BASE;
	uint32_t *pulFlashBase = (uint32_t *)FLASH_BL_START_ADDR;
	uint16_t i = 0;

	do{
		if (pulFlashBase[i] == 0xAABBCCDD)
			break;
		pulFlashOrigin[i] = pulFlashBase[i];
	} while (++i);
	__disable_irq();
	__HAL_SYSCFG_REMAPMEMORY_FLASH();
	__enable_irq();
}
