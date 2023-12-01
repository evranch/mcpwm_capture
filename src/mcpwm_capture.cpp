#include "mcpwm_capture.h"
#include "esp_timer.h"

bool mcpwm_capture::channel0_init = false;
bool mcpwm_capture::channel1_init = false;

static bool mcpwm_p_isr_1(mcpwm_unit_t mcpwm,
                          mcpwm_capture_channel_id_t cap_channel,
                          const cap_event_data_t *edata,
                          void *args)
{
    mcpwm_capture *isr_instance = (mcpwm_capture *)args;

    if (mcpwm_capture_signal_get_edge(MCPWM_UNIT_0, MCPWM_SELECT_CAP0) == 1)
    {
        isr_instance->pos_edge = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);

        if (isr_instance->pos_edge > isr_instance->neg_edge)
            isr_instance->low_pulse = isr_instance->pos_edge - isr_instance->neg_edge;
        isr_instance->period = isr_instance->pos_edge - isr_instance->last_pos_edge;
        isr_instance->last_pos_edge = isr_instance->pos_edge;
    }
    else if (mcpwm_capture_signal_get_edge(MCPWM_UNIT_0, MCPWM_SELECT_CAP0) == 2)
    {
        isr_instance->neg_edge = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);

        if (isr_instance->neg_edge > isr_instance->pos_edge)
            isr_instance->high_pulse = isr_instance->neg_edge - isr_instance->pos_edge;
    }

    isr_instance->last_pulse = millis();
    return 0;
}

static bool mcpwm_p_isr_2(mcpwm_unit_t mcpwm,
                          mcpwm_capture_channel_id_t cap_channel,
                          const cap_event_data_t *edata,
                          void *args)
{
    mcpwm_capture *isr_instance = (mcpwm_capture *)args;

    if (mcpwm_capture_signal_get_edge(MCPWM_UNIT_1, MCPWM_SELECT_CAP1) == 1)
    {
        isr_instance->pos_edge = mcpwm_capture_signal_get_value(MCPWM_UNIT_1, MCPWM_SELECT_CAP1);

        if (isr_instance->pos_edge > isr_instance->neg_edge)
            isr_instance->low_pulse = isr_instance->pos_edge - isr_instance->neg_edge;
        isr_instance->period = isr_instance->pos_edge - isr_instance->last_pos_edge;
        isr_instance->last_pos_edge = isr_instance->pos_edge;
    }

    else if (mcpwm_capture_signal_get_edge(MCPWM_UNIT_1, MCPWM_SELECT_CAP1) == 2)
    {
        isr_instance->neg_edge = mcpwm_capture_signal_get_value(MCPWM_UNIT_1, MCPWM_SELECT_CAP1);

        if (isr_instance->neg_edge > isr_instance->pos_edge)
            isr_instance->high_pulse = isr_instance->neg_edge - isr_instance->pos_edge;
    }

    isr_instance->last_pulse = millis();
    return 0;
}

mcpwm_capture::mcpwm_capture(mcpwm_unit_t unit, int set_pin)
    : sense_pin(set_pin), mcpwm_unit(unit)
{
    pinMode(sense_pin, INPUT);

    if (mcpwm_unit == 0 && !channel0_init)
    {
        mcpwm_capture::channel0_init = true;
        mcpwm_capture_config_t capture_config0 =
            {
                .cap_edge = MCPWM_BOTH_EDGE, /*!<Set capture edge*/
                .cap_prescale = 1,           /*!<Prescale of capture signal, ranging from 1 to 256 - 80 MHz*/
                .capture_cb = mcpwm_p_isr_1, /*!<User defined capture event callback, running under interrupt context */
                .user_data = this,           /*!<User defined ISR callback function args*/
            };

        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, sense_pin);
        mcpwm_capture_enable_channel(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, &capture_config0);
    }
    else if (mcpwm_unit == 1 && !channel1_init)
    {
        mcpwm_capture::channel1_init = true;
        mcpwm_capture_config_t capture_config1 =
            {
                .cap_edge = MCPWM_BOTH_EDGE, /*!<Set capture edge*/
                .cap_prescale = 1,           /*!<Prescale of capture signal, ranging from 1 to 256 - 80 MHz*/
                .capture_cb = mcpwm_p_isr_2, /*!<User defined capture event callback, running under interrupt context */
                .user_data = this,           /*!<User defined ISR callback function args*/
            };

        mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM_CAP_1, sense_pin);
        mcpwm_capture_enable_channel(MCPWM_UNIT_1, MCPWM_SELECT_CAP1, &capture_config1);
    }
}

uint32_t mcpwm_capture::calcSpeed()
{
    if (this->period && (millis() - this->last_pulse < 500))
    {
        this->freq = 1000000 / (this->period / 80); // Scale period down from 80Mhz counter
    }
    else
    {
        this->freq = 0;
    }
    return this->freq;
}

uint8_t mcpwm_capture::calcDC()
{
    if (this->low_pulse > 0)
        this->dc = this->high_pulse * 100 / (this->high_pulse + this->low_pulse);
    else
        this->dc = 0;

    return this->dc;
}
