/**
 * A program to create a shim to an executable. Optionally also accepts arguments for setting the working directory and
 * output file name. Utilises NTFS alternate streams to store data regarding the shim. Primary use is creating shims for
 * programs so that the shims can be kept in a central directory added to %PATH% thus avoiding cluttering %PATH% with
 * multiple entries.
 */

#include <Windows.h>
#include <stdio.h>
#include <wchar.h>

#define PATH_LENGTH 65535

void PrintHelp() {
    /**
     * Print help and exit.
     */
    printf("Creates a shim to an executable.\n\n");
    printf("Shim /E<path> [/D<dir>] [/O<out>]\n\n");
    printf("  /E<path>    Path to executable\n");
    printf("  /D<dir>     Working directory\n");
    printf("  /O<out>     Output path\n\n");

    exit(EXIT_SUCCESS);
}

int Create(int argc, char *argv[], LPWSTR self) {
    /**
     * Create a shim to an executable.
     *
     * @param int Argument count.
     * @param char*[] Command line arguments.
     * @param LPWSTR Path to self.
     *
     * @return Exit status.
     */
    // Parse arguments.
    char *exe = nullptr;
    char *wd = nullptr;
    char *out = nullptr;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "/?", 2) == 0) {
            PrintHelp();
        } else if (strncmp(argv[i], "/E", 2) == 0 || strncmp(argv[i], "/e", 2) == 0) {
            exe = argv[i] + 2;
        } else if (strncmp(argv[i], "/D", 2) == 0 || strncmp(argv[i], "/d", 2) == 0) {
            wd = argv[i] + 2;
        } else if (strncmp(argv[i], "/O", 2) == 0 || strncmp(argv[i], "/o", 2) == 0) {
            out = argv[i] + 2;
        }
    }

    // If exe is not provided show help.
    if (exe == nullptr) PrintHelp();

    // If output name is not provided, try to generate one.
    if (out == nullptr) {
        char *exeB = strrchr(exe, '\\');
        char *exeF = strrchr(exe, '/');

        if (exeB == nullptr && exeF == nullptr)
            out = "shim.out";
        else if (exeB == nullptr)
            out = exeF + 1;
        else if (exeF == nullptr)
            out = exeB + 1;
        else {
            out = strlen(exeB) < strlen(exeF) ? exeB + 1 : exeF + 1;
        }
    }

    // Print info.
    if (wd == nullptr)
        printf("Creating shim: %s -> %s\n", out, exe);
    else
        printf("Creating shim: %s -> %s with working directory %s\n", out, exe, wd);

    // Convert output name to wchar_t.
    int lenWout = strlen(out) + 1;
    LPWSTR wout = new WCHAR[lenWout];
    size_t cc = 0;
    mbstowcs_s(&cc, wout, lenWout, out, _TRUNCATE);

    // Copy self to output.
    if (!CopyFileW(self, wout, FALSE)) {
        fprintf(stderr, "Failed to copy self.");
        return EXIT_FAILURE;
    }

    // Write exe and wd to out:Shim.
    LPWSTR sout = new WCHAR[lenWout + 5];  // +5 for :Shim.
    wcscpy_s(sout, lenWout, wout);
    wcscpy_s(sout + lenWout - 1, 6, L":Shim");

    HANDLE hFile = CreateFileW(sout, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open output file.");
        return EXIT_FAILURE;
    }

    size_t lenExe = strlen(exe);
    size_t lenWd = wd == nullptr ? 0 : strlen(wd);
    char *buffer = new char[lenExe + lenWd + 1 + 1];  // +1 for '|', +1 for '\0'.
    strcpy_s(buffer, lenExe + 1, exe);
    if (wd != nullptr) {
        buffer[lenExe] = '|';
        strcpy_s(buffer + lenExe + 1, lenWd + 1, wd);
    }
    DWORD bytes = 0;
    if (!WriteFile(hFile, buffer, lenExe + lenWd + 1, &bytes, NULL)) {
        fprintf(stderr, "Failed to write to output file.");
        return EXIT_FAILURE;
    }

    // Clean up.
    CloseHandle(hFile);
    delete[] wout;
    delete[] sout;
    delete[] buffer;

    return EXIT_SUCCESS;
}

