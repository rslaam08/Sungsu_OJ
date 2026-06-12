#include "oaj.h"

static int next_problem_id(void) {
    int max_id = 1000;
    for (int i = 0; i < g_problem_count; i++) {
        if (g_problems[i].problem_id > max_id) max_id = g_problems[i].problem_id;
    }
    return max_id + 1;
}

static void make_testcase_dir(int problem_id) {
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "if not exist data\\testcases\\%d mkdir data\\testcases\\%d", problem_id, problem_id);
#else
    snprintf(cmd, sizeof(cmd), "mkdir -p data/testcases/%d", problem_id);
#endif
    system(cmd);
}

int add_problem(const char* title, const char* desc,
                const char* input_desc, const char* output_desc,
                int difficulty, const char* category,
                int time_limit, int testcase_count) {
    if (title == NULL || desc == NULL || input_desc == NULL || output_desc == NULL || category == NULL) return ERR_NULL_PTR;
    if (difficulty < 1 || difficulty > 5 || time_limit <= 0 || testcase_count <= 0)
        return ERR_INVALID_INPUT;
    if (g_problem_count >= MAX_PROBLEMS) return ERR_INVALID_INPUT;
    if (ensure_problem_capacity(g_problem_count + 1) != ERR_NONE) return ERR_MEMORY;

    Problem p;
    memset(&p, 0, sizeof(p));
    p.problem_id = next_problem_id();
    strncpy(p.title, title, MAX_TITLE_LEN - 1);
    strncpy(p.desc, desc, MAX_DESC_LEN - 1);
    strncpy(p.input_desc, input_desc, MAX_DESC_LEN - 1);
    strncpy(p.output_desc, output_desc, MAX_DESC_LEN - 1);
    p.difficulty = difficulty;
    strncpy(p.category, category, MAX_CATEGORY_LEN - 1);
    p.time_limit = time_limit;
    p.testcase_count = testcase_count;

    g_problems[g_problem_count++] = p;
    make_testcase_dir(p.problem_id);
    return save_problems();
}

Problem* find_problem_by_id(int problem_id) {
    for (int i = 0; i < g_problem_count; i++) {
        if (g_problems[i].problem_id == problem_id) return &g_problems[i];
    }
    return NULL;
}

static int compare_problem_id(const void* a, const void* b) {
    const Problem* pa = (const Problem*)a;
    const Problem* pb = (const Problem*)b;
    return pa->problem_id - pb->problem_id;
}

static int compare_problem_difficulty(const void* a, const void* b) {
    const Problem* pa = (const Problem*)a;
    const Problem* pb = (const Problem*)b;
    if (pa->difficulty != pb->difficulty) return pa->difficulty - pb->difficulty;
    return pa->problem_id - pb->problem_id;
}

static const char* solved_marker(int problem_id) {
    if (g_current_user != NULL && has_user_solved_problem(g_current_user->user_id, problem_id)) {
        return "[v] ";
    }
    return "    ";
}

#define PROBLEM_PAGE_SIZE 10

static void print_problem_summary(const Problem* problem) {
    if (problem == NULL) return;
    printf("%s[%d] %s\n", solved_marker(problem->problem_id), problem->problem_id, problem->title);
    printf("     난이도: %d | 카테고리: %s\n", problem->difficulty, problem->category);
    printf("     정답률: %.2f%% | 제출: %d\n", get_problem_correct_rate(problem), problem->submit_count);
}

static int result_contains_problem_id(const Problem* arr, int count, int problem_id) {
    for (int i = 0; i < count; i++) {
        if (arr[i].problem_id == problem_id) return 1;
    }
    return 0;
}

