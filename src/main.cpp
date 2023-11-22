// Including the necessary library files
#include "HelperFunctions.h"
#include "Config.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/16);
GxEPD_Class display(io, /*RST=*/16, /*BUSY=*/4);

unsigned long timeTokenRecieved = 0;

struct LastChecked {
  unsigned long int lastChecked;
  unsigned long int updateInterval;
  bool firstRun;
};

LastChecked realtimeUpdate = {
  .lastChecked = 0,
  .updateInterval = 300000,// 5 minutes
  .firstRun = false
};

LastChecked standardUpdate = {
  .lastChecked = 0,
  .updateInterval = 1440000,// 1 day
  .firstRun = false
};

DynamicJsonDocument doc(2048);

bool changeMade = false;

WiFiClientSecure *client = new WiFiClientSecure;
HTTPClient https;

struct AccessToken {
  String access_token;
  int expires_in;
  bool exists;
};

AccessToken t = {
  .access_token = "",
  .expires_in = 0,
  .exists = false
};

// take in the DynamicJsonDocument ref
int getValueFromJsonDoc(DynamicJsonDocument * d, String key){
  for(int i = 0; i < (*d)["rows"].size(); i++) {
    int value = (*d)["rows"][i]["metricValues"][0]["value"].as<int>();
    for(int j = 0; j < (*d)["rows"][i]["dimensionValues"].size(); j++){
      String name = (*d)["rows"][i]["dimensionValues"][j]["value"].as<String>();
      if(name.equals(key)) {
        return value;
      }
    }
  }

  return -1;
}

void getAccessToken(){
  if(https.begin(*client, googleapis_refresh_endpoint)) {

    char body[1024];
    sprintf(body, R"({
        "client_id": "%s",
        "client_secret": "%s",
        "refresh_token": "%s",
        "grant_type": "refresh_token"
    })", CLIENT_ID, CLIENT_SECRET, GOOGLE_REFRESH_TOKEN);

    if(DEBUG)
      Serial.println("Requesting for access token");

    https.addHeader("Content-Type", "application/json");
    https.addHeader("Accept", "*/*");
    https.addHeader("Content-Length", String(strlen(body)));

    int httpCode = https.POST(body);

    if(httpCode == 200){
      if(DEBUG)
        Serial.println("Access token received");
        
      deserializeJson(doc, https.getString());

      t.access_token = doc["access_token"].as<String>();
      t.expires_in = doc["expires_in"].as<int>();
      t.exists = true;

      doc.clear();

      timeTokenRecieved = millis() / 1000;

      if(DEBUG)
        Serial.printf("Access Token: %s, Expires in: %d\n", t.access_token.c_str(), t.expires_in);
    } else {
      if(DEBUG)
        Serial.printf("Error receiving access token: %d\n", httpCode);
      return;
    }

    https.end();
  }
}

