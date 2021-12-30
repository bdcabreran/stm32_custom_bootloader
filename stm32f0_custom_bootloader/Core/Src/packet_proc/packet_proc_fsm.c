#include "packet_proc_fsm.h"
#include "version.h"

/**@brief Enable/Disable debug messages */
#define PACKET_PROC_DEBUG 0
#define PACKET_PROC_TAG "packet proc : "

/**@brief uart debug function for server comm operations  */
#if PACKET_PROC_DEBUG
#define packet_proc_dbg(format, ...) printf(PACKET_PROC_TAG format, ##__VA_ARGS__)
#else
#define packet_proc_dbg(format, ...) \
	do                                       \
	{ /* Do nothing */                       \
	} while (0)
#endif

packet_proc_fsm_t packet_proc_handle;

static bool st_packet_proc_idle_on_react(packet_proc_fsm_t *handle);
static bool st_packet_proc_host_res_on_react(packet_proc_fsm_t *handle);
static bool st_packet_proc_host_cmd_on_react(packet_proc_fsm_t *handle);
static bool st_packet_proc_host_evt_on_react(packet_proc_fsm_t *handle);


/*##################################*/
/*#### Host Command Handlers #######*/
/*##################################*/
static void host_cmd_get_fw_ver_handler(void);
static void host_cmd_enter_boot_handler(packet_data_t *packet);      
static void host_cmd_exit_boot_handler(packet_data_t *packet);       
static void host_cmd_start_hex_flash_handler(packet_data_t *packet); 
static void host_cmd_proc_hex_line_handler(packet_data_t *packet);   
static void host_cmd_finish_hex_flash_handler(packet_data_t *packet);

/*##################################*/
/*#### Host Response Handlers #######*/
/*##################################*/
static void host_res_ack_handler(packet_data_t * packet);
static void host_res_nack_handler(packet_data_t * packet);


/*##################################*/
/*#### Host Events  Handlers #######*/
/*##################################*/


static void packet_proc_fsm_set_next_state(packet_proc_fsm_t *handle, packet_proc_states_t next_state)
{
	handle->state = next_state;
	handle->event.external.name = ev_ext_packet_proc_invalid;
}

static void entry_action_packet_proc_idle(packet_proc_fsm_t *handle)
{
    /*notify rx fsm the packet is processed and the fsm is ready for incoming packets*/
    host_comm_rx_fsm_set_ext_event(&host_comm_rx_handle, ev_ext_comm_rx_packet_proccessed);
}

static void enter_seq_packet_proc_idle(packet_proc_fsm_t *handle)
{
    packet_proc_dbg("enter seq \t[ st_packet_proc_idle ]\r\n");
    packet_proc_fsm_set_next_state(handle, st_packet_proc_idle);
    entry_action_packet_proc_idle(handle);
}

static void enter_seq_packet_proc_host_res(packet_proc_fsm_t *handle)
{
    packet_proc_dbg("enter seq \t[ st_proc_host_res ]\r\n");
    packet_proc_fsm_set_next_state(handle, st_packet_proc_host_res);

}

static void enter_seq_packet_proc_host_cmd(packet_proc_fsm_t *handle)
{
    packet_proc_dbg("enter seq \t[ st_proc_host_cmd ]\r\n");
    packet_proc_fsm_set_next_state(handle, st_packet_proc_host_cmd);
}

static void enter_seq_packet_proc_host_evt(packet_proc_fsm_t *handle)
{
    packet_proc_dbg("enter seq \t[ st_proc_host_evt ]\r\n");
    packet_proc_fsm_set_next_state(handle, st_packet_proc_host_evt);

}

static void host_comm_rx_fsm_enter(packet_proc_fsm_t *handle)
{
    enter_seq_packet_proc_idle(handle);
}

void packet_proc_fsm_init(packet_proc_fsm_t *handle)
{
    /*Clean event data*/
    handle->event.external.name = ev_ext_packet_proc_new;
    handle->event.external.data.packet = NULL;
    handle->state = st_packet_proc_invalid;

    /*Default Enter Sequence*/
	host_comm_rx_fsm_enter(handle);
}

void packet_proc_fsm_run(packet_proc_fsm_t *handle)
{
    switch (handle->state)
    {
    case st_packet_proc_idle:   st_packet_proc_idle_on_react(handle);   break;
    case st_packet_proc_host_cmd: st_packet_proc_host_cmd_on_react(handle); break;
    case st_packet_proc_host_res: st_packet_proc_host_res_on_react(handle); break;
    case st_packet_proc_host_evt: st_packet_proc_host_evt_on_react(handle); break;
    
    default:
        break;
    }
}

