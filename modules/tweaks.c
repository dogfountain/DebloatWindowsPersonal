#include "tweaks.h"
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Function to safely add registry value
bool add_registry_value(const RegistryOperation* op) {
    HKEY hkey;
    LONG result;
    
    printf("Applying: %s\n", op->description);
    
    // Create/open the key
    result = RegCreateKeyExA(op->root, op->subkey, 0, NULL, 
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL);
    
    if (result != ERROR_SUCCESS) {
        printf("ERROR: Failed to open key %s (Error: %ld)\n", op->subkey, result);
        return false;
    }
    
    // Set the value
    result = RegSetValueExA(hkey, op->value_name, 0, op->type, 
                           (const BYTE*)op->data, op->data_size);
    
    RegCloseKey(hkey);
    
    if (result != ERROR_SUCCESS) {
        printf("ERROR: Failed to set value %s (Error: %ld)\n", op->value_name, result);
        return false;
    }
    
    printf("SUCCESS: %s\n", op->description);
    return true;
}

// Function to safely delete registry key
bool delete_registry_key(HKEY root, const char* subkey, const char* description) {
    LONG result;
    
    printf("Deleting: %s\n", description);
    
    result = RegDeleteKeyA(root, subkey);
    
    if (result == ERROR_SUCCESS) {
        printf("SUCCESS: %s\n", description);
        return true;
    } else if (result == ERROR_FILE_NOT_FOUND) {
        printf("INFO: Key not found (already deleted): %s\n", description);
        return true; // Consider this success
    } else {
        printf("ERROR: Failed to delete key %s (Error: %ld)\n", subkey, result);
        return false;
    }
}

// Function to safely manage services
bool configure_service(const char* service_name, DWORD start_type, const char* description) {
    SC_HANDLE scm, service;
    bool success = false;
    
    printf("Configuring service: %s - %s\n", service_name, description);
    
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        printf("ERROR: Failed to open Service Control Manager (Error: %ld)\n", GetLastError());
        return false;
    }
    
    service = OpenServiceA(scm, service_name, SERVICE_CHANGE_CONFIG | SERVICE_STOP);
    if (service) {
        // Try to stop the service first if we're disabling it
        if (start_type == SERVICE_DISABLED) {
            SERVICE_STATUS status;
            if (ControlService(service, SERVICE_CONTROL_STOP, &status)) {
                printf("INFO: Service %s stopped\n", service_name);
            }
        }
        
        if (ChangeServiceConfigA(service, SERVICE_NO_CHANGE, start_type,
                                SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
            printf("SUCCESS: Service %s configured\n", service_name);
            success = true;
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_ACCESS_DENIED) {
                printf("WARNING: Access denied for service %s (protected service)\n", service_name);
            } else {
                printf("ERROR: Failed to configure service %s (Error: %ld)\n", service_name, error);
            }
        }
        CloseServiceHandle(service);
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
            printf("INFO: Service %s does not exist\n", service_name);
            success = true; // Consider this success
        } else {
            printf("WARNING: Cannot access service %s (Error: %ld)\n", service_name, error);
        }
    }
    
    CloseServiceHandle(scm);
    return success;
}

// Function to disable scheduled tasks
bool disable_scheduled_task(const char* task_path, const char* description) {
    char command[512];
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD exit_code;
    bool success = false;
    
    printf("Disabling task: %s\n", description);
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    snprintf(command, sizeof(command), "schtasks /Change /TN \"%s\" /Disable", task_path);
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                      NULL, NULL, &si, &pi)) {
        
        WaitForSingleObject(pi.hProcess, 5000); // 5 second timeout
        
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code == 0) {
                printf("SUCCESS: Task %s disabled\n", description);
                success = true;
            } else if (exit_code == 1) {
                printf("INFO: Task %s already disabled or not found\n", description);
                success = true; // Consider this success
            } else {
                printf("WARNING: Task %s disable failed (Exit code: %ld)\n", description, exit_code);
            }
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("ERROR: Failed to execute schtasks for %s\n", description);
    }
    
    return success;
}