static int paginate_problem_results(Problem* arr, int count, const char* title) {
    if (count <= 0) {
        printf("검색 결과가 없습니다.\n");
        return 0;
    }

    int page = 0;
    int total_pages = (count + PROBLEM_PAGE_SIZE - 1) / PROBLEM_PAGE_SIZE;
    char cmd[64];

    while (1) {
        clear_screen();
        print_separator();
        printf("%s\n", title ? title : "문제 목록");
        printf("페이지 %d / %d | 총 %d문제\n", page + 1, total_pages, count);
        print_separator();

        int begin = page * PROBLEM_PAGE_SIZE;
        int end = begin + PROBLEM_PAGE_SIZE;
        if (end > count) end = count;

        for (int i = begin; i < end; i++) {
            print_problem_summary(&arr[i]);
            if (i != end - 1) printf("\n");
        }

        print_separator();
        printf("문제 ID 입력: 상세 보기 | n: 다음 페이지 | p: 이전 페이지 | 0: 뒤로\n");
        printf("선택 > ");

        if (fgets(cmd, sizeof(cmd), stdin) == NULL) return 0;
        trim_newline(cmd);

        if (strcmp(cmd, "0") == 0 || strcmp(cmd, "q") == 0 || strcmp(cmd, "Q") == 0) {
            return 0;
        }
        if (strcmp(cmd, "n") == 0 || strcmp(cmd, "N") == 0) {
            if (page + 1 < total_pages) page++;
            else {
                printf("마지막 페이지입니다.\n");
                press_enter_to_continue();
            }
            continue;
        }
        if (strcmp(cmd, "p") == 0 || strcmp(cmd, "P") == 0) {
            if (page > 0) page--;
            else {
                printf("첫 페이지입니다.\n");
                press_enter_to_continue();
            }
            continue;
        }

        char* endptr = NULL;
        long id = strtol(cmd, &endptr, 10);
        if (endptr != cmd && *endptr == '\0') {
            if (result_contains_problem_id(arr, count, (int)id)) return (int)id;
            printf("현재 결과 목록에 없는 문제 ID입니다.\n");
            press_enter_to_continue();
            continue;
        }

        printf("잘못된 입력입니다. 문제 ID, n, p, 0 중 하나를 입력하세요.\n");
        press_enter_to_continue();
    }
}

int list_problems(int sort_by_difficulty) {
    if (g_problem_count == 0) {
        printf("등록된 문제가 없습니다.\n");
        press_enter_to_continue();
        return 0;
    }

    Problem* tmp = (Problem*)malloc(sizeof(Problem) * (size_t)g_problem_count);
    if (tmp == NULL) {
        printf("[오류] 메모리 할당 실패.\n");
        press_enter_to_continue();
        return 0;
    }

    memcpy(tmp, g_problems, sizeof(Problem) * (size_t)g_problem_count);
    qsort(tmp, (size_t)g_problem_count, sizeof(Problem),
          sort_by_difficulty ? compare_problem_difficulty : compare_problem_id);

    int selected = paginate_problem_results(tmp, g_problem_count,
                                            sort_by_difficulty ? "문제 목록 - 난이도순" : "문제 목록 - ID순");
    free(tmp);
    return selected;
}

int search_problem_by_title(const char* keyword) {
    if (keyword == NULL) return 0;

    Problem* results = (Problem*)malloc(sizeof(Problem) * (size_t)g_problem_count);
    if (results == NULL) {
        printf("[오류] 메모리 할당 실패.\n");
        press_enter_to_continue();
        return 0;
    }

    int count = 0;
    for (int i = 0; i < g_problem_count; i++) {
        if (strstr(g_problems[i].title, keyword) != NULL) {
            results[count++] = g_problems[i];
        }
    }

    int selected = paginate_problem_results(results, count, "제목 검색 결과");
    free(results);
    return selected;
}

int search_problem_by_category(const char* category) {
    if (category == NULL) return 0;

    Problem* results = (Problem*)malloc(sizeof(Problem) * (size_t)g_problem_count);
    if (results == NULL) {
        printf("[오류] 메모리 할당 실패.\n");
        press_enter_to_continue();
        return 0;
    }

    int count = 0;
    for (int i = 0; i < g_problem_count; i++) {
        if (strstr(g_problems[i].category, category) != NULL) {
            results[count++] = g_problems[i];
        }
    }

    int selected = paginate_problem_results(results, count, "카테고리 검색 결과");
    free(results);
    return selected;
}

