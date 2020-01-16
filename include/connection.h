#pragma once

#include <stdint.h>

extern int connection_listener_start(const char*, uint16_t);
extern int connection_listener_stop();
extern int connection_accepted_getfd();
