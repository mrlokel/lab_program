#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

int main()
{
    srand(time(NULL));
    uint8_t buf[4] = {};
    uint8_t comp[20] = {};
    const uint8_t *cur = comp;
    size_t size = 0;

    FILE *file = fopen("compressed.dat", "wb");
    printf("Before encode:\n");
    for (int i = 0; i < 5; i++)
    {
        uint32_t numb = generate_number();
        size = encode_varint(numb, buf);
        printf("\t%d = %x size = %ld\n", i + 1, numb, size);
        fwrite(buf, size, 1, file);
    }

    fclose(file);

    file = fopen("compressed.dat", "rb");
    fread(comp, sizeof(uint8_t), 100, file);

    printf("After decode:\n");
    for (int i = 0; i < 5; i++)
    {
        printf("\t %d = %x\n", i + 1, decode_varint(&cur));
    }

    printf("Size = %ld\n", strlen((char *)comp));

    fclose(file);
    return 0;
}