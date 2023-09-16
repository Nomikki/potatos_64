#include <drivers/serial/serial.h>
#include <drivers/display/vga.h>

void log(char *str)
{
    for (int i = 0; 1; i++)
    {
        char ch = (unsigned int)str[i];
        if (ch == '\0')
            return;

        write_serial(ch);
        print_char(ch);
    }
}