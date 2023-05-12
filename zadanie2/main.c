#include "coder.h"
#include "command.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc == 4)
    {
        if (strcmp(argv[1], "encode") == 0)
        {
            encode_file(argv[2], argv[3]);
        }
        else if (strcmp(argv[1], "decode") == 0)
        {
            decode_file(argv[2], argv[3]);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}
