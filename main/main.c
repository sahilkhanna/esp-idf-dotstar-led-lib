/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



/****************************************************************************
*
* This demo showcases BLE GATT client. It can scan BLE devices and connect to one device.
* Run the gatt_server demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. The two devices will then exchange
* data.
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"


#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_event_loop.h"

#include "../components/dotstar/include/dotstar.h"
#define LED_SDA 16
#define LED_CLK 14
#define TOTAL_LEDS 6

static uint32_t rainbowColors[180];

void setAllColor(uint8_t r,uint8_t g,uint8_t b){
    // uint32_t color = (r << 16) | (g << 8) | b ;
    for (uint8_t i = 0; i < 6; i++)
    {
        setPixelColor(i,r,g,b);
        // setPixel24bitColor(i, color);
    }
}


uint32_t h2rgb(uint32_t v1, uint32_t v2, uint32_t hue)
{
	if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
	if (hue < 180) return v2 * 60;
	if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
	return v1 * 60;
}

uint32_t makeColor(uint16_t hue, uint8_t saturation, uint8_t lightness)
{
	uint8_t red, green, blue;
	uint8_t var1, var2;

	if (hue > 359) hue = hue % 360;
	if (saturation > 100) saturation = 100;
	if (lightness > 100) lightness = 100;

	// algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
	if (saturation == 0) {
		red = green = blue = lightness * 255 / 100;
	} else {
		if (lightness < 50) {
			var2 = lightness * (100 + saturation);
		} else {
			var2 = ((lightness + saturation) * 100) - (saturation * lightness);
		}
		var1 = lightness * 200 - var2;
		red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
		green = h2rgb(var1, var2, hue) * 255 / 600000;
		blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
	}
	return (red << 16) | (green << 8) | blue;
}

void rainbow(int phaseShift, uint32_t wait)
{
  uint8_t color;
  for (color=0; color < 180; color++) {
    for (uint8_t pixel=0; pixel < TOTAL_LEDS; pixel++) {
        int index = (color + pixel*phaseShift/2) % 180;
        setPixel24bitColor(pixel, rainbowColors[index]);
    }
    printLED();
    vTaskDelay(wait/portTICK_RATE_MS);
  }
}

void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    for (uint8_t i=0; i<180; i++) {
        uint16_t hue = i * 2;
        uint8_t saturation = 100;
        uint8_t lightness = 40;
        // pre-compute the 180 rainbow colors
        rainbowColors[i] = makeColor(hue, saturation, lightness);
    }


    init_led(LED_SDA,LED_CLK,TOTAL_LEDS,DOTSTAR_RGB);
    printLED();
    while(true){
        rainbow(10, 10);
    }
}