void print_problem_detail(const Problem* problem) {
    if (problem == NULL) {
        printf("[오류] 문제 정보가 없습니다.\n");
        return;
    }

    print_separator();
    printf("문제 ID   : %d\n", problem->problem_id);
    printf("제목      : %s\n", problem->title);
    printf("난이도    : %d\n", problem->difficulty);
    printf("카테고리  : %s\n", problem->category);
    printf("시간 제한 : %d초 / 테스트케이스\n", problem->time_limit);
    printf("테스트 수 : %d개\n", problem->testcase_count);
    printf("정답률    : %.2f%%\n", get_problem_correct_rate(problem));
    print_separator();
    printf("[문제 설명]\n");
    printf("%s\n", problem->desc[0] ? problem->desc : "(설명 없음)");
    print_separator();
    printf("[입력]\n");
    printf("%s\n", problem->input_desc[0] ? problem->input_desc : "(입력 설명 없음)");
    print_separator();
    printf("[출력]\n");
    printf("%s\n", problem->output_desc[0] ? problem->output_desc : "(출력 설명 없음)");
    print_separator();
}

double get_problem_correct_rate(const Problem* problem) {
    if (problem == NULL || problem->submit_count <= 0) return 0.0;
    return ((double)problem->correct_count / (double)problem->submit_count) * 100.0;
}

int load_testcases(int problem_id, TestCase** out_cases) {
    if (out_cases == NULL) return ERR_NULL_PTR;
    Problem* p = find_problem_by_id(problem_id);
    if (p == NULL) return ERR_NOT_FOUND;

    TestCase* cases = (TestCase*)malloc(sizeof(TestCase) * (size_t)p->testcase_count);
    if (cases == NULL) return ERR_MEMORY;

    for (int i = 0; i < p->testcase_count; i++) {
        cases[i].problem_id = problem_id;
        cases[i].case_index = i + 1;
        snprintf(cases[i].input_file, MAX_FILEPATH_LEN, "%s/%d/input%d.txt", DIR_TESTCASES, problem_id, i + 1);
        snprintf(cases[i].output_file, MAX_FILEPATH_LEN, "%s/%d/output%d.txt", DIR_TESTCASES, problem_id, i + 1);
    }

    *out_cases = cases;
    return p->testcase_count;
}

void free_testcases(TestCase* cases) {
    free(cases);
}

