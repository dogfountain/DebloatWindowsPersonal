#ifndef TWEAKS_H
#define TWEAKS_H

#include <windows.h>
#include <stdbool.h>

// Registry operation structure
typedef struct {
    HKEY root;
    const char* subkey;
    const char* value_name;
    DWORD type;
    const void* data;
    DWORD data_size;
    const char* description;
} RegistryOperation;

// Scheduled task operation structure
typedef struct {
    const char* task_path;
    const char* description;
} TaskOperation;

// Function declarations
static bool add_registry_value(const RegistryOperation* op);
bool delete_registry_key(HKEY root, const char* subkey, const char* description);
bool configure_service(const char* service_name, DWORD start_type, const char* description);
bool disable_scheduled_task(const char* task_path, const char* description);
void run_telemetry_tweaks(void);

#endif // TWEAKS_H