#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 4        // ESP32-C3 연결된 GPIO 핀
#define NUMPIXELS 16     // 네오픽셀 LED 개수

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

String currentMode = "warm"; 
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnected(BLEServer* pServer) { 
      deviceConnected = true; 
      Serial.println("블루투스 연결됨!");
    };
    void onDisconnected(BLEServer* pServer) { 
      deviceConnected = false; 
      Serial.println("블루투스 연결 끊김 -> 저전류 차단 방지를 위해 은은한 감성등 모드로 유지");
      currentMode = "warm"; 
      pServer->startAdvertising(); 
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // 최신 ESP32 코어 버전에 맞게 String으로 바로 받도록 수정
      String received = pCharacteristic->getValue();
      
      if (received.length() > 0) {
        if (received.startsWith("PIN:")) {
          Serial.println("비밀번호 인증 수신 완료");
        } else {
          currentMode = received;
          Serial.println("받은 명령: " + currentMode);
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.show();

  // 💡 학생들이 수업 시 자신의 번호로 바꿀 부분 (예: LegoDiorama_01)
  BLEDevice::init("LegoDiorama_01");
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE 서버 준비 완료!");
}

void colorAll(uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void runAnimation() {
  if (currentMode == "warm") {
    for (int b = 10; b <= 70; b += 5) {
      if (currentMode != "warm") return;
      colorAll(b, b * 170 / 255, b * 40 / 255);
      delay(150);
    }
    for (int b = 70; b >= 10; b -= 5) {
      if (currentMode != "warm") return;
      colorAll(b, b * 170 / 255, b * 40 / 255);
      delay(150);
    }
  } 
  else if (currentMode == "sunset") {
    for (int i = 0; i < NUMPIXELS; i++) {
      if (currentMode != "sunset") return;
      if (i % 2 == 0) pixels.setPixelColor(i, pixels.Color(255, 80, 10));
      else pixels.setPixelColor(i, pixels.Color(180, 20, 150));
    }
    pixels.show();
    delay(200);
  } 
  else if (currentMode == "wave") {
    static uint16_t offset = 0;
    for (int i = 0; i < NUMPIXELS; i++) {
      if (currentMode != "wave") return;
      if ((i + offset) % 2 == 0) pixels.setPixelColor(i, pixels.Color(200, 200, 200)); 
      else pixels.setPixelColor(i, pixels.Color(0, 100, 255));   
    }
    pixels.show();
    offset++;
    delay(100);
  } 
  else if (currentMode == "rainbow") {
    static uint16_t j = 0;
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel((i * 256 / NUMPIXELS + j) & 255));
    }
    pixels.show();
    j += 4;
    delay(30);
  } 
  else if (currentMode == "party") {
    for (int i = 0; i < NUMPIXELS; i++) {
      if (random(3) == 0) pixels.setPixelColor(i, pixels.Color(random(50, 255), random(50, 255), random(50, 255)));
      else pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    delay(100);
  } 
  else if (currentMode == "off") {
    colorAll(0, 0, 0);
    delay(100);
  }
  else {
    colorAll(20, 10, 0);
    delay(100);
  }
}

void loop() {
  runAnimation();
}