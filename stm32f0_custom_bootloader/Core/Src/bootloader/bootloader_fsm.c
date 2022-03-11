#include "bootloader_fsm.h"
#include "led_animation.h"
#include "string.h"
#include "host_comm_tx_fsm.h"
#include "jump_function.h"

/**@brief Enable/Disable debug messages */
#define BOOT_DEBUG 0
#define BOOT_TAG "boot : "

/**@brief uart debug function for server comm operations  */
#if BOOT_DEBUG
#define boot_dbg(format, ...) printf(BOOT_TAG format, ##__VA_ARGS__)
#else
#define boot_dbg(format, ...) \
	do                                       \
	{ /* Do nothing */                       \
	} while (0)
#endif

#define send_packet_np(packet) host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, \
                                                                       packet,               \
                                                                       MAX_NUM_OF_TRANSFER_RETRIES);

/**@ Extern handle for bootloader */
boot_fsm_t boot_handle;


/*STATE 2*/
static bool boot_countdown_on_react(boot_fsm_t *handle);
static bool boot_mode_on_react(boot_fsm_t *handle);
static bool boot_jump_to_app_on_react(boot_fsm_t *handle);
static bool boot_start_hex_flash_on_react(boot_fsm_t *handle);
static bool boot_write_hex_line_on_react(boot_fsm_t *handle);
static bool boot_finish_hex_flash_on_react(boot_fsm_t *handle);


static void enter_seq_boot_countdown(boot_fsm_t *handle);
static void enter_seq_boot_mode(boot_fsm_t *handle);
static void enter_seq_boot_jump_to_app(boot_fsm_t *handle);
static void enter_seq_boot_start_hex_flash(boot_fsm_t *handle);
static void enter_seq_boot_write_hex_line(boot_fsm_t *handle);
static void enter_seq_boot_finish_hex_flash(boot_fsm_t *handle);

static uint8_t is_computed_app_integrity_ok(boot_fsm_t *handle);
static void boot_save_integrity_status(boot_fsm_t *handle);
static uint8_t process_hex_line(boot_fsm_t *handle);
static uint8_t parse_hex_line(boot_fsm_t *handle);


static void hex_record_data_handler(boot_fsm_t *handle);
static void hex_record_end_of_file_handler(boot_fsm_t *handle);
static void hex_record_extended_linear_addr_handler(boot_fsm_t *handle);


static void boot_fsm_set_next_state(boot_fsm_t *handle, boot_states_t next_state)
{
	handle->state = next_state;
	handle->event.internal.name = ev_int_boot_invalid;
	handle->event.external.name = ev_ext_boot_invalid;
    handle->event.external.data.packet = NULL;
}

void boot_fsm_init(boot_fsm_t *handle)
{
    /*Init Interface*/
    handle->iface.hex_file.total_lines = 0;
    handle->iface.hex_file.line.idx = 0;
    memset((uint8_t *)&handle->iface.hex_file.line.str, 0, MAX_HEX_FRAME_LEN);
    handle->iface.flash.addr_offset.word = 0x00;
    handle->iface.flash.base_addr.word = 0x00;
    handle->iface.flash.app_crc32 = 0x00;
    handle->iface.flash.byte_cnt = 0x00;

    /*Clear events*/
    time_event_stop(&handle->event.time.boot_timeout);
    time_event_stop(&handle->event.time.msg_timeout);

    /*default entry sequence*/
    enter_seq_boot_countdown(handle);
}

void boot_fsm_run(boot_fsm_t *handle)
{
    switch (handle->state)
    {
    case st_boot_countdown:        boot_countdown_on_react(handle);     
    case st_boot_mode:             boot_mode_on_react(handle);  
    case st_boot_jump_to_app:      boot_jump_to_app_on_react(handle);   
    case st_boot_start_hex_flash:  boot_start_hex_flash_on_react(handle);   
    case st_boot_write_hex_line:   boot_write_hex_line_on_react(handle);    
    case st_boot_finish_hex_flash: boot_finish_hex_flash_on_react(handle);   
    
    default:
        break;
    }
}