int create_sample_problem_if_needed(void) {
    typedef struct {
        int id; const char* title; const char* desc; const char* input_desc; const char* output_desc;
        int difficulty; const char* category; int testcase_count;
    } DefaultProblem;

    const DefaultProblem defaults[] = {
        {1001, "위대하신 우리의 창조주 <박성수> 만만세", "당신이 지금 사용하고 있는 SOJ(Sungsu Offline Judge)는 서울과학고 36기 <박성수>님의 은혜로 인해 유지되고 있다. 이에 감사하는 마음을 표해보자.", "입력은 없다.", "첫 줄에 \"위대하신 우리의 창조주 <박성수> 만만세\"를 출력한다.", 1, "입출력", 1},
        {1002, "A+B", "자연수 두 개를 입력받은 뒤 덧셈의 결과를 출력한다.", "첫 줄에 자연수 A, B가 공백을 사이에 두고 주어진다. (A, B <= 100000)", "첫 줄에 A+B를 출력한다.", 1, "입출력", 10},
        {1003, "A-B", "자연수 두 개를 입력받은 뒤 뺄셈의 결과를 출력한다.", "첫 줄에 자연수 A, B가 공백을 사이에 두고 주어진다. (A, B <= 100000)", "첫 줄에 A-B를 출력한다.", 1, "입출력", 10},
        {1004, "윤창이를 찾아라", "설곽이는 서울과학고 학생들의 몸무게만 보고 가장 무거운 학생인 \"이윤창\"을 찾아야 한다. 설곽이가 윤창이를 찾는 걸 도와주자!", "첫 줄에 학생 수 n이 주어진다. (1 <= n <= 1000) 두 번째 줄에 서로 다른 n개의 자연수 a_1, a_2 ... a_n 이 주어진다. (1 <= a_i <= 10000)", "첫 줄에 윤창이가 몇 번째로 서 있는지를 출력한다.", 1, "수학", 10},
        {1005, "주하와 물실", "물리학의 신 오주하는 3학년 공강실에서 롤을 하느라 진자 운동의 주기를 구하는 물실 수업에 불참하고 말았다. CS를 먹느라 바쁜 주하는 2 * pi * sqrt(100L/9.81)이라는 간단한 공식도 적용할 틈이 없다. 주하를 위해 데이터를 조작해보자!", "첫 줄에 실의 길이 L이 주어진다. (단위: cm, 1 <= L <= 100)", "첫 줄에 진자 운동의 주기를 출력한다. 계산한 값을 소수점 첫째 자리까지 반올림한다.", 1, "수학", 10},
        {1006, "주영이와 화실", "주영이는 화실 F를 면하기 위해 총 8개의 보고서의 점수의 합을 40 이상으로 맞춰야 한다. 주영이는 보고서를 제출하지 않으면 0점, 클로드 단일 프롬포트 버전을 제출하면 4점, 선배님의 보고서를 적절히 선형 결합하면 7점, 브롤스타즈를 포기하고 8시간동안 열심히 보고서를 작성하면 9점을 얻는다. 주영이가 지금까지 n번의 실험을 했을 때, 주영이가 화실 F를 피할 수 있는 경우의 수를 계산해주자!", "첫 줄에 이미 제출한 보고서 수 n이 주어진다. (1 <= n <= 7) 두 번째 줄에 주영이가 지금까지 받은 n개의 보고서의 점수들 a_1, a_2 ... a_n 이 주어진다. (a_i는 0, 4, 7, 9 중 하나)", "첫 줄에 주영이가 총점 40 이상을 만들 수 있는 이후 보고서들의 점수 경우의 수를 출력한다. 주영이는 각 보고서에서 0점, 4점, 7점, 9점 중 하나만 받는다고 가정한다.", 2, "브루트 포스, 다이나믹 프로그래밍, 재귀함수", 10},
        {1007, "리듬게임 고수 김도현", "도현이는 총 k의 체력으로 n개의 리듬게임 채보를 플레이하려고 한다. 각 채보를 플레이 할 때마다 그에 해당되는 체력이 소모되기 때문에 도현이는 모든 채보를 할 수는 없다. 그러나, 도현이가 좋아하는 각성제 \"몬스터 딸기맛\"을 먹는다면 최대 a개의 채보에 대해 \"각성 상태\"가 되어, 그 채보를 플레이할 때 소모되는 정신력이 절반이 된다. 도현이가 플레이할 수 있는 리듬게임 채보의 최대 개수를 구해보자!", "첫 줄에 도현이의 체력 k, 도현이가 플레이하고 싶은 채보의 수 n, \"각성 상태\"가 유지되는 최대 채보의 수 a가 주어진다. (1 <= n, a <= 10000, 1 <= k <= 100000) 두 번째 줄에 도현이가 각 채보를 플레이할 때마다 소모되는 체력 x_1, x_2 ... x_n 이 주어진다. (1 <= x_i <= 10000)", "도현이가 플레이할 수 있는 리듬게임 채보의 최대 개수를 출력한다.", 3, "그리디 알고리즘", 10},
        {1008, "재원이와 이진수", "재원이는 고프 프로젝트를 하다가 C언어가 너무 재미없다고 느낀 나머지 0, 1로 이루어진 이진수로만 코드를 작성하기 시작했다. 그러나 재원이의 컴퓨터에는 1을 입력할 때마다 이전에 있던 모든 0이 1로 바뀌고, 1이 0으로 바뀌는 심각한 오류가 있다! 예를 들어 컴퓨터에 \"1010\"이 써 있다면, 0을 입력하면 \"10100\"이 되지만 1을 입력하면 \"01011\"이 된다. 재원이가 컴퓨터에 입력하고 싶은 이진수가 주어졌을 때, 재원이가 어떤 순서로 컴퓨터에 0과 1을 입력해야 하는지를 구해보자.", "첫 줄에 재원이가 입력하고 싶은 이진 문자열 S가 띄어쓰기 없이 주어진다. (S의 길이 <= 10000)", "첫 줄에 재원이가 실제로 컴퓨터에 S를 띄우기 위해 어떤 순서로 0과 1을 입력해야 하는지를 띄어쓰기 없이 출력한다.", 3, "문자열, 애드 혹, 스택", 10},
        {1009, "서언이와 정수론 과제", "서언이는 정수론 과제의 마지막 한 문제를 풀지 못하고 있다. 해당 문제는 \"소수로만 이루어진 길이가 n인 등차수열\"을 구하는 문제이다. 여기서의 소수는 prime number를 뜻한다. 서언이를 도와 문제를 대신 해결해주자!", "첫 줄에 등차수열의 길이 n이 주어진다. (n <= 100)", "소수로만 이루어진 길이가 n인 등차수열을 출력한다. 만약 가능한 경우가 여러 가지일 경우 수열 원소의 합이 가장 작은 경우를 출력한다.", 2, "수학, 애드 혹", 10},
        {1010, "축구의 신 송현지선생님", "축구의 신 송현지 선생님께서 경기 도중 n명의 일렬로 되어 있는 수비수를 마주하셨다. 송현지 선생님께서는 수비수 한 명을 제치는 \"숏 드리블\", 수비수 두 명을 제치는 \"롱 드리블\"이라는 두 가지의 고급 기술을 사용하실 수 있다. 송현지 선생님께서 정확히 n명의 수비수를 제치는 방법의 수를 출력해보자! (참고로 수비수가 한 명 남았을 때는 \"롱 드리블\"을 사용할 수 없다.)", "첫 줄에 송현지 선생님께서 제쳐야 하는 수비수의 수 n이 주어진다. (1 <= n <= 25)", "첫 줄에 송현지 선생님께서 정확히 n명의 수비수를 제치는 방법의 수를 출력한다.", 2, "다이나믹 프로그래밍", 10},
        {1011, "은성이와 소수", "은성이가 가장 좋아하는 알고리즘은 밀러-라빈 소수 판별법이다. 이를 통해 큰 수 n이 주어졌을 때 이 수가 소수인지 판별해보자.", "첫 줄에 자연수 n이 주어진다. (n <= 10^20)", "첫 줄에 n이 소수이면 \"n is prime number\", 소수가 아니면 \"n is not prime number\"를 출력한다.", 4, "수학", 10},
        {1012, "LIS", "어떤 수열의 부분 증가 수열(LIS)은, 어떤 수열의 부분 수열 중 원소가 오름차순으로 배치되어 있는 수열을 뜻한다. 예를 들어 [1, 9, 4, 8, 7, 6]에서 [1, 4, 8]은 부분 증가 수열이나 [1, 4, 9]나 [4, 8, 7]은 부분 증가 수열이 아니다. 주어진 수열의 최장 길이의 부분 증가 수열, 즉 LIS의 길이를 구해보자.", "첫 줄에 자연수 n이 주어진다. (1 <= n <= 500000) 둘째 줄에 길이가 n인 수열이 띄어쓰기를 사이에 두고 주어진다. 이때 각 수의 절댓값은 10^6 이하이다.", "첫 줄에 주어진 수열의 최장 길이 부분 증가 수열의 길이를 출력한다.", 4, "다이나믹 프로그래밍, 이분 탐색", 10},
    };

    int changed = 0;
    int count = (int)(sizeof(defaults) / sizeof(defaults[0]));
    for (int i = 0; i < count; i++) {
        if (find_problem_by_id(defaults[i].id) != NULL) continue;
        if (ensure_problem_capacity(g_problem_count + 1) != ERR_NONE) return ERR_MEMORY;
        Problem p;
        memset(&p, 0, sizeof(p));
        p.problem_id = defaults[i].id;
        strncpy(p.title, defaults[i].title, MAX_TITLE_LEN - 1);
        strncpy(p.desc, defaults[i].desc, MAX_DESC_LEN - 1);
        strncpy(p.input_desc, defaults[i].input_desc, MAX_DESC_LEN - 1);
        strncpy(p.output_desc, defaults[i].output_desc, MAX_DESC_LEN - 1);
        p.difficulty = defaults[i].difficulty;
        strncpy(p.category, defaults[i].category, MAX_CATEGORY_LEN - 1);
        p.time_limit = DEFAULT_TIME_LIMIT;
        p.testcase_count = defaults[i].testcase_count;
        g_problems[g_problem_count++] = p;
        make_testcase_dir(p.problem_id);
        changed = 1;
    }
    return changed ? save_problems() : ERR_NONE;
}