void getRealtimeData(int x, int y) {
  // checking to see if its time to update
  if((millis() - realtimeUpdate.lastChecked >= realtimeUpdate.updateInterval) || !realtimeUpdate.firstRun) {
      
    HTTPClient https;

    char url[256];

    sprintf(url, "https://content-analyticsdata.googleapis.com/v1beta/properties/%s:runRealtimeReport", PROPERTY_ID);

    if(https.begin(*client, url)) {
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Accept", "*/*");
      https.addHeader("Authorization", "Bearer " + t.access_token);

      char body[1024] = R"({
          "dimensions": [
              {
                  "name": "eventName"
              }
          ],
          "dimensionFilter": {
              "filter": {
                  "fieldName": "eventName",
                  "stringFilter": {
                      "value": "Loaded_Tinder"
                  }
              }
          },
          "metrics": [
              {
                  "name": "eventCount"
              }
          ],
          "minuteRanges": [
              {
                  "name": "lastFiveMinutes",
                  "startMinutesAgo": 5,
                  "endMinutesAgo": 0
              },
              {
                  "name": "lastThirtyMinutes",
                  "startMinutesAgo": 29
              }
          ]
      })";

      int httpCode = https.POST(body);

      if(httpCode == 200) {
        deserializeJson(doc, https.getString());

        int last30Mins = getValueFromJsonDoc(&doc, "lastThirtyMinutes");
        int last5Mins = getValueFromJsonDoc(&doc, "lastFiveMinutes");

        if(DEBUG) 
          Serial.printf("Last 30 mins: %d, Last 5 mins: %d\n", last30Mins, last5Mins);

        display.fillRect(x, y, 60, 128-y, GxEPD_WHITE);
        display.setFont(&FreeMonoBold12pt7b);


        // 30 mins box
				display.drawRoundRect(x, y, 80, 52, 10, GxEPD_BLACK);
				display.drawBitmap(ClockIcon, x + 5, y + 5, 24, 24, GxEPD_BLACK);
				display.setCursor(x+33, y+22);
				display.println("30m");
        String thirtyMinsStr = String(last30Mins);
        int spaceToRight = 55 - (thirtyMinsStr.length() * 12);
        display.setCursor(x + spaceToRight, y+47);
				display.println(thirtyMinsStr);


       // 5 mins box
        display.drawRoundRect(x, y + 55, 80, 45, 10, GxEPD_BLACK);
				display.drawBitmap(ClockIcon, x + 5, y + 65, 24, 24, GxEPD_BLACK);
				display.setCursor(x+36, y+75);
				display.println("5m");
        String fiveMinsStr = String(last5Mins);
        spaceToRight = 58 - (fiveMinsStr.length() * 12);
        display.setCursor(x + spaceToRight, y+41+55);
				display.println(fiveMinsStr);

        changeMade = true;
      } else {
        if(DEBUG)
          Serial.printf("Error receiving response, HTTP Code:%d, Content: %s\n", httpCode, https.getString().c_str());
      }
    }
    https.end();
    realtimeUpdate.lastChecked = millis();
    realtimeUpdate.firstRun = true;
  }
}

void getStandardData(int x, int y){
  // checking to see if its time to update
  if((millis() - standardUpdate.lastChecked >= standardUpdate.updateInterval) || !standardUpdate.firstRun) {
     HTTPClient https;

    char url[256];

    sprintf(url, "https://content-analyticsdata.googleapis.com/v1beta/properties/%s:runReport", PROPERTY_ID);

    if(https.begin(*client, url)) {
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Accept", "*/*");
      https.addHeader("Authorization", "Bearer " + t.access_token);

      char body[1250] = R"({
        "dateRanges": [
          {
            "startDate": "7daysAgo",
            "endDate": "yesterday"
          }
        ],
        "dimensions": [
          {
              "name": "eventName"
          }
        ],
        "metrics": [
          {
              "name": "totalUsers"
          }
        ],
        "dimensionFilter": {
          "orGroup": {
            "expressions": [
              {
                "filter": {
                  "fieldName": "eventName",
                  "stringFilter": {
                    "value": "install"
                  }
                }
              },
              {
                "filter": {
                  "fieldName": "eventName",
                  "stringFilter": {
                    "value": "uninstall"
                  }
                }
              },
              {
                "filter": {
                    "fieldName": "eventName",
                    "stringFilter": {
                        "value": "Loaded_Tinder"
                    }
                }
              }
            ]
          }
        }
      })";

      int httpCode = https.POST(body);

      if(httpCode == 200) {
        deserializeJson(doc, https.getString());

        int installCount = getValueFromJsonDoc(&doc, "install");
        int uninstallCount = getValueFromJsonDoc(&doc, "uninstall");
        int activeUsers = getValueFromJsonDoc(&doc, "Loaded_Tinder");


        if(DEBUG) Serial.printf("Install Count: %d, Uninstall Count: %d, Active Users: %d\n", installCount, uninstallCount, activeUsers);
        display.setFont(&FreeMonoBold12pt7b);


        // Users Box
        display.fillRect(x, y, 125, 62, GxEPD_WHITE);
				display.drawRoundRect(x, y, 125, 62, 10, GxEPD_BLACK);
				display.drawBitmap(PeopleIcon, x + 5, y + 5, 49, 38, GxEPD_BLACK);
				display.setCursor(x+50, y+30);
				display.println("Users");
        String activeUsersStr = String(activeUsers);
        display.setCursor(x + 20, y+58);
				display.println(activeUsersStr);

        changeMade = true;
      } else {
        if(DEBUG)
          Serial.printf("Error receiving response, HTTP Code:%d, Content: %s\n", httpCode, https.getString().c_str());
      }
    }
    https.end();
    standardUpdate.lastChecked = millis();
    standardUpdate.firstRun = true;
  }
}

void setup()
{
  // Initialize the display
  display.init(115200);
  display.setRotation(1);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextSize(1);
  display.setTextWrap(true);
  showLines("Loading...", 1, 10, &display);
  display.update();

  if(DEBUG) 
    Serial.begin(115200);
  
  if(DEBUG)
    Serial.printf("Connecting to wifi... %s\n", SSID);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  // Wait for the connection to establish
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    if(DEBUG) 
      Serial.println("Connection Failed! Restarting...");
    ESP.restart();
    
  }

  if (DEBUG)
    Serial.printf("\nConnected To: %s, IP: %s\n", SSID, WiFi.localIP().toString().c_str());

  // Set the SSL certificates for the client
  client->setCACert(googleapis_root_ca);
  display.fillScreen(GxEPD_WHITE);// clearing the screen for the next update
}

void loop() {
  if (client) {
    // Calling the function to get the access token
    if (!t.exists || (t.expires_in + timeTokenRecieved) <= (millis() / 1000)) {
      getAccessToken();
    }

    // Calling the function to get Google Analytics data
    getRealtimeData(0, 22);
    getStandardData(82, 22);
    if(changeMade) {
      // display.drawExampleBitmap(LighterfuelLogo, 167, 45, 80, 80, GxEPD_BLACK);
      display.setFont(&FreeMonoBold12pt7b);
      display.setCursor(0, 16);
      display.println("LighterFuel Stats");
      display.update();

      if(DEBUG)
        Serial.println("Updating the display");
        
      changeMade = false;
    }

  }
}
