#ifndef OAJ_H
#define OAJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =============================================
   SOJ / OAJ 공통 상수
   ============================================= */
#define MAX_USERS              1000
#define MAX_PROBLEMS            500
#define MAX_SUBMISSIONS        5000
#define MAX_PROMOTIONS         1000

#define MAX_USERNAME_LEN         50
#define MAX_PASSWORD_LEN         50
#define MAX_TITLE_LEN           200
#define MAX_DESC_LEN           2000
#define MAX_CATEGORY_LEN         150
#define MAX_RESULT_LEN           10
#define MAX_TIMESTAMP_LEN        20
#define MAX_FILEPATH_LEN        260

#define PROMOTION_PROBLEM_COUNT   3
#define PROMOTION_PASS_COUNT      2
#define PROMOTION_TIME_LIMIT   1800
#define PROMOTION_FAIL_PENALTY  100
#define TIER_COUNT               10

#define DEFAULT_COMPILE_CMD      "gcc"
#define DEFAULT_C_STANDARD       "c11"
#define DEFAULT_SCORE_MODE       0
#define DEFAULT_TIME_LIMIT       2

#define FILE_USERS        "data/users.dat"
#define FILE_PROBLEMS     "data/problems.dat"
#define FILE_SUBMISSIONS  "data/submissions.dat"
#define FILE_PROMOTIONS   "data/promotions.dat"
#define DIR_TESTCASES     "data/testcases"
#define DIR_WORKSPACE     "workspace"
#define DIR_SOURCES       "workspace/sources"
#define DIR_EXECUTABLES   "workspace/executables"
#define DIR_OUTPUTS       "workspace/outputs"
#define DIR_ERRORS        "workspace/errors"

#define ERR_NONE            0
#define ERR_FILE_OPEN      -1
#define ERR_NULL_PTR       -2
#define ERR_DUPLICATE_ID   -3
#define ERR_NOT_FOUND      -4
#define ERR_INVALID_INPUT  -5
#define ERR_TIME_OVER      -6
#define ERR_MEMORY         -7
#define ERR_SYSTEM         -8

/* =============================================
   열거형
   ============================================= */
typedef enum {
    TIER_IRON = 0,
    TIER_BRONZE,
    TIER_SILVER,
    TIER_GOLD,
    TIER_PLATINUM,
    TIER_EMERALD,
    TIER_DIAMOND,
    TIER_MASTER,
    TIER_GRANDMASTER,
    TIER_CHALLENGER
} Tier;

typedef enum {
    JUDGE_AC = 0,
    JUDGE_WA,
    JUDGE_TLE,
    JUDGE_RE,
    JUDGE_CE
} JudgeResult;

typedef enum {
    PROMO_IDLE = 0,
    PROMO_ONGOING,
    PROMO_SUCCESS,
    PROMO_FAIL
} PromoStatus;

typedef enum {
    SCORE_STANDARD = 0,
    SCORE_SPEEDRUN,
    SCORE_ACCURACY
} ScoreMode;

/* =============================================
   구조체
   ============================================= */
typedef struct {
    int  user_id;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int  score;
    int  tier;
    int  solved_count;
    int  submit_count;
    int  is_in_promotion;
    int  promotion_passed;
    char last_promo[MAX_TIMESTAMP_LEN];
    int  is_admin;
} User;

typedef struct {
    int  problem_id;
    char title[MAX_TITLE_LEN];
    char desc[MAX_DESC_LEN];         /* 문제 설명 */
    char input_desc[MAX_DESC_LEN];   /* 입력 형식 설명 */
    char output_desc[MAX_DESC_LEN];  /* 출력 형식 설명 */
    int  difficulty;       /* 1~5 */
    char category[MAX_CATEGORY_LEN];
    int  time_limit;       /* 각 테스트케이스 실행 제한, 초 */
    int  submit_count;
    int  correct_count;
    int  testcase_count;
} Problem;

typedef struct {
    int  submission_id;
    int  user_id;
    int  problem_id;
    char result[MAX_RESULT_LEN];
    int  time_taken;       /* 전체 테스트케이스 실행에 걸린 대략적 시간, 초 */
    int  attempt_count;
    int  score_earned;
    char timestamp[MAX_TIMESTAMP_LEN];
    char source_file[MAX_FILEPATH_LEN];
} Submission;

typedef struct {
    int  problem_id;
    int  case_index;
    char input_file[MAX_FILEPATH_LEN];
    char output_file[MAX_FILEPATH_LEN];
} TestCase;

typedef struct {
    int         promotion_id;
    int         user_id;
    int         problem_ids[PROMOTION_PROBLEM_COUNT];
    int         solved[PROMOTION_PROBLEM_COUNT];
    time_t      start_time;
    int         time_limit;
    PromoStatus result;
} PromotionExam;

/* 함수 포인터 */
typedef int (*CompareFunc)(const User*, const User*);
typedef int (*ScoreFunc)(int difficulty, int time_taken, int attempts);

