#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define BT_RXD 3      // 블루투스 recevice
#define BT_TXD 2      // 블루투스 transfer

#define analog A2     // analog A2번 포트 심볼릭 상수
#define button 10     // digital 10번 포트 심볼릭 상수

#define P_len 12      // NeoPixel LED 갯수
#define P_LED 9       // digital 9번 포트 심볼릭 상수

#define DHTPIN A0     // LCD포트 (analog A0번) 심볼릭 상수
#define DHTTYPE DHT11 // LCD 타입

#define AA 5          // 팬 (digital 5번) 심볼릭 상수
#define AB 4          // 팬 (digital 4번) 심볼릭 상수
#define BA 6          // 팬 (digital 6번) 심볼릭 상수
#define BB 7          // 팬 (digital 7번) 심볼릭 상수

int cnt = 0;                    // 버튼 클릭 횟수 관리 변수
int r = 0, g = 0, b = 255;      // rgb값 변경 시 사용하는 변수

float fMax_hum = 70.0, fMax_temp = 30.0;  // 적정최대습도, 적정최대온도 변수

char bt_input, buf[80];       // 블루투스 input받는 변수, 디스플레이 출력 시 사용할 버퍼 함수

bool ON_FLAG = true;          // 팬 ON/OFF를 제어할 bool 함수

DHT dht (DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial bluetooth(BT_TXD, BT_RXD);
Adafruit_NeoPixel RGB_LED = Adafruit_NeoPixel(P_len, P_LED, NEO_GRB);

void setup() {
  lcd.init();
  Fan_init();
  dht.begin();
  NeoPixel_init();

  pinMode(button, INPUT_PULLUP);

  Serial.begin(9600);
  bluetooth.begin(9600);
}

void loop() {
  int Soil_moisture = map(analogRead(A1), 1023, 0, 0, 100);
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  int val = map(analogRead(analog), 0, 1023, 60, 255);
  int d_sig = digitalRead(button);

  if (d_sig == 0) {
    cnt++;
    if (cnt == 3) {
      cnt = 0;
    }
    delay(500);
  }

  if (cnt == 0) {
    RGB_LED.setBrightness(val);
    LED_Light(hum, temp);

    ON_FLAG = true;
  }
  else if (cnt == 1) {
    LED_Light(hum, temp);

    ON_FLAG = true;
  }
  else if (cnt == 2) {
    if (bluetooth.available() < 2) {
      bt_input = bluetooth.read();
    }
    else {
      r = bluetooth.read();
      g = bluetooth.read();
      b = bluetooth.read();
    }

    bluetooth.flush();

    if (bt_input == 'A') {
      RGB_LED.setBrightness(60);
    }
    else if (bt_input == 'B') {
      RGB_LED.setBrightness(0);
    }
    else if (bt_input == 'C') {
      ON_FLAG = true;
    }
    else if (bt_input == 'D') {
      ON_FLAG = false;
    }
    else {
      RGB_Color(RGB_LED.Color(r, g, b), 0);
    }
  }

  if (hum >= fMax_hum && ON_FLAG) {
    FanON(cnt, val);
  }
  else {
    FanOFF();
  }

  bluetooth.print(String((int)temp));
  bluetooth.print(",");
  bluetooth.print(String((int)hum));
  bluetooth.print(",");
  
  Display(hum, temp, Soil_moisture, cnt);
}

void Fan_init() {
  pinMode(AA, OUTPUT);
  pinMode(AB, OUTPUT);
  pinMode(BA, OUTPUT);
  pinMode(BB, OUTPUT);
}

void NeoPixel_init() {
  RGB_LED.begin();
  RGB_LED.setBrightness(10);
  RGB_LED.clear();
}

void RGB_Color(float c, int wait) {
  for (int i = 0; i < RGB_LED.numPixels(); i++) {
    RGB_LED.setPixelColor(i, c);
    RGB_LED.show();
    delay(wait);
  }
}

void Display(float hum, float temp, int Soil_moisture, int cnt) {
  lcd.backlight();
  sprintf(buf, "T:%02dc H:%02d%% S:%02d", (int)temp, (int)hum, Soil_moisture);
  lcd.setCursor(0, 0);
  lcd.print(buf);

  if(cnt == 0) {
    lcd.setCursor(1, 1);
    lcd.print("MODE:LED LIGHT");
  }
  else if (cnt == 1) {
    lcd.setCursor(1, 1);
    lcd.print("MODE:FAN SPEED");
  }
  else if (cnt == 2) {
    lcd.setCursor(1, 1);
    lcd.print("MODE:Bluetooth");
  }
}

void FanON(int cnt, int val) {
  if (cnt == 1) {
    analogWrite(AA, val);
    digitalWrite(AB, LOW);
    analogWrite(BA, val);
    digitalWrite(BB, LOW);
  }
  else {
    analogWrite(AA, 60);
    digitalWrite(AB, LOW);
    analogWrite(BA, 60);
    digitalWrite(BB, LOW);
  }
}

void FanOFF() {
  digitalWrite(AA, LOW);
  digitalWrite(AB, LOW);
  digitalWrite(BA, LOW);
  digitalWrite(BB, LOW);
}

void LED_Light(float hum, float temp) {
  if (((int)hum > 30 && (int)hum < 70) && ((int)temp > 21 && (int)temp < 26)) {
    RGB_Color(RGB_LED.Color(0, 0, 255), 0);
  }
  else {
    RGB_Color(RGB_LED.Color(255, 0, 0), 0);
  }
}