/*####################################################################################################################*/
/*##### Boot Countdown State #########################################################################################*/
/*####################################################################################################################*/

static void enter_seq_boot_countdown(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ boot countdown ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_countdown);
    entry_action_boot_countdown(handle);
}

static void entry_action_boot_countdown(boot_fsm_t *handle)
{
    time_event_start(&handle->event.time.cntdown_timeout, COUTDOWN_TIMEOUT_MS);
    time_event_start(&handle->event.time.msg_timeout, COUTDOWN_MSG_TIMEOUT_MS);

    if(get_boot_flag_from_flash())
    {
        handle->event.internal.name = ev_int_boot_flag;
    }
}

static void exit_action_boot_countdown(boot_fsm_t *handle)
{
    time_event_stop(&handle->event.time.boot_timeout);
    time_event_stop(&handle->event.time.cntdown_timeout);
}

static bool boot_countdown_on_react(boot_fsm_t *handle)
{
    boot_event_t *ev = &handle->event;

    if (ev->external.name == ev_ext_boot_enter ||
        ev->internal.name == ev_int_boot_flag)
    {
        exit_action_boot_countdown(handle);
        enter_seq_boot_mode(handle);
    }

    else if (ev->external.name == ev_ext_boot_exit ||
             time_event_is_raised(&ev->time.cntdown_timeout) == true)
    {
        exit_action_boot_countdown(handle);
        enter_seq_boot_jump_to_app(handle);
    }

    else if(time_event_is_raised(&ev->time.msg_timeout) == true)
    {
        /*reload time event */
        time_event_start(&ev->time.msg_timeout, COUTDOWN_MSG_TIMEOUT_MS);
        /*notify Boot Countdown */
        send_packet_np(T2H_EVT_BOOTLOADER_COUNTDOWN);
    }

    return true;
}

/*####################################################################################################################*/
/*##### Boot Mode State ##############################################################################################*/
/*####################################################################################################################*/

static void enter_seq_boot_mode(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ boot mode ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_mode);
    entry_action_seq_boot_mode(handle);
}

static void entry_action_seq_boot_mode(boot_fsm_t *handle)
{
    time_event_start(&handle->event.time.boot_timeout, BOOT_TIMEOUT_MS);

}
static void exit_action_seq_boot_mode(boot_fsm_t *handle)
{
    time_event_stop(&handle->event.time.boot_timeout);
}

static bool boot_mode_on_react(boot_fsm_t *handle)
{
    boot_event_t *ev = &handle->event;
    
    if(ev->external.name == ev_ext_boot_start_hex)
    {
        exit_action_seq_boot_mode(handle);
        enter_seq_boot_start_hex_flash(handle);
    }
    else if(ev->external.name == ev_ext_boot_proc_line)
    {
        exit_action_seq_boot_mode(handle);
        enter_seq_boot_write_hex_line(handle);
    }
    else if(ev->external.name == ev_ext_boot_finish_hex)
    {
        exit_action_seq_boot_mode(handle);
        enter_seq_boot_finish_hex_flash(handle);
    }
    else if (time_event_is_raised(&ev->time.boot_timeout) == true ||
             ev->external.name == ev_ext_boot_exit)
    {
        exit_action_seq_boot_mode(handle);
        enter_seq_boot_jump_to_app(handle);
    }

    return true;
}

/*####################################################################################################################*/
/*##### Jump to App state ############################################################################################*/
/*####################################################################################################################*/

static void enter_seq_boot_jump_to_app(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ jump to app ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_jump_to_app);
    entry_action_jump_to_app(handle);
}

