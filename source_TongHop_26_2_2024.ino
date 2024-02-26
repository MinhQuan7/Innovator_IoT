/*
 * Example sketch for reporting on readings from the LD2410 using whatever settings are currently configured.
 * 
 * This has been tested on the following platforms...
 * 
 * On ESP32, connect the LD2410 to GPIO pins 32&33 ===>>>Tx : 32 . Rx :33
 * On ESP32S2, connect the LD2410 to GPIO pins 8&9
 * On ESP32C3, connect the LD2410 to GPIO pins 4&5
 * On Arduino Leonardo or other ATmega32u4 board connect the LD2410 to GPIO pins TX & RX hardware serial
 * 
 * The serial configuration for other boards will vary and you'll need to assign them yourself
 * 
 * There is no example for ESP8266 as it only has one usable UART and will not boot if the alternate UART pins are used for the radar.
 * 
 * For this sketch and other examples to be useful the board needs to have two usable UARTs.
 * 
 */

#if defined(ESP32)
#ifdef ESP_IDF_VERSION_MAJOR  // IDF 4+
#if CONFIG_IDF_TARGET_ESP32   // ESP32/PICO-D4
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33
#elif CONFIG_IDF_TARGET_ESP32S2
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 9
#define RADAR_TX_PIN 8
#elif CONFIG_IDF_TARGET_ESP32C3
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 4
#define RADAR_TX_PIN 5
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else  // ESP32 Before IDF 4.0
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33
#endif
#elif defined(__AVR_ATmega32U4__)
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 0
#define RADAR_TX_PIN 1
#endif

//______ĐỊNH NGHĨA CÁC CHÂN CỦA SENSOR ___________
#define ir1 25

#define led_battinh 13  //____________Ở ĐÂY LED_BATTINH ĐẠI DIỆN CHO TÍN HIỆU Ở CHÂN CỦA TINKER_________

#include <ld2410.h>
ld2410 radar;
void setting_nguong(int x, int y, int& value_Mov, int& value_Station, int& gate);
int value_Mov = 0;
int value_Station = 0;
uint32_t lastReading = 0;
bool radarConnected = false;

bool ir1Triggered = false;

int flag_warning = 0;
void setup(void) {

  MONITOR_SERIAL.begin(115200);  //Feedback over Serial Monitor
//radar.debug(MONITOR_SERIAL); //Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
#if defined(ESP32)
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);  //UART for monitoring the radar
#elif defined(__AVR_ATmega32U4__)
  RADAR_SERIAL.begin(256000);  //UART for monitoring the radar
#endif
  delay(500);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  if (radar.begin(RADAR_SERIAL)) {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  } else {
    MONITOR_SERIAL.println(F("not connected"));
  }
  pinMode(ir1, INPUT_PULLUP);
  pinMode(led_battinh,OUTPUT);
}

bool people = false;
int noDistanceCount = 0;

void setting_nguong(int x, int y, int& value_Mov, int& value_Station, int& gate) {

  if (x <= 75 && y <= 75) {
    value_Mov = 50;
    value_Station = 50;
    gate = 1;
  }

  if ((x > 75 && x <= 150) && (y > 75 && y <= 150)) {
    value_Mov = 40;
    value_Station = 40;
    gate = 2;
  }

  if ((x > 150 && x <= 225) && (y > 150 && y <= 225)) {
    value_Mov = 35;
    value_Station = 35;
    gate = 3;
  }

  if ((x > 225 && x <= 300) && (y > 225 && y <= 300)) {
    value_Mov = 30;
    value_Station = 30;
    gate = 4;
  }

  if ((x > 300 && x <= 375) && (y > 300 && y <= 375)) {
    value_Mov = 30;
    value_Station = 30;
    gate = 5;
  }

  if ((x > 375 && x <= 450) && (y > 375 && y <= 450)) {
    value_Mov = 25;
    value_Station = 25;
    gate = 6;
  }

  if ((x > 450 && x <= 525) && (y > 450 && y <= 525)) {
    value_Mov = 25;
    value_Station = 25;
    gate = 7;
  }

  if ((x > 525 && x <= 600) && (y > 525 && y <= 600)) {
    value_Mov = 20;
    value_Station = 20;
    gate = 8;
  }
}

