#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <Wire.h>
#include "RTClib.h"
#include "config.h"

EthernetClient client;
EthernetServer server = EthernetServer(SERVER_PORT);
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

unsigned long time1;
unsigned long time2;
unsigned long time3;
float distanceK = 600;
float distanceU = 0;
float speedVeh = 0;
int loopAc = 0;
unsigned int axil1 = 0;
unsigned int axil2 = 0;
unsigned int Prev_loopAc = 0;
unsigned int Prev_axil1 = 0;
unsigned int Prev_axil2 = 0;
int period = 100;
unsigned long time_now = 0;
int prev = 1;
int prev2 = 1;
int prev3 = 1;
int hit = 0;
int hit2 = 0;
int hit3 = 0;
int hardrst = 13;
String incom = "";

int lanid = 0;
int DataRead = 1;
long   EmbeddedSeqN = 0;

String AVCNE = "START,AVCC,";
String AliveN = "START,ALIVE,";

String   Times = "HH:mm:ss";
int indexlen = 0;
int forward = 0;
int height = 0;
int ier = 0;
int switchn = 0;
String readString;

int loopact = 0;
int t1po = 0;
int t2po = 0;
unsigned long t1 = 0;
unsigned long t2 = 0;
float velocity;

int h1 = 0;
int h2 = 0;
int h3 = 0;
int h4 = 0;

int jser = 0;
String Date = "DD:MM:YYYY";



void checkHardReset();
String getMessageEthernet();
int parseCommand(String str);
void updateRelayState(int relay, int state);
void printAlive();
long EEPROMReadlong(long address);
void EEPROMWritelong(int address, long value);

