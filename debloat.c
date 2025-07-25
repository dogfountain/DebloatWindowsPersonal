#include <windows.h>
#include <stdio.h>
#include "modules/tweaks.h"
#include "modules/setManual.h"
#include "modules/disableFeatures.h"
#include "modules/uninstallApps.h"

#pragma comment(lib, "Advapi32.lib")

int is_admin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

int main(void) {
    if (!is_admin()) {
        printf("This program must be run as administrator!\n");
        getchar();
        return 1;
    }
    printf("Start Debloat? Press Enter to continue\n");
    getchar();


    
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    printf("Running Telemetry Tweaks\n");
    run_telemetry_tweaks();

    printf("Running Services Manual\n");
    set_services_manual();
    
    printf("Running Disable Features\n");
    disable_features();

    QueryPerformanceCounter(&end);
    double elapsed = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    printf("Time taken: %.2f seconds\n", elapsed);

    printf("Running App Removal\n");
    run_app_removal();

    

    printf("\nDebloat complete. Press Enter to exit...\n");
    getchar();
    return 0;
}
