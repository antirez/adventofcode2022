#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* WARNING: for part 2, even after the LCM trick the computation
 * overflowed the 'int' type, so I brutally searched & replaced "int"
 * with "long" in this program, compared to 1.c. So types will be wild... */

#define MAX_ITEMS 64
typedef struct monkey {
    long items[MAX_ITEMS];   /* Items handled by this monkey. */
    long numitems;           /* Number of valid items in item[]. */
    long activity;           /* Number of inspected items. */
    char expr[64];      /* Expression performed to obtain new worry level. */
    long div;            /* Divisibility test. */
    long next[2];        /* Item goes to next[1] if divisibility test is true.
                           Otherwise to next[0] if false. */
} monkey;

/* Move the item with worry level 'item' to the specified monkey. */
void passItemToMonkey(monkey *m, long item) {
    assert(m->numitems+1 < MAX_ITEMS);
    m->items[m->numitems++] = item;
}

/* Evaluate a monkey expression in the form: "old [*+] [old|value]" and
 * return the result of the operation. */
long evalExpression(char *exp, long old) {
    assert(strlen(exp) > 4 && !memcmp(exp,"old ",4));
    long mathop = exp[4];
    long val = !strcmp(exp+6,"old") ? old : atoi(exp+6);
    switch(mathop) {
        case '*': old *= val; break;
        case '+': old += val; break;
        default: assert(1 != 1); break;
    }
    return old;
}

/* Perform a round as for Day 11 puzzle 1 description. */
void simulateRound(monkey *monkeys, long num) {
    for (long j = 0; j < num; j++) {
        monkey *m = monkeys+j; /* Current monkey. */
        for (long k = 0; k < m->numitems; k++) {
            /* For each item... */
            long old = m->items[k];
            long new = evalExpression(m->expr,old);
            /* In order to avoid overflows, we can use the trick
             * of performing the operation modulo LCM, but since all the
             * divisors are prime here, this is equivalent to just multiplying
             * all the divisors together.
             *
             * Note that even so it overflows a 32 bit integer, so we
             * need to resort to long. */
            long modulo = 1;
            for (long i = 0; i < num; i++) modulo *= monkeys[i].div;
            new = new % modulo;
            long isdiv = (new % m->div) == 0;
            passItemToMonkey(monkeys+m->next[isdiv],new);
            m->activity++;
        }
        m->numitems = 0;
    }
}

/* Show items of each monkey. */
void printMonkeys(monkey *monkeys, long num) {
    for (long j = 0; j < num; j++) {
        monkey *m = monkeys+j; /* Current monkey. */
        printf("Monkey %ld [inspected: %ld]: ", j, m->activity);
        for (long k = 0; k < m->numitems; k++)
            printf("%ld, ", m->items[k]);
        printf("\n");
    }
}

/* qsort() helper. */
int qsort_cmp_int(const void *a, const void *b) {
    long *pa = (long*)a;
    long *pb = (long*)b;
    return (int)(*pb - *pa);
}

/* Get Puzzle 1 answer. Sort monkeys by activity, take the first two
 * activity levels, multiply. */
long getMonkeyBusiness(monkey *monkeys, long num) {
    long *act = malloc(num*sizeof(int));
    assert(act != NULL);
    for (long j = 0; j < num; j++) act[j] = monkeys[j].activity;
    qsort(act,num,sizeof(long),qsort_cmp_int);
    long business = act[0]*act[1];
    free(act);
    return business;
}

#define MAX_MONKEYS 8
int main(void) {
    char buf[1024];
    monkey monkeys[MAX_MONKEYS], *m = NULL; /* m points to current monkey. */
    long mid = 0; /* ID of this monkey. */
    while(fgets(buf,sizeof(buf),stdin) != NULL) {
        /* Skip empty lines. */
        size_t l = strlen(buf);
        if (l <= 9) continue;

        /* Strip newline. */
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        /* Parse monkeys. */
        if (buf[0] == 'M') { /* Monkey ID ... */
            if (m != NULL) mid++;
            assert(mid < MAX_MONKEYS);
            m = &monkeys[mid];
            m->numitems = 0;
            m->activity = 0;
            continue;
        }

        assert(m != NULL);
        if (buf[2] == 'S') { /* Starting items: ... */
            assert(l > 16);
            char *p = strchr(buf+15,' ');
            assert(p != NULL);

            while(1) {
                char *c = strchr(p,',');
                if (c) *c = 0;
                passItemToMonkey(m,atoi(p+1));
                if (c == NULL) break; /* End of items. */
                p = c+1;
            }
        } else if (buf[2] == 'O') { /* Operation: ... */
            assert(l > 20);
            strncpy(m->expr,buf+19,sizeof(m->expr));
        } else if (buf[2] == 'T') { /* Test: ... */
            assert (l > 21);
            m->div = atoi(buf+21);
        } else if (buf[4] == 'I') { /* If true/false ... */
            char *p = strstr(buf,"to monkey");
            assert(p != NULL);
            m->next[buf[7] == 'f' ? 0 : 1] = atoi(p+10);
        } else {
            printf("Can't parse: %s\n", buf);
            exit(1);
        }
    }

    printf("%ld monkeys definitions loaded. Simulating...\n", mid+1);
    long rounds = 10000;
    while(rounds--) simulateRound(monkeys,mid+1);
    printMonkeys(monkeys,mid+1);
    long business = getMonkeyBusiness(monkeys,mid+1);
    printf("Puzzle 2 monkeys business level: %ld\n", business);
}
