#include "setManual.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#pragma comment(lib, "Advapi32.lib")

// Enhanced function that checks existence and configures in one go
bool configure_service_to_manual(const char* service_name) {
    SC_HANDLE scm, service;
    bool success = false;
    
    // Open Service Control Manager
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        printf("ERROR: Failed to open Service Control Manager\n");
        return false;
    }
    
    // Try to open the service
    service = OpenServiceA(scm, service_name, SERVICE_CHANGE_CONFIG);
    if (service) {
        // Configure service to manual start
        if (ChangeServiceConfigA(service, SERVICE_NO_CHANGE, SERVICE_DEMAND_START,
                                SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
            printf("SUCCESS: Set service %s to Manual\n", service_name);
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
            // Don't count this as failure - service doesn't exist is fine
            success = true;
        } else if (error == ERROR_ACCESS_DENIED) {
            printf("WARNING: Access denied when opening service %s\n", service_name);
        } else {
            printf("WARNING: Cannot access service %s (Error: %ld)\n", service_name, error);
        }
    }
    
    CloseServiceHandle(scm);
    return success;
}

// Alternative: Keep your original approach but with better error handling
bool configure_service_with_sc_command(const char* service_name) {
    char command[256];
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    DWORD exit_code;
    bool success = false;
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Hide the command window
    
    snprintf(command, sizeof(command), "sc config %s start= demand", service_name);
    
    if (CreateProcessA(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
                      NULL, NULL, &si, &pi)) {
        
        // Wait for the process to complete (5 second timeout)
        WaitForSingleObject(pi.hProcess, 5000);
        
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code == 0) {
                printf("SUCCESS: Set service %s to Manual\n", service_name);
                success = true;
            } else {
                printf("ERROR: Failed to configure service %s (Exit code: %ld)\n", 
                       service_name, exit_code);
            }
        } else {
            printf("ERROR: Could not get exit code for service %s\n", service_name);
        }
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("ERROR: Failed to execute sc command for service %s\n", service_name);
    }
    
    return success;
}

// Check if service exists (your original function is good)
int service_exists(const char* service_name) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 0;
    
    SC_HANDLE svc = OpenServiceA(scm, service_name, SERVICE_QUERY_STATUS);
    if (svc) {
        CloseServiceHandle(svc);
        CloseServiceHandle(scm);
        return 1;
    }
    
    CloseServiceHandle(scm);
    return 0;
}

void set_services_manual(void) {
    const char* services[] = {
        "ALG","AppIDSvc","AppMgmt","AppReadiness","AppXSvc","Appinfo","AxInstSV",
        "BDESVC","BTAGService","Browser","CDPSvc","COMSysApp","CertPropSvc",
        "ClipSVC","CscService","DcpSvc","DevQueryBroker","DeviceAssociationService",
        "DeviceInstall","DisplayEnhancementService","DmEnrollmentSvc","EFS",
        "EapHost","EntAppSvc","FDResPub","Fax","FrameServer","FrameServerMonitor",
        "GraphicsPerfSvc","HomeGroupListener","HomeGroupProvider","HvHost",
        "IEEtwCollectorService","IKEEXT","InstallService","InventorySvc",
        "IpxlatCfgSvc","KtmRm","LicenseManager","LxpSvc","MSDTC","MSiSCSI",
        "McpManagementService","MicrosoftEdgeElevationService","MixedRealityOpenXRSvc",
        "MsKeyboardFilter","NaturalAuthentication","NcaSvc","NcbService",
        "NcdAutoSetup","NetSetupSvc","Netman","NgcCtnrSvc","NgcSvc","NlaSvc",
        "PNRPAutoReg","PNRPsvc","PcaSvc","PeerDistSvc","PerfHost","PhoneSvc",
        "PlugPlay","PolicyAgent","PrintNotify","PushToInstall","QWAVE","RasAuto",
        "RasMan","RetailDemo","RmSvc","RpcLocator","SCPolicySvc","SCardSvr",
        "SDRSVC","SEMgrSvc","SNMPTRAP","SNMPTrap","SSDPSRV","ScDeviceEnum",
        "SecurityHealthService","Sense","SensorDataService","SensorService",
        "SensrSvc","SessionEnv","SharedAccess","SharedRealitySvc","SmsRouter",
        "SstpSvc","StiSvc","StorSvc","TabletInputService","TapiSrv",
        "TieringEngineService","TimeBroker","TimeBrokerSvc","TokenBroker",
        "TroubleshootingSvc","TrustedInstaller","UI0Detect","UmRdpService",
        "UsoSvc","VSS","VacSvc","WaaSMedicSvc","WalletService","WarpJITSvc",
        "WbioSrvc","WcsPlugInService","WdNisSvc","WdiServiceHost","WdiSystemHost",
        "WebClient","Wecsvc","WerSvc","WiaRpc","WinHttpAutoProxySvc","WinRM",
        "WpcMonSvc","WpnService","XblAuthManager","XblGameSave","XboxGipSvc",
        "XboxNetApiSvc","autotimesvc","bthserv","camsvc","cloudidsvc","dcsvc",
        "defragsvc","diagnosticshub.standardcollector.service","diagsvc",
        "dmwappushservice","dot3svc","edgeupdate","edgeupdatem","embeddedmode",
        "fdPHost","fhsvc","hidserv","icssvc","lfsvc","lltdsvc","lmhosts",
        "msiserver","netprofm","p2pimsvc","p2psvc","perceptionsimulation","pla",
        "seclogon","smphost","spectrum","svsvc","swprv","upnphost","vds",
        "vm3dservice","vmicguestinterface","vmicheartbeat","vmickvpexchange",
        "vmicrdv","vmicshutdown","vmictimesync","vmicvmsession","vmicvss","vmvss",
        "wbengine","wcncsvc","webthreatdefsvc","wercplsupport","wisvc","wlidsvc",
        "wlpasvc","wmiApSrv","workfolderssvc","wuauserv","wudfsvc"
    };
    
    int num_services = sizeof(services) / sizeof(services[0]);
    int success_count = 0;
    int processed_count = 0;
    
    printf("\n=== Setting Services to Manual Start ===\n");
    printf("Processing %d services...\n\n", num_services);
    
    for (int i = 0; i < num_services; ++i) {
        // Skip wildcard services (removed from array above)
        // Only process services that don't contain wildcards
        if (strchr(services[i], '*') == NULL) {
            processed_count++;
            
            // Use the improved API-based approach
            if (configure_service_to_manual(services[i])) {
                success_count++;
            }
            
            // Alternative: Use the command-line approach with better error handling
            // if (configure_service_with_sc_command(services[i])) {
            //     success_count++;
            // }
        } else {
            printf("Skipping wildcard service: %s\n", services[i]);
        }
    }
    
    printf("\n=== SUMMARY ===\n");
    printf("Services processed: %d\n", processed_count);
    printf("Successfully configured: %d\n", success_count); 
    printf("Failed or inaccessible: %d\n", processed_count - success_count);
    printf("Service configuration completed.\n");
}