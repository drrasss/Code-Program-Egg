#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT22
#define FAN_PIN 12
#define RELAY_PIN 13
#define MOTOR_PIN 14
#define SG_PIN 17
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL647aEkQuW"
#define BLYNK_TEMPLATE_NAME "Temperature"
#define BLYNK_AUTH_TOKEN "rJ48qWtl4E5FAyIvqxAtx526IqOo3ZQ9"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

char ssid[] = "Xiaomi 14T";
char pass[] = "51942511";

int M = 0, F = 0;
DHT dht(DHTPIN, DHTTYPE);
bool isConnected = false;

unsigned long lastDHTReadTime = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastFanMotorCheck = 0;
const long dhtInterval = 1000;
const long wifiCheckInterval = 5000;
const long fanMotorInterval = 1000;

void setup() {
    dht.begin();
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("T & H START");
    delay(2000);
    lcd.clear();

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(SG_PIN, OUTPUT);

    WiFi.begin(ssid, pass);
    Blynk.config(BLYNK_AUTH_TOKEN);
}

void loop() {
    checkWiFiConnection();
    readDHT();
    controlDevices();
    updateLCD();
    delay(500);

}

//  เช็คและเชื่อมต่อ WiFi ใหม่ ถ้าหลุด
void checkWiFiConnection() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastWiFiCheck >= wifiCheckInterval) {
        lastWiFiCheck = currentMillis;

        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.begin(ssid, pass);
            isConnected = false;
        } else {
            if (!Blynk.connected()) {
                Blynk.connect();
            }
            isConnected = Blynk.connected();
        }
    }

    if (isConnected) {
        Blynk.run();
    }
}

//  อ่านค่าเซ็นเซอร์ DHT และส่งไปยัง Blynk
void readDHT() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastDHTReadTime >= dhtInterval) {
        lastDHTReadTime = currentMillis;

        float h = dht.readHumidity();
        float t = dht.readTemperature();

            if (isConnected) {
                Blynk.virtualWrite(V0, t);
                Blynk.virtualWrite(V1, h);
            }
        } 
    }


//  ควบคุมหลอดไฟ พัดลม มอเตอร์ ตัวทำความชื้น
void controlDevices() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (h < 55.0) {
        digitalWrite(SG_PIN, HIGH);
    } else if (h >= 70.0) {
        digitalWrite(SG_PIN, LOW);
    }
    
    if (t < 36.3) {
        digitalWrite(RELAY_PIN, HIGH);
    } else if (t > 37.0) {
        digitalWrite(RELAY_PIN, LOW);
    }

    if (M >= 28800) {
        digitalWrite(MOTOR_PIN, HIGH);
        delay(1500);
        digitalWrite(MOTOR_PIN, LOW);
        M = 0;
    }

    if (F >= 4000) {
        digitalWrite(FAN_PIN, HIGH);
        delay(3000);
        digitalWrite(FAN_PIN, LOW);
        F = 0;
    }

    M++;
    F++;
}


//  อัปเดตจอ LCD
void updateLCD() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    

    lcd.setCursor(3, 0);
    lcd.print(isConnected ? "WiFi Connected" : "No WiFi");
    lcd.setCursor(0, 1);
    lcd.print("H="); lcd.print(h, 1); lcd.print("%");
    lcd.setCursor(7, 1);
    lcd.print("T="); lcd.print(t, 1); lcd.print("C");

}