void avcc() {
  if (loopAc == 1) {
    if ( digitalRead(DI2) == 1) {
      if (prev == 0) {
        prev = 1;
        prev = digitalRead(DI2);
        hit++;
        if (hit == 1) {
          time1 = millis();
        }
        if (hit == 2) {
          time2 = millis();
        }
      }
    } else {
      prev = 0;
    }
    if (digitalRead(DI6) != prev2) {
      prev2 = digitalRead(DI6);
      hit2++;
    }
    if (digitalRead(DI5) != prev3) {
      prev3 = digitalRead(DI5);
      hit3++;
    }
  }

  if (digitalRead(DI1) == 1) {
    loopAc = 1;
    if (loopact == 0)
    {
      loopact = 1;
    }
    if (digitalRead(DI4) == 1) {
      h1 = 1;
    }
    if (digitalRead(DI5) == 1) {
      h2 = 1;
    }
    if (digitalRead(DI6) == 1) {
      h3 = 1;
    }
    if (digitalRead(DI7) == 1) {
      h4 = 1;
    }
    height = h1 + h2 + h3 + h4;

    if (digitalRead(DI1) == 1) { //edited DI2
      if (t1po == 0) {
        t1po = 1;
        t1 = millis();
      }
      if (Prev_axil1 == 0) {
        Prev_axil1 = 1;
        axil1 = axil1 + 1;
      }
    } else {
      Prev_axil1 = 0;
    }
    if (digitalRead(DI3) == 1) {
      if (t2po == 0) {
        t2po = 1;
        t2 = millis();
      }
      if (Prev_axil2 == 0) {
        Prev_axil2 = 1;
        axil2 = axil2 + 1;
      }
    }
    else {
      Prev_axil2 = 0;
    }
  } else {
    if (loopAc == 1) {
      hit = hit * 2;
      loopAc = 0;
      t2po = 0;
      t1po = 0;
      if (t2 < t1) {
        velocity = t1 - t2;
        velocity = velocity / 1000;       //convert millisecond to second
        velocity = (0.75 / velocity) * 3.6; // km/s//0.2 meters ||-----||
        //distanceU=(speedVeh*(time3-time1));
      } else {
        velocity = t2 - t1;
        velocity = velocity / 1000;       //convert millisecond to second
        velocity = (0.75 / velocity) * 3.6; // km/s//0.2 meters ||-----||
      }

      //Serial.println(velocity);

      if (t1 < time2) {
        forward = 1;
        float diff = (float)time2 - (float)t1;
        speedVeh = (float)distanceK / (float)diff;
        distanceU = (velocity * (time2 - t1)) / 10;
      } else {
        forward = 0;
        float diff = (float)t1 - (float)time2;
        speedVeh = (float)distanceK / (float)diff;
        distanceU = (velocity * (t1 - time2)) / 10;
      }
      //checked till here
      

      DateTime now = rtc.now();
      Date = String(now.day()) + ":" + String(now.month()) + ":" + String(now.year());
      Times = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
      time_now += period;
      indexlen = 11;
      loopact = 0;

      AVCNE = AVCNE + "0,";
      AVCNE = AVCNE + String(lanid) + ",";
      AVCNE = AVCNE + String(EmbeddedSeqN) + ",";
      AVCNE = AVCNE + String(Date) + ",";
      AVCNE = AVCNE + String(Times) + ",";
      if (distanceU <= 2700 && distanceU >= 1200 && ((hit) / 2) <= 2) {
        if (height >= 2) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,LCV," + stry + "," + fv + ",";

        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";
        }

      } else if (distanceU <= 3500 && distanceU >= 2700 && ((hit) / 2) <= 2) {
        if (height == 1)
        {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";

        } else {

          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "2,LCV," + stry + "," + fv + ",";

        }
      } else if (distanceU <= 7500 && distanceU >= 3500 && ((hit) / 2) <= 2) {
        if ((hit2 / 2) > 4 || (hit3 / 2) > 4) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "3,BUS," + stry + "," + fv + ",";

        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "3,TRUCK," + stry + "," + fv + ",";

        }

      } else if (((hit) / 2) == 3) {
        if (height <= 1) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";
        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "5,MAV," + stry + "," + fv + ",";
        }
      } else if (((hit) / 2) == 4) {
        if (height <= 1) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";
        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "6,MAV," + stry + "," + fv + ",";
        }
      } else if (((hit) / 2) == 5) {
        if (height <= 1) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";
        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "6,MAV," + stry + "," + fv + ",";
        }

      } else if (((hit) / 2) == 6) {
        if (height <= 1) {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "1,CAR," + stry + "," + fv + ",";
        } else {
          String stry = String(((hit) / 2));
          String fv = (forward == 1) ? "F" : "R";
          AVCNE = AVCNE + "6,MAV," + stry + "," + fv + ",";
        }

      } else if (((hit) / 2) == 7) {
        String stry = String(((hit) / 2));
        String fv = (forward == 1) ? "F" : "R";
        AVCNE = AVCNE + "7,MAV," + stry + "," + fv + ",";

      } else if (((hit) / 2) == 8) {
        String stry = String(((hit) / 2));
        String fv = (forward == 1) ? "F" : "R";
        AVCNE = AVCNE + "8,MAV," + stry + "," + fv + ",";

      } else if (((hit) / 2) >= 9 && ((hit) / 2) <= 10) {
        String stry = String(((hit) / 2));
        String fv = (forward == 1) ? "F" : "R";
        AVCNE = AVCNE + "9,MAV," + stry + "," + fv + ",";

      } else {
        String stry = String(((hit) / 2));
        String fv = (forward == 1) ? "F" : "R";
        AVCNE = AVCNE + "0,III," + stry + "," + fv + ",";


      }

      AVCNE = AVCNE + String(distanceU) + ",";

      if (height == 1) {
        AVCNE = AVCNE + "050" + ",";
        String speeder = String(velocity);
        AVCNE = AVCNE + speeder + "," + "1,0,0,0" + ",";
      }
      if (height == 2) {
        AVCNE = AVCNE + "100" + ",";
        String speeder = String(velocity);
        AVCNE = AVCNE + speeder + "," + "1,1,0,0" + ",";
      }
      if (height == 3) {
        AVCNE = AVCNE + "150" + ",";
        String speeder = String(velocity);
        AVCNE = AVCNE + speeder + "," + "1,1,1,0" + ",";
      }
      if (height == 4) {
        AVCNE = AVCNE + "200" + ",";

        String speeder = String(velocity);
        AVCNE = AVCNE + speeder + "," + "1,1,1,1" + ",";

      }
      if (height == 0) {
        AVCNE = AVCNE + "0" + ",";
        String speeder = String(velocity);
        AVCNE = AVCNE + speeder + "," + "0,0,0,0" + ",";

      }
      AVCNE = AVCNE + "END";

      server.println(AVCNE);
      SerialInterface.println(AVCNE);
      //newLineWrt(AVCNE);
      EmbeddedSeqN++;
      EEPROMWritelong(25, EmbeddedSeqN);
      AVCNE = "START,AVCC,";
      hit = 0;
      hit2 = 0;
      hit3 = 0;
      axil1 = 0;
      height = 0;
      h1 = 0;
      h2 = 0;
      h3 = 0;
      h4 = 0;
      t1 = 0;
      t2 = 0;
    }
    time1 = 0;
    time2 = 0;
    time3 = 0;
    distanceU = 0;
    speedVeh = 0;
    loopAc = 0;
    axil1 = 0;
    axil2 = 0;
    Prev_loopAc = 0;
    Prev_axil1 = 0;
    Prev_axil2 = 0;
    hit = 0;
  }
}

