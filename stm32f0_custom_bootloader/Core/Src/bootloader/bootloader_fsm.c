#include "bootloader_fsm.h"
#include "led_animation.h"
#include "string.h"
#include "host_comm_tx_fsm.h"

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
    handle->iface.hex_file.crc32 = 0;
    handle->iface.hex_file.len_bytes = 0;
    handle->iface.hex_file.line_cnt = 0;
    memset((uint8_t *)&handle->iface.hex_file.line, 0, MAX_HEX_FRAME_LEN);

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
        boot_jump_to_user_app();

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
static void entry_action_start_hex_flash(boot_fsm_t *handle)
{
    /*Clear previous user app in flash */
    boot_clear_user_app();

    /*Store expected crc, and hex lines to be processed */
    handle->iface.app_len_bytes = 
}

static void enter_seq_boot_start_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ start hex flash ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_start_hex_flash);
    entry_action_start_hex_flash(handle);
}

static bool boot_start_hex_flash_on_react(boot_fsm_t *handle)
{
    enter_seq_boot_mode(handle);
    return true;
}

/*####################################################################################################################*/
/*##### Finish Hex Flash State #######################################################################################*/
/*####################################################################################################################*/

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

static void enter_seq_boot_finish_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ finish hex ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_finish_hex_flash);
    entry_action_finish_hex_flash(handle);
}

static bool boot_finish_hex_flash_on_react(boot_fsm_t *handle)
{
    boot_event_t *ev = &handle->event;
    if(ev->internal.name == ev_int_boot_hex_file_succeed)
    {
        enter_seq_boot_mode(handle);
    }
    else if(ev->internal.name == ev_int_boot_hex_file_fail)
    {
        enter_seq_boot_jump_to_app(handle);
    }
    return true;
}

/*####################################################################################################################*/
/*##### Write Hex Line State #########################################################################################*/
/*####################################################################################################################*/

static void entry_action_write_hex_line(boot_fsm_t *handle)
{
    /*write new line in flash */
    if(process_hex_line())
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
        handle->iface.hex_file.crc32 = packet->payload.h2t.start_hex_flash.crc32;
        handle->iface.hex_file.len_bytes = packet->payload.h2t.start_hex_flash.len_bytes;
        handle->iface.hex_file.line_cnt = packet->payload.h2t.start_hex_flash.line_cnt;
    }
    break;

    case ev_ext_boot_proc_line:
    {
        memcpy((uint8_t *)&handle->iface.hex_file.line, packet->payload.h2t.proc_hex_line.line,
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

static void is_computed_app_integrity_ok(boot_fsm_t *handle)
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

static void boot_jump_to_user_app(void)
{

    return true;
}

static void boot_clear_user_app(void)
{
    /*clear user app */
    
    /*erase crc32 and data len */
}

static void process_hex_line(void)
{

}