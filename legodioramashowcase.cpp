#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

#define PIN 4        // 네오픽셀 데이터 핀 (ESP32-C3 SuperMini 기준 GPIO 4)
#define NUMPIXELS 16 // 16구 네오픽셀 링

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// ==========================================
// ⭐ [설정] 학생 번호에 맞게 이 숫자만 1~12로 변경하세요!
const int student_id = 1; 
// ==========================================

char deviceName[30];
bool deviceConnected = false;
BLECharacteristic *pCharacteristic;

// 웹페이지와 일치해야 하는 고유 통신 UUID
#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✨ 쇼케이스와 스마트폰 연결 성공!");
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("🔌 연결 끊김. 다시 검색 가능하도록 대기합니다...");
      pServer->startAdvertising(); // 연결이 끊어지면 다시 주변에서 찾을 수 있게 광고 재시작
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        String cmd = "";
        for (int i = 0; i < value.length(); i++) {
          cmd += value[i];
        }
        cmd.trim();
        Serial.println("받은 제어 명령: " + cmd);

        // 웹페이지 버튼에 따른 네오픽셀 동작 실행
        if (cmd == "off") {
          colorAll(0, 0, 0);
        } else if (cmd == "warm") {
          colorAll(255, 180, 50); // 따뜻한 감성등 (전구색)
        } else if (cmd == "blue") {
          colorAll(0, 100, 255);   // 신비로운 오션 (파란색)
        } else if (cmd == "rainbow") {
          rainbowEffect();         // 무지개 파티
        } else if (cmd == "party") {
          partyEffect();           // 반짝반짝 축제
        }
      }
    }
};

void colorAll(uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void rainbowEffect() {
  for(int i=0; i<pixels.numPixels(); i++) {
    int hue = (i * 65536L / pixels.numPixels());
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(hue, 255, 255)));
  }
  pixels.show();
}

void partyEffect() {
  for(int i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, pixels.Color(random(256), random(256), random(256)));
  }
  pixels.show();
}

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.show(); // 시작 시 조명 끄기

  // 학생 번호에 따라 블루투스 기기 이름 자동 생성 (예: LegoStore_01, LegoStore_02 ...)
  sprintf(deviceName, "LegoStore_%02d", student_id);

  // BLE 초기화 및 서버 시작
  BLEDevice::init(deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  
  // 블루투스 신호 송출(Advertising) 시작
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); 
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.print("BLE 블루투스 준비 완료! 기기 이름: ");
  Serial.println(deviceName);
}

void loop() {
  // BLE 연결 상태 유지 대기
  delay(100);
}