#include "oaj.h"

int check_promotion_condition(const User* user) {
    if (user == NULL) return ERR_NULL_PTR;
    if (user->tier >= TIER_CHALLENGER) return 0;
    if (user->is_in_promotion) return 0;
    return user->score >= get_tier_threshold((Tier)user->tier);
}

int get_promotion_min_difficulty(Tier tier) {
    switch (tier) {
        case TIER_IRON:        return 1;  /* IRON -> BRONZE */
        case TIER_BRONZE:      return 1;  /* BRONZE -> SILVER */
        case TIER_SILVER:      return 1;  /* SILVER -> GOLD */
        case TIER_GOLD:        return 2;  /* GOLD -> PLATINUM */
        case TIER_PLATINUM:    return 2;  /* PLATINUM -> EMERALD */
        case TIER_EMERALD:     return 3;  /* EMERALD -> DIAMOND */
        case TIER_DIAMOND:     return 3;  /* DIAMOND -> MASTER */
        case TIER_MASTER:      return 4;  /* MASTER -> GRANDMASTER */
        case TIER_GRANDMASTER: return 5;  /* GRANDMASTER -> CHALLENGER */
        default:               return 5;
    }
}

int get_promotion_max_difficulty(Tier tier) {
    switch (tier) {
        case TIER_IRON:        return 1;  /* IRON -> BRONZE */
        case TIER_BRONZE:      return 2;  /* BRONZE -> SILVER */
        case TIER_SILVER:      return 3;  /* SILVER -> GOLD */
        case TIER_GOLD:        return 3;  /* GOLD -> PLATINUM */
        case TIER_PLATINUM:    return 4;  /* PLATINUM -> EMERALD */
        case TIER_EMERALD:     return 4;  /* EMERALD -> DIAMOND */
        case TIER_DIAMOND:     return 5;  /* DIAMOND -> MASTER */
        case TIER_MASTER:      return 5;  /* MASTER -> GRANDMASTER */
        case TIER_GRANDMASTER: return 5;  /* GRANDMASTER -> CHALLENGER */
        default:               return 5;
    }
}

const char* get_promotion_pool_description(Tier tier) {
    switch (tier) {
        case TIER_IRON:
            return "난이도 1 문제 3개";
        case TIER_BRONZE:
            return "난이도 1~2 문제 3개";
        case TIER_SILVER:
            return "난이도 1~3 문제 3개";
        case TIER_GOLD:
            return "난이도 2~3 문제 3개";
        case TIER_PLATINUM:
            return "난이도 2~4 문제 3개";
        case TIER_EMERALD:
            return "난이도 3~4 문제 3개";
        case TIER_DIAMOND:
            return "난이도 3~5 문제 3개, 단 난이도 5는 반드시 1개만 포함";
        case TIER_MASTER:
            return "난이도 4 문제 1개 + 난이도 5 문제 2개";
        case TIER_GRANDMASTER:
            return "난이도 5 문제 3개";
        default:
            return "승급전 없음";
    }
}

static int is_unsolved_problem_for_user(const User* user, int problem_index) {
    if (user == NULL) return 0;
    if (problem_index < 0 || problem_index >= g_problem_count) return 0;
    return !has_user_solved_problem(user->user_id, g_problems[problem_index].problem_id);
}

static int count_candidates_in_range(const User* user, int min_diff, int max_diff) {
    int count = 0;
    for (int i = 0; i < g_problem_count; i++) {
        if (g_problems[i].difficulty >= min_diff &&
            g_problems[i].difficulty <= max_diff &&
            is_unsolved_problem_for_user(user, i)) {
            count++;
        }
    }
    return count;
}

static int count_candidates_by_difficulty(const User* user, int difficulty) {
    int count = 0;
    for (int i = 0; i < g_problem_count; i++) {
        if (g_problems[i].difficulty == difficulty &&
            is_unsolved_problem_for_user(user, i)) {
            count++;
        }
    }
    return count;
}

int count_promotion_candidates(const User* user) {
    if (user == NULL) return 0;

    Tier tier = (Tier)user->tier;
    switch (tier) {
        case TIER_DIAMOND: {
            int diff5 = count_candidates_by_difficulty(user, 5);
            int diff34 = count_candidates_in_range(user, 3, 4);
            int possible = 0;
            possible += (diff5 >= 1) ? 1 : diff5;
            possible += (diff34 >= 2) ? 2 : diff34;
            return possible; /* 3이면 출제 가능 */
        }
        case TIER_MASTER: {
            int diff4 = count_candidates_by_difficulty(user, 4);
            int diff5 = count_candidates_by_difficulty(user, 5);
            int possible = 0;
            possible += (diff4 >= 1) ? 1 : diff4;
            possible += (diff5 >= 2) ? 2 : diff5;
            return possible; /* 3이면 출제 가능 */
        }
        case TIER_GRANDMASTER: {
            int diff5 = count_candidates_by_difficulty(user, 5);
            return (diff5 >= 3) ? 3 : diff5; /* 3이면 출제 가능 */
        }
        default:
            return count_candidates_in_range(user,
                                             get_promotion_min_difficulty(tier),
                                             get_promotion_max_difficulty(tier));
    }
}

