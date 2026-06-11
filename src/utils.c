#include "oaj.h"

void get_current_timestamp(char* buf, int buf_size) {
    if (buf == NULL || buf_size <= 0) return;
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (t == NULL) {
        snprintf(buf, buf_size, "unknown");
        return;
    }
    strftime(buf, (size_t)buf_size, "%Y-%m-%d %H:%M:%S", t);
}

Tier score_to_tier(int score) {
    Tier tier = TIER_IRON;
    for (int i = 0; i < TIER_COUNT - 1; i++) {
        if (score >= get_tier_threshold((Tier)i)) tier = (Tier)(i + 1);
    }
    return tier;
}

const char* tier_to_string(Tier tier) {
    static const char* names[TIER_COUNT] = {
        "IRON", "BRONZE", "SILVER", "GOLD", "PLATINUM",
        "EMERALD", "DIAMOND", "MASTER", "GRANDMASTER", "CHALLENGER"
    };
    if (tier < 0 || tier >= TIER_COUNT) return "UNKNOWN";
    return names[tier];
}

const char* judge_result_to_string(JudgeResult result) {
    switch (result) {
        case JUDGE_AC:  return "AC";
        case JUDGE_WA:  return "WA";
        case JUDGE_TLE: return "TLE";
        case JUDGE_RE:  return "RE";
        case JUDGE_CE:  return "CE";
        default:        return "UNKNOWN";
    }
}

const char* judge_result_to_display(JudgeResult result) {
    switch (result) {
        case JUDGE_AC:  return "AC(ACCEPTED)";
        case JUDGE_WA:  return "WA(WRONG ANSWER)";
        case JUDGE_TLE: return "TLE(TIME LIMIT EXCEEDED)";
        case JUDGE_RE:  return "RE(RUNTIME ERROR)";
        case JUDGE_CE:  return "CE(COMPILE ERROR)";
        default:        return "UNKNOWN";
    }
}

const char* judge_result_code_to_display(const char* result_code) {
    if (result_code == NULL) return "UNKNOWN";
    if (strcmp(result_code, "AC") == 0)  return "AC(ACCEPTED)";
    if (strcmp(result_code, "WA") == 0)  return "WA(WRONG ANSWER)";
    if (strcmp(result_code, "TLE") == 0) return "TLE(TIME LIMIT EXCEEDED)";
    if (strcmp(result_code, "RE") == 0)  return "RE(RUNTIME ERROR)";
    if (strcmp(result_code, "CE") == 0)  return "CE(COMPILE ERROR)";
    return result_code;
}

const char* promo_status_to_string(PromoStatus status) {
    switch (status) {
        case PROMO_IDLE:    return "IDLE";
        case PROMO_ONGOING: return "ONGOING";
        case PROMO_SUCCESS: return "SUCCESS";
        case PROMO_FAIL:    return "FAIL";
        default:            return "UNKNOWN";
    }
}

int get_tier_threshold(Tier tier) {
    static const int threshold_for_next[TIER_COUNT] = {
        200,   /* IRON -> BRONZE */
        500,   /* BRONZE -> SILVER */
        900,   /* SILVER -> GOLD */
        1400,  /* GOLD -> PLATINUM */
        2000,  /* PLATINUM -> EMERALD */
        2700,  /* EMERALD -> DIAMOND */
        3500,  /* DIAMOND -> MASTER */
        4400,  /* MASTER -> GRANDMASTER */
        5400,  /* GRANDMASTER -> CHALLENGER */
        2147483647
    };
    if (tier < 0 || tier >= TIER_COUNT) return 2147483647;
    return threshold_for_next[tier];
}

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_separator(void) {
    printf("------------------------------------------------------------\n");
}

void press_enter_to_continue(void) {
    printf("\nEnter 키를 누르면 계속합니다...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int file_exists(const char* path) {
    if (path == NULL) return 0;
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) return 0;
    fclose(fp);
    return 1;
}

void trim_newline(char* s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}

int copy_file(const char* src, const char* dst) {
    if (src == NULL || dst == NULL) return ERR_NULL_PTR;

    FILE* in = fopen(src, "rb");
    if (in == NULL) return ERR_FILE_OPEN;

    FILE* out = fopen(dst, "wb");
    if (out == NULL) {
        fclose(in);
        return ERR_FILE_OPEN;
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return ERR_FILE_OPEN;
        }
    }

    fclose(in);
    fclose(out);
    return ERR_NONE;
}
