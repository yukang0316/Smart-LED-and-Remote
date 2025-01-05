#include <ArduinoBLE.h>

#define ON_BUTTON 2        // ON 버튼 핀 번호
#define OFF_BUTTON 3       // OFF 버튼 핀 번호
#define AUTO_BUTTON 4      // AUTO 버튼 핀 번호
#define UP_BUTTON 5        // 밝기 UP 버튼 핀 번호
#define DOWN_BUTTON 6      // 밝기 DOWN 버튼 핀 번호

int brightness = 0;        // 현재 밝기 값 (0 ~ 9)

BLEService remoteService("180A"); // BLE 서비스 UUID
BLECharacteristic remoteCharacteristic("2A57", BLEWrite, 10); // BLE 특성 UUID

void setup() {
  Serial.begin(9600);      // 시리얼 통신 초기화
  while (!Serial);         // 시리얼 포트가 준비될 때까지 대기

  // 버튼 핀 초기화 (내부 풀업 저항 사용)
  pinMode(ON_BUTTON, INPUT_PULLUP);
  pinMode(OFF_BUTTON, INPUT_PULLUP);
  pinMode(AUTO_BUTTON, INPUT_PULLUP);
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);

  // BLE 초기화
  if (!BLE.begin()) {
    Serial.println("BLE 초기화 실패!"); // BLE 초기화 실패 시 메시지 출력
    while (1);                         // 무한 루프로 실행 중단
  }

  Serial.println("BLE 초기화 성공!");

  // BLE Central로 스캔 시작
  BLE.scanForUuid("180A");             // Service UUID "180A"를 가진 기기 검색
  Serial.println("BLE 스캔 시작...");
}

void loop() {
  BLEDevice peripheral = BLE.available(); // 주변 BLE 기기 확인

  if (peripheral) { // BLE 기기가 감지되었을 때
    Serial.print("발견된 디바이스: ");
    Serial.println(peripheral.localName()); // 발견된 기기의 이름 출력

    // 이름이 "SmartLED"인 기기와 연결 시도
    if (peripheral.localName() == "SmartLED") {
      Serial.println("SmartLED 연결 시도...");
      BLE.stopScan(); // 스캔 중단

      if (peripheral.connect()) { // 연결 성공
        Serial.println("SmartLED 연결 성공!");
        if (peripheral.discoverService("180A")) { // Service 검색
          BLECharacteristic characteristic = peripheral.characteristic("2A57"); // Characteristic 검색
          if (characteristic) {
            controlLED(peripheral, characteristic); // LED 제어 함수 호출
          } else {
            Serial.println("Characteristic 찾기 실패!"); // Characteristic 검색 실패
          }
        } else {
          Serial.println("Service 찾기 실패!"); // Service 검색 실패
        }
      } else {
        Serial.println("SmartLED 연결 실패."); // 연결 실패
      }

      BLE.scanForUuid("180A"); // 재스캔 시작
    }
  }
}

// LED 제어 함수
void controlLED(BLEDevice &peripheral, BLECharacteristic &characteristic) {
  while (peripheral.connected()) { // BLE 연결이 유지되는 동안 실행
    // ON 버튼 눌림 감지
    if (digitalRead(ON_BUTTON) == LOW) {
      sendCommand(characteristic, "1"); // ON 명령 전송
      delay(200);                       // 디바운싱 처리
    }
    // OFF 버튼 눌림 감지
    if (digitalRead(OFF_BUTTON) == LOW) {
      sendCommand(characteristic, "0"); // OFF 명령 전송
      delay(200);                       // 디바운싱 처리
    }
    // AUTO 버튼 눌림 감지
    if (digitalRead(AUTO_BUTTON) == LOW) {
      sendCommand(characteristic, "2"); // AUTO 명령 전송
      delay(200);                       // 디바운싱 처리
    }
    // UP 버튼 눌림 감지
    if (digitalRead(UP_BUTTON) == LOW) {
      if (brightness < 9) brightness++; // 밝기 증가 (최대값 9 제한)
      sendCommand(characteristic, "B" + String(brightness)); // 밝기 명령 전송
      delay(200);                       // 디바운싱 처리
    }
    // DOWN 버튼 눌림 감지
    if (digitalRead(DOWN_BUTTON) == LOW) {
      if (brightness > 0) brightness--; // 밝기 감소 (최소값 0 제한)
      sendCommand(characteristic, "B" + String(brightness)); // 밝기 명령 전송
      delay(200);                       // 디바운싱 처리
    }
  }
  
  Serial.println("연결이 끊어졌습니다."); // 연결 종료 메시지 출력
}

// 명령 전송
void sendCommand(BLECharacteristic &characteristic, String command) {
  characteristic.writeValue(command.c_str()); // BLE Characteristic에 값 전송
  Serial.print("명령 전송: ");
  Serial.println(command); // 전송된 명령 출력
}