/* 전역 데이터 */
extern User*       g_users;
extern int         g_user_count;
extern int         g_user_capacity;
extern Problem*    g_problems;
extern int         g_problem_count;
extern int         g_problem_capacity;
extern Submission* g_submissions;
extern int         g_submission_count;
extern int         g_submission_capacity;
extern PromotionExam* g_promotions;
extern int         g_promotion_count;
extern int         g_promotion_capacity;
extern User*       g_current_user;
extern ScoreMode   g_score_mode;

/* state.c */
int ensure_user_capacity(int required);
int ensure_problem_capacity(int required);
int ensure_submission_capacity(int required);
int ensure_promotion_capacity(int required);
void reset_state(void);

/* storage.c */
int  init_all(void);
void free_all(void);
int  ensure_directories(void);
int  init_users(void);
int  init_problems(void);
int  init_submissions(void);
int  init_promotions(void);
int  save_users(void);
int  save_problems(void);
int  save_submissions(void);
int  save_promotions(void);
int  save_submission(const Submission* sub);
int  save_promotion(const PromotionExam* exam);

/* utils.c */
void        get_current_timestamp(char* buf, int buf_size);
Tier        score_to_tier(int score);
const char* tier_to_string(Tier tier);
const char* judge_result_to_string(JudgeResult result);
const char* judge_result_to_display(JudgeResult result);
const char* judge_result_code_to_display(const char* result_code);
const char* promo_status_to_string(PromoStatus status);
int         get_tier_threshold(Tier tier);
void        clear_screen(void);
void        print_separator(void);
int         utf8_display_width(const char* s);
void        print_padded_utf8(const char* s, int width);
void        press_enter_to_continue(void);
int         file_exists(const char* path);
void        trim_newline(char* s);
int         copy_file(const char* src, const char* dst);

/* user.c */
int   register_user(const char* username, const char* password);
User* login_user(const char* username, const char* password);
void  logout_user(void);
User* find_user_by_id(int user_id);
User* find_user_by_username(const char* username);
void  print_user_profile(const User* user);
int   update_user_score(User* user, int delta_score);
int   update_tier(User* user);

/* problem.c *//*test*/
int      add_problem(const char* title, const char* desc,
                     const char* input_desc, const char* output_desc,
                     int difficulty, const char* category,
                     int time_limit, int testcase_count);
Problem* find_problem_by_id(int problem_id);
int      list_problems(int sort_by_difficulty);
int      search_problem_by_title(const char* keyword);
int      search_problem_by_category(const char* category);
void     print_problem_detail(const Problem* problem);
double   get_problem_correct_rate(const Problem* problem);
int      load_testcases(int problem_id, TestCase** out_cases);
void     free_testcases(TestCase* cases);
int      create_sample_problem_if_needed(void);

/* judge.c */
int         submit_source(User* user, int problem_id, const char* source_file);
JudgeResult judge_source(int problem_id, const char* source_file,
                         int submission_id, int* out_time_taken);
int         get_attempt_count(int user_id, int problem_id);
int         has_user_solved_problem(int user_id, int problem_id);
void        print_submission_history(int user_id);

/* runner.c */
JudgeResult compile_source(const char* source_file,
                           const char* exe_file,
                           const char* error_file);
JudgeResult run_testcase(const char* exe_file,
                         const char* input_file,
                         const char* user_output_file,
                         const char* run_error_file,
                         int time_limit);

/* compare.c */
int compare_output_files(const char* user_output_file,
                         const char* expected_output_file);

/* score.c */
int score_standard(int difficulty, int time_taken, int attempts);
int score_speedrun(int difficulty, int time_taken, int attempts);
int score_accuracy(int difficulty, int time_taken, int attempts);
int calculate_score(int difficulty, int time_taken, int attempts, ScoreFunc score_fn);
ScoreFunc get_score_func(ScoreMode mode);

/* ranking.c */
void show_ranking_all(CompareFunc cmp_fn);
void show_ranking_by_tier(Tier tier);
void sort_users(User* arr, int n, CompareFunc cmp_fn);
int  compare_by_score(const User* a, const User* b);
int  compare_by_solved(const User* a, const User* b);
int  compare_by_tier(const User* a, const User* b);

/* promotion.c */
int check_promotion_condition(const User* user);
int get_promotion_min_difficulty(Tier tier);
int get_promotion_max_difficulty(Tier tier);
const char* get_promotion_pool_description(Tier tier);
int count_promotion_candidates(const User* user);
int start_promotion(User* user, PromotionExam* out_exam);
int check_promotion_time(const PromotionExam* exam);
int submit_promotion_source(PromotionExam* exam, User* user,
                            int problem_index, const char* source_file);
int submit_promotion_source_silent(PromotionExam* exam, User* user,
                                   int problem_index, const char* source_file,
                                   JudgeResult* out_result,
                                   int* out_time_taken,
                                   int* out_newly_solved);
int finish_promotion(PromotionExam* exam, User* user);

/* menu.c */
void menu_main(void);
void menu_login(void);
void menu_register(void);
void menu_user_home(void);
void menu_problem_list(void);
void menu_problem_detail(int problem_id);
void menu_submit(int problem_id);
void menu_ranking(void);
void menu_my_profile(void);
void menu_promotion(void);
void menu_admin(void);

#endif
