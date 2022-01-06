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
}

void boot_fsm_init(boot_fsm_t *handle)
{
    /*Init Interface*/
    handle->iface.crc_accumulated = 0;
    handle->iface.exp_hex_lines = 0;

    /*Clear events*/
    time_event_stop(&handle->event.time.boot_timeout);
    time_event_stop(&handle->event.time.msg_timeout);

    /*Clear event data */
    memset((uint8_t *)&handle->event.external.data.packet, 0, sizeof(packet_data_t));

    /*default entry sequence*/
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

static void entry_action_boot_countdown(boot_fsm_t *handle)
{
    time_event_start(&handle->event.time.cntdown_timeout, COUTDOWN_TIMEOUT_MS);
    time_event_start(&handle->event.time.msg_timeout, COUTDOWN_MSG_TIMEOUT_MS);

    if(get_boot_request_from_flash())
    {
        handle->event.internal.name = ev_int_boot_flag;
    }
}

static void exit_action_boot_countdown(boot_fsm_t *handle)
{
    time_event_stop(&handle->event.time.boot_timeout);
    time_event_stop(&handle->event.time.cntdown_timeout);
}

static void enter_seq_boot_countdown(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ boot countdown ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_countdown);
    entry_action_boot_countdown(handle);
}


static void enter_seq_boot_mode(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ boot mode ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_mode);
    entry_action_seq_boot_mode(handle)
}

static void enter_seq_boot_jump_to_app(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ jump to app ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_jump_to_app);
}

static void enter_seq_boot_start_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ start hex flash ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_start_hex_flash);
}

static void enter_seq_boot_write_hex_line(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ write hex line ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_write_hex_line);
}

static void enter_seq_boot_finish_hex_flash(boot_fsm_t *handle)
{
    boot_dbg("enter seq \t [ finish hex ]\r\n");
    host_comm_rx_fsm_set_next_state(handle, st_boot_finish_hex_flash);
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
        boot_notify_countdown();
    }

    return true;
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
    
    return true;
}

static bool boot_jump_to_app_on_react(boot_fsm_t *handle)
{
    bool did_transition = false;
    
    if(1)
        did_transition = true;
    
    else
        did_transition = false;

    return did_transition; 
}

static bool boot_start_hex_flash_on_react(boot_fsm_t *handle)
{
    bool did_transition = false;
    
    if(1)
        did_transition = true;
    
    else
        did_transition = false;

    return did_transition; 
}

static bool boot_write_hex_line_on_react(boot_fsm_t *handle)
{
    bool did_transition = false;
    
    if(1)
        did_transition = true;
    
    else
        did_transition = false;

    return did_transition; 
}

static bool boot_finish_hex_flash_on_react(boot_fsm_t *handle)
{
    bool did_transition = false;
    
    if(1)
        did_transition = true;
    
    else
        did_transition = false;

    return did_transition; 
}


bool boot_fsm_is_active(const boot_fsm_t *handle);
uint8_t boot_fsm_set_ext_event(boot_fsm_t *handle, boot_external_event_t *event);

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
/*##### Notify Events to Host ###*/
/*###############################*/

static void boot_notify_countdown(void)
{
    packet_data_t packet; 
    packet.header.dir = TARGET_TO_HOST;
    packet.header.payload_len = 0;
    packet.header.type.evt = T2H_EVT_BOOTLOADER_COUNTDOWN;

    host_comm_tx_fsm_send_packet(&host_comm_tx_handle, &packet, MAX_NUM_OF_TRANSFER_RETRIES);
}


/*###############################*/
/*##### Flash Operations  #######*/
/*###############################*/

static bool get_boot_request_from_flash(void)
{
    /*read memory sector in flash containing boot request */
    return false;
}