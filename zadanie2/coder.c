#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>

#include "coder.h"

static void unit_clear(CodeUnits *code_units)
{
    code_units->code[0] = 0;
    code_units->code[1] = 0;
    code_units->code[2] = 0;
    code_units->code[3] = 0;
    code_units->length = 0;
}

uint32_t decode(const CodeUnits *code_unit)
{
    uint32_t value = 0;
    size_t len = code_unit->length;
    int index = MaxCodeLength - len;
    const uint8_t *byte = &code_unit->code[index];

    if ((*byte & 0xf0) == 0xf0)
    {                                    // 4 bytes
        value += (*byte++ & 0x7) << 18;  //     byte & 0000 0111
        value += (*byte++ & 0x3f) << 12; //     byte & 0011 1111
        value += (*byte++ & 0x3f) << 6;  //     byte & 0011 1111
        value += (*byte & 0x3f);         //     byte & 0011 1111
    }
    else if ((*byte & 0xe0) == 0xe0)
    {                                   // 3 bytes
        value += (*byte++ & 0xf) << 12; //     byte & 0000 1111
        value += (*byte++ & 0x3f) << 6; //     byte & 0011 1111
        value += (*byte & 0x3f);        //     byte & 0011 1111
    }
    else if ((*byte & 0xc0) == 0xc0)
    {                                   // 2 bytes
        value += (*byte++ & 0x1f) << 6; //     byte & 0001 1111
        value += (*byte & 0x3f);        //     byte & 0011 1111
    }
    else if ((*byte & 0x80) == 0)
    { // 1 byte
        value += *byte;
    }

    return value;
}

int encode(uint32_t code_point, CodeUnits *code_unit)
{
    assert(code_unit != NULL);
    uint8_t *dest = code_unit->code;
    if (code_point <= 0x7f)
    { // 1 byte
        dest[3] = code_point;
        dest[2] = 0;
        dest[1] = 0;
        dest[0] = 0;
        code_unit->length = 1;
    }
    else if (code_point <= 0x7ff)
    {                                         // 2 bytes
        dest[3] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[2] = (code_point & 0x1f) | 0xc0; // (byte & 0001 1111) | 1100 0000
        dest[1] = 0;
        dest[0] = 0;
        code_unit->length = 2;
    }
    else if (code_point <= 0xffff)
    {                                         // 3 bytes
        dest[3] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[2] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[1] = (code_point & 0xf) | 0xe0;  // (byte & 1111 0000) | 1110 0000
        dest[0] = 0;
        code_unit->length = 3;
    }
    else if (code_point <= 0x1fffff)
    {                                         // 4 bytes
        dest[3] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[2] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[1] = (code_point & 0x3f) | 0x80; // (byte & 0011 1111) | 1000 0000
        code_point >>= 6;                     // 6 bits recorded => 6-bit shift
        dest[0] = (code_point & 0x7) | 0xf0;  // (byte & 0000 0111) | 1111 0000
        code_unit->length = 4;
    }
    else
    { // code_point >= 0x200000
        fprintf(stderr, "%" PRIx32 " > UTF-8 max value\n", code_point);
        code_unit->length = 0;
        return -1;
    }
    return 0;
}

int write_code_unit(FILE *out, const CodeUnits *code_unit)
{
    if (!out || !code_unit)
    {
        return -1;
    }

    if (code_unit->length == 0)
        return -1;

    size_t len = code_unit->length;
    int index = MaxCodeLength - len;
    const uint8_t *dest = &code_unit->code[index];

    for (size_t i = 0; i < len; i++)
    {
        fwrite(dest + i, sizeof(*dest), 1, out);
    }

    return 0;
}

static int template_check(uint8_t byte)
{
    // 1000 0000 <= byte <= 1011 1111
    if (byte >= 0x80 && byte <= 0xbf)
        return 0;
    else
        return -1;
}

int read_next_code_unit(FILE *in, CodeUnits *code_units)
{
    uint8_t byte;
    uint8_t *dest = code_units->code;
    fread(&byte, sizeof(byte), 1, in);

    bool process_next_b_flag = false;

    while (!feof(in) && !ferror(in))
    {
        // 4 bytes
        if ((byte & 0xf0) == 0xf0)
        { // (byte & 1111 0000) == 1111 0000
            dest[0] = byte;
            for (int i = 1; i < 4; i++)
            {
                fread(&byte, sizeof(byte), 1, in);
                if (template_check(byte) == -1)
                {                               // if byte != 10xx xxxx
                    unit_clear(code_units);     // clear code_units
                    process_next_b_flag = true; // need process next byte
                    break;                      //
                }                               //
                dest[i] = byte;                 // if byte == 10xx xxxx
            }

            if (process_next_b_flag == true)
            {
                process_next_b_flag = false;
                continue; // process next byte
            }

            code_units->length = 4;
            return 0;
        }
        // 3 bytes
        else if ((byte & 0xe0) == 0xe0)
        { // (byte & 1110 0000) == 1110 0000
            dest[0] = 0;
            dest[1] = byte;
            for (int i = 2; i < 4; i++)
            {
                fread(&byte, sizeof(byte), 1, in);
                if (template_check(byte) == -1)
                {                               // if byte != 10xx xxxx
                    unit_clear(code_units);     // clear code_units
                    process_next_b_flag = true; // need process next byte
                    break;                      //
                }                               //
                dest[i] = byte;                 // if byte == 10xx xxxx
            }

            if (process_next_b_flag == true)
            {
                process_next_b_flag = false;
                continue; // process next byte
            }

            code_units->length = 3;
            return 0;
        }
        // 2 bytes
        else if ((byte & 0xc0) == 0xc0)
        { // (byte & 1100 0000) == 1100 0000
            dest[0] = 0;
            dest[1] = 0;
            dest[2] = byte;
            fread(&byte, sizeof(byte), 1, in);
            if (template_check(byte) == -1)
            {                           // if byte != 10xx xxxx
                unit_clear(code_units); // clear code_units
                continue;               // process next byte
            }

            dest[3] = byte;
            code_units->length = 2;
            return 0;
        }
        // 1 byte
        else if ((byte & 0x80) == 0)
        { // (byte & 1000 0000) == 0000 0000
            dest[0] = 0;
            dest[1] = 0;
            dest[2] = 0;
            dest[3] = byte;
            code_units->length = 1;
            return 0;
        }
        // broken byte
        // nead read and process next byte
        else
        {
            fread(&byte, sizeof(byte), 1, in);
        }
    }

    return EOF;
}