/**
 * @file packet_proc_fsm.h
 * @author Bayron Cabrera (bdcabreran@unal.edu.com)
 * @brief  Packet processing state machine 
 * @copyright Copyright (c) 2021
 */

#ifndef PACKET_PROC_FSM_H
#define PACKET_PROC_FSM_H

#include "host_comm_fsm.h"

/*
 * Enum of states names in the statechart.
 */
typedef enum
{
    st_packet_proc_invalid,
    st_packet_proc_idle,
    st_packet_proc_host_res,
    st_packet_proc_host_cmd,
    st_packet_proc_host_evt,
    st_packet_proc_last,
} packet_proc_states_t;

/*
 * Enum of internal event names in the statechart.
 */
typedef enum
{
    ev_int_packet_proc_invalid, 
    ev_int_packet_proc_last,

} packet_proc_internal_events_t;

/*
 * Enum of external event names in the statechart.
 */

typedef enum
{
    ev_ext_packet_proc_invalid,
    ev_ext_packet_proc_new,
    ev_ext_packet_proc_last,

} packet_proc_external_event_name_t;

typedef struct
{
    packet_data_t *packet;

}packet_proc_external_event_data_t;

typedef struct
{
   packet_proc_external_event_name_t name;
   packet_proc_external_event_data_t data;
}packet_proc_external_events_t;


typedef struct
{
    packet_proc_external_events_t external;
} packet_proc_event_t;


/*! 
 * Type definition of the data structure for the communication state machine.
 * This data structure has to be allocated by the client code. 
 */
typedef struct
{
    packet_proc_states_t state;
    packet_proc_event_t event;
} packet_proc_fsm_t;

extern packet_proc_fsm_t packet_proc_handle;

/**@Exported Functions*/
void packet_proc_fsm_init(packet_proc_fsm_t *handle);
void packet_proc_fsm_run(packet_proc_fsm_t *handle);

void packet_proc_fsm_time_event_update(packet_proc_fsm_t *handle);
bool packet_proc_fsm_is_active(const packet_proc_fsm_t *handle);
uint8_t packet_proc_fsm_set_ext_event(packet_proc_fsm_t *handle, packet_proc_external_events_t *event);

#endif