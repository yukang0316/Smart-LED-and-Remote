#include <ArduinoBLE.h>

#define LED_PIN 2          // LED 핀 (PWM 사용)
#define LIGHT_SENSOR A2    // 조도 센서 핀

int brightness = 255;      // LED 밝기 (0~255 범위)
bool autoMode = true;      // AUTO 모드 활성 여부
bool ledOn = false;        // LED ON/OFF 상태

// BLE 서비스와 특성 설정
BLEService ledService("180A"); // BLE 서비스 UUID
BLEStringCharacteristic ledCharacteristic("2A57", BLERead | BLEWrite, 10); // BLE 특성 UUID

void setup() {
  Serial.begin(9600);      // 시리얼 통신 초기화
  while (!Serial);         // 시리얼 포트가 준비될 때까지 대기

  // BLE 초기화
  if (!BLE.begin()) {
    Serial.println("BLE 초기화 실패!"); // BLE 초기화 실패 시 메시지 출력
    while (1);                         // 무한 루프
  }

  // BLE 기본 설정
  BLE.setLocalName("SmartLED");                  // BLE 장치 이름 설정
  BLE.setAdvertisedService(ledService);          // BLE 서비스 광고 설정

  // BLE 서비스 및 특성 추가
  ledService.addCharacteristic(ledCharacteristic);
  BLE.addService(ledService);

  // 초기 상태 설정 (LED OFF)
  ledCharacteristic.writeValue("0");

  // BLE 광고 시작
  BLE.advertise();
  Serial.println("BLE 장치가 광고 중입니다.");

  // LED 핀 및 조도 센서 초기화
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0); // 초기 LED OFF
}

void loop() {
  // BLE 클라이언트 연결 확인
  BLEDevice central = BLE.central(); // 중앙 장치와의 연결 상태 확인

  if (central) { // 중앙 장치가 연결되었을 때
    Serial.print("Central 연결됨: ");
    Serial.println(central.address());

    while (central.connected()) { // 연결 유지 중일 때
      if (ledCharacteristic.written()) { // BLE 특성이 쓰여졌는지 확인
        String value = ledCharacteristic.value(); // 특성 값 읽기
        handleCommand(value); // 명령 처리 함수 호출
      }

      if (autoMode) { // AUTO 모드 활성화 시
        int lightValue = analogRead(LIGHT_SENSOR); // 조도 센서 값 읽기
        if (lightValue > 300) { // 어두운 환경일 경우
          analogWrite(LED_PIN, brightness); // LED 밝기 설정
          ledOn = true;
        } else { // 밝은 환경일 경우
          analogWrite(LED_PIN, 0); // LED 끄기
          ledOn = false;
        }
      }
    }

    // BLE 중앙 장치가 연결 해제된 경우
    Serial.print("Central 연결 종료: ");
    Serial.println(central.address());
  }
}

// 명령 처리 함수
void handleCommand(String value) {
  if (value == "0") { // OFF 명령 처리
    analogWrite(LED_PIN, 0); // LED 끄기
    ledOn = false;
    autoMode = false; // AUTO 모드 비활성화
    Serial.println("LED OFF");
  } else if (value == "1") { // ON 명령 처리
    analogWrite(LED_PIN, brightness); // LED 켜기
    ledOn = true;
    autoMode = false; // AUTO 모드 비활성화
    Serial.println("LED ON");
  } else if (value == "2") { // AUTO 모드 활성화
    autoMode = true;
    Serial.println("AUTO MODE ON");
  } else if (value.startsWith("B")) { // 밝기 조정 명령 처리
    int level = value.substring(1).toInt(); // 명령에서 숫자 부분 추출
    if (level >= 0 && level <= 9) { // 유효한 밝기 값인지 확인
      brightness = map(level, 0, 9, 0, 255); // 0~9 값을 0~255로 매핑
      if (ledOn) { // LED가 켜져 있으면 밝기 적용
        analogWrite(LED_PIN, brightness);
      }
      Serial.print("밝기 설정: ");
      Serial.println(brightness);
    }
  }
}
