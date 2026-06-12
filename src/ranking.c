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

static void print_rank_header(int include_tier, int include_admin) {
    print_padded_utf8("순위", 6);
    print_padded_utf8("아이디", 18);
    if (include_tier) print_padded_utf8("티어", 15);
    print_padded_utf8("점수", 10);
    print_padded_utf8("푼문제", 10);
    if (include_admin) print_padded_utf8("관리자", 8);
    printf("\n");
}

static void print_rank_row(int rank, const User* user, int include_tier, int include_admin) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", rank);
    print_padded_utf8(buf, 6);
    print_padded_utf8(user->username, 18);
    if (include_tier) print_padded_utf8(tier_to_string((Tier)user->tier), 15);
    snprintf(buf, sizeof(buf), "%d", user->score);
    print_padded_utf8(buf, 10);
    snprintf(buf, sizeof(buf), "%d", user->solved_count);
    print_padded_utf8(buf, 10);
    if (include_admin) print_padded_utf8(user->is_admin ? "O" : "-", 8);
    printf("\n");
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
    print_rank_header(1, 1);
    print_separator();
    for (int i = 0; i < g_user_count; i++) {
        print_rank_row(i + 1, &tmp[i], 1, 1);
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
    print_rank_header(0, 0);
    print_separator();
    for (int i = 0; i < count; i++) {
        print_rank_row(i + 1, &tmp[i], 0, 0);
    }
    print_separator();
    free(tmp);
}
