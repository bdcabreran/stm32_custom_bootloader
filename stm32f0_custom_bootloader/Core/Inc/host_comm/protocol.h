/**
 * @file protocol.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief Protocol Scheme for Host-Target communication
 * @version 0.1
 * @date 2021-08-17
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "stdint.h"
#include "stdio.h"

#define MAX_PAYLOAD_SIZE	(256)

/* 1 byte = 256 possible cmd/res/evt */
#define CMD_START   (0x00)
#define CMD_END     (0x55)
#define EVT_START   (0x56)
#define EVT_END     (0xAB)
#define RES_START   (0xAC)
#define RES_END     (0xFF)

/* Host / Target IDs */
#define TARGET_TO_HOST_DIR   (0xAA)
#define HOST_TO_TARGET_DIR   (0xBB)

/* Packet Format Sizes */
#define PREAMBLE_SIZE_BYTES     sizeof(uint32_t)
#define POSTAMBLE_SIZE_BYTES    sizeof(uint32_t)
#define HEADER_SIZE_BYTES       sizeof(packet_header_t)
#define CRC_SIZE_BYTES          sizeof(uint32_t)

/* Packet structure 
    ------------------------------------------------------------------------------
   | PREAMBLE : 4B | HEADER : 4B | PAYLOAD : [0 - 256]B | CRC : 4B | POSTAMBLE :4B| 
    ------------------------------------------------------------------------------
*/

/* Preamble / Postamble bytes */
#define PREAMBLE              (0xAA55AA55)
#define POSTAMBLE             (0xBB55BB55)

typedef union
{
    uint32_t byte; 
    uint8_t  bit[4];
}byte_t;


typedef enum
{
    HOST_TO_TARGET = HOST_TO_TARGET_DIR,
    TARGET_TO_HOST = TARGET_TO_HOST_DIR,
}packet_dir_t;

/*####################################################################################################################*/
/*##### Host to Target Types #######################################################################################*/
/*####################################################################################################################*/

typedef enum
{
    H2T_CMD_START = CMD_START,
    H2T_CMD_GET_FW_VERSION,
    H2T_CMD_ENTER_BOOTLOADER,
    H2T_CMD_EXIT_BOOTLOADER,
    H2T_CMD_START_HEX_FLASH,
    H2T_CMD_PROC_HEX_LINE,
    H2T_CMD_FINISH_HEX_FLASH,
    H2T_CMD_SYNC,
    H2T_CMD_END = CMD_END
}host_to_target_cmd_t;
#define IS_H2T_CMD(cmd) ((cmd > H2T_CMD_START) && (cmd < H2T_CMD_END))

typedef enum
{
    H2T_EVT_START = EVT_START,
    H2T_EVT_END = EVT_END
}host_to_target_evt_t;
#define IS_H2T_EVT(evt) ((evt > H2T_EVT_START) && (evt < H2T_EVT_END))


typedef enum
{
    H2T_RES_START = RES_START,
    H2T_RES_ACK,
    H2T_RES_NACK,
    H2T_RES_END = RES_END
}host_to_target_resp_t;
#define IS_H2T_RES(res) ((res > H2T_RES_START) && (res < H2T_RES_END))


/*####################################################################################################################*/
/*##### Target to Host Types #######################################################################################*/
/*####################################################################################################################*/

typedef enum
{
    T2H_CMD_START = CMD_START,
    T2H_CMD_END = CMD_END
}target_to_host_cmd_t;
#define IS_T2H_CMD(cmd) ((cmd > T2H_CMD_START) && (cmd < T2H_CMD_END))


typedef enum
{
    T2H_EVT_START = EVT_START,
    T2H_EVT_HANDLER_ERROR,
    T2H_EVT_PRINT_DBG_MSG,
    T2H_EVT_BOOTLOADER_COUNTDOWN,
    T2H_EVT_BOOTLOADER_TIMEOUT,
    T2H_EVT_JUMP_TO_USER_APP_FAIL,
    T2H_EVT_JUMP_TO_USER_APP,
    T2H_EVT_USER_APP_INTEGRITY_FAIL,
    T2H_EVT_USER_APP_INTEGRITY_OK,
    T2H_EVT_END = EVT_END
}target_to_host_evt_t;
#define IS_T2H_EVT(evt) ((evt > T2H_EVT_START) && (evt < T2H_EVT_END))


typedef enum
{
    T2H_RES_START = RES_START,
    T2H_RES_ACK,
    T2H_RES_NACK,
    T2H_RES_FW_VERSION,
    T2H_RES_START_HEX_FLASH_OK,
    T2H_RES_FINISH_HEX_FLASH_OK,
    T2H_RES_FINISH_HEX_FLASH_FAIL,
    T2H_RES_HEX_LINE_OK,
    T2H_RES_HEX_LINE_FAIL,
    T2H_RES_ENTER_BOOTLOADER_OK,    
    T2H_RES_EXIT_BOOTLOADER_OK,
    T2H_RES_EXIT_BOOTLOADER_FAIL,
    T2H_RES_SYNC,
    T2H_RES_END = RES_END
}target_to_host_resp_t;
#define IS_T2H_RES(res) ((res > T2H_RES_START) && (res < T2H_RES_END))


/*####################################################################################################################*/
/*##### Target to Host Payload #######################################################################################*/
/*####################################################################################################################*/

typedef union
{
    struct
    {
        uint16_t total_lines;
    }start_hex_flash;

    struct
    {
        uint8_t line_str[MAX_PAYLOAD_SIZE];
    }proc_hex_line;

}h2t_payload_t;

/*####################################################################################################################*/
/*##### Host to Target Payload #######################################################################################*/
/*####################################################################################################################*/

typedef union
{
    struct
    {
        uint8_t version[MAX_PAYLOAD_SIZE];
    }get_version;

}t2h_payload_t;

/*####################################################################################################################*/
/*##### Host to Target Payload #######################################################################################*/
/*####################################################################################################################*/


typedef struct
{
    union
    {
        uint8_t cmd;
        uint8_t res;
        uint8_t evt;
    }type;

    packet_dir_t  dir;
    uint16_t payload_len;

}packet_header_t;

typedef struct
{
    union
    {
        h2t_payload_t h2t;
        t2h_payload_t t2h;
	    uint8_t buffer[MAX_PAYLOAD_SIZE];
    };
}packet_payload_t;

typedef struct
{
    packet_header_t  header;
    packet_payload_t payload;

}packet_data_t;


extern const byte_t protocol_preamble;
extern const byte_t protocol_postamble;

void print_buff_ascii(uint8_t *buff, size_t len);
void print_buff_hex(uint8_t *buff, size_t len);
uint8_t protocol_check_valid_header(packet_data_t *packet);


#endif
