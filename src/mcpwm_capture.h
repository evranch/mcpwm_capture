#ifndef MCPWMISR_H
#define MCPWMISR_H

#include "driver/mcpwm.h"
#include "Arduino.h"

class mcpwm_capture
{
public:
	static bool channel0_init, channel1_init;
	uint32_t pos_edge = 0;
	uint32_t neg_edge = 0;
	uint32_t last_pos_edge = 0;
	uint32_t low_pulse = 0;
	uint32_t high_pulse = 0;
	unsigned long last_pulse = 0;
	uint32_t period = 0;
	uint32_t freq = 0;
	uint32_t freq_ema = 0;
	uint8_t dc = 0;
	uint8_t dc_ema = 0;

	int sense_pin = 0;
	int timeout_millis = 500;
	int ema_samples = 10;
	mcpwm_unit_t mcpwm_unit = MCPWM_UNIT_0;

	mcpwm_capture(mcpwm_unit_t unit, int set_pin);
	uint32_t calcSpeed();
	uint32_t speed_EMA();
	uint8_t calcDC();
	uint8_t DC_EMA();
	void setTimeout(int timeout);
	void calculate();
};

static bool mcpwm_p_isr_1(mcpwm_unit_t mcpwm,
						  mcpwm_capture_channel_id_t cap_channel,
						  const cap_event_data_t *edata,
						  void *args);

static bool mcpwm_p_isr_2(mcpwm_unit_t mcpwm,
						  mcpwm_capture_channel_id_t cap_channel,
						  const cap_event_data_t *edata,
						  void *args);

#endif