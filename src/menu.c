#include "oaj.h"
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#include <process.h>
#endif

static void read_line(const char* prompt, char* buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    trim_newline(buf);
}

static void read_multiline(const char* title, char* buffer, int buffer_size) {
    char line[512];

    if (buffer == NULL || buffer_size <= 0) return;
    buffer[0] = '\0';

    printf("%s\n", title);
    printf("여러 줄 입력 가능. 입력을 끝내려면 한 줄에 . 만 입력하세요.\n");

    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        if (strcmp(line, ".\n") == 0 || strcmp(line, ".\r\n") == 0 || strcmp(line, ".") == 0) {
            break;
        }

        if ((int)(strlen(buffer) + strlen(line) + 1) >= buffer_size) {
            printf("[안내] 최대 입력 길이에 도달했습니다.\n");
            break;
        }

        strcat(buffer, line);
    }

    trim_newline(buffer);
}

typedef void (*PromotionRenderFunc)(void* ctx, int remaining);

static void format_remaining(int remaining, char* buf, int buf_size) {
    if (remaining < 0) remaining = 0;
    snprintf(buf, (size_t)buf_size, "%02d:%02d", remaining / 60, remaining % 60);
}

#ifdef _WIN32
static int read_int_with_timer(PromotionExam* exam,
                               PromotionRenderFunc render,
                               void* ctx,
                               const char* prompt,
                               int* out_value) {
    char input[32] = "";
    int len = 0;
    time_t last_draw = 0;

    while (1) {
        int remaining = check_promotion_time(exam);
        if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;

        time_t now = time(NULL);
        if (now != last_draw) {
            render(ctx, remaining);
            printf("%s%s", prompt, input);
            fflush(stdout);
            last_draw = now;
        }

        if (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) { /* 방향키 등 확장키 무시 */
                if (_kbhit()) _getch();
                continue;
            }
            if (ch == '\r' || ch == '\n') {
                if (len == 0) continue;
                *out_value = atoi(input);
                printf("\n");
                return ERR_NONE;
            }
            if (ch == 8) { /* Backspace */
                if (len > 0) input[--len] = '\0';
                last_draw = 0;
                continue;
            }
            if ((isdigit(ch) || (ch == '-' && len == 0)) && len < (int)sizeof(input) - 1) {
                input[len++] = (char)ch;
                input[len] = '\0';
                last_draw = 0;
            }
        }
        Sleep(50);
    }
}

static int read_line_with_timer(PromotionExam* exam,
                                PromotionRenderFunc render,
                                void* ctx,
                                const char* prompt,
                                char* out,
                                int out_size) {
    int len = 0;
    time_t last_draw = 0;
    if (out == NULL || out_size <= 0) return ERR_INVALID_INPUT;
    out[0] = '\0';

    while (1) {
        int remaining = check_promotion_time(exam);
        if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;

        time_t now = time(NULL);
        if (now != last_draw) {
            render(ctx, remaining);
            printf("%s%s", prompt, out);
            fflush(stdout);
            last_draw = now;
        }

        if (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) {
                if (_kbhit()) _getch();
                continue;
            }
            if (ch == '\r' || ch == '\n') {
                printf("\n");
                return ERR_NONE;
            }
            if (ch == 8) {
                if (len > 0) out[--len] = '\0';
                last_draw = 0;
                continue;
            }
            if (ch >= 32 && ch < 256 && len < out_size - 1) {
                out[len++] = (char)ch;
                out[len] = '\0';
                last_draw = 0;
            }
        }
        Sleep(50);
    }
}
#else
static int read_int_with_timer(PromotionExam* exam,
                               PromotionRenderFunc render,
                               void* ctx,
                               const char* prompt,
                               int* out_value) {
    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;
    render(ctx, remaining);
    printf("%s", prompt);
    if (scanf("%d", out_value) != 1) {
        scanf("%*s");
        return ERR_INVALID_INPUT;
    }
    getchar();
    return ERR_NONE;
}

