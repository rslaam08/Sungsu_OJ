#include "oaj.h"

#ifdef _WIN32
#include <windows.h>

static void set_working_directory_to_project_root(void) {
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) return;

    /* exe_path: C:\\...\\SOJ_project\\build\\soj.exe */
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash == NULL) return;
    *last_slash = '\0';  /* C:\\...\\SOJ_project\\build */

    char* folder_name = strrchr(exe_path, '\\');
    if (folder_name != NULL && strcmp(folder_name + 1, "build") == 0) {
        *folder_name = '\0';  /* C:\\...\\SOJ_project */
    }

    SetCurrentDirectoryA(exe_path);
}
#endif

int main(void) {
#ifdef _WIN32
    system("chcp 65001 > nul");
    set_working_directory_to_project_root();
#endif
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
