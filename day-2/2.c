#include <stdio.h>
#include <stdlib.h>

/* HIGHLIGHTS
 * ==========
 *
 * 1. The program shows that gameScore() of puzzle 3 was at the right
 *    level of abstraction. We are able to reuse it.
 * 2. Use a functional approach to leave the logic identical but call
 *    the nested function selectMove() to obtain one of the arguments
 *    of gameScore().
 * 3. Avoiding of a lookup table by discovering the mathematical rule that
 *    allows us to immediately select what shape to reply for the desired
 *    outcome.
 */

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

/* Given an outcome (X = lose, Y = draw, Z = win) and the opponent
 * move, select the move to do in order to reach the desired outcome. */
int selectMove(int opp, int outcome) {
    int out = outcome-'X'; /* 0 = Lose, 1 = Draw, 2 = Win. */
    opp -= 'A';            /* 0 = Rock, 1 = Paper, 2 = Scissor. */

    /* If you think at the sequence [Rock, Paper, Scissor], if you want
     * to lose against one of those, you have to pick the shape that
     * is two places foward (for exaple for Rock is Scissor), if you
     * want to draw the shape after three positions, that is the same
     * shape your opponent is using, and finally you to win you want
     * to chose the next one (that is the same as the shape after four
     * positions, since we rotate).
     *
     * Now given that the outcome is 0, 1, 2, we can summarize the above
     * into the following: */
    return 'A'+((opp+2+out)%3);
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
        int opp = buf[0];               /* Opponent move. */
        int outcome = buf[2];           /* How the game should end. */
        score += gameScore(selectMove(opp,outcome),opp);
    }
    fclose(fp);
    printf("%ld\n", score);
    return 0;
}
