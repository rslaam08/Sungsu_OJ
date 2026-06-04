#include "oaj.h"

int check_promotion_condition(const User* user) {
    if (user == NULL) return ERR_NULL_PTR;
    if (user->tier >= TIER_CHALLENGER) return 0;
    if (user->is_in_promotion) return 0;
    return user->score >= get_tier_threshold((Tier)user->tier);
}

int start_promotion(User* user, PromotionExam* out_exam) {
    if (user == NULL || out_exam == NULL) return ERR_NULL_PTR;
    if (!check_promotion_condition(user)) return ERR_INVALID_INPUT;
    if (g_problem_count < PROMOTION_PROBLEM_COUNT) {
        printf("승급전에 필요한 문제 수가 부족합니다. 최소 %d개가 필요합니다.\n", PROMOTION_PROBLEM_COUNT);
        return ERR_INVALID_INPUT;
    }

    memset(out_exam, 0, sizeof(*out_exam));
    out_exam->promotion_id = (int)time(NULL);
    out_exam->user_id = user->user_id;
    out_exam->start_time = time(NULL);
    out_exam->time_limit = PROMOTION_TIME_LIMIT;
    out_exam->result = PROMO_ONGOING;

    int target_diff = user->tier / 2 + 2;
    if (target_diff > 5) target_diff = 5;

    int* candidates = (int*)malloc(sizeof(int) * (size_t)g_problem_count);
    if (candidates == NULL) return ERR_MEMORY;

    int cand_count = 0;
    for (int i = 0; i < g_problem_count; i++) {
        if (g_problems[i].difficulty >= target_diff) candidates[cand_count++] = i;
    }
    if (cand_count < PROMOTION_PROBLEM_COUNT) {
        cand_count = 0;
        for (int i = 0; i < g_problem_count; i++) candidates[cand_count++] = i;
    }

    srand((unsigned int)time(NULL));
    for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
        int j = i + rand() % (cand_count - i);
        int tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
        out_exam->problem_ids[i] = g_problems[candidates[i]].problem_id;
        out_exam->solved[i] = 0;
    }
    free(candidates);

    user->is_in_promotion = 1;
    user->promotion_passed = 0;
    save_users();

    printf("승급전이 시작되었습니다. 30분 안에 %d문제 이상 맞히면 승급합니다.\n", PROMOTION_PASS_COUNT);
    return ERR_NONE;
}

int check_promotion_time(const PromotionExam* exam) {
    if (exam == NULL) return ERR_NULL_PTR;
    int elapsed = (int)(time(NULL) - exam->start_time);
    int remaining = exam->time_limit - elapsed;
    if (remaining <= 0) return ERR_TIME_OVER;
    return remaining;
}

int submit_promotion_source(PromotionExam* exam, User* user,
                            int problem_index, const char* source_file) {
    if (exam == NULL || user == NULL || source_file == NULL) return ERR_NULL_PTR;
    if (problem_index < 0 || problem_index >= PROMOTION_PROBLEM_COUNT) return ERR_INVALID_INPUT;

    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) {
        printf("[시간 초과] 승급전이 실패 처리됩니다.\n");
        finish_promotion(exam, user);
        return ERR_TIME_OVER;
    }

    int problem_id = exam->problem_ids[problem_index];
    int submission_id = (int)time(NULL) + problem_index;
    int time_taken = 0;
    JudgeResult result = judge_source(problem_id, source_file, submission_id, &time_taken);

    printf("채점 결과: %s\n", judge_result_to_string(result));
    if (result == JUDGE_AC && exam->solved[problem_index] == 0) {
        exam->solved[problem_index] = 1;
        user->promotion_passed++;
        printf("승급전 통과 문제: %d / %d\n", user->promotion_passed, PROMOTION_PASS_COUNT);
    }

    return ERR_NONE;
}

int finish_promotion(PromotionExam* exam, User* user) {
    if (exam == NULL || user == NULL) return ERR_NULL_PTR;

    int success = user->promotion_passed >= PROMOTION_PASS_COUNT;
    if (success) {
        exam->result = PROMO_SUCCESS;
        if (user->tier < TIER_CHALLENGER) user->tier++;
        update_user_score(user, 50);
        printf("승급 성공! 현재 티어: %s (+50점)\n", tier_to_string((Tier)user->tier));
    } else {
        exam->result = PROMO_FAIL;
        update_user_score(user, -PROMOTION_FAIL_PENALTY);
        printf("승급 실패. -%d점 처리되었습니다.\n", PROMOTION_FAIL_PENALTY);
    }

    user->is_in_promotion = 0;
    user->promotion_passed = 0;
    get_current_timestamp(user->last_promo, sizeof(user->last_promo));
    save_promotion(exam);
    save_users();
    return success;
}
