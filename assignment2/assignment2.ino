#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>


#define LIS3DH_ADDRESS 0x18

//taken from acceldemo for LIS3DH example code
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
#define LIS3DH_CS 10
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

//taken from ssd1306_128x64_i2c example code
#define OLED_RESET -1
#define OLED_ADDRESS 0x3D
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const int BATTERY_PIN =35;
const float BATTERY_FULLISH = 4.0;
const float BATTERY_EMPTYISH = 3.5;
const int ACCEL_TIMER_MS = 20;
const int OLED_TIMER_MS = 250;
const int BUTTON_PIN = A3;
const int DEBOUNCE_MS = 30;

const int ARABIC_INTS[] = {1, 4, 5, 9, 10, 40, 50, 90, 100, 400, 500, 900, 1000};
const String ROMAN_INTS[] = {"I", "IV", "V", "IX", "X", "XL", "L", "XC", "C", "CD", "D", "CM", "M"};
const int ROMAN_CUTOFF = 5000;
const int TEXT_HEIGHT = 12;
const int CHAR_WIDTH = 12;

const int ROOF_HEIGHT = 20;
const int COLUMN_WIDTH = 15;
const int COLUMN_ENDS_HEIGHT = 6;
const int COLUMN_CENTER_INDENT = 3;

const int CHALLENGE_STEP_COUNTS[] = {50, 100, 500, 1000, 2000, 3000};
const int CHALLENGE_TIMES_MS[] = {25000, 55000, 300000, 700000, 1600000, 2500000};

float x_acc;
float y_acc;
float z_acc;
int saved_button_value;
unsigned long button_read_millis;
unsigned long accel_millis;
unsigned long oled_millis;
float battery_voltage;
int total_steps;
int challenge_remaining;
int challenge_number;
unsigned long challenge_deadline;
String big_message;
bool inverted;

void setup() {
  Serial.begin(9600);

  //taken from acceldemo for LIS3DH example code
  while (!Serial) delay(10);
  if (! lis.begin(LIS3DH_ADDRESS)) {
    while (1) yield();
  }
  lis.setRange(LIS3DH_RANGE_4_G);
  lis.setDataRate(LIS3DH_DATARATE_50_HZ);

  //taken from ssd1306_128x64_i2c example code
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    for(;;);
  }


  pinMode(BATTERY_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  x_acc = 0.0;
  y_acc = 0.0;
  z_acc = 0.0;
  saved_button_value = 0;
  accel_millis = millis();
  oled_millis = millis();
  total_steps = 0;
  challenge_remaining = 0;
  challenge_number = 0;
  challenge_deadline = millis();
  big_message = "Rome wasn't built in a day.";
  inverted = false;
}

void loop() {
  if (millis() >= button_read_millis) {
    int button_value = digitalRead(BUTTON_PIN);
    if (button_value != saved_button_value) {
      saved_button_value = button_value;
      if (button_value == 1) {
        if (big_message.length() > 0) {
          big_message = "";
        }
        else if (challenge_remaining <= 0) {
          challenge_number = millis() % 6;
          challenge_remaining = CHALLENGE_STEP_COUNTS[challenge_number];
          challenge_deadline = millis() + CHALLENGE_TIMES_MS[challenge_number];

        }
        else {
          big_message = "The challenge delenda est.";
        }
      }
      
      button_read_millis = millis() + DEBOUNCE_MS;
    }
  }
  if (millis() > accel_millis) {
    sensors_event_t event;
    lis.getEvent(&event);
    
    x_acc = event.acceleration.x;
    y_acc = event.acceleration.y;
    z_acc = event.acceleration.z;
    accel_millis += ACCEL_TIMER_MS;
    // Serial.print("x_acc:");
    // Serial.print(x_acc);
    // Serial.print(",y_acc:");
    // Serial.print(y_acc);
    // Serial.print(",z_acc:");
    // Serial.println(z_acc);
  }
  if (millis() > oled_millis) {

    display.clearDisplay();
    display.invertDisplay(inverted);
    if (big_message.length() > 0){
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.setTextColor(SSD1306_INVERSE);
      display.println(big_message);
    }
    else {
      display.setTextSize(1);
      display.setTextColor(SSD1306_INVERSE);
      centeredPrint(romanNumeral(CHALLENGE_STEP_COUNTS[challenge_number]), ROOF_HEIGHT * 3 / 8);
      centeredPrint(romanNumeral(challenge_remaining), ROOF_HEIGHT);
      centeredPrint(time(challenge_deadline - millis()), ROOF_HEIGHT + TEXT_HEIGHT);
      centeredPrint(romanNumeral(total_steps), ROOF_HEIGHT + TEXT_HEIGHT * 2);

      // battery math taken from 
      // https://cuddletech.com/2017/12/battery-monitoring-on-the-adafruit-huzzah32-esp32-with-mongooseos/
      battery_voltage = analogRead(BATTERY_PIN) / 4095.0  * 2.0 * 3.3 * 1.1;
      if (battery_voltage > BATTERY_FULLISH) {
        centeredPrint("charge:H", ROOF_HEIGHT + TEXT_HEIGHT * 3);
      }
      else if (battery_voltage < BATTERY_EMPTYISH) {
        centeredPrint("charge:L", ROOF_HEIGHT+ TEXT_HEIGHT * 3);
      }
      else {
        centeredPrint("charge:M", ROOF_HEIGHT+ TEXT_HEIGHT * 3);
      }
      drawBuilding(1.0 - 1.0 * challenge_remaining / CHALLENGE_STEP_COUNTS[challenge_number] , timeTaken());
    }
    
    display.display();
    oled_millis += OLED_TIMER_MS;
  }
  
}

