#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    char buf[64];

    FILE *fp = fopen("input.txt","r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    long total_priority = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t l = strlen(buf);
        if (l < 2) continue;
        buf[l-1] = 0; l--;  /* Remove newline at the end. */

        assert(l%2 == 0);   /* Specification of input says so. */

        /* Scan the whole string left-to-right, populating an 'seen-item'
         * table for each item. However after the second half is reached
         * we detect already seen items. */
        unsigned char seen[53] = {0};
        for (size_t j = 0; j < l; j++) {
            int p; /* the current char priority. */
            if (buf[j] >= 'a' && buf[j] <= 'z')
                p = buf[j]-'a'+1;
            else
                p = buf[j]-'A'+27;

            if (j == l/2) printf("|"); /* Mark second part of backpack. */
            printf("%c", buf[j]);

            if (j < l/2) {
                seen[p] = 1;
            } else {
                if (seen[p]) {
                    printf(" <-");
                    total_priority += p;
                    break; /* Go to next backpack. */
                }
            }
        }
        printf("\n");
    }
    fclose(fp);
    printf("%ld\n", total_priority);
    return 0;
}
