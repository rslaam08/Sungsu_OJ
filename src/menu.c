#include "oaj.h"

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
        print_separator();
        printf("  1. 문제 목록/검색\n");
        printf("  2. 내 프로필\n");
        printf("  3. 랭킹 보기\n");
        if (g_current_user->is_admin) printf("  4. 관리자 메뉴\n");
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
            case 4:
                if (g_current_user->is_admin) menu_admin();
                break;
            case 0:
                logout_user();
                return;
            default:
                printf("잘못된 입력입니다.\n");
                press_enter_to_continue();
                break;
        }

        if (g_current_user && check_promotion_condition(g_current_user)) {
            printf("\n승급전 조건을 달성했습니다. 지금 진행할까요? (1: 예, 0: 아니오) > ");
            int promo_choice;
            scanf("%d", &promo_choice);
            getchar();
            if (promo_choice == 1) menu_promotion();
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

        if (choice == 1) {
            list_problems(0);
        } else if (choice == 2) {
            list_problems(1);
        } else if (choice == 3) {
            char keyword[100];
            read_line("검색할 제목 키워드: ", keyword, sizeof(keyword));
            search_problem_by_title(keyword);
        } else if (choice == 4) {
            char category[100];
            read_line("검색할 카테고리: ", category, sizeof(category));
            search_problem_by_category(category);
        } else if (choice == 5) {
            int problem_id;
            printf("문제 ID 입력 > ");
            scanf("%d", &problem_id);
            getchar();
            menu_problem_detail(problem_id);
            continue;
        } else {
            printf("잘못된 입력입니다.\n");
            continue;
        }

        printf("\n상세히 볼 문제 ID를 입력하세요. 뒤로 가려면 0 > ");
        int problem_id;
        if (scanf("%d", &problem_id) == 1) {
            getchar();
            if (problem_id != 0) menu_problem_detail(problem_id);
        } else {
            getchar();
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

    clear_screen();
    print_problem_detail(p);
    printf("  1. C 소스 파일 제출\n");
    printf("  0. 뒤로\n");
    print_separator();
    printf("선택 > ");

    int choice;
    scanf("%d", &choice);
    getchar();
    if (choice == 1) menu_submit(problem_id);
}

void menu_submit(int problem_id) {
    char source_file[MAX_FILEPATH_LEN];

    clear_screen();
    print_separator();
    printf("C 소스 파일 제출\n");
    print_separator();
    printf("예: examples/answer_1001.c\n");
    read_line("제출할 .c 파일 경로: ", source_file, sizeof(source_file));

    int ret = submit_source(g_current_user, problem_id, source_file);
    if (ret != ERR_NONE) printf("제출 실패. 코드: %d\n", ret);
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

    PromotionExam exam;
    int ret = start_promotion(g_current_user, &exam);
    if (ret != ERR_NONE) {
        printf("승급전을 시작할 수 없습니다. 코드: %d\n", ret);
        press_enter_to_continue();
        return;
    }

    while (1) {
        int remaining = check_promotion_time(&exam);
        if (remaining == ERR_TIME_OVER) {
            finish_promotion(&exam, g_current_user);
            break;
        }

        clear_screen();
        print_separator();
        printf("승급전 | 남은 시간 %02d:%02d | 통과 %d/%d\n",
               remaining / 60, remaining % 60,
               g_current_user->promotion_passed, PROMOTION_PASS_COUNT);
        print_separator();
        for (int i = 0; i < PROMOTION_PROBLEM_COUNT; i++) {
            Problem* p = find_problem_by_id(exam.problem_ids[i]);
            printf("%d. [%s] %d - %s\n", i + 1, exam.solved[i] ? "AC" : "미해결",
                   exam.problem_ids[i], p ? p->title : "문제 없음");
        }
        printf("0. 포기\n");
        print_separator();
        printf("선택 > ");

        int choice;
        scanf("%d", &choice);
        getchar();
        if (choice == 0) {
            finish_promotion(&exam, g_current_user);
            break;
        }
        if (choice < 1 || choice > PROMOTION_PROBLEM_COUNT) continue;

        char source_file[MAX_FILEPATH_LEN];
        read_line("제출할 .c 파일 경로: ", source_file, sizeof(source_file));
        submit_promotion_source(&exam, g_current_user, choice - 1, source_file);

        if (g_current_user->promotion_passed >= PROMOTION_PASS_COUNT) {
            finish_promotion(&exam, g_current_user);
            break;
        }
        press_enter_to_continue();
    }
    press_enter_to_continue();
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
