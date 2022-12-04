#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define min(a,b) ((a)<(b) ? (a):(b))
#define max(a,b) ((a)>(b) ? (a):(b))

/* Define the range data type. */
typedef struct {
    int start;
    int end;
} range;

range rangeNew(int s, int e) {
    range r = {s,e};
    return r;
}

int rangeLen(range r) {
    return r.end-r.start;
}

range rangeOverlap(range a, range b) {
    return rangeNew(max(a.start,b.start),min(a.end,b.end));
}

/* We expect the range to be in the form <number>-<number><nulterm>.
 * Return 0 on parse error, otherwise 1 is returned. */
int parseRange(const char *s, range *r) {
    char buf[64];
    size_t len = strlen(s);
    if (len < 2 || len+1 > sizeof(buf)) return 0;
    memcpy(buf,s,len+1);
    if (buf[len-1] == '\n') buf[len-1] = 0; /* newline -> nulterm. */

    /* Split the two parts. */
    char *p = strchr(buf,'-');
    if (!p) return 0;
    *p = 0;
    p++; /* Seek the second number. */
    r->start = atoi(buf); /* Assume input is sane here. */
    r->end = atoi(p); /* Assume input is sane here. */
    return 1;
}

int main(void) {
    char buf[64];

    FILE *fp = fopen("input.txt","r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    long full_overlaps = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        char *p = strchr(buf,',');
        if (!p) continue;
        *p = 0;
        p++; /* Seek second range. */

        range a,b;
        parseRange(buf,&a);
        parseRange(p,&b);
        range overlap = rangeOverlap(a,b);
        if (rangeLen(overlap) == rangeLen(a) ||
            rangeLen(overlap) == rangeLen(b))
        {
            full_overlaps++;
        }
    }
    fclose(fp);
    printf("%ld\n", full_overlaps);
    return 0;
}
