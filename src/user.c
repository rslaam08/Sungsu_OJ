#include "oaj.h"

static int next_user_id(void) {
    int max_id = 0;
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].user_id > max_id) max_id = g_users[i].user_id;
    }
    return max_id + 1;
}

User* find_user_by_id(int user_id) {
    for (int i = 0; i < g_user_count; i++) {
        if (g_users[i].user_id == user_id) return &g_users[i];
    }
    return NULL;
}

User* find_user_by_username(const char* username) {
    if (username == NULL) return NULL;
    for (int i = 0; i < g_user_count; i++) {
        if (strcmp(g_users[i].username, username) == 0) return &g_users[i];
    }
    return NULL;
}

int register_user(const char* username, const char* password) {
    if (username == NULL || password == NULL) return ERR_NULL_PTR;
    if (strlen(username) == 0 || strlen(password) == 0) return ERR_INVALID_INPUT;
    if (find_user_by_username(username) != NULL) return ERR_DUPLICATE_ID;
    if (g_user_count >= MAX_USERS) return ERR_INVALID_INPUT;
    if (ensure_user_capacity(g_user_count + 1) != ERR_NONE) return ERR_MEMORY;

    User user;
    memset(&user, 0, sizeof(user));
    user.user_id = next_user_id();
    strncpy(user.username, username, MAX_USERNAME_LEN - 1);
    strncpy(user.password, password, MAX_PASSWORD_LEN - 1);
    user.score = 0;
    user.tier = TIER_IRON;
    user.is_admin = 0;

    g_users[g_user_count++] = user;
    return save_users();
}

User* login_user(const char* username, const char* password) {
    User* user = find_user_by_username(username);
    if (user == NULL) return NULL;
    if (strcmp(user->password, password) != 0) return NULL;
    g_current_user = user;
    return user;
}

void logout_user(void) {
    g_current_user = NULL;
}

void print_user_profile(const User* user) {
    if (user == NULL) {
        printf("[오류] 로그인 정보가 없습니다.\n");
        return;
    }

    print_separator();
    printf("아이디       : %s\n", user->username);
    printf("유저 ID      : %d\n", user->user_id);
    printf("티어         : %s\n", tier_to_string((Tier)user->tier));
    printf("점수         : %d\n", user->score);
    printf("푼 문제 수   : %d\n", user->solved_count);
    printf("제출 횟수    : %d\n", user->submit_count);
    printf("관리자 여부  : %s\n", user->is_admin ? "예" : "아니오");
    printf("승급전 상태  : %s\n", user->is_in_promotion ? "진행 중" : "없음");
    if (strlen(user->last_promo) > 0) printf("마지막 승급전: %s\n", user->last_promo);
    print_separator();
}

int update_user_score(User* user, int delta_score) {
    if (user == NULL) return ERR_NULL_PTR;
    user->score += delta_score;
    if (user->score < 0) user->score = 0;
    save_users();
    return ERR_NONE;
}

int update_tier(User* user) {
    if (user == NULL) return ERR_NULL_PTR;
    if (user->tier < TIER_CHALLENGER && user->score < get_tier_threshold((Tier)user->tier)) {
        /* 점수가 낮아져도 현재 구현에서는 강등하지 않는다. */
        return ERR_NONE;
    }
    return ERR_NONE;
}
