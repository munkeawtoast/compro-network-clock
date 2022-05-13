# compro-network-clock

![Poster ขนาด A1](./PJcomputerprogramming.png "Network Clock Demonstration")

## Documentation

https://docs.google.com/document/d/1G2B9BMHmOz2hFHDoLlv_7UWTqBriXel-5by09ApE7SI/edit?usp=sharing

## Detail

โปรเจคนี้ เราได้ทำตัวงานเป็น นาฬิกา ที่เขียนโค้ดลงในบอร์ด ESP-32 ซึ่งแบ่งออกเป็นสองส่วน คือส่วนที่ส่งข้อมูลออกทำหน้าที่เหมือนรีัโมท 
และอีกส่วนคือ ตัวนาฬิกาตัวรับ ที่จะรับเวลาจากตัวรีโมทมาแสดง 
การนำไปใช้เช่น การตั้งนาฬิกาหลายๆตัวในครัวเรือนหรือ บริษัทให้มีความตรงกันมากที่สุด ตั้งค่าได้พร้อมๆกัน
ตัวงานนี้ สามารถเขียนโค้ดเพิ่มเติมได้ หากอยากเพิ่มฟังก์ชั่นเสริมเข้าไปตามต้องการ

<details>
  <summary>Code ฝั่งส่ง</summary>
  
  import library { PubSubClient }
  ```cpp
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



// isLeapYear(1970 + year);


// int hour = (epochTime/(60*60*24*)) % 24;
// Serial.print("Epoch Time: ");
// Serial.println(epochTime);
// Serial.print(" M");
// Serial.print((int)((epochTime/(24*60*60))-));
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
  ```
</details>

<details>

  <summary>Code ฝั่งรับ</summary>
  
  import library { LiquidCrystal I2C, PubSubClient }
  ```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Update these with values suitable for your network.
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.mqttdashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  int i=0;

  char line1[17] = "";
  char line2[17] = "";

  while (i<length) {
	char curChar = (char)payload[i++];
	if (curChar == '_') {
	  msg += " ";
	} else {
	  msg += curChar;
	}

	if (i == 16) {
	  msg.toCharArray(line1, 17);
	  msg = "";
	}
  }
  msg.toCharArray(line2, 17);
  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}

void reconnect() {
  // Loop until were reconnected
  while (!client.connected()) {
	Serial.print("Attempting MQTT connection...");
	// Create a random client ID
	String clientId = "ESP8266Client-145212";
	clientId += String(random(0xffff), HEX);
	// Attempt to connect
	if (client.connect(clientId.c_str())) {
	  Serial.println("connected");

	  client.subscribe("testtopic/45");
	} else {
	  Serial.print("failed, rc=");
	  Serial.print(client.state());
	  Serial.println(" try again in 5 seconds");
	  // Wait 5 seconds before retrying
	  delay(5000);
	}
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  lcd.init();
  lcd.backlight();
}

void loop() {

  if (!client.connected()) {
	reconnect();
  }
  client.loop();

  // long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   ++value;
  //   snprintf (msg, 75, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish("testtopic/45", msg);
  //   ledcWrite(1, 10);
  // }
}
  ```
</details>

## Demonstration Video

https://www.youtube.com/watch?v=WprAIt0EywY

	
## สมาชิก

- 64070203 นาย ภัทรชัย เทิบจันทึก
- 64070214 นางสาว เมธานุช บุญไทย 
- 64070222 นาย เลิศนริทธิ์ เทอดสถีรศักดิ์
- 64070223 นาย วงศพัทธ์ พันธุประภาส