static void entry_action_jump_to_app(boot_fsm_t *handle)
{
    /*Check user app integrity */
    if (is_stored_app_integrity_ok(handle))
    {
        handle->event.internal.name = ev_int_boot_integrity_ok;
        send_packet_np(T2H_EVT_USER_APP_INTEGRITY_OK);
        send_packet_np(T2H_EVT_JUMP_TO_USER_APP);
    }
    else
    {
        handle->event.internal.name = ev_int_boot_integrity_fail;
        send_packet_np(T2H_EVT_USER_APP_INTEGRITY_FAIL);
        send_packet_np(T2H_EVT_JUMP_TO_USER_APP_FAIL);
    }
}

static bool boot_jump_to_app_on_react(boot_fsm_t *handle)
{
    boot_event_t *ev = &handle->event;
    if(ev->internal.name = ev_int_boot_integrity_ok)
    {
        /*Transition Action */
        boot_jump_to_user_app(handle);

        /*this line will never be reached*/
    }
    else if(ev->internal.name = ev_int_boot_integrity_fail)
    {
        enter_seq_boot_countdown(handle);
    }

    return true;
}

/*####################################################################################################################*/
/*##### Jump to App state ############################################################################################*/
/*####################################################################################################################*/
static void enter_seq_boot_start_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ start hex flash ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_start_hex_flash);
    entry_action_start_hex_flash(handle);
}

static void entry_action_start_hex_flash(boot_fsm_t *handle)
{
    /*Clear previous user app in flash */
    boot_clear_user_app(handle);
    handle->iface.hex_file.line.idx = 0;
}


static bool boot_start_hex_flash_on_react(boot_fsm_t *handle)
{
    enter_seq_boot_mode(handle);
    return true;
}

/*####################################################################################################################*/
/*##### Finish Hex Flash State #######################################################################################*/
/*####################################################################################################################*/
static void enter_seq_boot_finish_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ finish hex ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_finish_hex_flash);
    entry_action_finish_hex_flash(handle);
}

static void entry_action_finish_hex_flash(boot_fsm_t *handle)
{
    /* compute crc */
    if( is_computed_app_integrity_ok(handle) )
    {
        /*Store calculated crc, and data len in flash */
        boot_save_integrity_status(handle);
        send_packet_np(T2H_RES_FINISH_HEX_FLASH_OK);
        handle->event.internal.name = ev_int_boot_hex_file_succeed;
    }
    else
    {
        send_packet_np(T2H_RES_FINISH_HEX_FLASH_FAIL);
        handle->event.internal.name = ev_int_boot_hex_file_fail;
    }
}

static bool boot_finish_hex_flash_on_react(boot_fsm_t *handle)
{
    boot_event_t *ev = &handle->event;
    if(ev->internal.name == ev_int_boot_hex_file_succeed)
    {
        enter_seq_boot_jump_to_app(handle);
    }
    else if(ev->internal.name == ev_int_boot_hex_file_fail)
    {
        enter_seq_boot_mode(handle);
    }
    return true;
}

/*####################################################################################################################*/
/*##### Write Hex Line State #########################################################################################*/
/*####################################################################################################################*/

static void entry_action_write_hex_line(boot_fsm_t *handle)
{
    /*write new line in flash */ 
    if(process_hex_line(handle))
    {
        send_packet_np(T2H_RES_HEX_LINE_OK);
    }
    else
    {
        send_packet_np(T2H_RES_HEX_LINE_FAIL);
    }
}

static void enter_seq_boot_write_hex_line(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ write hex line ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_write_hex_line);
}

static bool boot_write_hex_line_on_react(boot_fsm_t *handle)
{
    enter_seq_boot_mode(handle);
    return true;
}

/*####################################################################################################################*/
/*##### End States####################################################################################################*/
/*####################################################################################################################*/


bool boot_fsm_is_active(const boot_fsm_t *handle);

