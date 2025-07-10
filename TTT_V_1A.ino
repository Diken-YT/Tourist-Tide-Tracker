#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Firebase_ESP_Client.h>

const char* ssid = "State B";
const char* password = "NagariTambang";
#define API_KEY "-iVevKFp0"
#define DATABASE_URL "/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

BLEScan* pBLEScan;
int wifiDeviceCountThisLoop = 0;
int wifiTotalDeviceCount = 0;
int bleDeviceCountThisLoop = 0;
int bleTotalDeviceCount = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {}
};

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up successful");
  } else {
    Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  BLEDevice::init("ESP32");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  // WiFi scan
  int scanResults = WiFi.scanNetworks();
  if (scanResults == WIFI_SCAN_RUNNING) {
    Serial.println("WiFi scan in progress...");
  } else if (scanResults == 0) {
    Serial.println("No WiFi networks found");
  } else {
    wifiDeviceCountThisLoop = scanResults;
    wifiTotalDeviceCount += scanResults;
    if (Firebase.ready()) {
      if (Firebase.RTDB.setInt(&fbdo, "A_wifi_device_count_this_loop", wifiDeviceCountThisLoop)) {
        Serial.println("WiFi count for this loop sent to Firebase");
        Serial.println("WiFi count for this loop: " + String(wifiDeviceCountThisLoop));
      } else {
        Serial.println("Failed to send WiFi count for this loop to Firebase");
      }
      if (Firebase.RTDB.setInt(&fbdo, "A_total_wifi_device_count", wifiTotalDeviceCount)) {
        Serial.println("Total WiFi count sent to Firebase");
        Serial.println("Total WiFi count: " + String(wifiTotalDeviceCount));
      } else {
        Serial.println("Failed to send total WiFi count to Firebase");
      }
    } else {
      Serial.println("Firebase not ready");
    }
  }

  // BLE scan
  BLEScanResults foundDevices = pBLEScan->start(10);
  bleDeviceCountThisLoop = foundDevices.getCount();
  bleTotalDeviceCount += bleDeviceCountThisLoop;
  if (Firebase.ready()) {
    if (Firebase.RTDB.setInt(&fbdo, "A_ble_device_count_this_loop", bleDeviceCountThisLoop)) {
      Serial.println("BLE count for this loop sent to Firebase");
      Serial.println("BLE count for this loop: " + String(bleDeviceCountThisLoop));
    } else {
      Serial.println("Failed to send BLE count for this loop to Firebase");
    }
    if (Firebase.RTDB.setInt(&fbdo, "A_total_ble_device_count", bleTotalDeviceCount)) {
      Serial.println("Total BLE count sent to Firebase");
      Serial.println("Total BLE count: " + String(bleTotalDeviceCount));
    } else {
      Serial.println("Failed to send total BLE count to Firebase");
    }
  } else {
    Serial.println("Firebase not ready");
  }

  delay(60000); // delay 1 Menit
}
