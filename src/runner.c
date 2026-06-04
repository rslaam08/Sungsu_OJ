#include "oaj.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#endif

static int system_exit_code(int ret) {
#ifdef _WIN32
    return ret;
#else
    if (ret == -1) return -1;
    if (WIFEXITED(ret)) return WEXITSTATUS(ret);
    return ret;
#endif
}

JudgeResult compile_source(const char* source_file,
                           const char* exe_file,
                           const char* error_file) {
    if (source_file == NULL || exe_file == NULL || error_file == NULL) return JUDGE_CE;

    char cmd[1200];
    snprintf(cmd, sizeof(cmd), "%s \"%s\" -O2 -std=%s -Wall -Wextra -o \"%s\" 2> \"%s\"",
             DEFAULT_COMPILE_CMD, source_file, DEFAULT_C_STANDARD, exe_file, error_file);

    int ret = system(cmd);
    int code = system_exit_code(ret);
    return (code == 0) ? JUDGE_AC : JUDGE_CE;
}

#ifdef _WIN32
static JudgeResult run_testcase_windows(const char* exe_file,
                                        const char* input_file,
                                        const char* user_output_file,
                                        const char* run_error_file,
                                        int time_limit) {
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hInput = CreateFileA(input_file,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                &sa,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if (hInput == INVALID_HANDLE_VALUE) return JUDGE_RE;

    HANDLE hOutput = CreateFileA(user_output_file,
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 &sa,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
    if (hOutput == INVALID_HANDLE_VALUE) {
        CloseHandle(hInput);
        return JUDGE_RE;
    }

    HANDLE hError = CreateFileA(run_error_file,
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                &sa,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
    if (hError == INVALID_HANDLE_VALUE) {
        CloseHandle(hInput);
        CloseHandle(hOutput);
        return JUDGE_RE;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hInput;
    si.hStdOutput = hOutput;
    si.hStdError = hError;

    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "\"%s\"", exe_file);

    BOOL ok = CreateProcessA(NULL,
                             cmdline,
                             NULL,
                             NULL,
                             TRUE,
                             CREATE_NO_WINDOW,
                             NULL,
                             NULL,
                             &si,
                             &pi);

    CloseHandle(hInput);
    CloseHandle(hOutput);
    CloseHandle(hError);

    if (!ok) return JUDGE_RE;

    DWORD wait_ms = (time_limit <= 0) ? 1000U : (DWORD)time_limit * 1000U;
    DWORD wait_result = WaitForSingleObject(pi.hProcess, wait_ms);

    JudgeResult result = JUDGE_AC;
    if (wait_result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 1000);
        result = JUDGE_TLE;
    } else if (wait_result != WAIT_OBJECT_0) {
        result = JUDGE_RE;
    } else {
        DWORD exit_code = 0;
        if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
            result = JUDGE_RE;
        } else if (exit_code != 0) {
            result = JUDGE_RE;
        }
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return result;
}
#endif

JudgeResult run_testcase(const char* exe_file,
                         const char* input_file,
                         const char* user_output_file,
                         const char* run_error_file,
                         int time_limit) {
    if (exe_file == NULL || input_file == NULL || user_output_file == NULL || run_error_file == NULL)
        return JUDGE_RE;

#ifdef _WIN32
    return run_testcase_windows(exe_file, input_file, user_output_file, run_error_file, time_limit);
#else
    char cmd[1200];
    snprintf(cmd, sizeof(cmd), "timeout %ds \"%s\" < \"%s\" > \"%s\" 2> \"%s\"",
             time_limit, exe_file, input_file, user_output_file, run_error_file);

    int ret = system(cmd);
    int code = system_exit_code(ret);

    if (code == 124) return JUDGE_TLE;
    if (code != 0) return JUDGE_RE;
    return JUDGE_AC;
#endif
}