static int collect_candidates_in_range(const User* user, int min_diff, int max_diff, int* out_indices, int max_count) {
    int count = 0;
    for (int i = 0; i < g_problem_count && count < max_count; i++) {
        if (g_problems[i].difficulty >= min_diff &&
            g_problems[i].difficulty <= max_diff &&
            is_unsolved_problem_for_user(user, i)) {
            out_indices[count++] = i;
        }
    }
    return count;
}

static int collect_candidates_by_difficulty(const User* user, int difficulty, int* out_indices, int max_count) {
    int count = 0;
    for (int i = 0; i < g_problem_count && count < max_count; i++) {
        if (g_problems[i].difficulty == difficulty &&
            is_unsolved_problem_for_user(user, i)) {
            out_indices[count++] = i;
        }
    }
    return count;
}

static void shuffle_ints(int* arr, int n) {
    if (arr == NULL || n <= 1) return;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

static int select_from_range(const User* user, int min_diff, int max_diff, int need, int* out_problem_ids, int* out_count) {
    int* candidates = (int*)malloc(sizeof(int) * (size_t)g_problem_count);
    if (candidates == NULL) return ERR_MEMORY;

    int cand_count = collect_candidates_in_range(user, min_diff, max_diff, candidates, g_problem_count);
    if (cand_count < need) {
        free(candidates);
        return ERR_NOT_FOUND;
    }

    shuffle_ints(candidates, cand_count);
    for (int i = 0; i < need; i++) {
        out_problem_ids[(*out_count)++] = g_problems[candidates[i]].problem_id;
    }

    free(candidates);
    return ERR_NONE;
}

static int select_from_difficulty(const User* user, int difficulty, int need, int* out_problem_ids, int* out_count) {
    int* candidates = (int*)malloc(sizeof(int) * (size_t)g_problem_count);
    if (candidates == NULL) return ERR_MEMORY;

    int cand_count = collect_candidates_by_difficulty(user, difficulty, candidates, g_problem_count);
    if (cand_count < need) {
        free(candidates);
        return ERR_NOT_FOUND;
    }

    shuffle_ints(candidates, cand_count);
    for (int i = 0; i < need; i++) {
        out_problem_ids[(*out_count)++] = g_problems[candidates[i]].problem_id;
    }

    free(candidates);
    return ERR_NONE;
}

static int select_promotion_problem_ids(const User* user, int* out_problem_ids) {
    if (user == NULL || out_problem_ids == NULL) return ERR_NULL_PTR;

    int out_count = 0;
    Tier tier = (Tier)user->tier;
    int ret = ERR_NONE;

    switch (tier) {
        case TIER_DIAMOND:
            /* DIAMOND -> MASTER: 난이도 3~5, 난이도 5는 반드시 정확히 1개 */
            ret = select_from_difficulty(user, 5, 1, out_problem_ids, &out_count);
            if (ret != ERR_NONE) return ret;
            ret = select_from_range(user, 3, 4, 2, out_problem_ids, &out_count);
            if (ret != ERR_NONE) return ret;
            break;
        case TIER_MASTER:
            /* MASTER -> GRANDMASTER: 난이도 4 한 개, 난이도 5 두 개 */
            ret = select_from_difficulty(user, 4, 1, out_problem_ids, &out_count);
            if (ret != ERR_NONE) return ret;
            ret = select_from_difficulty(user, 5, 2, out_problem_ids, &out_count);
            if (ret != ERR_NONE) return ret;
            break;
        case TIER_GRANDMASTER:
            /* GRANDMASTER -> CHALLENGER: 난이도 5 세 개 */
            ret = select_from_difficulty(user, 5, 3, out_problem_ids, &out_count);
            if (ret != ERR_NONE) return ret;
            break;
        default:
            ret = select_from_range(user,
                                    get_promotion_min_difficulty(tier),
                                    get_promotion_max_difficulty(tier),
                                    PROMOTION_PROBLEM_COUNT,
                                    out_problem_ids,
                                    &out_count);
            if (ret != ERR_NONE) return ret;
            break;
    }

    if (out_count != PROMOTION_PROBLEM_COUNT) return ERR_NOT_FOUND;
    shuffle_ints(out_problem_ids, PROMOTION_PROBLEM_COUNT);
    return ERR_NONE;
}

int start_promotion(User* user, PromotionExam* out_exam) {
    if (user == NULL || out_exam == NULL) return ERR_NULL_PTR;
    if (!check_promotion_condition(user)) return ERR_INVALID_INPUT;

    int cand_total = count_promotion_candidates(user);
    if (cand_total < PROMOTION_PROBLEM_COUNT) {
        printf("승급전에 필요한 미해결 문제가 부족합니다. 필요: %d개 / 가능: %d개\n",
               PROMOTION_PROBLEM_COUNT, cand_total);
        printf("현재 승급전 문제 pool: %s\n", get_promotion_pool_description((Tier)user->tier));
        return ERR_NOT_FOUND;
    }

    memset(out_exam, 0, sizeof(*out_exam));
    out_exam->promotion_id = (int)time(NULL);
    out_exam->user_id = user->user_id;
    out_exam->start_time = time(NULL);
    out_exam->time_limit = PROMOTION_TIME_LIMIT;
    out_exam->result = PROMO_ONGOING;

    srand((unsigned int)time(NULL));
    int ret = select_promotion_problem_ids(user, out_exam->problem_ids);
    if (ret != ERR_NONE) {
        printf("승급전 문제 pool 조건을 만족하는 미해결 문제가 부족합니다.\n");
        printf("현재 승급전 문제 pool: %s\n", get_promotion_pool_description((Tier)user->tier));
        return ret;
    }

    for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
        out_exam->solved[i] = 0;
    }

    user->is_in_promotion = 1;
    user->promotion_passed = 0;
    save_users();

    printf("승급전이 시작되었습니다. 30분 안에 %d문제 이상 맞히면 승급합니다.\n", PROMOTION_PASS_COUNT);
    printf("문제 pool: %s / 이미 맞힌 문제는 출제하지 않습니다.\n",
           get_promotion_pool_description((Tier)user->tier));
    return ERR_NONE;
}

