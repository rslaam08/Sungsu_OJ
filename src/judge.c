#include "oaj.h"

static int next_submission_id(void) {
    int max_id = 0;
    for (int i = 0; i < g_submission_count; i++) {
        if (g_submissions[i].submission_id > max_id) max_id = g_submissions[i].submission_id;
    }
    return max_id + 1;
}

static void ensure_workspace_for_user(int user_id) {
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "if not exist workspace\\sources\\user_%d mkdir workspace\\sources\\user_%d", user_id, user_id);
#else
    snprintf(cmd, sizeof(cmd), "mkdir -p workspace/sources/user_%d", user_id);
#endif
    system(cmd);
}

int get_attempt_count(int user_id, int problem_id) {
    int count = 0;
    for (int i = 0; i < g_submission_count; i++) {
        if (g_submissions[i].user_id == user_id && g_submissions[i].problem_id == problem_id) {
            count++;
        }
    }
    return count;
}

int has_user_solved_problem(int user_id, int problem_id) {
    for (int i = 0; i < g_submission_count; i++) {
        if (g_submissions[i].user_id == user_id &&
            g_submissions[i].problem_id == problem_id &&
            strcmp(g_submissions[i].result, "AC") == 0) {
            return 1;
        }
    }
    return 0;
}

JudgeResult judge_source(int problem_id, const char* source_file,
                         int submission_id, int* out_time_taken) {
    Problem* p = find_problem_by_id(problem_id);
    if (p == NULL || source_file == NULL) return JUDGE_RE;

    char exe_file[MAX_FILEPATH_LEN];
    char compile_error_file[MAX_FILEPATH_LEN];
    #ifdef _WIN32
    snprintf(exe_file, sizeof(exe_file), "%s/sub_%d.exe", DIR_EXECUTABLES, submission_id);
#else
    snprintf(exe_file, sizeof(exe_file), "%s/sub_%d", DIR_EXECUTABLES, submission_id);
#endif
    snprintf(compile_error_file, sizeof(compile_error_file), "%s/sub_%d_compile.txt", DIR_ERRORS, submission_id);

    JudgeResult compile_result = compile_source(source_file, exe_file, compile_error_file);
    if (compile_result != JUDGE_AC) {
        if (out_time_taken) *out_time_taken = 0;
        return JUDGE_CE;
    }

    TestCase* cases = NULL;
    int case_count = load_testcases(problem_id, &cases);
    if (case_count <= 0) return JUDGE_RE;

    time_t started = time(NULL);
    JudgeResult final_result = JUDGE_AC;

    for (int i = 0; i < case_count; i++) {
        if (!file_exists(cases[i].input_file) || !file_exists(cases[i].output_file)) {
            final_result = JUDGE_RE;
            break;
        }

        char user_output[MAX_FILEPATH_LEN];
        char run_error[MAX_FILEPATH_LEN];
        snprintf(user_output, sizeof(user_output), "%s/sub_%d_case_%d.txt", DIR_OUTPUTS, submission_id, cases[i].case_index);
        snprintf(run_error, sizeof(run_error), "%s/sub_%d_case_%d_err.txt", DIR_ERRORS, submission_id, cases[i].case_index);

        JudgeResult run_result = run_testcase(exe_file, cases[i].input_file,
                                             user_output, run_error,
                                             p->time_limit);
        if (run_result != JUDGE_AC) {
            final_result = run_result;
            break;
        }

        if (!compare_output_files(user_output, cases[i].output_file)) {
            final_result = JUDGE_WA;
            break;
        }
    }

    time_t ended = time(NULL);
    if (out_time_taken) *out_time_taken = (int)(ended - started);
    free_testcases(cases);
    return final_result;
}

