#include <stdio.h>
#include "coder.h"

uint32_t decode(const CodeUnits *code_unit)
{
    uint32_t code_point = 0;

    if ((code_unit->code[0] >> 7) == 0)
    {
        code_point = code_unit->code[0];
    }
    else if (code_unit->code[0] <= 0xdf)
    {
        code_point = ((code_unit->code[0] & 0x1f) << 6 | (code_unit->code[1] & 0x3f));
    }
    else if (code_unit->code[0] <= 0xef)
    {
        code_point = (((code_unit->code[0] & 0xf) << 12) | ((code_unit->code[1] & 0x3f) << 6) | (code_unit->code[2] & 0x3f));
    }
    else if (code_unit->code[0] <= 0xf7)
    {
        code_point = (((code_unit->code[0] & 0xf) << 18) | ((code_unit->code[1] & 0x3f) << 12) | ((code_unit->code[2] & 0x3f) << 6) | (code_unit->code[3] & 0x3f));
    }

    return code_point;
}

int encode(uint32_t code_point, CodeUnits *code_unit)
{
    uint8_t count = 0;
    uint32_t i;

    for (i = code_point; i > 0; i >>= 1)
    {
        count++;
    }

    if (count <= 7)
    {
        code_unit->code[0] = code_point;
        code_unit->length = 1;
        return 0;
    }

    if (count <= 11)
    {
        code_unit->code[0] = 0xc0 | (code_point >> 6);
        code_unit->code[1] = 0x80 | (code_point & 0x3f);
        code_unit->length = 2;
        return 0;
    }

    if (count <= 16)
    {
        code_unit->code[0] = 0xe0 | (code_point >> 12);
        code_unit->code[1] = 0x80 | ((code_point & 0xfc0) >> 6);
        code_unit->code[2] = 0x80 | (code_point & 0x3f);
        code_unit->length = 3;
        return 0;
    }

    if (count <= 21)
    {
        code_unit->code[0] = 0xf0 | (code_point >> 18);
        code_unit->code[1] = 0x80 | ((code_point & 0x3f000) >> 12);
        code_unit->code[2] = 0x80 | ((code_point & 0xfc0) >> 6);
        code_unit->code[3] = 0x80 | (code_point & 0x3f);
        code_unit->length = 4;
        return 0;
    }

    return -1;
}

int write_code_unit(FILE *out, const CodeUnits *code_unit)
{
    fwrite(code_unit, sizeof(uint8_t), code_unit->length, out);
    return 0;
}

int read_next_code_unit(FILE *in, CodeUnits *code_unit)
{
    code_unit->length = 0;
    uint8_t byte;

    while (code_unit->length == 0)
    {
        fread(&byte, sizeof(uint8_t), 1, in);
        if (feof(in))
        {
            return -1;
        }
        if (byte < 0x80)
        {
            code_unit->code[code_unit->length++] = byte;
            return 0;
        }
        if (byte >= 0xc0)
        {
            int i = 3;
            uint8_t buf;
            for (buf = byte >> 4; buf != 1; buf >>= 1, i--)
            {
                if ((buf == 0x3) || (buf == 0x7) || (buf == 0xf))
                {
                    code_unit->code[0] = byte;
                    int j = 0;
                    for (j; j != i; j++)
                    {
                        fread(&byte, sizeof(uint8_t), 1, in);
                        if ((byte >= 0x80) && (byte <= 0xbf))
                        {
                            code_unit->code[j + 1] = byte;
                        }
                        else if (!feof(in))
                        {
                            fseek(in, -1, SEEK_CUR);
                            return -1;
                        }
                        else
                        {
                            return -1;
                        }
                    }
                    code_unit->length = i + 1;
                    break;
                }
            }
        }
    }
    return 0;
}