static int read_line_with_timer(PromotionExam* exam,
                                PromotionRenderFunc render,
                                void* ctx,
                                const char* prompt,
                                char* out,
                                int out_size) {
    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;
    render(ctx, remaining);
    read_line(prompt, out, out_size);
    return ERR_NONE;
}
#endif

static void print_promotion_time_line(int remaining, const User* user) {
    char time_text[16];
    format_remaining(remaining, time_text, sizeof(time_text));
    print_separator();
    printf("승급전 진행 중 | 남은 시간: %s | 통과: %d/%d\n",
           time_text,
           user ? user->promotion_passed : 0,
           PROMOTION_PASS_COUNT);
    print_separator();
}

typedef struct {
    PromotionExam* exam;
} PromotionListCtx;

typedef struct {
    PromotionExam* exam;
    int index;
} PromotionProblemCtx;

static void render_promotion_list(void* raw_ctx, int remaining) {
    PromotionListCtx* ctx = (PromotionListCtx*)raw_ctx;
    PromotionExam* exam = ctx->exam;

    clear_screen();
    print_promotion_time_line(remaining, g_current_user);
    printf("승급전 문제 목록\n");
    printf("문제 pool: %s | 이미 맞힌 문제는 출제 제외\n",
           g_current_user ? get_promotion_pool_description((Tier)g_current_user->tier) : "승급전 없음");
    print_separator();

    for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
        Problem* p = find_problem_by_id(exam->problem_ids[i]);
        printf("  %d. [%s] %d - %s\n",
               i + 1,
               exam->solved[i] ? "AC(ACCEPTED)" : "미해결",
               exam->problem_ids[i],
               p ? p->title : "문제 없음");
    }
    printf("  0. 포기\n");
    printf("\n선택 방법: 왼쪽 번호(1~3) 또는 실제 문제 ID를 입력하세요.\n");
    print_separator();
}


static int promotion_choice_to_index(const PromotionExam* exam, int choice) {
    if (exam == NULL) return -1;

    /* 기존 UI 번호 1~3으로 선택 */
    if (choice >= 1 && choice <= PROMOTION_PROBLEM_COUNT) {
        return choice - 1;
    }

    /* 실제 문제 ID로 선택 */
    for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
        if (exam->problem_ids[i] == choice) {
            return i;
        }
    }

    return -1;
}

static void render_promotion_problem(void* raw_ctx, int remaining) {
    PromotionProblemCtx* ctx = (PromotionProblemCtx*)raw_ctx;
    PromotionExam* exam = ctx->exam;
    int index = ctx->index;
    Problem* p = find_problem_by_id(exam->problem_ids[index]);

    clear_screen();
    print_promotion_time_line(remaining, g_current_user);
    printf("승급전 문제 %d/%d | 문제 ID: %d | 상태: %s\n",
           index + 1,
           PROMOTION_PROBLEM_COUNT,
           exam->problem_ids[index],
           exam->solved[index] ? "AC(ACCEPTED)" : "미해결");
    if (p) print_problem_detail(p);
    else printf("문제 정보를 찾을 수 없습니다.\n");

    printf("  1. C 소스 파일 제출\n");
    printf("  2. 다른 승급전 문제로 이동\n");
    printf("  0. 승급전 문제 목록으로\n");
    print_separator();
}


typedef struct {
    PromotionExam* exam;
    User* user;
    int problem_index;
    char source_file[MAX_FILEPATH_LEN];
    int ret;
    JudgeResult result;
    int time_taken;
    int newly_solved;
    int old_promotion_passed;
    int old_solved[PROMOTION_PROBLEM_COUNT];
    int done;
} PromotionSubmitCtx;

