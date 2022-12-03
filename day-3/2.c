#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Highlight in this implementation:
 *
 * - Use of a bitmap to save memory when the information to retain
 *   is small.
 * - Avoid using a new loop to scan for in-all-three-backpacks item:
 *   we can detect it ASAP once the byte value is 7 (all bits set).
 */

int main(void) {
    char buf[64];

    FILE *fp = fopen("input.txt","r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    long total_priority = 0;
    long line_number = 0;
    unsigned char seen[53] = {0}; /* Array used to remember seen items. */
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        line_number++;
        int elf_id = line_number % 3; /* Elf in the group: 0, 1, or 2. */

        size_t l = strlen(buf);
        if (l < 2) continue;
        buf[l-1] = 0; l--;  /* Remove newline at the end. */
        assert(l%2 == 0);   /* Specification of input says so. */

        /* For each elf of a group of three (elf_id = 0, 1, 2), scan
         * the backpack content. What we do here is to use each item
         * of the array as a bitmap. So we set bit 0 for elf_id 0, and
         * so forth. */
        for (size_t j = 0; j < l; j++) {
            int p; /* the current char priority. */
            if (buf[j] >= 'a' && buf[j] <= 'z')
                p = buf[j]-'a'+1;
            else
                p = buf[j]-'A'+27;

            seen[p] |= 1 << elf_id;

            /* All the three bits set? Common item found. Note how
             * we can stop ASAP without ending the scan and without
             * using a new loop to scan again. Also the loop is guaranteed
             * to found the common item when we are at the third elf
             * of the group. */
            if (seen[p] == 7) {
                total_priority += p;
                memset(seen,0,sizeof(seen)); /* Reset state. */
                break;
            }
        }
    }
    fclose(fp);
    printf("%ld\n", total_priority);
    return 0;
}
