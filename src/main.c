#include "oaj.h"

#ifdef _WIN32
#include <windows.h>

static int directory_exists_win(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static void set_working_directory_to_project_root(void) {
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) return;

    /* exe_path: C:\\...\\SOJ_project\\build\\soj.exe 또는 C:\\...\\SOJ_project\\soj.exe */
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash == NULL) return;
    *last_slash = '\0';

    char* folder_name = strrchr(exe_path, '\\');
    if (folder_name != NULL && strcmp(folder_name + 1, "build") == 0) {
        *folder_name = '\0';
    }

    SetCurrentDirectoryA(exe_path);
}

static int validate_project_root(void) {
    if (!directory_exists_win("src") ||
        !directory_exists_win("include") ||
        !directory_exists_win("data") ||
        !directory_exists_win("data\\testcases") ||
        !directory_exists_win("workspace")) {
        printf("[ERROR] SOJ_project 루트 폴더에서 실행되지 않았습니다.\n");
        printf("실행 파일은 반드시 SOJ_project\\build\\soj.exe 또는 SOJ_project\\soj.exe 위치에 있어야 합니다.\n");
        printf("현재 실행 위치에 src, include, data, workspace 폴더가 모두 있어야 합니다.\n");
        return 0;
    }
    return 1;
}
#else
#include <sys/stat.h>
static int directory_exists_posix(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
static int validate_project_root(void) {
    return directory_exists_posix("src") && directory_exists_posix("include") &&
           directory_exists_posix("data") && directory_exists_posix("data/testcases") &&
           directory_exists_posix("workspace");
}
#endif

int main(void) {
#ifdef _WIN32
    system("chcp 65001 > nul");
    set_working_directory_to_project_root();
#endif

    if (!validate_project_root()) {
        printf("프로젝트 루트 검증에 실패했습니다. 프로그램을 종료합니다.\n");
        press_enter_to_continue();
        return 1;
    }

    int ret = init_all();
    if (ret != ERR_NONE) {
        printf("초기화 실패. 코드: %d\n", ret);
        return 1;
    }

    printf("SOJ 초기화 완료.\n");
    printf("기본 관리자 계정: admin / admin123\n");
    press_enter_to_continue();

    menu_main();
    free_all();

    printf("프로그램을 종료합니다.\n");
    return 0;
}