static void render_promotion_submit_status(void* raw_ctx, int remaining) {
    PromotionSubmitCtx* ctx = (PromotionSubmitCtx*)raw_ctx;
    Problem* p = NULL;
    int problem_id = 0;

    if (ctx && ctx->exam && ctx->problem_index >= 0 && ctx->problem_index < PROMOTION_PROBLEM_COUNT) {
        problem_id = ctx->exam->problem_ids[ctx->problem_index];
        p = find_problem_by_id(problem_id);
    }

    clear_screen();
    print_promotion_time_line(remaining, g_current_user);
    printf("승급전 채점 화면\n");
    print_separator();
    printf("문제: %d - %s\n", problem_id, p ? p->title : "문제 정보 없음");
    printf("제출 파일: %s\n", ctx ? ctx->source_file : "");
    print_separator();

    if (ctx == NULL || !ctx->done) {
        printf("채점 중입니다. 남은 시간은 계속 갱신됩니다.\n");
        printf("컴파일 또는 큰 테스트케이스 실행 중에는 잠시 걸릴 수 있습니다.\n");
        return;
    }

    if (ctx->ret == ERR_FILE_OPEN) {
        printf("[오류] 파일을 찾지 못했습니다: %s\n", ctx->source_file);
    } else if (ctx->ret == ERR_TIME_OVER) {
        printf("[시간 초과] 승급전 제한 시간이 지났습니다.\n");
    } else if (ctx->ret != ERR_NONE) {
        printf("[오류] 승급전 제출 실패. 코드: %d\n", ctx->ret);
    } else {
        printf("채점 결과: %s\n", judge_result_to_display(ctx->result));
        printf("실행 시간: 약 %d초\n", ctx->time_taken);
        printf("승급전 제출은 점수, 정답률, 제출 이력에 반영되지 않습니다.\n");
        if (ctx->result == JUDGE_AC) {
            if (ctx->newly_solved) {
                printf("승급전 통과 문제: %d / %d\n",
                       ctx->user ? ctx->user->promotion_passed : 0,
                       PROMOTION_PASS_COUNT);
            } else {
                printf("이미 맞힌 승급전 문제입니다.\n");
            }
        }
    }
}

#ifdef _WIN32
static unsigned __stdcall promotion_submit_worker(void* raw_ctx) {
    PromotionSubmitCtx* ctx = (PromotionSubmitCtx*)raw_ctx;
    ctx->ret = submit_promotion_source_silent(ctx->exam, ctx->user,
                                              ctx->problem_index, ctx->source_file,
                                              &ctx->result, &ctx->time_taken,
                                              &ctx->newly_solved);
    ctx->done = 1;
    return 0;
}
#endif

static int wait_enter_with_timer(PromotionExam* exam,
                                 PromotionRenderFunc render,
                                 void* ctx,
                                 const char* prompt) {
#ifdef _WIN32
    time_t last_draw = 0;
    while (1) {
        int remaining = check_promotion_time(exam);
        if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;

        time_t now = time(NULL);
        if (now != last_draw) {
            render(ctx, remaining);
            printf("%s", prompt ? prompt : "계속하려면 Enter를 누르세요...");
            fflush(stdout);
            last_draw = now;
        }

        if (_kbhit()) {
            int ch = _getch();
            if (ch == 0 || ch == 224) {
                if (_kbhit()) _getch();
                continue;
            }
            if (ch == '\r' || ch == '\n') {
                printf("\n");
                return ERR_NONE;
            }
        }
        Sleep(50);
    }
#else
    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) return ERR_TIME_OVER;
    render(ctx, remaining);
    printf("%s", prompt ? prompt : "계속하려면 Enter를 누르세요...");
    getchar();
    return ERR_NONE;
#endif
}

static int run_promotion_submission_with_timer(PromotionExam* exam,
                                               PromotionProblemCtx* problem_ctx,
                                               const char* source_file) {
    PromotionSubmitCtx submit_ctx;
    memset(&submit_ctx, 0, sizeof(submit_ctx));
    submit_ctx.exam = exam;
    submit_ctx.user = g_current_user;
    submit_ctx.problem_index = problem_ctx ? problem_ctx->index : -1;
    submit_ctx.ret = ERR_NONE;
    submit_ctx.result = JUDGE_RE;
    submit_ctx.old_promotion_passed = g_current_user ? g_current_user->promotion_passed : 0;
    if (exam != NULL) {
        for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
            submit_ctx.old_solved[i] = exam->solved[i];
        }
    }
    strncpy(submit_ctx.source_file, source_file ? source_file : "", MAX_FILEPATH_LEN - 1);

