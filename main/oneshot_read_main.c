#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
// #include "esp_timer.h" // Include esp_timer for microsecond timestamps
#include "driver/dac_oneshot.h" // Include DAC driver
// #include "esp_adc_cal.h"

const static char *TAG = "EXAMPLE";

// ADC1 Channels
#if CONFIG_IDF_TARGET_ESP32
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_4
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_5
#else
#define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_2
#define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_3
#endif

#if (SOC_ADC_PERIPH_NUM >= 2) && !CONFIG_IDF_TARGET_ESP32C3
#define EXAMPLE_USE_ADC2            1               
#endif // setting to use third ADC ^^^

#if EXAMPLE_USE_ADC2
// ADC2 Channels
#if CONFIG_IDF_TARGET_ESP32
#define EXAMPLE_ADC2_CHAN0          ADC_CHANNEL_0
#else
#define EXAMPLE_ADC2_CHAN0          ADC_CHANNEL_0
#endif
#endif  // #if EXAMPLE_USE_ADC2

#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_12

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t err = adc_cali_create_scheme_line_fitting(&cali_config, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize calibration: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
    adc_cali_delete_scheme_line_fitting(handle);
}
static int adc_raw[2][10];
static int voltage[2][10];
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);


// better feedback code
uint8_t process_voltage_custom(int raw_voltage, float offset, float mult) {
    // Convert raw voltage to actual voltage (float)
    float actual_voltage = raw_voltage / 1000.0;

    // Subtract the difference from 0.22V
    float adjusted_voltage = actual_voltage - offset;

    // Multiply the signal by -42
    adjusted_voltage *= mult;

    // Add a 1.25V offset
    adjusted_voltage += 1.25;

    // Ensure the adjusted voltage does not exceed the DAC range
    if (adjusted_voltage > 3.3) {
        adjusted_voltage = 3.3;
    } else if (adjusted_voltage < 0) {
        adjusted_voltage = 0;
    }

    // Convert voltage to 8-bit DAC value
    return (uint8_t)((adjusted_voltage * 255) / 3.3);
}

void app_main(void)
{
    // ADC1 Init
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    // ADC1 Calibration Init
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    adc_cali_handle_t adc1_cali_chan1_handle = NULL;
    bool do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
    bool do_calibration1_chan1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);