void loop() {

  int old_locationStation = radar.stationaryTargetDistance();
  int old_locationMoving = radar.movingTargetDistance();

  int new_locationStation = 0;
  new_locationStation = old_locationStation;

  int new_locationMoving = 0;
  new_locationMoving = old_locationMoving;
  int gate = 0;
  radar.read();

  int x = radar.movingTargetDistance();
  int y = radar.stationaryTargetDistance();

  setting_nguong(x, y, value_Mov, value_Station, gate);
  //hồng ngoại 1 nhận xong tới hồng ngoại 2 ở phía trong
  if (digitalRead(ir1) == 0) {
    ir1Triggered = true;  //cảm biến siêu âm
  }

  if (ir1Triggered) {
    if (radar.isConnected() && millis() - lastReading > 250)  //Report every 1000ms
    {
      lastReading = millis();
      if (radar.presenceDetected()) {
        if (radar.stationaryTargetDetected()) {
          Serial.print(F("Stationary target: "));
          Serial.print(radar.stationaryTargetDistance());
          Serial.print(F("cm energy:"));
          Serial.print(radar.stationaryTargetEnergy());
          Serial.print(' ');
        }
        if (radar.movingTargetDetected()) {
          Serial.print(F("Moving target: "));
          Serial.print(radar.movingTargetDistance());
          Serial.print(F("cm energy:"));
          Serial.print(radar.movingTargetEnergy());
        }
        Serial.println();

        if (radar.movingTargetDetected() && radar.stationaryTargetDetected()) {
          int movingDistance = radar.movingTargetDistance();
          int stationaryDistance = radar.stationaryTargetDistance();
          if (abs(movingDistance - stationaryDistance) <= 20) {
            people = true;  // chắn chắn hẳn là có người vào phòng
          }
        }
        if (people) {
          if (gate != 0) {
            Serial.print("Co nguoi o gate: ");
            Serial.println(gate);
            //Check bất tỉnh khi đã nằm im một chổ và fix trường hợp đi ra ngoài tụt energy
            if ((new_locationStation == old_locationStation) && (new_locationMoving == old_locationMoving)) {
              if ((radar.stationaryTargetEnergy() < value_Station) && (radar.movingTargetEnergy() < value_Mov)) {  //value_Mov và value_Station là ngưỡng theo từng khoảng cách
                flag_warning++;
                if (flag_warning == 40) {
                  Serial.println("Cannh bao !!! CHET NGUOI ROIII ");

                  //____________Ở ĐÂY LED_BATTINH ĐẠI DIỆN CHO TÍN HIỆU Ở CHÂN CỦA TINKER_________
                  digitalWrite(led_battinh,HIGH);
                  Serial.print("Value nguong cua Moving: ");
                  Serial.print(value_Mov);
                  Serial.print("Value nguong cua Station: ");
                  Serial.print(value_Station);
                  //Test biến đếm
                  Serial.print("Flag Warning: ");
                  Serial.println(flag_warning);
                }  //const có thể tùy chỉnh do cách lắp và test tùy ý ứng biến
              }
            }
            //Hàm check vị trí người theo khoảng cách coi thử có bao nhiêu người
            if ((digitalRead(ir1) == 0) && (radar.stationaryTargetEnergy() > value_Mov) && (radar.movingTargetEnergy() > value_Station) && (gate != 0)) {  //Vẫn có người ở gate
              //có người  ra khỏi phòng mà ở trong phòng vẫn có 1 người khác
              ir1Triggered = true;
              Serial.print("Van co nguoi o trong phong !!");
              digitalWrite(led_battinh,LOW);
            }
          }  //end gate
        }    //end people

        if ((radar.movingTargetDistance() < 50) && (radar.stationaryTargetDistance() < 40) && (digitalRead(ir1) == 1) && !radar.movingTargetDetected()) {  //kt lại 1 lần nữa nếu không còn người trong phòng(sau 10 lần quét)
          if (noDistanceCount >= 10) {
            ir1Triggered = false;  //sensor không còn giá trị
            people = false;        //không con phát hiện người với moving và station
            Serial.println(F("Co nguoi ra khoi phong"));
            Serial.print("Bien dem: ");
            Serial.println(noDistanceCount);
            noDistanceCount = 0;
          } else {
            noDistanceCount++;
          }
          digitalWrite(led_battinh,LOW);
        } else {
          noDistanceCount = 0;
        }
      }
    }
  }  //end trigger
}
