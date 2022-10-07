

#include <string.h>
#include <stdio.h>

#include <sx127x.h>
#include <ssd1306.h>


#define TAG __FILENAME__

ssd1306_t oled_config;

esp_err_t on_receive_cb(size_t pcklength)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGD(TAG, "intruppt recv len %d", (int)pcklength);

    uint8_t *recv = ESP_MALLOC(pcklength);
    ESP_NON_NULL_CHECK(recv);
    size_t recv_len = pcklength;
    ret = lora_sx127x_receive(recv, &(recv_len));
    ESP_ERROR_RETURN(ret != ESP_OK, ret, "failed to recv data over lora");

    ESP_LOGI(TAG, "lora recv data %.*s", (unsigned int)recv_len, (const char *)recv);

    return ret;
}

esp_err_t on_tx_done_cb(void)
{
    // static int i = 0;
    ESP_LOGI(TAG, "data transitted.");
    return ESP_OK;
}

void lora_tx_task(void *pvParamter)
{
    (void)pvParamter;

    esp_err_t ret = ESP_OK;
    const char *msg = "HELLO RECEIVER!";

    uint8_t data[50] = {0};
    while (1)
    {
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        static size_t i = 0;
        size_t len = snprintf((char *)data, 50, "%s %d", msg, i);
        if (len > 0)
        {
            int32_t rssi = lora_sx127x_get_rssi();
            size_t bw = lora_sx127x_get_signal_bandwidth();
            int32_t sf = lora_sx127x_get_spreading_factor();

            ESP_LOGI(TAG, "Sending data...");
            ESP_LOGI(TAG, "rssi %d\t bandwidth %u\t spreading factor %d\n", rssi, bw, sf);
            ssd1306_clear_screen(&oled_config, false);
            ssd1306_contrast(&oled_config, 0xff);
            ssd1306_display_text(&oled_config, 1, "Sending data...", strlen("Sending data..."), false);
            ssd1306_hardware_scroll(&oled_config, SCROLL_RIGHT);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            ret = lora_sx127x_send((uint8_t *)data, len);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to send data over lora.");
                i++;
            }
            else
            {
                ESP_LOGI(TAG, "data sent");
                ssd1306_clear_screen(&oled_config, false);
                ssd1306_contrast(&oled_config, 0xff);
                ssd1306_display_text(&oled_config, 1, "data sent", strlen("data sent"), false);
                ssd1306_hardware_scroll(&oled_config, SCROLL_RIGHT);
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    /* init ssd1306 driver */
    oled_config.interface_type = OLED_I2C;
    oled_config.display_type = OLED_DISPLAY_128x32;
    oled_config._flip = false;
    oled_config.interface.ssd1306_i2c.sda_pin = 4;
    oled_config.interface.ssd1306_i2c.scl_pin = 15;
    oled_config.interface.ssd1306_i2c.rst_pin = 16;

    ESP_ERROR_CHECK(ssd1306_init(&(oled_config)));

    char *msg = "LORA INIT...";
    ssd1306_display_text_x3(&(oled_config), 0, msg, strlen(msg), false);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    /* init lora driver */
    lora_config_t lora_conf = LORA_127x_CONFIG_DEFAULT();
    lora_conf.onRecvCb = on_receive_cb;
    lora_conf.onTxDoneCb = on_tx_done_cb;
    ESP_ERROR_CHECK(lora_sx127x_init(&(lora_conf)));

    ssd1306_clear_screen(&(oled_config), false);
    ssd1306_display_text_x3(&(oled_config), 0, "DONE", strlen("DONE"), false);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskCreate(lora_tx_task, "tx task", 1024 * 3, NULL, 5, NULL);

    return;
}



// #include <string.h>
// #include <stdio.h>

// #include <sx127x.h>
// #include <ssd1306.h>


// #define TAG __FILENAME__

// ssd1306_t oled_config;

// esp_err_t on_receive_cb(size_t pcklength)
// {
//     esp_err_t ret = ESP_OK;

//     ESP_LOGD(TAG, "intruppt recv len %d", (int)pcklength);

//     uint8_t *recv = ESP_MALLOC(pcklength);
//     ESP_NON_NULL_CHECK(recv);
//     size_t recv_len = pcklength;
//     ret = lora_sx127x_receive(recv, &(recv_len));
//     ESP_ERROR_RETURN(ret != ESP_OK, ret, "failed to recv data over lora");

//     ESP_LOGI(TAG, "lora recv data %.*s", (unsigned int)recv_len, (const char *)recv);

//     return ret;
// }

// esp_err_t on_tx_done_cb(void)
// {
//     // static int i = 0;
//     ESP_LOGI(TAG, "data transitted.");
//     return ESP_OK;
// }

// void lora_tx_task(void *pvParamter)
// {
//     (void)pvParamter;

//     esp_err_t ret = ESP_OK;
//     const char *msg = "HELLO RECEIVER!";

//     uint8_t data[50] = {0};
//     while (1)
//     {
//         vTaskDelay(3000 / portTICK_PERIOD_MS);

//         static size_t i = 0;
//         size_t len = snprintf((char *)data, 50, "%s %d", msg, i);
//         if (len > 0)
//         {
//             int32_t rssi = lora_sx127x_get_rssi();
//             size_t bw = lora_sx127x_get_signal_bandwidth();
//             int32_t sf = lora_sx127x_get_spreading_factor();

//             ESP_LOGI(TAG, "Sending data...");
//             ESP_LOGI(TAG, "rssi %d\t bandwidth %u\t spreading factor %d\n", rssi, bw, sf);
//             ssd1306_clear_screen(&oled_config, false);
//             ssd1306_contrast(&oled_config, 0xff);
//             ssd1306_display_text(&oled_config, 1, "Sending data...", strlen("Sending data..."), false);
//             ssd1306_hardware_scroll(&oled_config, SCROLL_RIGHT);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             ret = lora_sx127x_send((uint8_t *)data, len);
//             if (ret != ESP_OK)
//             {
//                 ESP_LOGE(TAG, "Failed to send data over lora.");
//                 i++;
//             }
//             else
//             {
//                 ESP_LOGI(TAG, "data sent");
//                 ssd1306_clear_screen(&oled_config, false);
//                 ssd1306_contrast(&oled_config, 0xff);
//                 ssd1306_display_text(&oled_config, 1, "data sent", strlen("data sent"), false);
//                 ssd1306_hardware_scroll(&oled_config, SCROLL_RIGHT);
//             }
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }

//     vTaskDelete(NULL);
// }

// void app_main(void)
// {
    /* init ssd1306 driver */
    // oled_config.interface_type = OLED_I2C;
    // oled_config.display_type = OLED_DISPLAY_128x32;
    // oled_config._flip = false;
    // oled_config.interface.ssd1306_i2c.sda_pin = 4;
    // oled_config.interface.ssd1306_i2c.scl_pin = 15;
    // oled_config.interface.ssd1306_i2c.rst_pin = 16;

    // ESP_ERROR_CHECK(ssd1306_init(&(oled_config)));

    // char *msg = "LORA INIT...";
    // ssd1306_display_text_x3(&(oled_config), 0, msg, strlen(msg), false);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // /* init lora driver */
    // lora_config_t lora_conf = LORA_127x_CONFIG_DEFAULT();
    // lora_conf.onRecvCb = on_receive_cb;
    // lora_conf.onTxDoneCb = on_tx_done_cb;
    // ESP_ERROR_CHECK(lora_sx127x_init(&(lora_conf)));

    // ssd1306_clear_screen(&(oled_config), false);
    // ssd1306_display_text_x3(&(oled_config), 0, "DONE", strlen("DONE"), false);
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // xTaskCreate(lora_tx_task, "tx task", 1024 * 3, NULL, 5, NULL);

//     return;
// }