void checkHardReset() {
  static unsigned long st_time = millis();
  if (digitalRead(HARDRST) == LOW) {
    if (millis() - st_time > 10000) {
      if (digitalRead(HARDRST) == LOW) {
        EEPROM.write(0, 192);
        EEPROM.write(1, 168);
        EEPROM.write(2, 0);
        EEPROM.write(3, 1);
        EEPROM.write(5, 255);
        NVIC_SystemReset();
      }
    }
  }
}

// String getMessageEthernet() {
//   String str = "";
//   EthernetClient client = server.available();
//  // while (client.available()) {
//     char ch = client.read();
//     str = str + String(ch);
// //  }
//   str = str + "\0";
//   return str;
// }



String getMessageEthernet() {
  String str = "";
  char ch;
  client = server.available();
  while (client.connected() && !client.available()) {
    delay(1);  // Wait for data to be available
  }
  while (client.available()) {
    ch = client.read();
    if (ch == '\n') {
      break;  // End of message
    }
    str += ch;
  }
  return str;
}



int parseCommand(String str) {
  EthernetClient client = server.available();
  if (str.indexOf("txt") > 0)
  {
    client.setTimeout(900);
    String firstValue1 = str.substring(5, 17);
    SerialInterface.println(firstValue1);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Content-disposition:attachment; filename=" + firstValue1);
    client.println("Connection: close");
    client.println();
    //send HTTP file to client
    /*myFile = fatfs.open(firstValue1, FILE_WRITE);
      if (myFile)
      {
      while (myFile.available()) client.write(myFile.read());
      myFile.close();
      } else {
      client.println("NO RECORDS");
      }*/
    delay(100);
    client.stop();
    server.begin();
  }

  if (str != "") {
    int commaIndex = str.indexOf(',');
    int secondCommaIndex = str.indexOf(',', commaIndex + 1);
    String firstValue = str.substring(0, commaIndex);
    String secondValue = str.substring(commaIndex + 1);

    if (firstValue == "SET") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);
      int fourthCommaIndex1 = str.indexOf(',', thirdCommaIndex1 + 1);
      int fifthCommaIndex1 = str.indexOf(',', fourthCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);
      String thirdValue1 = str.substring(secondCommaIndex1 + 1, thirdCommaIndex1);
      String fourthValue1 = str.substring(thirdCommaIndex1 + 1, fourthCommaIndex1);
      String fifthValue1  = str.substring(fourthCommaIndex1 + 1, fifthCommaIndex1);

      EEPROM.write(0, secondValue1.toInt());
      EEPROM.write(1, thirdValue1.toInt());
      EEPROM.write(2, fourthValue1.toInt());
      EEPROM.write(3, fifthValue1.toInt());
  

      NVIC_SystemReset();
    }
    else if (firstValue == "SETS") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);

      EEPROM.write(5, secondValue1.toInt());

      NVIC_SystemReset();
    }
    else if (firstValue == "SETP") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);

      EEPROM.write(4, secondValue1.toInt());
      NVIC_SystemReset();
    }
    else if (firstValue == "DATE") {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);
      int fourthCommaIndex1 = str.indexOf(',', thirdCommaIndex1 + 1);
      int fifthCommaIndex1 = str.indexOf(',', fourthCommaIndex1 + 1);
      int SixCommaIndex1 = str.indexOf(',', fifthCommaIndex1 + 1);
      int SevenCommaIndex1 = str.indexOf(',', SixCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);
      String thirdValue1 = str.substring(secondCommaIndex1 + 1, thirdCommaIndex1);
      String fourthValue1 = str.substring(thirdCommaIndex1 + 1, fourthCommaIndex1);
      String fifthValue1  = str.substring(fourthCommaIndex1 + 1, fifthCommaIndex1);
      String sixthValue1  = str.substring(fifthCommaIndex1 + 1, SixCommaIndex1);
      String seventhValue1  = str.substring(SixCommaIndex1 + 1, SevenCommaIndex1);

      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      rtc.adjust(DateTime(secondValue1.toInt(), thirdValue1.toInt(), fourthValue1.toInt(), fifthValue1.toInt(), sixthValue1.toInt(), seventhValue1.toInt()));
    }
    else {
      int commaIndex1 = str.indexOf(',');
      int secondCommaIndex1 = str.indexOf(',', commaIndex1 + 1);
      int thirdCommaIndex1 = str.indexOf(',', secondCommaIndex1 + 1);

      String firstValue1 = str.substring(0, commaIndex1);
      String secondValue1 = str.substring(commaIndex1 + 1, secondCommaIndex1);

      if (firstValue1 == "R1") {
        updateRelayState(RELAY1, secondValue1.toInt());
      } else if (firstValue1 == "R2") {
        updateRelayState(RELAY2, secondValue1.toInt());
      } else if (firstValue1 == "R3") {
        updateRelayState(RELAY3, secondValue1.toInt());
      } else if (firstValue1 == "R4") {
        updateRelayState(RELAY4, secondValue1.toInt());
      } else if (firstValue1 == "R5") {
        updateRelayState(RELAY5, secondValue1.toInt());
      } else if (firstValue1 == "R6") {
        updateRelayState(RELAY6, secondValue1.toInt());
      }
    }
  }
  return -1;
}

