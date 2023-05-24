#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1000000

uint32_t generate_number()
{
    const int r = rand();
    const int p = r % 100;
    if (p < 90)
    {
        return r % 128;
    }
    if (p < 95)
    {
        return r % 16384;
    }
    if (p < 99)
    {
        return r % 2097152;
    }
    return r % 268435455;
}

size_t encode_varint(uint32_t value, uint8_t *buf)
{
    assert(buf != NULL);
    uint8_t *cur = buf;
    while (value >= 0x80)
    {
        const uint8_t byte = (value & 0x7f) | 0x80;
        *cur = byte;
        value >>= 7;
        ++cur;
    }
    *cur = value;
    ++cur;
    return cur - buf;
}

uint32_t decode_varint(const uint8_t **bufp)
{
    const uint8_t *cur = *bufp;
    uint8_t byte = *cur++;
    uint32_t value = byte & 0x7f;
    size_t shift = 7;
    while (byte >= 0x80)
    {
        byte = *cur++;
        value += (byte & 0x7f) << shift;
        shift += 7;
    }
    *bufp = cur;
    return value;
}

// записывать в файл
size_t write_bin_file(FILE *file_unc, FILE *file_c)
{
    assert(file_unc != NULL);
    assert(file_c != NULL);

    uint8_t buf[4] = {};
    size_t size = 0;
    size_t full_size = 0;

    for (int i = 0; i < N; i++)
    {
        uint32_t numb = generate_number();
        fwrite(&numb, sizeof(numb), 1, file_unc);
        size = encode_varint(numb, buf);
        full_size += size;
        fwrite(buf, size, 1, file_c);
    }

    return full_size;
}

// считывать из файла
int read_bin_file(FILE *file_unc, FILE *file_c, size_t size)
{
    assert(file_unc != NULL);
    assert(file_c != NULL);

    fseek(file_c, 0, SEEK_SET);
    fseek(file_unc, 0, SEEK_SET);

    uint32_t num_uncomp, num_comp;
    uint8_t *buf = malloc(sizeof(uint8_t) * size);
    const uint8_t *cur = buf;
    fread(buf, sizeof(uint8_t), size, file_c);

    for (int i = 0; i < N; i++)
    {
        num_comp = decode_varint(&cur);
        fread(&num_uncomp, sizeof(uint32_t), 1, file_unc);
        if (i < 5)
            printf("decode %d encode %d\n", num_comp, num_uncomp);
        if (num_comp != num_uncomp)
            return -1;
    }

    free(buf);
    return 0;
}

int main()
{
    srand(time(NULL));

    FILE *file_c;
    if ((file_c = fopen("compressed.dat", "wb+")) == NULL)
    {
        fprintf(stderr, "Can't open compresed.dat\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_unc;
    if ((file_unc = fopen("uncompressed.dat", "wb+")) == NULL)
    {
        fprintf(stderr, "Can't open uncompressed.dat\n");
        exit(EXIT_FAILURE);
    }

    size_t full_size = write_bin_file(file_unc, file_c);

    size_t size = ftell(file_c);
    printf("Size with no encode: %d\nSize with encode: %ld\n",
           N * 4,
           full_size);

    printf("Коэффициент сжатия: %.1f\n\n", (double)(N * 4) / size);

    int test = read_bin_file(file_unc, file_c, size);
    if (test == -1)
        printf("Последовательность чисел неверна\n");
    else
        printf("Последовательность в файлах верна\n");

    fclose(file_c);
    fclose(file_unc);
    return 0;
}