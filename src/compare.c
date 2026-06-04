#include "oaj.h"

static char* read_all_text(const char* path, long* out_len) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);

    char* buf = (char*)malloc((size_t)len + 1);
    if (buf == NULL) {
        fclose(fp);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)len, fp);
    fclose(fp);
    buf[n] = '\0';
    if (out_len != NULL) *out_len = (long)n;
    return buf;
}

static void rstrip(char* s) {
    if (s == NULL) return;
    long len = (long)strlen(s);
    while (len > 0) {
        char c = s[len - 1];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            s[len - 1] = '\0';
            len--;
        } else {
            break;
        }
    }
}

int compare_output_files(const char* user_output_file, const char* expected_output_file) {
    if (user_output_file == NULL || expected_output_file == NULL) return 0;

    char* user = read_all_text(user_output_file, NULL);
    char* expected = read_all_text(expected_output_file, NULL);
    if (user == NULL || expected == NULL) {
        free(user);
        free(expected);
        return 0;
    }

    rstrip(user);
    rstrip(expected);
    int same = (strcmp(user, expected) == 0);

    free(user);
    free(expected);
    return same;
}