#ifdef _WIN32
    uintptr_t th = _beginthreadex(NULL, 0, promotion_submit_worker, &submit_ctx, 0, NULL);
    if (th == 0) {
        submit_ctx.ret = submit_promotion_source_silent(exam, g_current_user,
                                                        submit_ctx.problem_index,
                                                        submit_ctx.source_file,
                                                        &submit_ctx.result,
                                                        &submit_ctx.time_taken,
                                                        &submit_ctx.newly_solved);
        submit_ctx.done = 1;
    } else {
        HANDLE handle = (HANDLE)th;
        time_t last_draw = 0;
        while (WaitForSingleObject(handle, 50) == WAIT_TIMEOUT) {
            int remaining = check_promotion_time(exam);
            if (remaining == ERR_TIME_OVER) {
                render_promotion_submit_status(&submit_ctx, 0);
                printf("승급전 시간이 종료되었습니다. 진행 중인 채점이 끝나면 실패 처리됩니다.\n");
                fflush(stdout);
                WaitForSingleObject(handle, INFINITE);
                CloseHandle(handle);
                if (g_current_user) g_current_user->promotion_passed = submit_ctx.old_promotion_passed;
                if (exam != NULL) {
                    for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
                        exam->solved[i] = submit_ctx.old_solved[i];
                    }
                }
                submit_ctx.ret = ERR_TIME_OVER;
                submit_ctx.done = 1;
                return ERR_TIME_OVER;
            }
            time_t now = time(NULL);
            if (now != last_draw) {
                render_promotion_submit_status(&submit_ctx, remaining);
                fflush(stdout);
                last_draw = now;
            }
        }
        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
    }
#else
    submit_ctx.ret = submit_promotion_source_silent(exam, g_current_user,
                                                    submit_ctx.problem_index,
                                                    submit_ctx.source_file,
                                                    &submit_ctx.result,
                                                    &submit_ctx.time_taken,
                                                    &submit_ctx.newly_solved);
    submit_ctx.done = 1;
#endif

    int remaining = check_promotion_time(exam);
    if (remaining == ERR_TIME_OVER) {
        render_promotion_submit_status(&submit_ctx, 0);
        return ERR_TIME_OVER;
    }

    int wait_ret = wait_enter_with_timer(exam, render_promotion_submit_status,
                                         &submit_ctx,
                                         "계속하려면 Enter를 누르세요...");
    if (wait_ret == ERR_TIME_OVER) return ERR_TIME_OVER;
    return submit_ctx.ret;
}

static int menu_promotion_problem_detail(PromotionExam* exam, int start_index) {
    if (exam == NULL) return 0;

    PromotionProblemCtx ctx;
    ctx.exam = exam;
    ctx.index = start_index;

    while (1) {
        int choice = -1;
        int ret = read_int_with_timer(exam, render_promotion_problem, &ctx, "선택 > ", &choice);
        if (ret == ERR_TIME_OVER) return ERR_TIME_OVER;

        if (choice == 0) return ERR_NONE;

        if (choice == 1) {
            char source_file[MAX_FILEPATH_LEN];
            ret = read_line_with_timer(exam, render_promotion_problem, &ctx,
                                       "제출할 .c 파일 경로: ", source_file, sizeof(source_file));
            if (ret == ERR_TIME_OVER) return ERR_TIME_OVER;

            int submit_ret = run_promotion_submission_with_timer(exam, &ctx, source_file);
            if (submit_ret == ERR_TIME_OVER) return ERR_TIME_OVER;

            if (g_current_user && g_current_user->promotion_passed >= PROMOTION_PASS_COUNT) {
                return 1; /* 성공 조건 달성 */
            }
        } else if (choice == 2) {
            int target = -1;
            ret = read_int_with_timer(exam, render_promotion_problem, &ctx,
                                      "이동할 승급전 문제 번호(1~3) 또는 문제 ID, 0: ", &target);
            if (ret == ERR_TIME_OVER) return ERR_TIME_OVER;
            if (target == 0) {
                return ERR_NONE;
            }
            int target_index = promotion_choice_to_index(exam, target);
            if (target_index >= 0) {
                ctx.index = target_index;
            } else {
                printf("[오류] 해당 승급전 문제를 찾지 못했습니다.\n");
                ret = wait_enter_with_timer(exam, render_promotion_problem, &ctx,
                                            "계속하려면 Enter를 누르세요...");
                if (ret == ERR_TIME_OVER) return ERR_TIME_OVER;
            }
        }
    }
}