void run_telemetry_tweaks(void) {
    printf("=== Starting Windows Debloat and Privacy Tweaks ===\n\n");
    
    // Define registry operations
    DWORD disable_value = 0;
    DWORD enable_value = 1;
    DWORD two_value = 2;
    const char* deny_string = "Deny";
    
    RegistryOperation registry_ops[] = {
        // Disable Consumer Features
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent", 
         "DisableConsumerFeatures", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Consumer Features"},
        
        // Disable Widgets
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Dsh", 
         "AllowNewsAndInterests", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable News and Interests"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\Windows Feeds", 
         "EnableFeeds", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Windows Feeds"},
        {HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Feeds", 
         "ShellFeedsTaskbarViewMode", REG_DWORD, &two_value, sizeof(DWORD),
         "Hide Feeds from Taskbar"},
        
        // Disable Telemetry
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection", 
         "AllowTelemetry", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Telemetry"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection", 
         "AllowTelemetry", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Telemetry (Secondary)"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection", 
         "DoNotShowFeedbackNotifications", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Feedback Notifications"},
        
        // Disable Feedback and Tailored Experiences
        {HKEY_CURRENT_USER, "SOFTWARE\\Policies\\Microsoft\\Windows\\CloudContent", 
         "DisableTailoredExperiencesWithDiagnosticData", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Tailored Experiences"},
        
        // Disable Error Reporting
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\Windows Error Reporting", 
         "Disabled", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Windows Error Reporting"},
        
        // Disable Advertising ID
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\AdvertisingInfo", 
         "DisabledByGroupPolicy", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Advertising ID"},
        
        // Disable SIUF feedback count
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Siuf\\Rules", 
         "NumberOfSIUFInPeriod", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable SIUF Feedback"},
        
        // Disable Recall (Windows 11)
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsAI", 
         "DisableAIRecall", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Windows Recall"},
        
        // Disable Copilot
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsCopilot", 
         "TurnOffWindowsCopilot", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Windows Copilot (System)"},
        {HKEY_CURRENT_USER, "Software\\Policies\\Microsoft\\Windows\\WindowsCopilot", 
         "TurnOffWindowsCopilot", REG_DWORD, &enable_value, sizeof(DWORD),
         "Disable Windows Copilot (User)"},
        
        // ContentDeliveryManager settings
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "ContentDeliveryAllowed", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Content Delivery"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "OemPreInstalledAppsEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable OEM Pre-installed Apps"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "PreInstalledAppsEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Pre-installed Apps"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "PreInstalledAppsEverEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Pre-installed Apps (Ever)"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SilentInstalledAppsEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Silent App Installation"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SubscribedContent-338387Enabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Subscribed Content 338387"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SubscribedContent-338388Enabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Subscribed Content 338388"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SubscribedContent-338389Enabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Subscribed Content 338389"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SubscribedContent-353698Enabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Subscribed Content 353698"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ContentDeliveryManager", 
         "SystemPaneSuggestionsEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable System Pane Suggestions"},
        
        // Additional privacy and UI tweaks
        {HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 
         "HideSCAMeetNow", REG_DWORD, &enable_value, sizeof(DWORD),
         "Hide Meet Now Button"},
        {HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\UserProfileEngagement", 
         "ScoobeSystemSettingEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Profile Engagement"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\DeliveryOptimization\\Config", 
         "DODownloadMode", REG_DWORD, &enable_value, sizeof(DWORD),
         "Set Delivery Optimization Mode"},
        {HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Remote Assistance", 
         "fAllowToGetHelp", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Remote Assistance"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\OperationStatusManager", 
         "EnthusiastMode", REG_DWORD, &enable_value, sizeof(DWORD),
         "Enable Enthusiast Mode"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 
         "ShowTaskViewButton", REG_DWORD, &disable_value, sizeof(DWORD),
         "Hide Task View Button"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\People", 
         "PeopleBand", REG_DWORD, &disable_value, sizeof(DWORD),
         "Hide People Band"},
        {HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 
         "LaunchTo", REG_DWORD, &enable_value, sizeof(DWORD),
         "Set Explorer Launch Target"},
        {HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\FileSystem", 
         "LongPathsEnabled", REG_DWORD, &enable_value, sizeof(DWORD),
         "Enable Long Paths"},
        
        // Location and privacy settings
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\location", 
         "Value", REG_SZ, deny_string, strlen(deny_string) + 1,
         "Deny Location Access"},
        {HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Sensor\\Overrides\\{BFA794E4-F964-4FDB-90F6-51056BFE4B44}", 
         "SensorPermissionState", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Location Sensor"},
        {HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\lfsvc\\Service\\Configuration", 
         "Status", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Location Service"},
        {HKEY_LOCAL_MACHINE, "SYSTEM\\Maps", 
         "AutoUpdateEnabled", REG_DWORD, &disable_value, sizeof(DWORD),
         "Disable Maps Auto Update"}
    };
    
    // Scheduled tasks to disable
    TaskOperation task_ops[] = {
        {"\\Microsoft\\Windows\\Customer Experience Improvement Program\\Consolidator", "CEIP Consolidator"},
        {"\\Microsoft\\Windows\\Customer Experience Improvement Program\\UsbCeip", "USB CEIP"},
        {"\\Microsoft\\Windows\\Application Experience\\ProgramDataUpdater", "Program Data Updater"},
        {"\\Microsoft\\Windows\\Application Experience\\Microsoft Compatibility Appraiser", "Compatibility Appraiser"},
        {"\\Microsoft\\Windows\\Application Experience\\StartupAppTask", "Startup App Task"},
        {"\\Microsoft\\Windows\\Application Experience\\PcaPatchDbTask", "PCA Patch Database Task"},
        {"\\Microsoft\\Windows\\Application Experience\\MareBackup", "Mare Backup"},
        {"\\Microsoft\\Windows\\Autochk\\Proxy", "Autochk Proxy"},
        {"\\Microsoft\\Windows\\DiskDiagnostic\\Microsoft-Windows-DiskDiagnosticDataCollector", "Disk Diagnostic Data Collector"},
        {"\\Microsoft\\Windows\\Feedback\\Siuf\\DmClient", "Feedback DM Client"},
        {"\\Microsoft\\Windows\\Feedback\\Siuf\\DmClientOnScenarioDownload", "Feedback DM Client Scenario"},
        {"\\Microsoft\\Windows\\Windows Error Reporting\\QueueReporting", "Error Reporting Queue"},
        {"\\Microsoft\\Windows\\Maps\\MapsUpdateTask", "Maps Update Task"}
    };
    
    // Apply registry operations
    printf("=== Applying Registry Tweaks ===\n");
    int reg_ops_count = sizeof(registry_ops) / sizeof(registry_ops[0]);
    int reg_success_count = 0;
    
    for (int i = 0; i < reg_ops_count; i++) {
        if (add_registry_value(&registry_ops[i])) {
            reg_success_count++;
        }
    }
    
    // Delete Edge policy key
    printf("\n=== Removing Edge Policies ===\n");
    bool edge_deleted = delete_registry_key(HKEY_LOCAL_MACHINE, 
                                           "SOFTWARE\\Policies\\Microsoft\\Edge",
                                           "Remove Edge 'Managed by organization' policies");
    
    // Configure services
    printf("\n=== Configuring Services ===\n");
    int service_success_count = 0;
    if (configure_service("DiagTrack", SERVICE_DISABLED, "Disable Diagnostic Tracking Service")) {
        service_success_count++;
    }
    if (configure_service("dmwappushservice", SERVICE_DISABLED, "Disable Device Management WAP Push Service")) {
        service_success_count++;
    }
    
    // Disable scheduled tasks
    printf("\n=== Disabling Scheduled Tasks ===\n");
    int task_ops_count = sizeof(task_ops) / sizeof(task_ops[0]);
    int task_success_count = 0;
    
    for (int i = 0; i < task_ops_count; i++) {
        if (disable_scheduled_task(task_ops[i].task_path, task_ops[i].description)) {
            task_success_count++;
        }
    }
    
    // Summary
    printf("\n=== SUMMARY ===\n");
    printf("Registry operations: %d/%d successful\n", reg_success_count, reg_ops_count);
    printf("Service operations: %d/2 successful\n", service_success_count);
    printf("Task operations: %d/%d successful\n", task_success_count, task_ops_count);
    printf("Edge policy cleanup: %s\n", edge_deleted ? "SUCCESS" : "FAILED");
    printf("\nAll selected telemetry and privacy tweaks have been applied.\n");
}