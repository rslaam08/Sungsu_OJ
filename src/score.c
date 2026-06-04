#include "oaj.h"

static int max_int(int a, int b) { return a > b ? a : b; }

int score_standard(int difficulty, int time_taken, int attempts) {
    int base = difficulty * 100;
    int penalty = (time_taken / 2) + (attempts - 1) * 10;
    return max_int(base - penalty, difficulty * 50);
}

int score_speedrun(int difficulty, int time_taken, int attempts) {
    int base = difficulty * 100 + 50;
    int penalty = time_taken + (attempts - 1) * 15;
    return max_int(base - penalty, difficulty * 40);
}

int score_accuracy(int difficulty, int time_taken, int attempts) {
    int base = difficulty * 110;
    int penalty = (attempts - 1) * 25 + (time_taken / 4);
    return max_int(base - penalty, difficulty * 45);
}

int calculate_score(int difficulty, int time_taken, int attempts, ScoreFunc score_fn) {
    if (difficulty < 1) difficulty = 1;
    if (time_taken < 0) time_taken = 0;
    if (attempts < 1) attempts = 1;
    if (score_fn == NULL) score_fn = score_standard;
    return score_fn(difficulty, time_taken, attempts);
}

ScoreFunc get_score_func(ScoreMode mode) {
    switch (mode) {
        case SCORE_SPEEDRUN: return score_speedrun;
        case SCORE_ACCURACY: return score_accuracy;
        case SCORE_STANDARD:
        default:             return score_standard;
    }
}