int submit_source(User* user, int problem_id, const char* source_file) {
    if (user == NULL || source_file == NULL) return ERR_NULL_PTR;
    if (!file_exists(source_file)) return ERR_FILE_OPEN;

    Problem* p = find_problem_by_id(problem_id);
    if (p == NULL) return ERR_NOT_FOUND;

    int submission_id = next_submission_id();
    ensure_workspace_for_user(user->user_id);

    char copied_source[MAX_FILEPATH_LEN];
    snprintf(copied_source, sizeof(copied_source), "%s/user_%d/sub_%d.c", DIR_SOURCES, user->user_id, submission_id);
    int copy_ret = copy_file(source_file, copied_source);
    if (copy_ret != ERR_NONE) return copy_ret;

    int time_taken = 0;
    int attempts = get_attempt_count(user->user_id, problem_id) + 1;
    int already_solved = has_user_solved_problem(user->user_id, problem_id);

    JudgeResult result = judge_source(problem_id, copied_source, submission_id, &time_taken);
    const char* result_text = judge_result_to_string(result);

    int score_earned = 0;
    int raw_score_earned = 0;
    int score_was_capped = 0;
    int tier_cap = 0;

    if (result == JUDGE_AC && !already_solved) {
        raw_score_earned = calculate_score(p->difficulty, time_taken, attempts, get_score_func(g_score_mode));
        score_earned = raw_score_earned;

        /* 승급전 조건 점수에 도달하면, 승급 전까지 현재 티어의 최고점을 넘지 못한다. */
        if (user->tier < TIER_CHALLENGER) {
            tier_cap = get_tier_threshold((Tier)user->tier);
            if (user->score >= tier_cap) {
                score_earned = 0;
                score_was_capped = 1;
            } else if (user->score + score_earned > tier_cap) {
                score_earned = tier_cap - user->score;
                score_was_capped = 1;
            }
        }

        if (score_earned > 0) {
            update_user_score(user, score_earned);
        }
        user->solved_count++;
        p->correct_count++;
    }

    user->submit_count++;
    p->submit_count++;

    Submission sub;
    memset(&sub, 0, sizeof(sub));
    sub.submission_id = submission_id;
    sub.user_id = user->user_id;
    sub.problem_id = problem_id;
    strncpy(sub.result, result_text, MAX_RESULT_LEN - 1);
    sub.time_taken = time_taken;
    sub.attempt_count = attempts;
    sub.score_earned = score_earned;
    strncpy(sub.source_file, copied_source, MAX_FILEPATH_LEN - 1);
    get_current_timestamp(sub.timestamp, sizeof(sub.timestamp));

    save_submission(&sub);
    save_users();
    save_problems();

    printf("\n채점 결과: %s\n", judge_result_to_display(result));
    printf("실행 시간: 약 %d초\n", time_taken);
    printf("시도 횟수: %d회\n", attempts);
    if (result == JUDGE_AC) {
        if (already_solved) {
            printf("이미 맞힌 문제라 추가 점수는 없습니다.\n");
        } else {
            if (score_was_capped) {
                printf("원래 획득 예정 점수: %d점\n", raw_score_earned);
                if (tier_cap > 0 && score_earned > 0) {
                    printf("[안내] 현재 티어의 최고점은 %d점입니다. 승급전을 통과해야 점수를 더 올릴 수 있습니다.\n", tier_cap);
                    printf("점수 제한으로 실제 획득 점수: %d점\n", score_earned);
                } else if (tier_cap > 0) {
                    printf("[안내] 이미 승급전 조건 점수(%d점)에 도달했습니다. 승급전을 진행하기 전까지 추가 점수를 얻을 수 없습니다.\n", tier_cap);
                    printf("점수 제한으로 실제 획득 점수: 0점\n");
                }
            } else {
                printf("획득 점수: %d점\n", score_earned);
            }
        }
    } else if (result == JUDGE_CE) {
        printf("컴파일 오류 로그: %s/sub_%d_compile.txt\n", DIR_ERRORS, submission_id);
    }

    return ERR_NONE;
}

void print_submission_history(int user_id) {
    int found = 0;
    print_separator();
    printf("%-6s %-8s %-25s %-8s %-8s %-20s\n", "제출ID", "문제ID", "결과", "시간", "점수", "시각");
    print_separator();
    for (int i = 0; i < g_submission_count; i++) {
        if (g_submissions[i].user_id == user_id) {
            printf("%-6d %-8d %-25s %-8d %-8d %-20s\n",
                   g_submissions[i].submission_id,
                   g_submissions[i].problem_id,
                   judge_result_code_to_display(g_submissions[i].result),
                   g_submissions[i].time_taken,
                   g_submissions[i].score_earned,
                   g_submissions[i].timestamp);
            found = 1;
        }
    }
    if (!found) printf("제출 이력이 없습니다.\n");
    print_separator();
}
