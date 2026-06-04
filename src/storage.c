#include "oaj.h"

static int load_binary_array(const char* path, void** out_arr, int* out_count,
                             int* out_capacity, size_t elem_size) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        *out_count = 0;
        *out_capacity = 0;
        *out_arr = NULL;
        return ERR_NONE;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        fclose(fp);
        *out_count = 0;
        *out_capacity = 0;
        *out_arr = NULL;
        return ERR_NONE;
    }

    int count = (int)(size / (long)elem_size);
    void* arr = malloc(elem_size * (size_t)count);
    if (arr == NULL) {
        fclose(fp);
        return ERR_MEMORY;
    }

    size_t read_count = fread(arr, elem_size, (size_t)count, fp);
    fclose(fp);

    if ((int)read_count != count) {
        free(arr);
        return ERR_FILE_OPEN;
    }

    *out_arr = arr;
    *out_count = count;
    *out_capacity = count;
    return ERR_NONE;
}

static int save_binary_array(const char* path, const void* arr, int count, size_t elem_size) {
    FILE* fp = fopen(path, "wb");
    if (fp == NULL) return ERR_FILE_OPEN;

    if (count > 0 && arr != NULL) {
        size_t written = fwrite(arr, elem_size, (size_t)count, fp);
        if ((int)written != count) {
            fclose(fp);
            return ERR_FILE_OPEN;
        }
    }

    fclose(fp);
    return ERR_NONE;
}

int ensure_directories(void) {
#ifdef _WIN32
    system("if not exist data mkdir data");
    system("if not exist data\\testcases mkdir data\\testcases");
    system("if not exist workspace mkdir workspace");
    system("if not exist workspace\\sources mkdir workspace\\sources");
    system("if not exist workspace\\executables mkdir workspace\\executables");
    system("if not exist workspace\\outputs mkdir workspace\\outputs");
    system("if not exist workspace\\errors mkdir workspace\\errors");
#else
    system("mkdir -p data/testcases workspace/sources workspace/executables workspace/outputs workspace/errors build");
#endif
    return ERR_NONE;
}


#define LEGACY_MAX_DESC_LEN 500

typedef struct {
    int  problem_id;
    char title[MAX_TITLE_LEN];
    char desc[LEGACY_MAX_DESC_LEN];
    int  difficulty;
    char category[MAX_CATEGORY_LEN];
    int  time_limit;
    int  submit_count;
    int  correct_count;
    int  testcase_count;
} LegacyProblem;

