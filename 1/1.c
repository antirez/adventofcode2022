#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char buf[64];
    long max = 0, sum = 0;
    while(fgets(buf,sizeof(buf),stdin) != NULL) {
        if (buf[0] == '\n' || buf[0] == '\r') {
            /* We are done with this Elf. Check if it is the winner. */
            if (sum > max) max = sum;
            sum = 0;
            continue;
        }

        /* Sum this mean calories to the current Elf total calories. */
        sum += strtol(buf,NULL,10);
    }
    printf("Elf with more cals has %ld cals.\n", max);
    return 0;
}