int check_promotion_time(const PromotionExam* exam) {
    if (exam == NULL) return ERR_NULL_PTR;
    int elapsed = (int)(time(NULL) - exam->start_time);
    int remaining = exam->time_limit - elapsed;
    if (remaining <= 0) return ERR_TIME_OVER;
    return remaining;
}

int submit_promotion_source_silent(PromotionExam* exam, User* user,
                                   int problem_index, const char* source_file,
                                   JudgeResult* out_result,
                                   int* out_time_taken,
                                   int* out_newly_solved) {
    if (out_result) *out_result = JUDGE_RE;
    if (out_time_taken) *out_time_taken = 0;
    if (out_newly_solved) *out_newly_solved = 0;

    if (exam == NULL || user == NULL || source_file == NULL) return ERR_NULL_PTR;
    if (problem_index < 0 || problem_index >= PROMOTION_PROBLEM_COUNT) return ERR_INVALID_INPUT;
    if (!file_exists(source_file)) return ERR_FILE_OPEN;

    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;

    int problem_id = exam->problem_ids[problem_index];
    int submission_id = (int)time(NULL) + problem_index + 1000000;
    int time_taken = 0;
    JudgeResult result = judge_source(problem_id, source_file, submission_id, &time_taken);

    if (out_result) *out_result = result;
    if (out_time_taken) *out_time_taken = time_taken;

    if (result == JUDGE_AC && exam->solved[problem_index] == 0) {
        exam->solved[problem_index] = 1;
        user->promotion_passed++;
        if (out_newly_solved) *out_newly_solved = 1;
    }

    return ERR_NONE;
}

int submit_promotion_source(PromotionExam* exam, User* user,
                            int problem_index, const char* source_file) {
    JudgeResult result = JUDGE_RE;
    int time_taken = 0;
    int newly_solved = 0;

    int ret = submit_promotion_source_silent(exam, user, problem_index, source_file,
                                             &result, &time_taken, &newly_solved);
    if (ret == ERR_TIME_OVER) {
        printf("[시간 초과] 승급전이 실패 처리됩니다.\n");
        return ret;
    }
    if (ret != ERR_NONE) return ret;

    printf("채점 결과: %s\n", judge_result_to_display(result));
    printf("실행 시간: 약 %d초\n", time_taken);
    printf("승급전 제출은 점수, 정답률, 제출 이력에 반영되지 않습니다.\n");

    if (result == JUDGE_AC) {
        if (newly_solved) {
            printf("승급전 통과 문제: %d / %d\n", user->promotion_passed, PROMOTION_PASS_COUNT);
        } else {
            printf("이미 맞힌 승급전 문제입니다.\n");
        }
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