uint8_t boot_fsm_set_ext_event(boot_fsm_t *handle, boot_external_event_t *event)
{
    handle->event.external.name = event->name;
    packet_data_t *packet = event->data.packet;
    
    switch (event->name)
    {
    case ev_ext_boot_exit: break;
    case ev_ext_boot_enter: break;
    case ev_ext_boot_finish_hex: break;
    case ev_ext_boot_start_hex:
    {
        handle->iface.hex_file.total_lines = packet->payload.h2t.start_hex_flash.total_lines;
    }
    break;

    case ev_ext_boot_proc_line:
    {
        memcpy((uint8_t *)&handle->iface.hex_file.line.str, packet->payload.h2t.proc_hex_line.line_str,
               packet->header.payload_len);
    }
    break;

    default:
        break;
    }
}

void boot_fsm_fsm_time_event_update(boot_fsm_t *handle)
{
	time_event_t *time_event = (time_event_t *)&handle->event.time;
	for (int tev_idx = 0; tev_idx < sizeof(handle->event.time) / sizeof(time_event_t); tev_idx++)
	{
		time_event_update(time_event);
		time_event++;
	}
}

/*###############################*/
/*##### Flash Operations  #######*/
/*###############################*/

static bool get_boot_flag_from_flash(void)
{
    /*read memory sector in flash containing boot request */
    return false;
}

static bool is_stored_app_integrity_ok(boot_fsm_t *handle)
{  
    // uint32_t app_len = flash_get_user_app_data_len();
    // uint32_t app_crc = flash_get_user_app_crc();
    // uint32_t exp_crc = handle->iface.exp_crc;

    // if(app_crc == exp_crc)
    //     return true;
    // else 
    //     return false;
}

static uint8_t is_computed_app_integrity_ok(boot_fsm_t *handle)
{
    // uint32_t app_crc = compute_app_crc(start_addr, data_len);
    // uint32_t exp_crc = handle->iface.exp_crc;

    // if(app_crc == exp_crc)
    //     return true;
    // else 
    //     return false;
}

static void boot_save_integrity_status(boot_fsm_t *handle)
{
    /*store crc and data len */
}

static void boot_jump_to_user_app(boot_fsm_t *handle)
{
    if(!jump_to_user_app())
    {
        handle->event.internal.name = ev_int_boot_integrity_fail;
    }

    return true;
}

static void boot_clear_user_app(boot_fsm_t *handle)
{
    /*clear user app */
    
    /*erase crc32 and data len */
    handle->iface.flash.addr_offset.word = 0x00;
    handle->iface.flash.base_addr.word = 0x00;
    handle->iface.flash.app_crc32 = 0x00;
}

static uint8_t process_hex_line(boot_fsm_t *handle)
{
    /*Process hex line, update control data */
    if(hex_line_str_to_intel_frame(handle->iface.hex_file.line.str, &handle->iface.hex_file.line.hex))
    {
        boot_dbg("\thex file ok\r\n");
        parse_hex_line(handle);
        return 1;
    }
    return 0;
}

static void hex_record_data_handler(boot_fsm_t *handle)
{
    intel_hex_frame_t *line_hex = &handle->iface.hex_file.line.hex;

    handle->iface.flash.addr_offset.word = (line_hex->addr_offset);
    uint32_t dest_flash_address = handle->iface.flash.base_addr.word + handle->iface.flash.addr_offset.word;
    boot_dbg("write line -> addr offset : [0x%.8lX]\r\n", handle->iface.flash.addr_offset.word);

    uint8_t data_len = line_hex->byte_count / sizeof(uint32_t);
    handle->iface.flash.byte_cnt += line_hex->byte_count;
    boot_dbg("write line -> dest addr : [0x%.8lX]\r\n", dest_flash_address);

    HAL_FLASH_Unlock();
    /* Write Bytes in flash */
    for (size_t i = 0; i < data_len; i++)
    {
        boot_dbg("dest addr : [0x%.8lX] | data : [0x%.8lX]\r\n", dest_flash_address, line_hex->data[i].word);
        flash_write_word(dest_flash_address, line_hex->data[i].word);
        dest_flash_address += 4;
    }
    HAL_FLASH_Lock();

    /* Update User App Flash CRC*/
    sw_crc32_accumulate(&line_hex->data[0].word, data_len, &handle->iface.flash.app_crc32);

    return 1;
}

