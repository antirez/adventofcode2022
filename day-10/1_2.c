#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    int X = 1;  /* X register. */
    int cc = 0; /* Clock cycle. */
    int signal_sum = 0; /* Puzzle 1 question. */

    char buf[1024];
    while(fgets(buf,sizeof(buf),stdin) != NULL) {
        /* Skip empty lines. */
        size_t l = strlen(buf);
        if (l <= 1) continue;

        /* Strip newline. */
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        /* Parse instruction. */
        char *p = strchr(buf,' '); /* Will be NULL for NOP. */

        /* Check how many cycles this instruction
         * requires to execute. */
        int wait = 1; /* Default: take 1 clock cycle. */
        switch(buf[0]) {
            case 'a': wait = 2; break; /* ADD takes 2 cycles. */
        }

        do {
            cc++; /* Start of this cycle. */
            wait--;

            /* Draw CRT image. */
            int hpos = (cc-1) % 40; /* Current horizonal position. */
            int pixel = abs(X-hpos) < 2 ? '#' : '.';
            printf("%c", pixel);
            if ((cc % 40) == 0) printf("\n");

            /* We are in the DURING clock cycle "cc" */
            if (((cc+20) % 40) == 0) {
                // printf("During cycle %d, X=%d\n", cc, X);
                signal_sum += cc*X;
            }
        } while(wait > 0);

        switch(buf[0]) {
            case 'n': /* NOP */
                break;
            case 'a': /* ADD */
                assert(p != NULL);
                X += atoi(p);
                break;
            default: assert(1 != 1); break; /* Shold never happen. */
        }

        /* We are in the AFTER clock cycle "cc" */
    }
    printf("Signal sum (puzzle 1): %d\n", signal_sum);
}