void menu_main(void) {
    int choice;
    while (1) {
        clear_screen();
        print_separator();
        printf("   SOJ - SUNGSOO Offline Judge\n");
        print_separator();
        printf("  1. 로그인\n");
        printf("  2. 회원가입\n");
        printf("  0. 종료\n");
        print_separator();
        printf("선택 > ");

        if (scanf("%d", &choice) != 1) {
            scanf("%*s");
            continue;
        }
        getchar();

        switch (choice) {
            case 1: menu_login(); break;
            case 2: menu_register(); break;
            case 0: return;
            default:
                printf("잘못된 입력입니다.\n");
                press_enter_to_continue();
                break;
        }
    }
}

void menu_login(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    clear_screen();
    print_separator();
    printf("로그인\n");
    print_separator();

    read_line("아이디: ", username, sizeof(username));
    read_line("비밀번호: ", password, sizeof(password));

    User* user = login_user(username, password);
    if (user == NULL) {
        printf("로그인 실패.\n");
        press_enter_to_continue();
        return;
    }

    printf("%s님 환영합니다.\n", user->username);
    press_enter_to_continue();
    menu_user_home();
}

void menu_register(void) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char confirm[MAX_PASSWORD_LEN];

    clear_screen();
    print_separator();
    printf("회원가입\n");
    print_separator();

    read_line("사용할 아이디: ", username, sizeof(username));
    read_line("비밀번호: ", password, sizeof(password));
    read_line("비밀번호 확인: ", confirm, sizeof(confirm));

    if (strcmp(password, confirm) != 0) {
        printf("비밀번호가 일치하지 않습니다.\n");
        press_enter_to_continue();
        return;
    }

    int ret = register_user(username, password);
    if (ret == ERR_NONE) printf("회원가입이 완료되었습니다.\n");
    else if (ret == ERR_DUPLICATE_ID) printf("이미 존재하는 아이디입니다.\n");
    else printf("회원가입 실패. 코드: %d\n", ret);
    press_enter_to_continue();
}

void menu_user_home(void) {
    int choice;
    while (g_current_user != NULL) {
        clear_screen();
        print_separator();
        printf("%s님 | %s | %d점\n", g_current_user->username,
               tier_to_string((Tier)g_current_user->tier), g_current_user->score);
        if (check_promotion_condition(g_current_user)) {
            printf("[알림] 승급전 진행 가능! 4번 승급전 메뉴에서 시작할 수 있습니다.\n");
        }
        print_separator();
        printf("  1. 문제 목록/검색\n");
        printf("  2. 내 프로필\n");
        printf("  3. 랭킹 보기\n");
        printf("  4. 승급전\n");
        if (g_current_user->is_admin) printf("  5. 관리자 메뉴\n");
        printf("  0. 로그아웃\n");
        print_separator();
        printf("선택 > ");

        if (scanf("%d", &choice) != 1) {
            scanf("%*s");
            continue;
        }
        getchar();

        switch (choice) {
            case 1: menu_problem_list(); break;
            case 2: menu_my_profile(); break;
            case 3: menu_ranking(); break;
            case 4: menu_promotion(); break;
            case 5:
                if (g_current_user->is_admin) menu_admin();
                else {
                    printf("잘못된 입력입니다.\n");
                    press_enter_to_continue();
                }
                break;
            case 0:
                logout_user();
                return;
            default:
                printf("잘못된 입력입니다.\n");
                press_enter_to_continue();
                break;
        }
    }
}