static void hex_record_end_of_file_handler(boot_fsm_t *handle)
{
    /* check the CRC from flash */
    uint32_t total_app_len = handle->iface.flash.base_addr.word + handle->iface.flash.byte_cnt;
    boot_dbg("calculating crc from -> [0x%.8lX] to [0x%.8lX] \r\n",
             (uint8_t *)FLASH_USER_APP_START_ADDR, handle->iface.flash.base_addr.word);

    uint32_t crc32 = flash_get_crc32b((uint32_t)FLASH_USER_APP_START_ADDR, total_app_len);

    if (handle->iface.flash.app_crc32 == crc32 )
    {
        boot_dbg("crc accumulated and calculated matched-> [0x%.8lX]\r\n", crc32);
        uint32_t app_len = (handle->iface.flash.base_addr.word - FLASH_USER_APP_START_ADDR);
        HAL_Delay(10);

        __disable_irq();
        /* Write CRC and App length to flash */
        flash_clear_crc32_and_len_page();

        HAL_FLASH_Unlock();
        flash_write_word(FLASH_BL_CRC_ADDR, handle->iface.flash.app_crc32);
        flash_write_word(FLASH_BL_DATA_LEN_ADDR, app_len);
        HAL_FLASH_Lock();

        __enable_irq();

        boot_dbg("crc stored at [0x%.8lX] -> val [0x%.8lX]\r\n",
                          (uint32_t *)FLASH_BL_CRC_ADDR, flash_read_user_app_crc());
        boot_dbg("data len  stored at [0x%.8lX] -> val [0x%.8lX]\r\n",
                          (uint32_t *)FLASH_BL_DATA_LEN_ADDR, flash_read_user_app_len());
        return 1;
    }
    else 
    {
        boot_dbg("crc not match-> accumulated [0x%.8lX] != calculated [0x%.8lX]\r\n",
                 handle->iface.flash.app_crc32, crc32);
        flash_clear_crc32_and_len_page();
    }

    return 0;
}

static void hex_record_extended_linear_addr_handler(boot_fsm_t *handle)
{
    /* Get Flash Base addres MSB */
    intel_hex_frame_t *line_hex = &handle->iface.hex_file.line.hex;
    handle->iface.flash.base_addr.word = hex_line_endian_swap_32(line_hex->data[0].word);
    handle->iface.flash.addr_offset.word = 0x00;
    boot_dbg("hex file base flash addr [0x%lX]\r\n", handle->iface.flash.base_addr.word);
    return 1;
}


static uint8_t parse_hex_line(boot_fsm_t *handle)
{
    hex_line_dbg("parsing hex frame ...\r\n");
    record_type_t record_type = handle->iface.hex_file.line.hex.record_type;

    switch (record_type)
    {

    case HEX_RECORD_DATA:
    {
        hex_line_dbg("data \r\n");
        hex_record_data_handler(handle);
    }
    break;

    case HEX_RECORD_END_OF_FILE:
    {
        hex_line_dbg("end of file\r\n");
        hex_record_end_of_file_handler(handle);
    }
    break;

    case HEX_RECORD_EXTENDED_LINEAR_ADDR:
    {
        hex_line_dbg("extended linear addr \r\n");
        hex_record_extended_linear_addr_handler(handle);
    }
    break;

    case HEX_RECORD_EXTENDED_SEG_ADDR:
    case HEX_RECORD_START_SEG_ADDR:
    case HEX_RECORD_START_LINEAR_ADDR:
    {
        boot_dbg("\t record type [0x%X] - not supported\r\n", record_type);
    }
    break;

    default:
        break;
    }
    return 1;
}