#include "esp_wifi.h"
#include <stdio.h>


#define DallasThermometers 15
#define Dht11Pin1 18   
#define Dht11Pin2 5 
#define CapacitiveSensor 32
#define HeatingLamp 14
#define HeatingMat 33
#define HeatingCable 27
#define Pump 12
#define PWMPin 13

const char* ssid = "POCO X3 Pro" ;
const char* pass ="wisniowaswieczka";
wifi_mode_t mode = WIFI_MODE_STA;

enum DS18X20_FAMILY_DS18B20;


void app_main() {
    wifi_init_config_t configuration = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err_init = esp_wifi_init(&configuration);
    esp_err_t err_mode = esp_wifi_set_mode(mode);
    wifi_config_t WIFI_DATA={
    .sta = {
        .ssid = ssid,
        .password = pass,
    },
};
    esp_err_t err_conf=esp_wifi_set_config(mode,&WIFI_DATA);
    esp_err_t err_start=esp_wifi_start();
    esp_err_t err_connect = esp_wifi_connect();
   int counter; 
    while (err_connect!=ESP_OK && counter<30)
    {
        printf("Waiting for Wifi...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        counter+=1;
    }
    if (err_connect==ESP_OK)
    {
        printf("Wifi connection established");
    }
    else{
        printf("Wifi connection failed");
    }

    size_t numberFound;
    size_t addr_count;
    uint64_t onewire_addr_t ;
    onewire_addr_t addr_list[addr_count];
    ds18x20_scan_devices(DallasThermometers, &addr_list, addr_count, numberFound);
        for(int i=0; i<=addr_count;i++)
    {   
        printf('%llx',addr_list[i]);

    }
    for(;;)
    {   
        //esp_err_t err_meas=ds18x20_measure(DallasThermometers); 

    }

}

float ReadingTemperature1()
{
float temperature;
        if (ds18b20_read_temperature(&temperature)) {
            printf("Temperatura: %.2f°C\n", temperature);
        } else {
            printf("Błąd odczytu temperatury\n");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Odczyt co 2 sekundy
}

