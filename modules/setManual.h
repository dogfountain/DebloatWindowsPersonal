#ifndef SETMANUAL_H
#define SETMANUAL_H

#include <stdbool.h> // Needed for bool return types

// Function declarations
bool configure_service_to_manual(const char* service_name);
bool configure_service_with_sc_command(const char* service_name);
int service_exists(const char* service_name);
void set_services_manual(void);

#endif // SETMANUAL_H