#if EXAMPLE_USE_ADC2
    // ADC2 Init
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    // ADC2 Calibration Init
    adc_cali_handle_t adc2_cali_handle = NULL;
    bool do_calibration2 = example_adc_calibration_init(ADC_UNIT_2, EXAMPLE_ADC2_CHAN0, EXAMPLE_ADC_ATTEN, &adc2_cali_handle);

    // ADC2 Config
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, EXAMPLE_ADC2_CHAN0, &config));
#endif  // #if EXAMPLE_USE_ADC2

    // DAC Init
    dac_oneshot_handle_t dac_handle_0, dac_handle_1;
    dac_oneshot_config_t dac_config = {
        .chan_id = DAC_CHAN_0,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_config, &dac_handle_0)); // DAC Channel 1 (GPIO25)
    dac_config.chan_id = DAC_CHAN_1;
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_config, &dac_handle_1)); // DAC Channel 2 (GPIO26)

    while (1) {
        // uint64_t start_time = esp_timer_get_time(); // Start time of the loop

        // uint64_t read_start_time = esp_timer_get_time(); // Start time before reading ADC1 Channel 0
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
        // uint64_t read_end_time = esp_timer_get_time(); // End time after reading ADC1 Channel 0
        // ESP_LOGI(TAG, "ADC1 Channel 0 Read Time: %llu us", read_end_time - read_start_time);

        if (do_calibration1_chan0) {
            // uint64_t cali_start_time = esp_timer_get_time(); // Start time before calibration
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0][0], &voltage[0][0]));
            // uint64_t cali_end_time = esp_timer_get_time(); // End time after calibration
            // ESP_LOGI(TAG, "Calibration Time: %llu us", cali_end_time - cali_start_time);

            // Output to DAC Channel 1
            // uint8_t dac_value1 = (uint8_t)((voltage[0][0] * 255) / 3300); // Convert mV to 8-bit DAC value
            // uint64_t print_start = esp_timer_get_time(); // Start time before calibration
            // ESP_LOGI(TAG, "Time: %llu us, ADC%d Channel[%d] Voltage: %d mV, DAC input 1: %d", cali_end_time, ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, voltage[0][0], dac_value1);
            // uint64_t print_end = esp_timer_get_time(); // End time after calibration
            
            float OFFSET = 0.22;
            float MULTIPLICATIVE_FACTOR = -4;
            uint8_t dac_value1 = process_voltage_custom(voltage[0][0], OFFSET, MULTIPLICATIVE_FACTOR);

            // ESP_LOGI(TAG, "Print Time: %llu us", print_end - print_start);
            ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac_handle_0, dac_value1));
        }

        // uint64_t read_start_time2 = esp_timer_get_time(); // Start time before reading ADC1 Channel 1
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[0][1]));
        // uint64_t read_end_time2 = esp_timer_get_time(); // End time after reading ADC1 Channel 1
        // ESP_LOGI(TAG, "ADC1 Channel 1 Read Time: %llu us", read_end_time2 - read_start_time2);

        if (do_calibration1_chan1) {
            // uint64_t cali_start_time2 = esp_timer_get_time(); // Start time before calibration
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan1_handle, adc_raw[0][1], &voltage[0][1]));
            // uint64_t cali_end_time2 = esp_timer_get_time(); // End time after calibration
            // ESP_LOGI(TAG, "Calibration Time: %llu us", cali_end_time2 - cali_start_time2);

            // uint64_t print_start = esp_timer_get_time(); // Start time before calibration
            // ESP_LOGI(TAG, "Time: %llu us, ADC%d Channel[%d] Cali Voltage: %d mV, DAC input 2: %d"", cali_end_time2, ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN1, voltage[0][1], dac_value2);
            // uint64_t print_end = esp_timer_get_time(); // End time after calibration
            // ESP_LOGI(TAG, "Print Time: %llu us", print_end - print_start);

            // // Output to DAC Channel 2
            // uint8_t dac_value2 = (uint8_t)((voltage[0][1] * 255) / 3300); // Convert mV to 8-bit DAC value
            // ESP_ERROR_CHECK(dac_oneshot_output_voltage(dac_handle_1, dac_value2));
        }

        #if EXAMPLE_USE_ADC2
        // uint64_t read_start_time3 = esp_timer_get_time(); // Start time before reading ADC2 Channel 0
        ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, EXAMPLE_ADC2_CHAN0, &adc_raw[1][0]));
        // uint64_t read_end_time3 = esp_timer_get_time(); // End time after reading ADC2 Channel 0
        // ESP_LOGI(TAG, "ADC2 Channel 0 Read Time: %llu us", read_end_time3 - read_start_time3);

        if (do_calibration2) {
            // uint64_t cali_start_time3 = esp_timer_get_time(); // Start time before calibration
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc2_cali_handle, adc_raw[1][0], &voltage[1][0]));
            // uint64_t cali_end_time3 = esp_timer_get_time(); // End time after calibration
            // ESP_LOGI(TAG, "Calibration Time: %llu us", cali_end_time3 - cali_start_time3);

            // uint64_t print_start = esp_timer_get_time(); // Start time before calibration
            // ESP_LOGI(TAG, "Time: %llu us, ADC%d Channel[%d] Cali Voltage: %d mV", cali_end_time3, ADC_UNIT_2 + 1, EXAMPLE_ADC2_CHAN0, voltage[1][0]);
            // uint64_t print_end = esp_timer_get_time(); // End time after calibration
            // ESP_LOGI(TAG, "Print Time: %llu us", print_end - print_start);
        }
        #endif  // #if EXAMPLE_USE_ADC2

        // uint64_t end_time = esp_timer_get_time(); // End time of the loop
        // ESP_LOGI(TAG, "Loop Time: %llu us", end_time - start_time);

        // vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }

    // Deinit
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    ESP_ERROR_CHECK(dac_oneshot_del_channel(dac_handle_0));
    ESP_ERROR_CHECK(dac_oneshot_del_channel(dac_handle_1));
    example_adc_calibration_deinit(adc1_cali_chan0_handle);
    example_adc_calibration_deinit(adc1_cali_chan1_handle);
#if EXAMPLE_USE_ADC2
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc2_handle));
    example_adc_calibration_deinit(adc2_cali_handle);
#endif  // #if EXAMPLE_USE_ADC2
}

