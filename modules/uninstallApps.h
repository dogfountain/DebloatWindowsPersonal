#ifndef UNINSTALL_APPS_H
#define UNINSTALL_APPS_H

#include <stdbool.h>

// Structure to hold app removal information
typedef struct {
    const char* package_name;
    const char* description;
} AppRemovalOperation;

// Function declarations
bool remove_copilot_dism(void);
bool remove_uwp_app(const char* package_name, const char* description);
bool remove_uwp_app_all_users(const char* package_name, const char* description);
void run_app_removal(void);

#endif // UNINSTALL_APPS_H