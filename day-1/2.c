#include <stdio.h>
#include <stdlib.h>

/* Take a list of the 'n' top scores observed. */
void updateNMax(long *max, int n, long sum) {
    for (int j = 0; j < n; j++) {
        if (sum <= max[j]) continue;
        /* Move all the lower-rank scores one position down and update. */
        for (int k = n-1; k > j; k--) max[k] = max[k-1];
        max[j] = sum;
        break;
    }
}

int main(void) {
    char buf[64];
    long max[3] = {0}, sum = 0;
    while(fgets(buf,sizeof(buf),stdin) != NULL) {
        if (buf[0] == '\n' || buf[0] == '\r') {
            /* We are done with this Elf. Update the top three scores. */
            updateNMax(max,3,sum);
            sum = 0;
            continue;
        }

        /* Sum this meal calories to the current Elf total calories. */
        sum += strtol(buf,NULL,10);
    }
    printf("Top three Elfs with more cals have %ld cals.\n",
        max[0]+max[1]+max[2]);
    return 0;
}
