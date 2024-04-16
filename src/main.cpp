/**
 * @file main.cpp
 * @author Forairaaaaa
 * @brief 
 * @version 0.1
 * @date 2023-10-31
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <mooncake.h>
#include <Arduino.h>
#include "hal/hal.h"
#include "servers/servers.h"


// PABOU: add simple blink to see which app is started
void blink(int n, int dur) {

    int _led_state = 0;

    for (int j=0;j<n;j++) {
    
    HAL::hal::GetHal()->setLed(_led_state);
    delay(dur);
    _led_state = !_led_state;

    HAL::hal::GetHal()->setLed(_led_state);
    delay(dur);
    _led_state = !_led_state;

    }

    HAL::hal::GetHal()->setLed(0);
}



void setup() 
{
    // Init 

    // PABOU
    // sleep to catch trace on putty

    delay(7000);

    // PABOU: get config from LittleFS json file or init json file with default values
    // depending on HAL::hal::GetHal()->getConfig() starts either poster, shooter,  or ap

    HAL::hal::GetHal()->init();

    // use GPIO to switch for stock firmware to modfied (streaming) firmware
    // pullup. default (unconnected) = 1, streaming
    // connect GPIO to GND , stock 
    int gpio_config_pabou = 44;

    /*
    EN          G44 (RXd0)
    G0          G43 (Txd0)
    GN          5V
    */

    pinMode(gpio_config_pabou, INPUT_PULLUP); 
    int val = digitalRead(gpio_config_pabou);

    //int val = 1;

    spdlog::info("PABOU: main.cpp: read GPIO {} (pullup) to switch between default firmware and modified streaming firmware", gpio_config_pabou);
    spdlog::info("PABOU: main.cpp: value {}. (default = unconnected = 1 = modified)", val);

    if (val)  {

        // ap_server will first start a web server on a wifi in AP mode (to enable wifi config)
        // then connect to home's network as client (using the above wifi credential)
        // The camera will get an IP from the home router, and this IP is used to access the camera's real time feed
        // This IP can be fixed, and NAT'ed to access the camera from outside the home network

        // code changes are in servers/server_ap.cpp

        spdlog::info("PABOU: main.cpp: starts modified AP server for streaming");
        
        blink(5,200);

        UserDemoServers::start_ap_server_pabou();

    }

    else {

        spdlog::info("PABOU: main.cpp: starts standard application");
        blink(5,600);


    // original logic here
  
    // Start server 
    if (HAL::hal::GetHal()->getConfig().start_poster == "yes")
        UserDemoServers::start_poster_server();
    else if (HAL::hal::GetHal()->getConfig().start_shooter == "yes")
        UserDemoServers::start_shooter_server();
    else 
        UserDemoServers::start_ap_server();

    }

}

/*
poster mode

[00:00] [I] hal init
[00:10] [I] no client, go on poster mode
[00:10] [I] stop ap server
[00:10] [I] try conncet:
Livebox-deec
6A107302513EE179674312638E
E (11186) ledc: ledc_get_duty(740): LEDC is not initialized
[00:12] [I] connected
[00:12] [I] connect ok, ip: 192.168.1.47
[00:12] [I] try init sd card..
[00:12] [I] cs:9 miso:40 mosi:38 clk:39
[00:12] [I] sd card init ok
[00:12] [I] size: 3G
[01:36] [I] get image num: 615
[01:36] [I] done
[01:36] [I] get mac: ECDA3B4FA784
[01:36] [I] start posting..
[01:36] [I] try save image at /captured/img_615.jpeg ..
[01:38] [I] done

20
{"code":200,"msg":"OK","data":1}
0

[01:43] [I] done, wait next..

*/


/*
modified PABOU. delay() 
[00:10] [I] hal init
[00:10] [I] cam init
[00:10] [I] cam init ok
[00:10] [I] PABOU: config init, start LittleFS
[00:10] [I] config init
[00:10] [I] PABOU: check if config file exist
[00:10] [I] /config.json already exsit
[00:10] [I] PABOU: getting config from: /config.json
[00:10] [I] try load config from sd..
[00:10] [I] try init sd card..
[00:10] [I] cs:9 miso:40 mosi:38 clk:39
[00:10] [I] sd card init ok
[00:10] [I] size: 3G
[00:10] [I] pass image path init, done
[00:10] [I] /config.json not exist
[00:10] [I] PABOU: print config
[00:10] [I] config:
wifi ssid:
wifi pass:
nickname: UnitCamS3
post interval: 5
time zone: GMT+0
wait ap first: no
start poster: no
start interval shooter: no
[00:10] [I] PABOU: main.cpp: always start AP server.
[00:10] [I] PABOU: (spdlog). ALWAYS start AP server (using wifi STA
[00:10] [I] PABOU: creates web server
[00:10] [I] PABOU: do not create softAP, but rather STA. client will connect to home network, and not on AP
[00:10] [I] PABOU: connecting to home network (STA mode)
[00:10] [I] PABOU: wait for wifi connection
[00:11] [I] PABOU: STA connected
[00:11] [I] PABOU: STA connect ok, ip: 192.168.1.47
[00:11] [I] PABOU: load API (ie URL end points)
[00:11] [I] PABOU: start web server (wifi in STA mode)
[00:11] [I] PABOU: set led on

*/


void loop() 
{
    delay(5000);
}
