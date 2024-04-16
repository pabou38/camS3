/**
 * @file server_ap.cpp
 * @author Forairaaaaa
 * @brief 
 * @version 0.1
 * @date 2023-12-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "servers.h"
#include <mooncake.h>
#include <Arduino.h>
#include "hal/hal.h"

#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFSEditor.h>

#include "apis/camera/api_cam.h"
#include "apis/system/api_system.h"
#include "apis/mic/api_mic.h"
#include "apis/poster/api_poster.h"
#include "apis/shooter/api_shooter.h"


static AsyncWebServer* _server = nullptr;


/*
PABOU: changes here


creates AP
starts web server on AP  (192.168.4.1)
wait 30sec for client to connect
 if client connects, wait until disconnect (ie client can update config)
put wifi in STA mode
connects to wifi in STA model (ssid, passwd from config)
print IP, MAC
blink led
set led on
*/
void UserDemoServers::start_ap_server_pabou()
{
    delay(200);
    Serial.begin(115200);
    Serial.printf("start ap server\n");

    spdlog::info("PABOU: start modified AP server");

    // Create web server on a given port
    _server = new AsyncWebServer(80);
    spdlog::info("PABOU: creates web server");


    // PABOU. added. get updated config ?
    auto config = HAL::hal::GetHal()->getConfig();

    // create AP to enable wifi config (otherwize hardcode, store in SD ..)

    spdlog::info("PABOU: starts AP to enable wifi config. proceed to STA mode if no connection within 30sec");
    // Start open ap 
    // uint64_t chipid = ESP.getEfuseMac();
    // String ap_ssid  = "UNIT-CAM-S3-" + String((uint32_t)(chipid >> 32), HEX);
    String ap_ssid = "UnitCamS3-WiFi-PABOU";
    // String ap_pass = "12345678";

    IPAddress IP;

    // WiFi.softAP(ap_ssid);
    WiFi.softAP(ap_ssid, emptyString, 1, 0, 1, false);
    // WiFi.softAP(ap_ssid, ap_pass);

    IP = WiFi.softAPIP();
    spdlog::info("PABOU: softAP IP address {}" , IP.toString().c_str());
    //WiFi.softAPmacAddress(mac);

    // PABOU. add code to start server, ie load URL, LittleFS, and beging

    spdlog::info("PABOU: start web server. load API (ie all URL end points, cam, system, mic, poster, shooter)");
    // Load apis 
    load_cam_apis(*_server);
    load_system_apis(*_server);
    load_mic_apis(*_server);
    load_poster_apis(*_server);
    load_shooter_apis(*_server);

    // Set file system support 
    _server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    _server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/404.html");
    });


    // Start web server
    spdlog::info("PABOU: start web server on AP");
    _server->begin();


    // PABOU: this code is leveraged from void UserDemoServers::start_poster_server()
    // wait n sec for someone to connect to the AP. if a client connect, wait for  it to disconnect.
    //   while connected, client can update wifi config, using image poster setup. 
    // after client disconnect from AP, or no client for 30 sec, connect to wifi in STA mode

    uint32_t ap_wait_time_cout = millis();
    bool led_state = true;
    while (millis() - ap_wait_time_cout < 30000) // wait 30 sec
        {
        // If get client 
        if (WiFi.softAPgetStationNum() != 0)
        {
            // Hold Ap mode 
            spdlog::info("PÄBOU: got client while in AP mode, hold ap mode");
            HAL::hal::GetHal()->setLed(true);

            // web server is active. can be used to configure things
            while (WiFi.softAPgetStationNum() != 0)
                delay(2000);

            // I guess this is because the client disconnected ?
            spdlog::info("PÄBOU: no more client while in AP mode");
            

            // If no config setting, reset counting
            ap_wait_time_cout = millis();
        }

    delay(500);
    // pulse led
    led_state = !led_state;
    HAL::hal::GetHal()->setLed(led_state);

    } // while 

    spdlog::info("PABOU: no client connected to AP. proceed to STA mode");


    // has config been updated ? yes
    spdlog::info("PABOU: ssid {}" , config.wifi_ssid.c_str());
    spdlog::info("PABOU: passwd {}" , config.wifi_password.c_str());


    // PABOU: change from AP to STA to allow access from any wifi devices already connected to home network (incl NAT)
    // vs having to connect to AP


    // with standard firmware, the wifi config is set when running on "normal" AP mode, when configuring poster or timelapse mode
    //spdlog::info("try to connect to:\n{}\n{}", config.wifi_ssid.c_str(), config.wifi_password.c_str());
    // WARNING: serial.print does not work on putty, or on platformio serial monitor
    
    // seem this is sufficient to move wifi from AP to STA
    WiFi.mode(WIFI_STA);

    // use wifi as configured
    WiFi.begin(config.wifi_ssid, config.wifi_password);

    //WiFi.begin("Livebox-deec", "xxxx");

    spdlog::info("PABOU: connecting to home network (ie camera is an endpoint)");

    int pabou_i;

    bool is_connected = false;
        while (1) {
            if (WiFi.status() == WL_CONNECTED)
                {
                    is_connected = true;
                    break;
                }
            delay(1000);

            pabou_i = pabou_i + 1;
            if (pabou_i > 20) {
                spdlog::error("PABOU: wifi STA connect failed, restart..");
                    // Reboot 
                    delay(300);
                    esp_restart();
                    delay(10000);
            }
        }
        // breaked, so connected

    // if we got here, it means wifi is connected (otherwize, reset)

    IP = WiFi.localIP();

    // serial print does not show up in putty
    spdlog::info("PABOU: connected to home's network, ip: {}", IP.toString().c_str());


    // can I used the existing _server ? yes it seems


    // print MAC, to configure fixed IP from router
    byte mac[6];
    WiFi.macAddress(mac);  // returns byte array

    spdlog::info("PABOU: MAC address {0:x}:{1:x}:{2:x}:{3:x}:{4:x}:{5:x}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //[01:36] [I] get mac: ECDA3B4FA784
    // PABOU MAC ec:da:3b:4f:a7:84

    spdlog::info("PABOU: blink led to signal we are ready");

    for (int i = 0; i <= 20; i++) {
        HAL::hal::GetHal()->setLed(true);
        delay(200);
        HAL::hal::GetHal()->setLed(false);
        delay(200);

    }


    // Light up 
    spdlog::info("PABOU: set led on. AP server running");
    HAL::hal::GetHal()->setLed(true);

    spdlog::info("PABOU: to get a realtime stream, please connect to: {}/api/v1/stream", WiFi.localIP().toString().c_str());

}


void UserDemoServers::stop_ap_server()
{
    spdlog::info("stop ap server");

    delete _server;
    delay(200);

    WiFi.softAPdisconnect();
    WiFi.disconnect();
}


// original code
void UserDemoServers::start_ap_server()
{
    delay(200);
    Serial.begin(115200);
    Serial.printf("start ap server\n");


    // Create web server 
    _server = new AsyncWebServer(80);


    // Start open ap 
    // uint64_t chipid = ESP.getEfuseMac();
    // String ap_ssid  = "UNIT-CAM-S3-" + String((uint32_t)(chipid >> 32), HEX);
    String ap_ssid = "UnitCamS3-WiFi";
    // String ap_pass = "12345678";

    // WiFi.softAP(ap_ssid);
    WiFi.softAP(ap_ssid, emptyString, 1, 0, 1, false);
    // WiFi.softAP(ap_ssid, ap_pass);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);


    // Load apis 
    load_cam_apis(*_server);
    load_system_apis(*_server);
    load_mic_apis(*_server);
    load_poster_apis(*_server);
    load_shooter_apis(*_server);

    // Set file system support 
    _server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    _server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/404.html");
    });


    // Start server
    _server->begin();

    // Light up 
    HAL::hal::GetHal()->setLed(true);
}