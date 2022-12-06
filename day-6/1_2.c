#include <stdio.h>

#define PREFIX_LEN 4 /* Use 14 for part 2. */

/* Usage: cat input.txt | ./part1_2 */
int main(void) {
    /* Here we want to detect a prefix of len PREFIX_LEN where each
     * symbol is different. If at every position we check for the
     * condition, the work we do is proportional to O(N*M), where N
     * is the total number of characters in the sequence and M the
     * length of the prefix we are searching. That is not great if M
     * is big. We want to solve the puzzle in O(N):
     *
     * First we need a table where we remember how many times we
     * saw each symbol in the last M symbols processed. */

    int ftable[256] = {0}; /* Frequency table. */
    int dupcount = 0; /* Track how many symbols in the table have a frequency
                         greater than 1, in a given moment. */

    /* We also need to remember at which position on the stream we are,
     * since we need to report the first time we saw M different symbols. */
    long index = 0;

    /* This approach gives us another key advantage: we can process the
     * stream one symbol each time. However we need to remember the latest
     * M characters even with this algorithm: there is to update the table
     * removing the symbols that are now out of the window. */
    int c; /* Current symbol. */
    char mem[PREFIX_LEN];
    while((c = getc(stdin)) != EOF) {
        int f = ++ftable[c];

        /* Update the table of duplicated chars after seeing this
         * character. */
        if (f == 2) dupcount++;
        index++;

        /* Also undo the effect of the first character that went
         * out of the window of the prefix len. */
        if (index > PREFIX_LEN) {
            int oldidx = index-PREFIX_LEN;
            int oldc = mem[oldidx % PREFIX_LEN];
            int f = --ftable[oldc];
            if (f == 1) dupcount--;
        }

        mem[index % PREFIX_LEN] = c; /* Populate memory of past symbols. */

        /* Check if we found a window with zero duplicated symbols. */
        if (index >= PREFIX_LEN && dupcount == 0) break;
    }
    printf("%ld\n", index);
    return 0;
}