static int load_problems_compatible(void) {
    FILE* fp = fopen(FILE_PROBLEMS, "rb");
    if (fp == NULL) {
        g_problem_count = 0;
        g_problem_capacity = 0;
        g_problems = NULL;
        return ERR_NONE;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    if (size <= 0) {
        fclose(fp);
        g_problem_count = 0;
        g_problem_capacity = 0;
        g_problems = NULL;
        return ERR_NONE;
    }

    if (size % (long)sizeof(Problem) == 0) {
        int count = (int)(size / (long)sizeof(Problem));
        Problem* arr = (Problem*)malloc(sizeof(Problem) * (size_t)count);
        if (arr == NULL) {
            fclose(fp);
            return ERR_MEMORY;
        }
        size_t read_count = fread(arr, sizeof(Problem), (size_t)count, fp);
        fclose(fp);
        if ((int)read_count != count) {
            free(arr);
            return ERR_FILE_OPEN;
        }
        g_problems = arr;
        g_problem_count = count;
        g_problem_capacity = count;
        return ERR_NONE;
    }

    if (size % (long)sizeof(LegacyProblem) == 0) {
        int count = (int)(size / (long)sizeof(LegacyProblem));
        LegacyProblem* old_arr = (LegacyProblem*)malloc(sizeof(LegacyProblem) * (size_t)count);
        if (old_arr == NULL) {
            fclose(fp);
            return ERR_MEMORY;
        }
        size_t read_count = fread(old_arr, sizeof(LegacyProblem), (size_t)count, fp);
        fclose(fp);
        if ((int)read_count != count) {
            free(old_arr);
            return ERR_FILE_OPEN;
        }

        if (ensure_problem_capacity(count) != ERR_NONE) {
            free(old_arr);
            return ERR_MEMORY;
        }

        for (int i = 0; i < count; i++) {
            Problem p;
            memset(&p, 0, sizeof(p));
            p.problem_id = old_arr[i].problem_id;
            strncpy(p.title, old_arr[i].title, MAX_TITLE_LEN - 1);
            strncpy(p.desc, old_arr[i].desc, MAX_DESC_LEN - 1);
            strncpy(p.input_desc, "입력 설명이 아직 등록되지 않았습니다.", MAX_DESC_LEN - 1);
            strncpy(p.output_desc, "출력 설명이 아직 등록되지 않았습니다.", MAX_DESC_LEN - 1);
            p.difficulty = old_arr[i].difficulty;
            strncpy(p.category, old_arr[i].category, MAX_CATEGORY_LEN - 1);
            p.time_limit = old_arr[i].time_limit;
            p.submit_count = old_arr[i].submit_count;
            p.correct_count = old_arr[i].correct_count;
            p.testcase_count = old_arr[i].testcase_count;
            g_problems[g_problem_count++] = p;
        }
        free(old_arr);
        save_problems();
        return ERR_NONE;
    }

    fclose(fp);
    printf("[경고] data/problems.dat 형식을 읽을 수 없어 문제 목록을 새로 초기화합니다.\n");
    g_problem_count = 0;
    g_problem_capacity = 0;
    g_problems = NULL;
    return ERR_NONE;
}

int init_users(void) {
    int ret = load_binary_array(FILE_USERS, (void**)&g_users, &g_user_count,
                                &g_user_capacity, sizeof(User));
    if (ret != ERR_NONE) return ret;

    if (g_user_count == 0) {
        if (ensure_user_capacity(1) != ERR_NONE) return ERR_MEMORY;
        User admin;
        memset(&admin, 0, sizeof(admin));
        admin.user_id = 0;
        strncpy(admin.username, "admin", MAX_USERNAME_LEN - 1);
        strncpy(admin.password, "admin123", MAX_PASSWORD_LEN - 1);
        admin.score = 0;
        admin.tier = TIER_IRON;
        admin.is_admin = 1;
        g_users[g_user_count++] = admin;
        save_users();
    }

    return ERR_NONE;
}

int init_problems(void) {
    int ret = load_problems_compatible();
    if (ret != ERR_NONE) return ret;
    return create_sample_problem_if_needed();
}

int init_submissions(void) {
    return load_binary_array(FILE_SUBMISSIONS, (void**)&g_submissions, &g_submission_count,
                             &g_submission_capacity, sizeof(Submission));
}

int init_promotions(void) {
    return load_binary_array(FILE_PROMOTIONS, (void**)&g_promotions, &g_promotion_count,
                             &g_promotion_capacity, sizeof(PromotionExam));
}

int save_users(void) {
    return save_binary_array(FILE_USERS, g_users, g_user_count, sizeof(User));
}

int save_problems(void) {
    return save_binary_array(FILE_PROBLEMS, g_problems, g_problem_count, sizeof(Problem));
}

int save_submissions(void) {
    return save_binary_array(FILE_SUBMISSIONS, g_submissions, g_submission_count, sizeof(Submission));
}

int save_promotions(void) {
    return save_binary_array(FILE_PROMOTIONS, g_promotions, g_promotion_count, sizeof(PromotionExam));
}

int save_submission(const Submission* sub) {
    if (sub == NULL) return ERR_NULL_PTR;
    if (ensure_submission_capacity(g_submission_count + 1) != ERR_NONE) return ERR_MEMORY;
    g_submissions[g_submission_count++] = *sub;
    return save_submissions();
}

int save_promotion(const PromotionExam* exam) {
    if (exam == NULL) return ERR_NULL_PTR;
    if (ensure_promotion_capacity(g_promotion_count + 1) != ERR_NONE) return ERR_MEMORY;
    g_promotions[g_promotion_count++] = *exam;
    return save_promotions();
}

int init_all(void) {
    ensure_directories();

    int ret;
    ret = init_users();       if (ret != ERR_NONE) return ret;
    ret = init_problems();    if (ret != ERR_NONE) return ret;
    ret = init_submissions(); if (ret != ERR_NONE) return ret;
    ret = init_promotions();  if (ret != ERR_NONE) return ret;

    return ERR_NONE;
}

void free_all(void) {
    save_users();
    save_problems();
    save_submissions();
    save_promotions();
    reset_state();
}