void menu_problem_list(void) {
    int choice;
    while (1) {
        clear_screen();
        print_separator();
        printf("문제 목록/검색\n");
        print_separator();
        printf("  1. 전체 목록(ID순)\n");
        printf("  2. 난이도순 목록\n");
        printf("  3. 제목 검색\n");
        printf("  4. 카테고리 검색\n");
        printf("  5. 문제 ID로 바로 보기\n");
        printf("  0. 뒤로\n");
        print_separator();
        printf("선택 > ");

        if (scanf("%d", &choice) != 1) {
            scanf("%*s");
            continue;
        }
        getchar();

        if (choice == 0) return;

        int selected_problem_id = 0;

        if (choice == 1) {
            selected_problem_id = list_problems(0);
        } else if (choice == 2) {
            selected_problem_id = list_problems(1);
        } else if (choice == 3) {
            char keyword[100];
            read_line("검색할 제목 키워드: ", keyword, sizeof(keyword));
            selected_problem_id = search_problem_by_title(keyword);
        } else if (choice == 4) {
            char category[100];
            read_line("검색할 카테고리: ", category, sizeof(category));
            selected_problem_id = search_problem_by_category(category);
        } else if (choice == 5) {
            int problem_id;
            printf("문제 ID 입력 > ");
            if (scanf("%d", &problem_id) == 1) {
                getchar();
                selected_problem_id = problem_id;
            } else {
                scanf("%*s");
                selected_problem_id = 0;
            }
        } else {
            printf("잘못된 입력입니다.\n");
            press_enter_to_continue();
            continue;
        }

        if (selected_problem_id != 0) {
            menu_problem_detail(selected_problem_id);
        }
    }
}

void menu_problem_detail(int problem_id) {
    Problem* p = find_problem_by_id(problem_id);
    if (p == NULL) {
        printf("문제를 찾을 수 없습니다.\n");
        press_enter_to_continue();
        return;
    }

    while (1) {
        clear_screen();
        print_problem_detail(p);
        if (g_current_user && has_user_solved_problem(g_current_user->user_id, problem_id)) {
            printf("[v] 이미 맞힌 문제입니다. 다시 제출할 수 있지만 추가 점수는 없습니다.\n");
        }
        printf("  1. C 소스 파일 제출\n");
        printf("  0. 뒤로\n");
        print_separator();
        printf("선택 > ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            scanf("%*s");
            continue;
        }
        getchar();
        if (choice == 0) return;
        if (choice == 1) menu_submit(problem_id);
    }
}

void menu_submit(int problem_id) {
    char source_file[MAX_FILEPATH_LEN];

    clear_screen();
    print_separator();
    printf("C 소스 파일 제출\n");
    print_separator();
    printf("제출할 C 파일은 code 폴더에 넣는 것을 권장합니다.\n");
    printf("예: code/answer_1001.c\n");
    read_line("제출할 .c 파일 경로: ", source_file, sizeof(source_file));

    int ret = submit_source(g_current_user, problem_id, source_file);
    if (ret == ERR_FILE_OPEN) {
        printf("[오류] 파일을 찾지 못했습니다: %s\n", source_file);
    } else if (ret != ERR_NONE) {
        printf("제출 실패. 코드: %d\n", ret);
    }
    press_enter_to_continue();
}

void menu_ranking(void) {
    int choice;
    clear_screen();
    print_separator();
    printf("랭킹\n");
    print_separator();
    printf("  1. 전체 랭킹(점수순)\n");
    printf("  2. 전체 랭킹(푼 문제 수순)\n");
    printf("  3. 전체 랭킹(티어순)\n");
    printf("  4. 티어별 랭킹\n");
    printf("  0. 뒤로\n");
    print_separator();
    printf("선택 > ");
    scanf("%d", &choice);
    getchar();

    if (choice == 0) return;
    clear_screen();

    switch (choice) {
        case 1: show_ranking_all(compare_by_score); break;
        case 2: show_ranking_all(compare_by_solved); break;
        case 3: show_ranking_all(compare_by_tier); break;
        case 4: {
            for (int i = 0; i < TIER_COUNT; i++) printf("%d. %s\n", i, tier_to_string((Tier)i));
            printf("티어 번호 선택 > ");
            int t;
            scanf("%d", &t);
            getchar();
            if (t >= 0 && t < TIER_COUNT) show_ranking_by_tier((Tier)t);
            else printf("잘못된 티어입니다.\n");
            break;
        }
        default: printf("잘못된 입력입니다.\n"); break;
    }
    press_enter_to_continue();
}

