/**
 * @file boot_fsm.h
 * @author Bayron Cabrera (bdcabreran@unal.edu.com)
 * @brief  Bootloader state machine 
 * @copyright Copyright (c) 2021
 */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "time_event.h"
#include "protocol.h"

#define COUTDOWN_TIMEOUT_MS (3000) 
#define COUTDOWN_MSG_TIMEOUT_MS (200) //send countdown msg to host every 0.2s
#define BOOT_TIMEOUT_MS     (1000*60) //safe exit in boot mode if no cmd is received 

/*
 * Enum of states names in the statechart.
 */
typedef enum
{
    st_boot_invalid,
    st_boot_countdown,
    st_boot_mode,
    st_boot_jump_to_app,
    st_boot_start_hex_flash,
    st_boot_write_hex_line,
    st_boot_finish_hex_flash,
    st_boot_last,
} boot_states_t;

/*
 * Enum of internal event names in the statechart.
 */
typedef enum
{
    ev_int_boot_invalid,
    ev_int_boot_assert_error, 
    ev_int_boot_integrity_fail, 
    ev_int_boot_integrity_ok, 
    ev_int_boot_hex_file_succeed, 
    ev_int_boot_hex_file_fail, 
    ev_int_boot_flag,
    ev_int_boot_last,

} boot_internal_event_name_t;

/*
 * Enum of external event names in the statechart.
 */

typedef enum
{
    ev_ext_boot_invalid,
    ev_ext_boot_exit,
    ev_ext_boot_enter,
    ev_ext_boot_write_line,
    ev_ext_boot_finish_hex,
    ev_ext_boot_start_hex,
    ev_ext_boot_last,

} boot_external_event_name_t;

typedef struct
{
    union  // time events running independently can share memory
    {
        time_event_t boot_timeout;
        time_event_t cntdown_timeout;
    };
    time_event_t msg_timeout;
} boot_time_event_t;

typedef struct
{
    packet_data_t packet;  
}boot_external_event_data_t;

typedef struct
{
   boot_external_event_name_t name;
   boot_external_event_data_t data;
}boot_external_event_t;

typedef struct
{
   boot_external_event_name_t name;
}boot_internal_event_t;


typedef struct
{
    boot_external_event_t external;
    boot_internal_event_t internal;
    boot_time_event_t time;
} boot_event_t;

typedef struct
{
    uint32_t crc_accumulated;
    uint32_t exp_hex_lines;
}boot_iface_t;


/*! 
 * Type definition of the data structure for the communication state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
    boot_states_t state;
    boot_event_t  event;
    boot_iface_t  iface;
} boot_fsm_t;

extern boot_fsm_t boot_handle;

/**@Exported Functions*/
void boot_fsm_init(boot_fsm_t *handle);
void boot_fsm_run(boot_fsm_t *handle);
bool boot_fsm_is_active(const boot_fsm_t *handle);
uint8_t boot_fsm_set_ext_event(boot_fsm_t *handle, boot_external_event_t *event);
void boot_fsm_fsm_time_event_update(boot_fsm_t *handle);

#endif