#ifndef _LOAD_H_
#define _LOAD_H_

#include "common_defs.h"
#include "cmd_spi_driver.h"

void load_task(void);

void load_start_data_capture(void);

bool load_get_data_capture_done(void);

bool load_get_ready(void);

void load_set_enable(bool enabled);

bool load_get_enable(void);

load_mode_t load_get_mode(void);

load_fault_t load_get_faults(void);

bool load_get_vsensrc(void);

bool load_get_not_in_reg(void);

uint32_t load_get_ena_time(void);

uint32_t load_get_total_mah(void);

uint32_t load_get_total_mwh(void);

uint32_t load_get_cc_level_ma(void);

uint32_t load_get_voltage_mv(void);

uint32_t load_get_total_current_ma(void);

void load_set_cc_level(uint16_t current_ma);

uint8_t load_get_temp(void);

#endif /* _LOAD_H_ */