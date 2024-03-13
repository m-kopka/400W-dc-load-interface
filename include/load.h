#ifndef _LOAD_H_
#define _LOAD_H_

#include "common_defs.h"

void load_task(void);

bool load_get_ready(void);

void load_set_enable(bool enabled);

bool load_get_enable(void);

uint32_t load_get_cc_level_ma(void);

uint32_t load_get_voltage_mv(void);

uint32_t load_get_total_current_ma(void);

void load_set_cc_level(uint16_t current_ma);

uint8_t load_get_temp(void);

#endif /* _LOAD_H_ */