String romanNumeral(int arabic) {
  if (arabic <= 0) {
    return "-";
  }
  String ret = "";
  if (arabic > ROMAN_CUTOFF) {
    ret = String(ROMAN_CUTOFF/1000 * (arabic / ROMAN_CUTOFF)) + "k+";
  }
  arabic = arabic % ROMAN_CUTOFF;

  for (int i = 12; i >= 0; i--) {
    while (arabic >= ARABIC_INTS[i]) {
      arabic -= ARABIC_INTS[i];
      ret = ret + ROMAN_INTS[i];
    }
  }
  return ret;
}

String time(int ms) {
  if (ms <= 0) {
    return "0:00";
  }
  int seconds = ms / 1000;
  if (seconds % 60 < 10) {
    return String(seconds / 60) + ":0" + String(seconds % 60);
  }
  else {
    return String(seconds / 60) + ":" + String(seconds % 60);
  }
}

void drawBuilding(float left, float right) {
  
    //roof
    display.fillTriangle(
      display.width()/2, 0,
      0, ROOF_HEIGHT - 3,
      display.width(), ROOF_HEIGHT - 3,
      SSD1306_INVERSE);

    //left column upper end
    display.fillRect(0, ROOF_HEIGHT,
     COLUMN_WIDTH, COLUMN_ENDS_HEIGHT - 2, SSD1306_INVERSE);
    //left column lower end
    display.fillRect(0, display.height() - COLUMN_ENDS_HEIGHT + 2,
     COLUMN_WIDTH, COLUMN_ENDS_HEIGHT - 2, SSD1306_INVERSE);
    //left column center
    display.fillRect(COLUMN_CENTER_INDENT, ROOF_HEIGHT + COLUMN_ENDS_HEIGHT,
     COLUMN_WIDTH - 2 * COLUMN_CENTER_INDENT, (int) ((display.height() - ROOF_HEIGHT - 2 * COLUMN_ENDS_HEIGHT) * left),
      SSD1306_INVERSE);

    //right column upper end
    display.fillRect(display.width() - COLUMN_WIDTH, ROOF_HEIGHT,
     COLUMN_WIDTH, COLUMN_ENDS_HEIGHT - 2, SSD1306_INVERSE);
    //right column lower end
    display.fillRect(display.width() - COLUMN_WIDTH, display.height() - COLUMN_ENDS_HEIGHT + 2,
     COLUMN_WIDTH, COLUMN_ENDS_HEIGHT - 2, SSD1306_INVERSE);
    //right column center
    display.fillRect(display.width() - COLUMN_WIDTH + COLUMN_CENTER_INDENT, ROOF_HEIGHT + COLUMN_ENDS_HEIGHT,
     COLUMN_WIDTH - 2 * COLUMN_CENTER_INDENT, (int) ((display.height() - ROOF_HEIGHT - 2 * COLUMN_ENDS_HEIGHT) * right),
      SSD1306_INVERSE);

}


void centeredPrint(String str, int height){
  display.setCursor((display.width() * 2 - str.length() * CHAR_WIDTH) / 4, height);
  display.println(str);
}

float timeTaken() {
  if (challenge_deadline > millis()){
    return 1.0 - (challenge_deadline - millis()) * 1.0 / CHALLENGE_TIMES_MS[challenge_number];
  }
  else {
    return 1.0;
  }
}

void onStep() {
  total_steps ++;
  inverted = !inverted;
  if (challenge_remaining > 0) {
    challenge_remaining --;
    if (challenge_remaining == 0 && millis() <= challenge_deadline) {
      big_message = "Veni. Vidi. Vici.";
    }
    else if (challenge_remaining == 0 && millis() > challenge_deadline) {
      big_message = "All roads lead to Rome.";
    }
  }
}