void menu_my_profile(void) {
    clear_screen();
    print_user_profile(g_current_user);
    printf("\n제출 이력\n");
    print_submission_history(g_current_user ? g_current_user->user_id : -1);
    press_enter_to_continue();
}

void menu_promotion(void) {
    if (g_current_user == NULL) return;

    clear_screen();
    print_separator();
    printf("승급전 메뉴\n");
    print_separator();

    if (g_current_user->tier >= TIER_CHALLENGER) {
        printf("이미 최고 티어입니다. 챌린저는 승급전이 없습니다.\n");
        press_enter_to_continue();
        return;
    }

    if (!check_promotion_condition(g_current_user)) {
        int need = get_tier_threshold((Tier)g_current_user->tier);
        printf("아직 승급전 조건을 만족하지 못했습니다.\n");
        printf("현재 점수: %d점 / 필요 점수: %d점\n", g_current_user->score, need);
        press_enter_to_continue();
        return;
    }

    int available = count_promotion_candidates(g_current_user);
    if (available < PROMOTION_PROBLEM_COUNT) {
        printf("승급전으로 출제할 수 있는 미해결 문제가 부족합니다.\n");
        printf("현재 승급전 문제 pool: %s\n", get_promotion_pool_description((Tier)g_current_user->tier));
        printf("필요 문제 수: %d개 / 가능한 문제 수: %d개\n", PROMOTION_PROBLEM_COUNT, available);
        printf("관리자 계정으로 해당 난이도 범위의 문제를 더 추가하거나, 문제 세트를 교체하세요.\n");
        press_enter_to_continue();
        return;
    }

    Tier current_tier = (Tier)g_current_user->tier;
    Tier next_tier = (Tier)(g_current_user->tier + 1);
    printf("승급전 가능 상태입니다.\n");
    printf("현재 티어: %s -> %s\n", tier_to_string(current_tier), tier_to_string(next_tier));
    printf("문제 pool: %s / 미해결 문제 중 %d문제 출제\n", get_promotion_pool_description(current_tier), PROMOTION_PROBLEM_COUNT);
    printf("승급 조건: 30분 안에 %d문제 이상 AC(ACCEPTED)\n", PROMOTION_PASS_COUNT);
    print_separator();
    printf("  1. 승급전 시작\n");
    printf("  0. 뒤로\n");
    print_separator();
    printf("선택 > ");

    int start_choice;
    if (scanf("%d", &start_choice) != 1) {
        scanf("%*s");
        return;
    }
    getchar();
    if (start_choice != 1) return;

    PromotionExam exam;
    int ret = start_promotion(g_current_user, &exam);
    if (ret != ERR_NONE) {
        printf("승급전을 시작할 수 없습니다. 코드: %d\n", ret);
        press_enter_to_continue();
        return;
    }

    PromotionListCtx list_ctx;
    list_ctx.exam = &exam;

    ret = wait_enter_with_timer(&exam, render_promotion_list, &list_ctx,
                                "승급전이 시작되었습니다. 계속하려면 Enter를 누르세요...");
    if (ret == ERR_TIME_OVER) {
        clear_screen();
        printf("[시간 초과] 승급전이 종료됩니다.\n");
        finish_promotion(&exam, g_current_user);
        press_enter_to_continue();
        return;
    }

    while (1) {
        if (g_current_user->promotion_passed >= PROMOTION_PASS_COUNT) {
            clear_screen();
            finish_promotion(&exam, g_current_user);
            press_enter_to_continue();
            return;
        }

        int choice = -1;
        ret = read_int_with_timer(&exam, render_promotion_list, &list_ctx, "선택 > ", &choice);
        if (ret == ERR_TIME_OVER) {
            clear_screen();
            printf("[시간 초과] 승급전이 종료됩니다.\n");
            finish_promotion(&exam, g_current_user);
            press_enter_to_continue();
            return;
        }

        if (choice == 0) {
            clear_screen();
            printf("승급전을 포기했습니다.\n");
            finish_promotion(&exam, g_current_user);
            press_enter_to_continue();
            return;
        }

        int selected_index = promotion_choice_to_index(&exam, choice);
        if (selected_index >= 0) {
            int sub_ret = menu_promotion_problem_detail(&exam, selected_index);
            if (sub_ret == ERR_TIME_OVER) {
                clear_screen();
                printf("[시간 초과] 승급전이 종료됩니다.\n");
                finish_promotion(&exam, g_current_user);
                press_enter_to_continue();
                return;
            }
            if (sub_ret == 1 || g_current_user->promotion_passed >= PROMOTION_PASS_COUNT) {
                clear_screen();
                finish_promotion(&exam, g_current_user);
                press_enter_to_continue();
                return;
            }
        } else {
            printf("[오류] 해당 승급전 문제를 찾지 못했습니다.\n");
            ret = wait_enter_with_timer(&exam, render_promotion_list, &list_ctx,
                                        "계속하려면 Enter를 누르세요...");
            if (ret == ERR_TIME_OVER) {
                clear_screen();
                printf("[시간 초과] 승급전이 종료됩니다.\n");
                finish_promotion(&exam, g_current_user);
                press_enter_to_continue();
                return;
            }
        }
    }
}