static bool st_packet_proc_idle_on_react(packet_proc_fsm_t *handle)
{
    bool did_transition = false;

    if (handle->event.external.name == ev_ext_packet_proc_new)
    {
        /*transition Action */
        packet_data_t *packet = handle->event.external.data.packet;

        if (IS_H2T_CMD(packet->header.type.cmd))
            enter_seq_packet_proc_host_cmd(handle);
        
        else if (IS_H2T_RES(packet->header.type.res))
            enter_seq_packet_proc_host_res(handle);
        
        else if (IS_H2T_EVT(packet->header.type.evt))
            enter_seq_packet_proc_host_evt(handle);
        
        else
            packet_proc_dbg("unknown type of packet \r\n");

        /*send ACK to host PC */
        host_comm_tx_fsm_send_packet_no_payload(&host_comm_tx_handle, T2H_RES_ACK, false);    
        
        did_transition = true;
    }
    else
        did_transition = false;

    return did_transition;
}

static bool st_packet_proc_host_res_on_react(packet_proc_fsm_t *handle)
{
    bool did_transition = true;
    packet_data_t *packet = handle->event.external.data.packet;

    /*Add host responses  here to process */
    switch (packet->header.type.res)
    {
    case H2T_RES_ACK:  host_res_ack_handler(packet); break;
    case H2T_RES_NACK: host_res_nack_handler(packet); break;
    default:
        break;
    }

    enter_seq_packet_proc_idle(handle);

    return did_transition;
}



static bool st_packet_proc_host_cmd_on_react(packet_proc_fsm_t *handle)
{
    bool did_transition = false;
    /*Add host commands here to process */
    packet_data_t *packet = handle->event.external.data.packet;

    switch (packet->header.type.cmd)
    {

    case H2T_CMD_GET_FW_VERSION:        host_cmd_get_fw_ver_handler();             break;
    case H2T_CMD_ENTER_BOOTLOADER:      host_cmd_enter_boot_handler(packet);       break; 
    case H2T_CMD_EXIT_BOOTLOADER:       host_cmd_exit_boot_handler(packet);        break; 
    case H2T_CMD_START_HEX_FILE_FLASH:  host_cmd_start_hex_flash_handler(packet);  break; 
    case H2T_CMD_PROCESS_HEX_FILE_LINE: host_cmd_proc_hex_line_handler(packet);    break; 
    case H2T_CMD_FINISH_HEX_FILE_FLASH: host_cmd_finish_hex_flash_handler(packet); break; 
    case H2T_CMD_SYNC: break; 

    default:
        break;
    }

    enter_seq_packet_proc_idle(handle);

    return did_transition;
}

static bool st_packet_proc_host_evt_on_react(packet_proc_fsm_t *handle)
{
    bool did_transition = false;
    /*Add host events here to process */

    enter_seq_packet_proc_idle(handle);

    return did_transition;
}

bool packet_proc_fsm_is_active(const packet_proc_fsm_t *handle)
{
	bool result = (handle->state > st_packet_proc_invalid && handle->state < st_packet_proc_last)? true : false;
	return result;
}

uint8_t packet_proc_fsm_set_ext_event(packet_proc_fsm_t *handle, packet_proc_external_events_t *event)
{
	handle->event.external.name = event->name;
	handle->event.external.data.packet = event->data.packet;
	return 1;
}

/*##################################*/
/*#### Host Command Handlers #######*/
/*##################################*/
static void host_cmd_get_fw_ver_handler(void)
{
    packet_data_t packet; 
    packet.header.dir = TARGET_TO_HOST;
    packet.header.type.res = T2H_RES_FW_VERSION;
    packet.header.payload_len = strlen(VERS) + 1;
    memcpy((uint8_t *)&packet.payload, (uint8_t*)&VERS, packet.header.payload_len);

    host_comm_tx_fsm_send_packet(&host_comm_tx_handle, &packet, MAX_NUM_OF_TRANSFER_RETRIES);
}

static void host_cmd_enter_boot_handler(packet_data_t *packet)
{
    
}

static void host_cmd_exit_boot_handler(packet_data_t *packet)
{
    
}

static void host_cmd_start_hex_flash_handler(packet_data_t *packet)
{
    
}

static void host_cmd_proc_hex_line_handler(packet_data_t *packet)
{
    
}

static void host_cmd_finish_hex_flash_handler(packet_data_t *packet)
{
    
}


/*##################################*/
/*#### Host Response Handlers #######*/
/*##################################*/
static void host_res_ack_handler(packet_data_t *packet)
{
    host_comm_tx_fsm_set_ext_event(&host_comm_tx_handle, ev_ext_comm_tx_ack_received);
}

static void host_res_nack_handler(packet_data_t *packet)
{
    host_comm_tx_fsm_set_ext_event(&host_comm_tx_handle, ev_ext_comm_tx_nack_received);
}

/*##################################*/
/*#### Host Events  Handlers #######*/
/*##################################*/



