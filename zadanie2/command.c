#include <inttypes.h>

#include "coder.h"
#include "command.h"

int encode_file(const char *in_file_name, const char *out_file_name)
{
    FILE *in, *out;
    in = fopen(in_file_name, "r");
    if (!in)
        return -1;

    out = fopen(out_file_name, "wb");
    if (!out)
    {
        fclose(in);
        return -1;
    }

    uint32_t value;
    CodeUnits unit;
    while (!feof(in))
    {
        if (fscanf(in, "%" SCNx32, &value) == 0)
        {
            fprintf(stderr,
                    "In file '%s' not a uint32_t value\n",
                    in_file_name);
            return -1;
        }
        encode(value, &unit);
        write_code_unit(out, &unit);
    }

    fclose(out);
    fclose(in);
    return 0;
}

int decode_file(const char *in_file_name, const char *out_file_name)
{
    FILE *in, *out;
    in = fopen(in_file_name, "rb");
    if (!in)
        return -1;

    out = fopen(out_file_name, "w");
    if (!out)
    {
        fclose(in);
        return -1;
    }

    uint32_t value;
    CodeUnits unit;
    while (!feof(in))
    {
        if (read_next_code_unit(in, &unit) == EOF)
            break;
        value = decode(&unit);
        fprintf(out, "%" PRIx32 "\n", value);
    }

    fclose(out);
    fclose(in);
    return 0;
}