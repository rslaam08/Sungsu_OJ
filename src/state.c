#include "oaj.h"

User* g_users = NULL;
int g_user_count = 0;
int g_user_capacity = 0;

Problem* g_problems = NULL;
int g_problem_count = 0;
int g_problem_capacity = 0;

Submission* g_submissions = NULL;
int g_submission_count = 0;
int g_submission_capacity = 0;

PromotionExam* g_promotions = NULL;
int g_promotion_count = 0;
int g_promotion_capacity = 0;

User* g_current_user = NULL;
ScoreMode g_score_mode = SCORE_STANDARD;

static int grow_array(void** arr, int* capacity, int required, size_t elem_size) {
    if (required <= *capacity) return ERR_NONE;

    int new_capacity = (*capacity == 0) ? 8 : *capacity;
    while (new_capacity < required) new_capacity *= 2;

    void* tmp = realloc(*arr, elem_size * new_capacity);
    if (tmp == NULL) return ERR_MEMORY;

    *arr = tmp;
    *capacity = new_capacity;
    return ERR_NONE;
}

int ensure_user_capacity(int required) {
    return grow_array((void**)&g_users, &g_user_capacity, required, sizeof(User));
}

int ensure_problem_capacity(int required) {
    return grow_array((void**)&g_problems, &g_problem_capacity, required, sizeof(Problem));
}

int ensure_submission_capacity(int required) {
    return grow_array((void**)&g_submissions, &g_submission_capacity, required, sizeof(Submission));
}

int ensure_promotion_capacity(int required) {
    return grow_array((void**)&g_promotions, &g_promotion_capacity, required, sizeof(PromotionExam));
}

void reset_state(void) {
    free(g_users);
    free(g_problems);
    free(g_submissions);
    free(g_promotions);

    g_users = NULL;
    g_user_count = 0;
    g_user_capacity = 0;

    g_problems = NULL;
    g_problem_count = 0;
    g_problem_capacity = 0;

    g_submissions = NULL;
    g_submission_count = 0;
    g_submission_capacity = 0;

    g_promotions = NULL;
    g_promotion_count = 0;
    g_promotion_capacity = 0;

    g_current_user = NULL;
}
