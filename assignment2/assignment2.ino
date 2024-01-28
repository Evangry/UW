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

const int ACCEL_TIMER_MS = 20;
const int OLED_TIMER_MS = 250;

float x_acc;
float y_acc;
float z_acc;
unsigned long accel_millis;
unsigned long oled_millis;

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

  x_acc = 0.0;
  y_acc = 0.0;
  z_acc = 0.0;
  accel_millis = millis();
  oled_millis = millis();
}

void loop() {
  if (millis() > accel_millis) {
    sensors_event_t event;
    lis.getEvent(&event);
    
    x_acc = event.acceleration.x;
    y_acc = event.acceleration.y;
    z_acc = event.acceleration.z;
    accel_millis += ACCEL_TIMER_MS;
    Serial.print("x_acc:");
    Serial.print(x_acc);
    Serial.print(",y_acc:");
    Serial.print(y_acc);
    Serial.print(",z_acc:");
    Serial.println(z_acc);
  }
  if (millis() > oled_millis) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("X acc:");
    display.println(String(x_acc));
    display.print("Y acc:");
    display.println(String(y_acc));
    display.print("Z acc:");
    display.println(String(z_acc));
    display.display();
    oled_millis += OLED_TIMER_MS;
  }
  
}