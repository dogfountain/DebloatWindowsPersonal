#include "uninstallApps.h"
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Function to remove Windows Copilot using DISM
bool remove_copilot_dism(void) {
    char command[512];
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD exit_code;
    bool success = false;
    
    printf("Removing Windows Copilot using DISM...\n");
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    // Create DISM command to remove Copilot
    snprintf(command, sizeof(command), 
        "dism /online /remove-package /package-name:Microsoft.Windows.Copilot");
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                      NULL, NULL, &si, &pi)) {
        
        WaitForSingleObject(pi.hProcess, 60000); // 60 second timeout for DISM
        
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code == 0) {
                printf("SUCCESS: Windows Copilot removed via DISM\n");
                success = true;
            } else if (exit_code == 2) {
                printf("INFO: Copilot package not found or already removed\n");
                success = true; // Consider this success since Copilot is gone
            } else {
                printf("WARNING: DISM command failed with exit code: %ld\n", exit_code);
                printf("This may require administrator privileges or the package may not exist\n");
            }
        } else {
            printf("WARNING: Could not get exit code for DISM command\n");
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("ERROR: Failed to execute DISM command\n");
        printf("Make sure you're running as administrator\n");
    }
    
    return success;
}

// Function to remove UWP app using PowerShell
bool remove_uwp_app(const char* package_name, const char* description) {
    char command[1024];
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD exit_code;
    bool success = false;
    
    printf("Removing: %s\n", description);
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    // Create PowerShell command to remove the app
    snprintf(command, sizeof(command), 
        "powershell.exe -WindowStyle Hidden -Command \"Get-AppxPackage '%s' | Remove-AppxPackage -ErrorAction SilentlyContinue\"", 
        package_name);
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                      NULL, NULL, &si, &pi)) {
        
        WaitForSingleObject(pi.hProcess, 30000); // 30 second timeout
        
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code == 0) {
                printf("SUCCESS: %s removed\n", description);
                success = true;
            } else {
                printf("INFO: %s may not be installed or already removed\n", description);
                success = true; // Consider this success since app is gone
            }
        } else {
            printf("WARNING: Could not get exit code for %s\n", description);
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("ERROR: Failed to execute PowerShell command for %s\n", description);
    }
    
    return success;
}

// Function to remove UWP app for all users using PowerShell
bool remove_uwp_app_all_users(const char* package_name, const char* description) {
    char command[1024];
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD exit_code;
    bool success = false;
    
    printf("Removing for all users: %s\n", description);
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    // Create PowerShell command to remove the app for all users
    snprintf(command, sizeof(command), 
        "powershell.exe -WindowStyle Hidden -Command \"Get-AppxPackage -AllUsers '%s' | Remove-AppxPackage -AllUsers -ErrorAction SilentlyContinue\"", 
        package_name);
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                      NULL, NULL, &si, &pi)) {
        
        WaitForSingleObject(pi.hProcess, 30000); // 30 second timeout
        
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code == 0) {
                printf("SUCCESS: %s removed for all users\n", description);
                success = true;
            } else {
                printf("INFO: %s may not be installed or already removed for all users\n", description);
                success = true; // Consider this success since app is gone
            }
        } else {
            printf("WARNING: Could not get exit code for %s (all users)\n", description);
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("ERROR: Failed to execute PowerShell command for %s (all users)\n", description);
    }
    
    return success;
}

