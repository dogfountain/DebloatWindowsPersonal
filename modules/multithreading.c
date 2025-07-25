/*
#include <windows.h>
#include <stdio.h>

#include "tweaks.h"
#include "setManual.h"
#include "disableFeatures.h"

DWORD WINAPI ThreadFuncTelemetry(LPVOID param) {
    (void)param;
    printf("[Thread] Running Telemetry Tweaks...\n");
    run_telemetry_tweaks();
    return 0;
}

DWORD WINAPI ThreadFuncManual(LPVOID param) {
    (void)param;
    printf("[Thread] Running Services Manual...\n");
    set_services_manual();
    return 0;
}

DWORD WINAPI ThreadFuncDisable(LPVOID param) {
    (void)param;
    printf("[Thread] Running Disable Features...\n");
    disable_features();
    return 0;
}

void launch_all_tweaks_parallel(void) {
    HANDLE threads[3];

    threads[0] = CreateThread(NULL, 0, ThreadFuncTelemetry, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, ThreadFuncManual, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, ThreadFuncDisable, NULL, 0, NULL);

    if (threads[0] == NULL || threads[1] == NULL || threads[2] == NULL) {
        printf("ERROR: Failed to create one or more threads.\n");
        // Close any created threads before return
        for (int i = 0; i < 3; ++i) {
            if (threads[i] != NULL) CloseHandle(threads[i]);
        }
        return;
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);

    // Close thread handles
    for (int i = 0; i < 3; ++i) {
        CloseHandle(threads[i]);
    }

    printf("All tweaks completed.\n");
}

*/