void menu_admin(void) {
    int choice;
    while (1) {
        clear_screen();
        print_separator();
        printf("관리자 메뉴\n");
        print_separator();
        printf("  1. 문제 등록\n");
        printf("  2. 유저 목록 조회\n");
        printf("  0. 뒤로\n");
        print_separator();
        printf("선택 > ");

        if (scanf("%d", &choice) != 1) {
            scanf("%*s");
            continue;
        }
        getchar();

        if (choice == 0) return;

        if (choice == 1) {
            char title[MAX_TITLE_LEN];
            char desc[MAX_DESC_LEN];
            char input_desc[MAX_DESC_LEN];
            char output_desc[MAX_DESC_LEN];
            char category[MAX_CATEGORY_LEN];
            int difficulty, time_limit, testcase_count;

            clear_screen();
            print_separator();
            printf("문제 등록\n");
            print_separator();
            read_line("제목: ", title, sizeof(title));
            read_multiline("문제 설명:", desc, sizeof(desc));
            read_multiline("입력:", input_desc, sizeof(input_desc));
            read_multiline("출력:", output_desc, sizeof(output_desc));
            read_line("카테고리: ", category, sizeof(category));
            printf("난이도(1~5): "); scanf("%d", &difficulty); getchar();
            printf("시간 제한(초): "); scanf("%d", &time_limit); getchar();
            printf("테스트케이스 수: "); scanf("%d", &testcase_count); getchar();

            int ret = add_problem(title, desc, input_desc, output_desc, difficulty, category, time_limit, testcase_count);
            if (ret == ERR_NONE) {
                int new_id = g_problems[g_problem_count - 1].problem_id;
                printf("문제 등록 완료. 문제 ID: %d\n", new_id);
                printf("다음 경로에 input/output 파일을 배치하세요:\n");
                printf("data/testcases/%d/input1.txt\n", new_id);
                printf("data/testcases/%d/output1.txt\n", new_id);
            } else {
                printf("문제 등록 실패. 코드: %d\n", ret);
            }
            press_enter_to_continue();
        } else if (choice == 2) {
            show_ranking_all(compare_by_score);
            press_enter_to_continue();
        } else {
            printf("잘못된 입력입니다.\n");
            press_enter_to_continue();
        }
    }
}
