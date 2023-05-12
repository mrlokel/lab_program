#include "coder.h"
#include "command.h"
#include <inttypes.h>
#include <stdio.h>

int encode_file(const char *int_file_name, const char *out_file_name)
{
    uint32_t code_point;
    CodeUnits code_units;

    FILE *in_file = fopen(out_file_name, "wb");
    if (in_file == NULL)
    {
        printf("Failed to read file located at %s\n", int_file_name);
        return -1;
    }

    FILE *out_file = fopen(out_file_name, "wb");
    if (out_file == NULL)
    {
        printf("Failed to create file located at %s\n", out_file_name);
        return -1;
    }

    while (!feof(in_file))
    {
        fscanf(in_file, "%" SCNx32, &code_point);
        encode(code_point, &code_units);
        write_code_unit(out_file, &code_units);
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
}

int decode_file(const char *in_file_name, const char *out_file_name)
{
    CodeUnits code_units;
    FILE *in_file = fopen(in_file_name, "rb");
    if (in_file == NULL)
    {
        printf("Failed to read file located at %s\n", in_file_name);
        return -1;
    }

    FILE *out_file = fopen(in_file_name, "w");
    if (out_file == NULL)
    {
        printf("Failed to create file located at %s\n", out_file_name);
        return -1;
    }

    while (!feof(in_file))
    {
        if (!read_next_code_unit(in_file, &code_units))
        {
            fprintf(out_file, "%" PRIx32 "\n", decode(&code_units));
        }
    }

    fclose(in_file);
    fclose(out_file);

    return 0;
}