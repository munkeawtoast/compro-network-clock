#include <WiFi.h>
#include "time.h"
#include <PubSubClient.h>


// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.mqttdashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

// NTP server to request epoch time
const char* ntpServer = "0.pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime = 0;

int addedTime = 0;

char weekDay[9][9] = {"Thu.", "Fri.", "Sat.", "Sun.", "Mon.", "Tue.", "Wed."};

int isLeapYear(int year) {
  if (year % 400 == 0) {
    return 1;
  }
  // not a leap year if divisible by 100
  // but not divisible by 400
  else if (year % 100 == 0) {
    return 0;
  }
  // leap year if not divisible by 100
  // but divisible by 4
  else if (year % 4 == 0) {
    return 1;
  }
  return 0;
}

int getLeapYearExtraDays() {
  int dayCount = 0;
  for (int i = 1970; i < 1970 + (int)(epochTime/(365*24*60*60)); i++) {
    if (isLeapYear(i)) {
      dayCount++;
    }
  }
  return dayCount;
}

int getMonthFromDays(int day, int leapYear) {
  int normalMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int leapMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  int sum = 0;

  for (int i = 0; i < 12; i++) {
    if (day < sum) {
      return i;
    }
    sum += (leapYear ? leapMonth[i] : normalMonth[i]);
  }
  return 12;
}

String textFormat(int value) {
  String text = "";
  if (value < 10) {
    text += "0";
  }
  text += value;
  return text;
}

int getDaysInMonthFromDays(int day, int leapYear) {
  int normalMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int leapMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  int sum = 0;

  for (int i = 0; i < 12; i++) {
    if (day < sum + (leapYear ? leapMonth[i+1] : normalMonth[i+1])) {
      return day-sum + 1;
    }
    sum += (leapYear ? leapMonth[i] : normalMonth[i]);
  }
  return 31;
}
// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("\nConnected to ");
  Serial.print(WiFi.localIP());
  Serial.println("!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-203214222223";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  configTime(0, 0, ntpServer);
  client.setServer(mqtt_server, 1883);
}

void loop() {
  epochTime = getTime();
  epochTime += 7 * 60 * 60;
  
  int sec = epochTime % 60;
  int minute = (epochTime/60) % 60;
  int hour = (epochTime/(60*60)) % 24;
  int year = epochTime/(365*24*60*60) + 1970;
  int daySinceEpoch = epochTime/(24*60*60);
  int dayInYear = ((epochTime/(60*60*24)) - getLeapYearExtraDays()) % 365;
  int month = getMonthFromDays(dayInYear, isLeapYear(year));
  int day = getDaysInMonthFromDays(dayInYear, isLeapYear(year));
  
  String output = "";
  output += weekDay[daySinceEpoch%7];
  output += "__";
  output += textFormat(day);
  output += "-";
  output += textFormat(month);
  output += "-";
  output += year;
  output += "________";
  output += textFormat(hour);
  output += ":";
  output += textFormat(minute);
  output += ":";
  output += textFormat(sec);
  char outputChar[33] = "";
  output.toCharArray(outputChar, 33);
  client.publish("testtopic/45", outputChar);
  Serial.println(outputChar);
  
  delay(1000);
  
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

