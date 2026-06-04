#include "oaj.h"

int compare_by_score(const User* a, const User* b) {
    if (a == NULL || b == NULL) return 0;
    if (a->score != b->score) return b->score - a->score;
    return b->solved_count - a->solved_count;
}

int compare_by_solved(const User* a, const User* b) {
    if (a == NULL || b == NULL) return 0;
    if (a->solved_count != b->solved_count) return b->solved_count - a->solved_count;
    return b->score - a->score;
}

int compare_by_tier(const User* a, const User* b) {
    if (a == NULL || b == NULL) return 0;
    if (a->tier != b->tier) return b->tier - a->tier;
    return b->score - a->score;
}

void sort_users(User* arr, int n, CompareFunc cmp_fn) {
    if (arr == NULL || cmp_fn == NULL) return;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (cmp_fn(&arr[j], &arr[j + 1]) > 0) {
                User tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

void show_ranking_all(CompareFunc cmp_fn) {
    if (g_user_count == 0) {
        printf("등록된 유저가 없습니다.\n");
        return;
    }
    if (cmp_fn == NULL) cmp_fn = compare_by_score;

    User* tmp = (User*)malloc(sizeof(User) * (size_t)g_user_count);
    if (tmp == NULL) {
        printf("[오류] 메모리 할당 실패.\n");
        return;
    }

    memcpy(tmp, g_users, sizeof(User) * (size_t)g_user_count);
    sort_users(tmp, g_user_count, cmp_fn);

    print_separator();
    printf("%-4s %-15s %-14s %-8s %-8s %-6s\n", "순위", "아이디", "티어", "점수", "푼문제", "관리자");
    print_separator();
    for (int i = 0; i < g_user_count; i++) {
        printf("%-4d %-15s %-14s %-8d %-8d %-6s\n",
               i + 1, tmp[i].username, tier_to_string((Tier)tmp[i].tier),
               tmp[i].score, tmp[i].solved_count, tmp[i].is_admin ? "O" : "-");
    }
    print_separator();
    free(tmp);
}

void show_ranking_by_tier(Tier tier) {
    int count = 0;
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].tier == (int)tier) count++;
    }

    if (count == 0) {
        printf("[%s] 티어 유저가 없습니다.\n", tier_to_string(tier));
        return;
    }

    User* tmp = (User*)malloc(sizeof(User) * (size_t)count);
    if (tmp == NULL) {
        printf("[오류] 메모리 할당 실패.\n");
        return;
    }

    int idx = 0;
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].tier == (int)tier) tmp[idx++] = g_users[i];
    }

    sort_users(tmp, count, compare_by_score);

    print_separator();
    printf("[%s] 티어 랭킹\n", tier_to_string(tier));
    print_separator();
    printf("%-4s %-15s %-8s %-8s\n", "순위", "아이디", "점수", "푼문제");
    for (int i = 0; i < count; i++) {
        printf("%-4d %-15s %-8d %-8d\n", i + 1, tmp[i].username, tmp[i].score, tmp[i].solved_count);
    }
    print_separator();
    free(tmp);
}