int Shim(HANDLE hFile, char *argv[]) {
    /**
     * Run as a shim. Read from :Shim stream and launch shimmed application.
     *
     * @param HANDLE Open handle to self.
     * @param char*[] Command line arguments.
     *
     * @return Exit status.
     */
    // Read data from :Shim stream.
    char *buffer = new char[PATH_LENGTH * 2 + 1 + 1];  // PATH_LENGTH * 2 for exe and wd, 1 for '|', 1 for '\0'.
    DWORD bytes = 0;
    if (!ReadFile(hFile, buffer, PATH_LENGTH * 2 + 1, &bytes, NULL)) {
        fprintf(stderr, "Failed to read self:Shim.\n");
        return EXIT_FAILURE;
    }
    buffer[bytes] = '\0';

    // Get exe and wd.
    char *exe = buffer;
    char *wd = strchr(buffer, '|');
    if (wd != nullptr) {
        wd[0] = '\0';
        wd = wd + 1;
    }

    // Get commandline.
    LPWSTR cl = GetCommandLineW();

    // Get the name/path of the executable and convert it to wchar_t.
    size_t len = strlen(argv[0]) + 1;  // +1 for '\0'.
    LPWSTR argv0 = new WCHAR[len];
    size_t cc = 0;
    mbstowcs_s(&cc, argv0, len, argv[0], _TRUNCATE);

    // Skip the name/path of executable in the command line.
    cl = wcsstr(cl, argv0);
    if (cl == nullptr) {
        fprintf(stderr, "argv0 not found as substring in cl.\n");
        return EXIT_FAILURE;
    }
    cl += wcslen(argv0);
    cl = wcschr(cl, L' ');

    // Handle extra arguments.
    LPWSTR ncl;
    if (cl == nullptr) {
        size_t lenNcl = strlen(exe) + 1;
        ncl = new WCHAR[lenNcl];
        mbstowcs_s(&cc, ncl, lenNcl, exe, _TRUNCATE);
    } else {
        size_t lenNcl = strlen(exe) + wcslen(cl) + 1;
        ncl = new WCHAR[lenNcl];
        mbstowcs_s(&cc, ncl, lenNcl, exe, _TRUNCATE);
        wcscat_s(ncl, lenNcl, cl);
    }

    // Convert working directory to wchar_t.
    LPWSTR wwd = NULL;
    if (wd != nullptr) {
        size_t lenWwd = strlen(wd) + 1;
        wwd = new WCHAR[lenWwd];
        mbstowcs_s(&cc, wwd, lenWwd, wd, _TRUNCATE);
    }

    // Initialize handlers.
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD exit_code;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create process.
    if (!CreateProcessW(NULL, ncl, NULL, NULL, FALSE, 0, NULL, wwd, &si, &pi)) {
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
    delete[] wwd;
    delete[] argv0;
    delete[] buffer;

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    /**
     * The main function.
     *
     * @param int Count of arguments.
     * @param char*[] Command line arguments.
     *
     * @return Exit status.
     */
    // Get path to self.
    LPWSTR file = new WCHAR[PATH_LENGTH];
    DWORD length = GetModuleFileNameW(NULL, file, PATH_LENGTH - 5);  // -5 to account for ":Shim"
    if (length == 0 || length == PATH_LENGTH - 5) {
        fprintf(stderr, "Failed to get path to self.\n");
        return EXIT_FAILURE;
    }
    // Add :Shim to file path.
    wcscpy(file + length, L":Shim");
    file[length + 5] = '\0';

    // Check if :Shim stream is present and switch behaviour accordingly.
    HANDLE hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    int exit_code;
    if (hFile == INVALID_HANDLE_VALUE) {
        file[length] = '\0';
        exit_code = Create(argc, argv, file);
    } else
        exit_code = Shim(hFile, argv);

    // Clean up.
    CloseHandle(hFile);
    delete[] file;

    return exit_code;
}