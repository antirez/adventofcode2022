#include <stdio.h>
#include <stdlib.h>

/* Given a game with me using shape 'me' and the opponent using
 * shape 'opp' (both 'A','B' or 'C') return the total score. */
int gameScore(int me, int opp) {
    /* Normalize shapes to 0-2:
     * 0 = Rock, 1 = Paper, 2 = Scissor. */
    int m = me-'A';
    int o = opp-'A';

    /* Each entry scoremap[m][o] represents the outcome of the game in
     * form of a lookup table. */
    int scoremap[3][3] = {
        {1+3, 1+0, 1+6},
        {2+6, 2+3, 2+0},
        {3+0, 3+6, 3+3}
    };

    return scoremap[m][o];
}

int main(void) {
    char buf[64];

    FILE *fp = fopen("input.txt","r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    long score = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        int opp = buf[0];           /* Opponent move. */
        int me = buf[2]-'X'+'A';    /* My move (normalize to A,B,C). */
        score += gameScore(me,opp);
    }
    fclose(fp);
    printf("%ld\n", score);
    return 0;
}
