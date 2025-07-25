#include "disableFeatures.h"
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Helper to add/set a registry value (supports REG_DWORD and REG_SZ)
static bool add_registry_value(HKEY root, const char* subkey, const char* value_name,
                        DWORD type, const void* data, DWORD data_size, const char* description) {
    HKEY hKey;
    LONG result;

    printf("Applying: %s\n", description);

    result = RegCreateKeyExA(root, subkey, 0, NULL, REG_OPTION_NON_VOLATILE,
                            KEY_WRITE, NULL, &hKey, NULL);

    if (result != ERROR_SUCCESS) {
        printf("ERROR: Failed to open/create key '%s' (Error: %ld)\n", subkey, result);
        return false;
    }

    result = RegSetValueExA(hKey, value_name, 0, type, (const BYTE*)data, data_size);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        printf("ERROR: Failed to set value '%s' in key '%s' (Error: %ld)\n", value_name, subkey, result);
        return false;
    }

    printf("SUCCESS: %s\n", description);
    return true;
}

// Helper to configure a service start type to manual (SERVICE_DEMAND_START)
bool configure_service_manual(const char* service_name, const char* description) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        printf("ERROR: Failed to open Service Control Manager (Error: %ld)\n", GetLastError());
        return false;
    }

    SC_HANDLE service = OpenServiceA(scm, service_name, SERVICE_CHANGE_CONFIG);
    if (!service) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            printf("INFO: Service '%s' does not exist\n", service_name);
            CloseServiceHandle(scm);
            return true; // Consider success if service not present
        }
        printf("ERROR: Failed to open service '%s' (Error: %ld)\n", service_name, err);
        CloseServiceHandle(scm);
        return false;
    }

    BOOL success = ChangeServiceConfigA(service, SERVICE_NO_CHANGE, SERVICE_DEMAND_START,
                                        SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    if (success) {
        printf("SUCCESS: %s\n", description);
    } else {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            printf("WARNING: Access denied for service '%s'\n", service_name);
        } else {
            printf("ERROR: Failed to configure service '%s' (Error: %ld)\n", service_name, err);
        }
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    return success == TRUE;
}

void disable_features(void) {
    printf("=== Starting disable_features tweaks ===\n\n");

    int success_count = 0;
    int processed_count = 0;

    // Services to set manual
    struct {
        const char* service_name;
        const char* description;
    } services[] = {
        {"HomeGroupListener", "Set HomeGroupListener service to manual start"},
        {"HomeGroupProvider", "Set HomeGroupProvider service to manual start"}
    };

    printf("=== Configuring Services ===\n");
    int num_services = sizeof(services) / sizeof(services[0]);
    for (int i = 0; i < num_services; i++) {
        processed_count++;
        if (configure_service_manual(services[i].service_name, services[i].description)) {
            success_count++;
        }
    }

    // Registry DWORD zero for disabling WiFi Sense values
    DWORD zero = 0;
    // Registry DWORD one for disabling Copilot (turn off)
    DWORD one = 1;

    struct {
        HKEY root;
        const char* subkey;
        const char* value_name;
        DWORD type;
        const void* data;
        DWORD data_size;
        const char* description;
    } registry_ops[] = {
        // Disable WiFi Sense
        {HKEY_LOCAL_MACHINE, "Software\\Microsoft\\PolicyManager\\default\\WiFi\\AllowWiFiHotSpotReporting", "Value",
         REG_DWORD, &zero, sizeof(DWORD), "Disable WiFi HotSpot Reporting"},
        {HKEY_LOCAL_MACHINE, "Software\\Microsoft\\PolicyManager\\default\\WiFi\\AllowAutoConnectToWiFiSenseHotspots", "Value",
         REG_DWORD, &zero, sizeof(DWORD), "Disable Auto Connect to WiFi Sense Hotspots"},

        // Disable Windows Copilot
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsCopilot", "TurnOffWindowsCopilot",
         REG_DWORD, &one, sizeof(DWORD), "Disable Windows Copilot (System)"},
        {HKEY_CURRENT_USER, "Software\\Policies\\Microsoft\\Windows\\WindowsCopilot", "TurnOffWindowsCopilot",
         REG_DWORD, &one, sizeof(DWORD), "Disable Windows Copilot (User)"},
        {HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", "ShowCopilotButton",
         REG_DWORD, &zero, sizeof(DWORD), "Hide Copilot Button"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\Shell\\Copilot", "IsCopilotAvailable",
         REG_DWORD, &zero, sizeof(DWORD), "Disable Copilot Availability"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\Shell\\Copilot", "CopilotDisabledReason",
         REG_SZ, "IsEnabledForGeographicRegionFailed", (DWORD)(strlen("IsEnabledForGeographicRegionFailed") + 1),
         "Set Copilot Disabled Reason"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsCopilot", "AllowCopilotRuntime",
         REG_DWORD, &zero, sizeof(DWORD), "Disallow Copilot Runtime"},

        // Block Shell Extensions GUID
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Blocked",
         "{CB3B0003-8088-4EDE-8769-8B354AB2FF8C}", REG_SZ, "", 1, "Block Shell Extension GUID"},

        // Disable BingChat eligibility
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\Shell\\Copilot\\BingChat", "IsUserEligible",
         REG_DWORD, &zero, sizeof(DWORD), "Disable BingChat User Eligibility"},
    };

    printf("\n=== Applying Registry Tweaks ===\n");
    int num_registry_ops = sizeof(registry_ops) / sizeof(registry_ops[0]);
    for (int i = 0; i < num_registry_ops; i++) {
        processed_count++;
        if (add_registry_value(
                registry_ops[i].root,
                registry_ops[i].subkey,
                registry_ops[i].value_name,
                registry_ops[i].type,
                registry_ops[i].data,
                registry_ops[i].data_size,
                registry_ops[i].description)) {
            success_count++;
        }
    }

    printf("\n=== SUMMARY ===\n");
    printf("Operations processed: %d\n", processed_count);
    printf("Successfully configured: %d\n", success_count); 
    printf("Failed or inaccessible: %d\n", processed_count - success_count);
    printf("Feature disabling completed.\n");
}