void updateRelayState(int relay, int state) {
  digitalWrite(relay, state);
}

long EEPROMReadlong(long address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void EEPROMWritelong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

void printAlive() {
  delay(50);
  //Date = String(day()) + ":" + String(month()) + ":" + String(year());
  //Times = String(hour()) + ":" + String(minute()) + ":" + String(second());
  AliveN = AliveN + ((digitalRead(DI1) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI2) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI3) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI4) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI5) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI6) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI7) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI8) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI9) == 1) ? '1' : '0');
  AliveN = AliveN + ((digitalRead(DI10) == 1) ? '1' : '0');
  AliveN = AliveN + ",";
  AliveN = AliveN + "E";
  AliveN = AliveN + "N";
  AliveN = AliveN + "D";
  AliveN = AliveN + "\0";

  server.println(AliveN);
  SerialInterface.println(AliveN);
  AliveN = "START,ALIVE,";
  //return AliveN;
}

void setup() {
  // SerialInterface.setRx(PA3); // using pin name PY_n
  // SerialInterface.setTx(PA2);
  SerialInterface.begin(115200);
  SerialInterface.println("SETUP");
  //EEPROM.begin(64);
  // Wire.setSDA(PB9);
  // Wire.setSCL(PB8);
  // if (! rtc.begin()) {
  //   Serial.println("Couldn't find RTC");
  // }

  // rtc.adjust(DateTime(__DATE__, __TIME__));

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);
  pinMode(RELAY6, OUTPUT);

  pinMode(DI1, INPUT_PULLDOWN);
  pinMode(DI2, INPUT_PULLDOWN);
  pinMode(DI3, INPUT_PULLDOWN);
  pinMode(DI4, INPUT_PULLDOWN);
  pinMode(DI5, INPUT_PULLDOWN);
  pinMode(DI6, INPUT_PULLDOWN);
  pinMode(DI7, INPUT_PULLDOWN);
  pinMode(DI8, INPUT_PULLDOWN);
  pinMode(DI9, INPUT_PULLDOWN);
  pinMode(DI10, INPUT_PULLDOWN);

  pinMode(HARDRST, INPUT_PULLUP);

  Ethernet.init(ETHERNET_SS_PIN);

  SerialInterface.print("PORT:");
  int port = SERVER_PORT;
  if (EEPROM.read(5) < 254) {
    port = EEPROM.read(5);
  }
  SerialInterface.println(port);

  if (EEPROM.read(7) != 0) {
    EmbeddedSeqN = EEPROMReadlong(25);
  }

  SPI.setMOSI(PA7);
  SPI.setMISO(PA6);
  SPI.setSCLK(PA5);
  if (EEPROM.read(0) != 255 && EEPROM.read(1) != 255 && EEPROM.read(2) != 255 && EEPROM.read(3) != 255) {
    IPAddress SERVER_IP(EEPROM.read(0), EEPROM.read(1), EEPROM.read(2), EEPROM.read(3));
    lanid = EEPROM.read(3);
    Ethernet.begin(SERVER_MAC, SERVER_IP);
  } else {
    IPAddress SERVER_IP(ip_array[0], ip_array[1], ip_array[2], ip_array[3]);
    lanid = 125;
    Ethernet.begin(SERVER_MAC, SERVER_IP);
  }
  server.begin();
  
  SerialInterface.print("My IP:");
  server.println(Ethernet.localIP());
  SerialInterface.println(Ethernet.localIP());

  updateRelayState(RELAY1, HIGH);
  updateRelayState(RELAY2, HIGH);
  updateRelayState(RELAY3, HIGH);
  updateRelayState(RELAY4, HIGH);
  updateRelayState(RELAY5, HIGH);
  updateRelayState(RELAY6, HIGH);
}

void loop() {
  String str = getMessageEthernet();
  parseCommand(str);
  printAlive();
  checkHardReset();
  avcc();
}