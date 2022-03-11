#include "intel_hex_parser.h"
#include <stdio.h>

/**@brief Enable/Disable debug messages */
#define INTEL_HEX_DBG 1

/**@brief uart debug function for server comm operations  */
#if INTEL_HEX_DBG
#define hex_line_dbg(...) printf(__VA_ARGS__)
#else
#define hex_line_dbg(...)
#endif

static uint8_t hex_line_str_to_byte_array(char *line_str, uint8_t *byte_array)
{
    /*remove ':', every two characters are 1 decimal */
    size_t bytes_len = (strlen(line_str) - 1) / 2;
    char val_str[4];

    /*Copy Start byte ':'*/
    byte_array[0] = line_str[0];

    for (size_t i = 0; i < bytes_len; i++)
    {
        /*Convert 2 ascii to 1 hex*/
        strncpy(val_str, (line_str + 1 + 2 * i), 2);
        val_str[3] = '\0';
        byte_array[i + 1] = strtol(val_str, NULL, 16);
    }

    return 1;
}

static void hex_line_print_byte_array(uint8_t *byte_array)
{
    hex_line_dbg("hex frame : %c", (char)byte_array[0]);
    uint8_t byte_cnt = HEX_FRAME_HEADER_LEN + byte_array[1] + 1;
    for (int i = 1; i < byte_cnt; i++)
        hex_line_dbg("%.2X", byte_array[i]);
    hex_line_dbg("\r\n");
}


static uint8_t get_twos_complement(uint8_t value)
{
    for (int i = 0; i < CHAR_BIT; i++)
    {
        if (HEX_GET_BIT(value, i))
            HEX_CLEAR_BIT(value, i);
        else
            HEX_SET_BIT(value, i);
    }

    return (value += 1);
}

static uint8_t hex_line_verify_checksum_integrity(uint8_t *byte_array)
{
    /* byte_cnt = byte_array[1] */
    uint8_t data_len = HEX_FRAME_HEADER_LEN + byte_array[1] - 1;
    uint32_t byte_sum = 0x00;

    for (size_t i = 0; i < data_len; i++)
        byte_sum += byte_array[i + 1];

    /* byte_array[data_len + 1] == checksum */
    if (get_twos_complement((uint8_t)byte_sum) == byte_array[data_len + 1])
        return 1;

    return 0;
}

static uint8_t intel_hex_byte_array_to_frame_struct(uint8_t *byte_array, intel_hex_frame_t *hex_frame)
{
    hex_frame->start_code = byte_array[0];
    hex_frame->byte_count = byte_array[1];
    hex_frame->addr_offset = ((hex_frame->addr_offset | byte_array[2]) << 8);
    hex_frame->addr_offset = ((hex_frame->addr_offset | byte_array[3]));
    hex_frame->record_type = byte_array[4];

    if (hex_frame->start_code == HEX_FRAME_START_CHAR &&
        hex_frame->record_type < HEX_RECORD_TYPE_LAST)
    {
        if (hex_frame->byte_count > 0)
            memcpy((uint8_t *)&hex_frame->data[0].byte[0], byte_array + HEX_FRAME_HEADER_LEN, hex_frame->byte_count);
        return 1;
    }

    return 0;
}

uint8_t hex_line_str_to_intel_frame(uint8_t *line_str, intel_hex_frame_t *line_hex)
{
    uint8_t byte_array[MAX_HEX_FRAME_LEN] = {0};

    hex_line_str_to_byte_array(line_str, byte_array);
    hex_line_print_byte_array(byte_array);

    if (!hex_line_verify_checksum_integrity(byte_array))
        return 0;

    if (!intel_hex_byte_array_to_frame_struct(byte_array, &line_hex))
        return 0;

    return 1;
}


uint32_t hex_line_endian_swap_32(uint32_t x)
{
    uint32_t y = 0;
    y += (x & 0x000000FF) << 24;
    y += (x & 0xFF000000) >> 24;
    y += (x & 0x0000FF00) << 8;
    y += (x & 0x00FF0000) >> 8;

    return y;
}
