#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_ITEMS 64
typedef struct monkey {
    int items[MAX_ITEMS];   /* Items handled by this monkey. */
    int numitems;           /* Number of valid items in item[]. */
    int activity;           /* Number of inspected items. */
    char expr[64];      /* Expression performed to obtain new worry level. */
    int div;            /* Divisibility test. */
    int next[2];        /* Item goes to next[1] if divisibility test is true.
                           Otherwise to next[0] if false. */
} monkey;

/* Move the item with worry level 'item' to the specified monkey. */
void passItemToMonkey(monkey *m, int item) {
    assert(m->numitems+1 < MAX_ITEMS);
    m->items[m->numitems++] = item;
}

/* Evaluate a monkey expression in the form: "old [*+] [old|value]" and
 * return the result of the operation. */
int evalExpression(char *exp, int old) {
    assert(strlen(exp) > 4 && !memcmp(exp,"old ",4));
    int mathop = exp[4];
    int val = !strcmp(exp+6,"old") ? old : atoi(exp+6);
    switch(mathop) {
        case '*': old *= val; break;
        case '+': old += val; break;
        default: assert(1 != 1); break;
    }
    return old;
}

/* Perform a round as for Day 11 puzzle 1 description. */
void simulateRound(monkey *monkeys, int num) {
    for (int j = 0; j < num; j++) {
        monkey *m = monkeys+j; /* Current monkey. */
        for (int k = 0; k < m->numitems; k++) {
            /* For each item... */
            int old = m->items[k];
            int new = evalExpression(m->expr,old);
            new /= 3;
            int isdiv = (new % m->div) == 0;
            passItemToMonkey(monkeys+m->next[isdiv],new);
            m->activity++;
        }
        m->numitems = 0;
    }
}

/* Show items of each monkey. */
void printMonkeys(monkey *monkeys, int num) {
    for (int j = 0; j < num; j++) {
        monkey *m = monkeys+j; /* Current monkey. */
        printf("Monkey %d [inspected: %d]: ", j, m->activity);
        for (int k = 0; k < m->numitems; k++)
            printf("%d, ", m->items[k]);
        printf("\n");
    }
}

/* qsort() helper. */
int qsort_cmp_int(const void *a, const void *b) {
    int *pa = (int*)a;
    int *pb = (int*)b;
    return *pb - *pa;
}

/* Get Puzzle 1 answer. Sort monkeys by activity, take the first two
 * activity levels, multiply. */
int getMonkeyBusiness(monkey *monkeys, int num) {
    int *act = malloc(num*sizeof(int));
    assert(act != NULL);
    for (int j = 0; j < num; j++) act[j] = monkeys[j].activity;
    qsort(act,num,sizeof(int),qsort_cmp_int);
    int business = act[0]*act[1];
    free(act);
    return business;
}

#define MAX_MONKEYS 8
int main(void) {
    char buf[1024];
    monkey monkeys[MAX_MONKEYS], *m = NULL; /* m points to current monkey. */
    int mid = 0; /* ID of this monkey. */
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

    printf("%d monkeys definitions loaded. Simulating...\n", mid+1);
    int rounds = 20;
    while(rounds--) simulateRound(monkeys,mid+1);
    printMonkeys(monkeys,mid+1);
    int business = getMonkeyBusiness(monkeys,mid+1);
    printf("Puzzle 1 monkeys business level: %d\n", business);
}
