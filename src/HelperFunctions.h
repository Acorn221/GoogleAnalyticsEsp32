#include <Arduino.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <GxEPD.h>
#include <GxGDE0213B72B/GxGDE0213B72B.h> // 2.13" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <ArduinoJson.h>
#include "Config.h"

#ifndef CONST_H
    #define CONST_H

    #define MAX_LENGTH_PER_LINE 22 // this is the maximum amount of text characters per line of the screen

    #define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds

    extern const char *googleapis_root_ca;
    extern const char *googleapis_refresh_endpoint;
    extern const unsigned char LighterfuelLogo[];
    extern const unsigned char ClockIcon[];
    extern const unsigned char PeopleIcon[];

    void showLines(String text, int maxLines, int y, GxEPD_Class* display);
    void setDisplayNotListening(GxEPD_Class * display);
    void drawRoundedLines(int distanceBetween, int width, int lengths[], int count, int x, int y, GxEPD_Class* display);
    void drawRoundedLine(int height, int width, int x, int y, GxEPD_Class* display);

#endif
