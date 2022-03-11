/**
 * @file intel_hex_parser.h
 * @author your name (you@domain.com)
 * @brief Process and Format data from Hex line Str to Intel Hex Format 
 * @version 0.1
 * @date 2022-03-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef INTEL_HEX_PARSER_H
#define INTEL_HEX_PARSER_H

#include <stdint.h>

/*
[:] [10] [0100] [00] [214601360121470136007EFE09D21901] [40]
1. Start Code
2. Byte Count 
3. Address
4. Record Type
5. Data 
6. Checksum
*/

#define HEX_FRAME_START_CHAR    (':')
#define HEX_DATA_BUFF_SIZE      (4)
#define MAX_HEX_FRAME_LEN       (64)
#define HEX_FRAME_HEADER_LEN    (5)

#ifndef CHAR_BIT
#define CHAR_BIT 	(8)
#endif


/**
 * @brief Enumeration list for Intel Hex record types
 * 
 */
typedef enum
{
    HEX_RECORD_DATA = 0x00,
    HEX_RECORD_END_OF_FILE,
    HEX_RECORD_EXTENDED_SEG_ADDR,
    HEX_RECORD_START_SEG_ADDR,
    HEX_RECORD_EXTENDED_LINEAR_ADDR,
    HEX_RECORD_START_LINEAR_ADDR,
    HEX_RECORD_TYPE_LAST,
}record_type_t;

/**
 * @brief Struct definition for word field
 *
 */
typedef union
{
    uint8_t byte[4];
    uint32_t word;
}word_t;


typedef struct
{
    char          start_code;
    uint8_t       byte_count;
    uint16_t      addr_offset;
    record_type_t record_type;
    word_t        data[HEX_DATA_BUFF_SIZE];
    uint8_t       checksum;
}intel_hex_frame_t;

uint8_t hex_line_str_to_intel_frame(uint8_t *line_str, intel_hex_frame_t *line_hex);
uint32_t hex_line_endian_swap_32(uint32_t x);

#endif 