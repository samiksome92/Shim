/**
 * A program which accepts two specific definitions EXE and WD and creates a shim executable which executes EXE with WD
 * as working directory and passes all arguments to EXE as is. Primary use is creating shims for programs so that the
 * shims can be kept in a central directory added to %PATH% thus avoiding cluttering %PATH% with multiple entries.
 */

#include <stdio.h>
#include <wchar.h>
#include <windows.h>

// If EXE is not provided externally use `cmd`.
#ifndef EXE
#define EXE L"cmd"
#endif

// If WD is not provided externally use `NULL`.
#ifndef WD
#define WD NULL
#endif

int main(int, char *argv[])
{
    /**
     * The main function.
     * 
     * @param int Unused.
     * @param char* Command line arguments.
     *
     * @return Exit status.
     */
    LPWSTR cl = GetCommandLineW();

    // Get the name/path of the executable and convert it to wchar_t.
    size_t len = strlen(argv[0]) + 1; // Pad by one to include null char.
    LPWSTR argv0 = new WCHAR[len];
    size_t cc = 0;
    mbstowcs_s(&cc, argv0, len, argv[0], _TRUNCATE);

    // Skip the name/path of executable in the command line.
    cl = wcsstr(cl, argv0);
    if (cl == nullptr)
    {
        fprintf(stderr, "argv0 not found as substring in cl.\n");
        return EXIT_FAILURE;
    }
    cl += wcslen(argv0);
    cl = wcschr(cl, L' ');

    // Handle extra arguments.
    LPWSTR ncl;
    if (cl == nullptr)
    {
        size_t ncl_len = wcslen(EXE) + 1;
        ncl = new WCHAR[ncl_len];
        wcscpy_s(ncl, ncl_len, EXE);
    }
    else
    {
        size_t ncl_len = wcslen(EXE) + wcslen(cl) + 1;
        ncl = new WCHAR[ncl_len];
        wcscpy_s(ncl, ncl_len, EXE);
        wcscat_s(ncl, ncl_len, cl);
    }

    // Initialize handlers.
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD exit_code;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create process.
    if (!CreateProcessW(
            NULL,
            ncl,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            WD,
            &si,
            &pi))
    {
        fprintf(stderr, "CreateProcessW failed (%ld).\n", GetLastError());
        return EXIT_FAILURE;
    }

    // Wait for process to end.
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &exit_code);

    // Clean up.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    delete[] ncl;
    delete[] argv0;

    return exit_code;
}