void run_app_removal(void) {
    printf("=== Starting UWP App Removal ===\n\n");
    
    // First, remove Windows Copilot using DISM
    printf("=== Removing Windows Copilot (DISM) ===\n");
    bool copilot_removed = remove_copilot_dism();
    printf("\n");
    
    // Define apps to remove
    AppRemovalOperation app_ops[] = {
        {"Microsoft.Microsoft3DViewer", "3D Viewer"},
        {"Microsoft.AppConnector", "App Connector"},
        {"Microsoft.BingFinance", "Bing Finance"},
        {"Microsoft.BingNews", "Bing News"},
        {"Microsoft.BingSports", "Bing Sports"},
        {"Microsoft.BingTranslator", "Bing Translator"},
        {"Microsoft.BingWeather", "Bing Weather"},
        {"Microsoft.BingFoodAndDrink", "Bing Food And Drink"},
        {"Microsoft.BingHealthAndFitness", "Bing Health And Fitness"},
        {"Microsoft.BingTravel", "Bing Travel"},
        {"Microsoft.MinecraftUWP", "Minecraft"},
        {"Microsoft.GamingServices", "Gaming Services"},
        {"Microsoft.GetHelp", "Get Help"},
        {"Microsoft.Getstarted", "Get Started"},
        {"Microsoft.Messaging", "Messaging"},
        {"Microsoft.MicrosoftSolitaireCollection", "Microsoft Solitaire Collection"},
        {"Microsoft.NetworkSpeedTest", "Network Speed Test"},
        {"Microsoft.News", "Microsoft News"},
        {"Microsoft.Office.Lens", "Office Lens"},
        {"Microsoft.Office.Sway", "Office Sway"},
        {"Microsoft.Office.OneNote", "OneNote"},
        {"Microsoft.OneConnect", "OneConnect"},
        {"Microsoft.People", "People"},
        {"Microsoft.Print3D", "Print 3D"},
        {"Microsoft.SkypeApp", "Skype"},
        {"Microsoft.Wallet", "Microsoft Wallet"},
        {"Microsoft.Whiteboard", "Microsoft Whiteboard"},
        {"Microsoft.WindowsAlarms", "Windows Alarms & Clock"},
        {"microsoft.windowscommunicationsapps", "Mail and Calendar"},
        {"Microsoft.WindowsFeedbackHub", "Feedback Hub"},
        {"Microsoft.WindowsMaps", "Maps"},
        {"Microsoft.YourPhone", "Your Phone"},
        {"Microsoft.WindowsSoundRecorder", "Voice Recorder"},
        {"Microsoft.XboxApp", "Xbox Console Companion"},
        {"Microsoft.ConnectivityStore", "Connectivity Store"},
        {"Microsoft.ScreenSketch", "Snip & Sketch"},
        {"Microsoft.Xbox.TCUI", "Xbox TCUI"},
        {"Microsoft.XboxGameOverlay", "Xbox Game Overlay"},
        {"Microsoft.XboxGameCallableUI", "Xbox Game Callable UI"},
        {"Microsoft.XboxSpeechToTextOverlay", "Xbox Speech To Text Overlay"},
        {"Microsoft.MixedReality.Portal", "Mixed Reality Portal"},
        {"Microsoft.XboxIdentityProvider", "Xbox Identity Provider"},
        {"Microsoft.ZuneMusic", "Groove Music"},
        {"Microsoft.ZuneVideo", "Movies & TV"},
        {"Microsoft.MicrosoftOfficeHub", "Office"},
        {"*EclipseManager*", "Eclipse Manager"},
        {"*ActiproSoftwareLLC*", "ActiproSoftware"},
        {"*AdobeSystemsIncorporated.AdobePhotoshopExpress*", "Adobe Photoshop Express"},
        {"*Duolingo-LearnLanguagesforFree*", "Duolingo"},
        {"*PandoraMediaInc*", "Pandora"},
        {"*CandyCrush*", "Candy Crush"},
        {"*BubbleWitch3Saga*", "Bubble Witch 3 Saga"},
        {"*Wunderlist*", "Wunderlist"},
        {"*Flipboard*", "Flipboard"},
        {"*Twitter*", "Twitter"},
        {"*Facebook*", "Facebook"},
        {"*Royal Revolt*", "Royal Revolt"},
        {"*Sway*", "Sway"},
        {"*Speed Test*", "Speed Test"},
        {"*Dolby*", "Dolby"},
        {"*Viber*", "Viber"},
        {"*ACGMediaPlayer*", "ACG Media Player"},
        {"*Netflix*", "Netflix"},
        {"*OneCalendar*", "OneCalendar"},
        {"*LinkedInforWindows*", "LinkedIn"},
        {"*HiddenCityMysteryofShadows*", "Hidden City: Mystery of Shadows"},
        {"*Hulu*", "Hulu"},
        {"*HiddenCity*", "Hidden City"},
        {"*AdobePhotoshopExpress*", "Adobe Photoshop Express"},
        {"*HotspotShieldFreeVPN*", "Hotspot Shield Free VPN"},
        {"*Microsoft.Advertising.Xaml*", "Microsoft Advertising Xaml"}
    };
    
    // Apply app removal operations
    printf("=== Removing Apps for Current User ===\n");
    int app_ops_count = sizeof(app_ops) / sizeof(app_ops[0]);
    int success_count = 0;
    
    for (int i = 0; i < app_ops_count; i++) {
        if (remove_uwp_app(app_ops[i].package_name, app_ops[i].description)) {
            success_count++;
        }
        printf("\n"); // Add spacing between operations
    }
    
    printf("=== Removing Apps for All Users ===\n");
    int all_users_success_count = 0;
    
    for (int i = 0; i < app_ops_count; i++) {
        if (remove_uwp_app_all_users(app_ops[i].package_name, app_ops[i].description)) {
            all_users_success_count++;
        }
        printf("\n"); // Add spacing between operations
    }
    
    // Summary
    printf("=== SUMMARY ===\n");
    printf("Windows Copilot (DISM): %s\n", copilot_removed ? "SUCCESS" : "FAILED");
    printf("Apps removed for current user: %d/%d\n", success_count, app_ops_count);
    printf("Apps removed for all users: %d/%d\n", all_users_success_count, app_ops_count);
    printf("\nApp removal process completed.\n");
    printf("Note: Some apps may not have been installed, which is normal.\n");
    printf("You may need to restart Windows to complete the removal process.\n");
}