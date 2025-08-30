#include "sip_wrapper.h"
#include "config.h"
#include <stdio.h>

void sip_init(void) {
    printf("SIP init (stub)\n");
}

void sip_make_call(const char *uri) {
    printf("Calling %s (stub)\n", uri);
}

void sip_set_dnd(bool enabled) {
    printf("DND mode: %d (stub)\n", enabled);
}
