#pragma once
#include <stdbool.h>

void sip_init(void);
void sip_make_call(const char *uri);
void sip_set_dnd(bool enabled);
