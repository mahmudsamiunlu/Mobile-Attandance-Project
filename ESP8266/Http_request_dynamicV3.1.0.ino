
#define Version "v3.1.0"

/**
  2015 Mahmud Sami Ünlü

  Butonla ders seciliyor
  Serial.timeout konuldu
  RFID Konuldu
  RFID ile birlikte butonlar çalışıyor
  LCD eklendi
  LCD Menü eklendi
  Pinler degisti
  LCD RESET=>VCC SCLK => GPIO15
  RFID RESET =>VCC
  Menude duzenleme yapildi
  Startattendance da gönderi yaparken MasterUid ile birlikte gonderiyoruz
  2.9.2
  Menu Duzenlemesi
  2.9.4
  EEPROM Düzeltildi ve Calisiyor
  Siteden Alternatif SSID Ve PASS Alıyor
  Yanlislikla secme butonu basınca secmesi düzeltildi
  Alternatif Wifiye gecince ekranda alternative yaziyor
  İslem sırasında wifi bağlantısı koparsa otomatik tekrar bağlanıyor  //** İşlemi unutmadan tekrarlıyor böylelikle işlemde bir aksama olmuyor.
  2.9.7
  Auto Update Yapıldı
  Requestlerde http -1,-11 dönerse tekrar request yapıyor. (Galiba timeouta düşüyor -1de )
  3.0.0
  Stabil versiyon
  Update kontrol ederken ekrana bildirim veriyor
  3.0.1
  Ekranda düzeltilmeler yapıldı
  Ekranda versiyon yazıyor ve update sırasında updating yazıyor
  3.0.2
  Stepler arasında internet koparsa tekrar bağlanıyor
  3.0.3
  Menü düzeltildi
  v3.0.3.1
  ESP12E de LCD reset pin 9. pinde iken wdt atıyordu. rst pinini 1. pine alarak iptal ettik.
  v3.0.3.2
  Adafruit Kütüphanesi eklendi ve lcd adafruitin kütüphanesi ile yapılıyor
  3.0.3.3
  Adafruit Kütüphanesi kullanarak menüler düzenleniyor
  Contrast 45te bazen güzel bazen 60 ta güzel bunu sıkça kontrol et
  Dersleri seçerken seçmek istediginiz dersin yazısı siyah-beyaz oluyor
  Attandance büyük bir biçimde yazabiliyoruz
  3.1.0
  Menude deki ufak değişiklikler
  Dersi seçtiginde onay ekranı


*/


/*      İleriki Planlar
   Ekran Yerleşiminin düzeltilmesi
   websiteyi globala çekilmesi
   updatete bazen buga giriyor, bu durumda wdt reset atıyor fakat orda takılı kalıyor bunun çözülmesi. (10 testte 1 kere)
   IR eklenmesi.
   Şarj devresi, şarj indicatörü.



*/

/*    Devam Eden Problemler
   Ekranda kararma oluyor
   Request süresi uzun.

   ß

*/


//      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
//       {"User":"Ramazan","IdList":[1,2,3],"CodeList":["CS101","MATH151","MATH152"]}

//Step1  http://webattendance.azurewebsites.net/Arduino/Login?data={"CardId":"a9a3222b"}
//Step3  http://webattendance.azurewebsites.net/Arduino/Startattendance?data={"ActivityId":"2"}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>
#include "MFRC522.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#define WebSite "muhyoklama.zirve.edu.tr"
#define CompanyName "TECHNOSMM"
//#define WebSite "webattendance.azurewebsites.net"
// muhyoklama.zirve.edu.tr

ESP8266WiFiMulti WiFiMulti;
/*
  static const uint8_t D0   = 16;
  static const uint8_t D1   = 5;
  static const uint8_t D2   = 4;
  static const uint8_t D3   = 0;
  static const uint8_t D4   = 2;
  static const uint8_t D5   = 14;
  static const uint8_t D6   = 12;
  static const uint8_t D7   = 13;
  static const uint8_t D8   = 15;
  static const uint8_t D9   = 3;
  static const uint8_t D10  = 1;
*/
// FOR RFID
/* wiring the MFRC522 to ESP8266 (ESP-12)
  RST     = GPIO15      D8                     //   RESET BAGLANMIYOR
  SDA(SS) = GPIO2       D4
  MOSI    = GPIO13      D7
  MISO    = GPIO12      D6
  SCK     = GPIO14
  GND     = GND
  3.3V    = 3.3V


*/

//For LCD
#define PIN_SCE   5  //D1
#define PIN_RESET 10  //D2  Reset to VCC // BAGLANMAYACAK // 9 du 1 aldık. 
#define PIN_DC    0  //D3
#define PIN_SDIN  2  //D4
#define PIN_SCLK  15 //D8
#define PIN_BL    13 //D6

#define RST_PIN  3 // RST-PIN für RC522 - RFID - SPI - Modul GPIO15 
#define SS_PIN  2  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2 


MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522

//char* ssid     = "Test";
//char* password = "0123456789";
char* ssid     = "TECHNOSMM 3G";
char* password = "88ofmsu99";
String WifiSSID = "";
String WifiPASS = "";
const int buttonPin1 = 16;     // the number of the pushbutton pin
const int buttonPin2 = 4;     // the number of the pushbutton pin
int WifiTimeoutTime = 27;
unsigned const int CursorY = 24;

Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);

//ESP8266_Nokia5110 display = ESP8266_Nokia5110(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_SCE, PIN_RESET);


/*

  // draw a single pixel
  display.drawPixel(10, 10, BLACK);
  display.display();
  delay(2000);
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Hello, world!");
  display.setTextColor(WHITE, BLACK); // 'inverted' text
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.println("0x"); display.print(0xDEADBEEF, HEX);
  display.display();
  delay(2000);
  // draw many lines
  // display.display(); // show splashscreen
   display.clearDisplay();
*/

void setup() {
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);
  display.display();
  display.clearDisplay();   // clears the screen and buffer




  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  EEPROM.begin(128);
  delay(1000);
  Serial.begin(115200);
  Serial.println("TEST");
  delay(1000);
  Serial.setTimeout(100);

  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.print(Version);
  display.setTextColor(BLACK);
  display.display();
  display.display();

  //  delay(3000);
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  // USE_SERIAL.setDebugOutput(true);
  //WiFiMulti.addAP("TECHNOSMM_3G", "technosmm8899");
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  Serial.println();
  Serial.println();


}

String MasterUID = ""; //"123456";        //input
String MasterName = "";
String StudentUID = ""; //"1234";       //input
String StudentName = "";
String StudentAttandance = "";
String MasterUsername = "";
String CourseIDlist = "";
String CourseIDlistInput = ""; //"2";               //input
String Courselist[9][2];
int AttendanceId;
int Data;
int Error;
boolean yoklama = true;
boolean waitrfid = true;
String Status = "";
String Step1 = "";
String Step3 = "";
String Step5 = "";
String Step7 = "";
int arrayread = 1;
int DeviceUpdate = 0;
String fileLocation = "";
int RequestTimeOut = 5;
boolean doonce = true;

//String Step1="";

boolean CheckUpdate(String DeviceVersion);
void updateDevice();

void ReadWifiConf(String &esid, String &epass ) {
  esid = "";
  epass = "";
  int ssidLength = int(EEPROM.read(0));
  int passLength = int(EEPROM.read(1));
  Serial.println("Reading EEPROM");
  for (int i = 2; i < 2 + ssidLength; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.print("SSID: ");
  Serial.println(esid);

  for (int i = 34; i < 34 + passLength; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.println("PASS: ");
  Serial.println(epass);



  Serial.println("Reading EEPROM DONE!");




}

void WriteWifiConf(String qsid, String qpass) {
  Serial.println("clearing eeprom");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
    delay(1);
  }
  EEPROM.write(0, int(qsid.length()));
  EEPROM.write(1, int(qpass.length()));
  Serial.println("writing eeprom ssid: ");
  for (int i = 0; i < qsid.length(); ++i)
  {
    EEPROM.write(2 + i, qsid[i]);
    Serial.print(qsid[i]);
  }
  Serial.println();
  Serial.println("writing eeprom pass: ");
  for (int i = 0; i < qpass.length(); ++i)
  {
    EEPROM.write(34 + i, qpass[i]);
    Serial.print(qpass[i]);
  }
  Serial.println();
  EEPROM.commit();
}



void JsonParser(char buf1[], int IdListArray, int CodeListArray, int whichstep) {
  StaticJsonBuffer<256> jsonBuffer;
  //  JsonObject* root;
  //root = &jsonBuffer.parseObject(json);
  JsonObject& root = jsonBuffer.parseObject(buf1);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if (whichstep == 0)
  {
    //Update Step
    int bufupdate = root["update"];
    String bufLocation = root["Location"];
    DeviceUpdate = bufupdate;
    fileLocation = bufLocation;
  }

  if (whichstep == 1)
  {
    int bufData = root["Data"];
    String bufMasterName = root["User"];
    String bufWifiSSID = root["SSID"];
    String bufWifiPASS = root["WPass"];
    if (bufData == 1) {
      WriteWifiConf(bufWifiSSID, bufWifiPASS);
      ReadWifiConf(WifiSSID, WifiSSID);

    }
    //String bufCourselist  = root["CodeList"][1];
    //Serial.print(bufCourselist);

    while (root["IdList"][arrayread] != 0) {
      //Serial.println("icindeyim");                                //debug
      arrayread++;
      delay(5);
    }
    //Serial.println(arrayread);                                   //debug
    int show = 0;
    arrayread--;
    while ( show <= arrayread) {                                                    // arrayread otomatik bulucak
      String bufCourselist  = root["CodeList"][show];
      String bufIdList = root["IdList"][show];
      // Serial.print(bufCourselist); Serial.println(bufIdList);
      Courselist[show][0] = bufCourselist;
      Courselist[show][1] = bufIdList;
      show++;
      delay(5);
    }
    //arrayread++;
    Data = bufData;
    MasterName = bufMasterName;
    // CourseIDlist=bufCourseIDlist;
    // Courselist=bufCourselist;
  }
  else if (whichstep == 3) {
    int bufAttendanceId =  root["AttendanceId"];
    AttendanceId = bufAttendanceId;
  }
  else if (whichstep == 5) {
    int bufError = root["Error"];
    String   bufStudentName = root["Name"];
    int bufStudentAttandance = root["Attendance"];
    Error = bufError;
    StudentName = bufStudentName;
    StudentAttandance = bufStudentAttandance;
  }
  else if (whichstep == 7) {
    String bufStatus = root["Status"];
    Status = bufStatus;
  }
}
//Serial.print("MasterUID"); Serial.print(MasterUID); Serial.print(" "); Serial.print("CourseIDlist"); Serial.print(CourseIDlist); Serial.print(" "); Serial.print("Courselist"); Serial.print(Courselist); Serial.print("Data"); Serial.println(Data);


boolean InitializeNetwork() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    return 1;
  }
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println(Version);
  display.setTextColor(BLACK);
  display.display();
  display.print("Connecting ");
  display.display();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println();
  WiFi.begin(ssid, password);
  String connetcting = "";
  boolean WifiTimeout = 1;
  int WifiTimeoutCount = 0;
  while ((WiFi.status() != WL_CONNECTED) && WifiTimeout) {

    delay(500);
    Serial.print(". ");
    display.print(".");
    display.display();
    WifiTimeoutCount++;
    if (WifiTimeoutCount > WifiTimeoutTime)
      WifiTimeout = 0;
  }
  Serial.println();
  delay(1000);

  if (WifiTimeoutCount > WifiTimeoutTime) {
    // JsonParser("{\"User\":\"Serdar\",\"IdList\":[6,10,10],\"CodeList\":[\"CS102\",\"AI301\",\"AI301\"],\"SSID\":\"TECHNOSMM_3G\",\"WPass\":\"Asdasdasd\",\"Data\":1}", 0, 0, 0);
    //WriteWifiConf("", "");
    display.clearDisplay();
    display.setTextColor(WHITE, BLACK);
    display.setCursor(0, 0);
    display.println(Version);
    display.setTextColor(BLACK);
    display.print("Alternative ");
    display.display();
    WiFi.disconnect();
    delay(30);
    Serial.println("Alternatif Wifi Hafızadan Okunuyor");
    WifiTimeoutCount = 0;
    WifiTimeout = 1;
    ReadWifiConf(WifiSSID, WifiPASS);
    //display.setCursor(0, 0);
    //display.println(WifiSSID);
    // WriteWifiConf(WifiSSID,WifiPASS);

    //  WifiSSID.toCharArray(ssid, WifiSSID.length());
    // WifiPASS.toCharArray(password, WifiPASS.length());
    Serial.println("Alternatif Wifi Deneniyor ");  Serial.print("SSID: "); Serial.print(WifiSSID); Serial.println(" PASS: "); Serial.println(WifiPASS);
    //EEPROM.write(95, 0);
    ///  EEPROM.commit();

    WiFi.begin(WifiSSID.c_str() ,          WifiPASS.c_str());

    while (WiFi.status() != WL_CONNECTED) {

      delay(500);
      Serial.print(". ");
      display.print(".");
      display.display();
    }
  }
  connetcting = "";

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected");
  display.println("Checking ");
  display.println("Update");
  display.display();
  if (doonce) {
    if (CheckUpdate(Version)) {
      display.setTextColor(WHITE, BLACK);
      display.println("Updating");
      display.setTextColor(BLACK);
      display.display();
      InitializeNetwork();
      updateDevice();
    }
    else {
      display.setTextColor(WHITE, BLACK);
      display.println("No Update");
      display.setTextColor( BLACK);
    }
    display.display();
    delay(1500);


    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(CompanyName);
    display.println("Enter Master");
    display.display();
    // display.clearDisplay();
    doonce = false;
  }
}



void Request(String request, int whichstep) {
  int counter = 0;
RErequest:
  Serial.println(request);                                                     //debug
  HTTPClient http;
  http.begin("muhyoklama.zirve.edu.tr", 80, request ); //HTTP
  int httpCode = http.GET();
  if (httpCode) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println(payload);                                                 //debug
      char buf[payload.length() + 1];
      payload.toCharArray(buf, payload.length() + 1);
      JsonParser(buf, CourseIDlistInput.toInt(), CourseIDlistInput.toInt(), whichstep);
      // Serial.print(buf);
      // Serial.println(payload);
    }

  } else {
    Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
  }
  if ((httpCode == -1 || httpCode == -11) && (counter < RequestTimeOut)) {
    counter++;  //wifi kopunca http -1 de dönüyor
    InitializeNetwork();
    delay(200);
    goto RErequest;
  }
}

void updateDevice() {
  String  UpdatePath = "http://"; UpdatePath += WebSite + fileLocation;
  Serial.println(UpdatePath);
  t_httpUpdate_return ret = ESPhttpUpdate.update(UpdatePath);
  //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("HTTP_UPDATE_FAILD");
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

boolean CheckUpdate(String DeviceVersion) {
  //Check Request For Update
  // http://webattendance.azurewebsites.net/Arduino/GetUpdate?data={"VersionNumber":"v1.3.5"}
  String UpdateReqest = "/Arduino/GetUpdate?data={\"VersionNumber\":\"" + DeviceVersion + "\"}";
  Request(UpdateReqest, 0);
  if (DeviceUpdate == 1) {
    Serial.println("Updateing ");

    return 1;

  }
  else {
    Serial.println("No Update");
    return 0;
  }

}

String reaDrfidcard() {
  String uids = "";
  for (int i = 0; i < mfrc522.uid.size; i++) {
    // readCard[i] = mfrc522.uid.uidByte[i];
    uids += String(mfrc522.uid.uidByte[i], HEX);
    //Serial.print(readCard[i], HEX);
  }
  return uids;
}

void loop() {
  while (!(InitializeNetwork())) {
    delay(1);
  }
  //Serial.println(MasterUID+" "+StudentUID+" "+CourseIDlistInput);

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.print("Enter Master UID : ");
    //  display.setCursor(0,1);
    //  display.println("Enter Master");
    MasterUID = ""; //S1001
    StudentUID = ""; //1234   S1010
    CourseIDlistInput = "";

    while (waitrfid) {
whileloop:
      if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
        delay(50);
        goto whileloop;
      }
      Serial.println(reaDrfidcard());
      MasterUID = reaDrfidcard();
      if (MasterUID != "") waitrfid = false;
      if (waitrfid) {
        mfrc522.PICC_HaltA();
        if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
          delay(50);
          goto whileloop;
        }
      }
    }
    //Serial.println(MasterUID);
    waitrfid = true;
    // MasterUID="020112004";
    display.clearDisplay();
    display.println(CompanyName);
    display.println("Requesting  ");
    display.display();

    Step1 = "";
    Step1 += "/Arduino/Login?data={\"CardId\":\""; Step1 += MasterUID; Step1 += "\"}";

    Step5 = ""; // "/Arduino/Saveattendance?data={\"AttendanceId\":\"3\",\"UserId\":\"1234\"}";
    Step7 = ""; //"/Arduino/Endattendance?data={\"AttendanceId\":\"3\"}";
    Request(Step1, 1);
CorrectSellect:
    if (!Data == 0) {
      //Serial.println("MASTER");

      Serial.print("MasterName : ");
      Serial.print(MasterName);
      Serial.println(" CourseList : ");
      display.clearDisplay();
      display.println(CompanyName);
      display.setTextColor(WHITE, BLACK);
      display.println(MasterName);
      display.setTextColor( BLACK);
      display.println("CourseList :");

      int cursorY = 24;  // Menunun baslangici  ( karakterler 6*8 )

      for (int i = 0; i <= arrayread; i++) {
        Serial.println(Courselist[i][0] + " " + Courselist[i][1]); // Serial.print(" Courselist : "); Serial.println(Courselist); //Serial.print("Data"); Serial.println(Data);

        display.setCursor((i % 2) * 36, cursorY); // Her satırda 2 ders kodu oldugundan % kalanını aldık. X=6pixel
        display.print(Courselist[i][0]);

        if ((i % 2) == 1) {
          cursorY += 8;
        }
      }
      display.display();
      Serial.println("Enter Course code : ");
      // display.clearDisplay();
      int CcursorY = CursorY;
      int a = 0;
      while (!(digitalRead(buttonPin2) == 1) || !(a > 0)) {

        if (digitalRead(buttonPin1) == 1) {

          display.clearDisplay();
          display.println(CompanyName);
          display.setTextColor(WHITE, BLACK);
          display.println(MasterName);
          display.setTextColor( BLACK);
          display.println("CourseList :");
          cursorY = CursorY;
          for (int i = 0; i <= arrayread; i++) {

            display.setCursor((i % 2) * 36, cursorY); // Her satırda 2 ders kodu oldugundan % kalanını aldık. X=6pixel  ( karakterler 6*8 )
            display.print(Courselist[i][0]);

            if ((i % 2) == 1) {
              cursorY += 8;
            }
          }
          display.display();

          boolean reset = false;        // Bu variable Cursoru tekrar 1. dersin başına getirebilmek için.
          if (a > arrayread) {
            a = 0;  //  a> arrayread olucak burası
            CcursorY = CursorY;
            //reset = true;
          }
          Serial.println( Courselist[a][0] + "  " + a );
          display.setCursor((a % 2) * 36, CcursorY); // Her satırda 2 ders kodu oldugundan % kalanını aldık. X=6pixel  ( karakterler 6*8 ) 14*6 karakter
          display.print(' ' * (Courselist[a][0]).length());
          display.setTextColor(WHITE, BLACK);
          display.setCursor((a % 2) * 36, CcursorY); // Her satırda 2 ders kodu oldugundan % kalanını aldık. X=6pixel  ( karakterler 6*8 )
          display.print(Courselist[a][0]);
          display.setTextColor(BLACK);

          if ((a % 2) == 1) {
            CcursorY += 8;
          }
          display.display();
          //   if (reset) {
          //    reset = false;
          //  }
          /* display.clearDisplay();
            display.println(CompanyName);
            display.println(MasterName);  display.println("CourseList :");
            display.setTextColor(WHITE, BLACK);
            display.print(Courselist[a][0]);
            display.setTextColor(BLACK);
            display.print("  ");
            display.println(Courselist[a + 1][0]);
            display.display();

            Serial.println( Courselist[a][0]);
            //if(a>0){display.setCursor(0,2+a); display.println("           "); }
            //  display.println(Courselist[a][0]);
            display.display();
          */

          a++;
          delay(500);
        }
        delay(10);
      }
      a--;

      String ChosenCourse = Courselist[a][0];
      display.clearDisplay();
    
      display.println(CompanyName);
      display.println(MasterName);
      display.setTextSize(2);
      display.setTextColor(WHITE, BLACK);
      display.drawFastVLine(2, 16, 18, BLACK);
      display.drawFastVLine(1, 16, 18, BLACK);
      display.drawFastHLine(2, 17, 61, BLACK);
      display.drawFastHLine(2, 16, 61, BLACK);
      display.setCursor(3, 18);
      display.println(ChosenCourse);
      display.setTextColor( BLACK);
      display.setTextSize(1);
      display.setCursor(0, 40);
      display.setTextColor(WHITE, BLACK);
      display.print(" YES ");
      display.setTextColor(BLACK);
      display.print("    ");
      display.setTextColor(WHITE, BLACK);
      display.print(" NO ");
      display.display();
      display.setTextColor(BLACK);
      delay(500);
      while (!(digitalRead(buttonPin1) == 1) && !(digitalRead(buttonPin2) == 1)) {
        delay(1);
      }
      if (digitalRead(buttonPin2) == 1)
        goto CorrectSellect;






      display.clearDisplay();
      display.println(CompanyName);
      display.println("Chosen :");
      display.setTextColor(WHITE, BLACK);
      display.println(ChosenCourse);
      display.setTextColor(BLACK);
      display.println("Attendance");
      display.println("Begin!");
      display.display();

      Serial.print("Chosen Course : ");
      Serial.println(ChosenCourse);
      CourseIDlistInput = Courselist[a][1];
      // /StartAttendance?data={"ActivityId":1,"CardId":"123456"}
      Step3 = "";
      Step3 += "/Arduino/Startattendance?data={\"ActivityId\":\""; Step3 += CourseIDlistInput.toInt(); Step3 += "\",\"CardId\":\"" + MasterUID + "\"}";
      Request(Step3, 3);
      Serial.print("AttendanceId : "); Serial.println(AttendanceId);
      delay(2000);
      display.clearDisplay();
      display.println(CompanyName);
      display.println("Read Student");
      display.display();

      while (yoklama) {
        Serial.print("Student ID : ");
        //display.setCursor(0,1); display.println("Student ID");

        while (waitrfid) {
whileloop1:
          if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
            delay(50);
            goto whileloop1;
          }
          Serial.println(reaDrfidcard());
          StudentUID = reaDrfidcard();
          if (StudentUID != "") waitrfid = false;
          if (waitrfid) {
            mfrc522.PICC_HaltA();
            if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
              delay(50);
              goto whileloop1;
            }
          }
        }
        //Serial.println(MasterUID);
        waitrfid = true;

        display.clearDisplay();
        display.println(CompanyName);
        display.println("Requesting  ");
        display.display();

        Serial.println(StudentUID);
        if (StudentUID != MasterUID) {
          Step5 += "/Arduino/Saveattendance?data={\"AttendanceId\":\""; Step5 += AttendanceId; Step5 += "\",\"UserId\":\""; Step5 += StudentUID; Step5 += "\"}";
          Request(Step5, 5);
          Step5 = "";
        }
        else yoklama = false;

        if (!Error == 1 && yoklama) {
          display.clearDisplay();
          display.println(CompanyName);
          //display.setCursor(0, 1); display.println("Name :");
          display.setTextColor(WHITE, BLACK);
          display.println(StudentName);
          display.setTextColor(BLACK);
          display.println("Attendance ");
          display.setTextSize(2);
          display.setCursor(2, 26);
          display.print(StudentAttandance);
          display.setTextSize(1);
          display.display();

          //display.setCursor(0, 4); display.println(StudentAttandance);
          Serial.print ("Student Name : "); Serial.print(StudentName); Serial.print(" Student Attendance : "); Serial.println(StudentAttandance);

          delay(2000);
          display.clearDisplay();
          display.println(CompanyName);
          display.println("Read Student");
          display.display();

        }
        if (Error == 1 && yoklama) {
          Serial.println("Student Not Registered");
          display.clearDisplay();
          display.println(CompanyName);
          display.println("Student Not Registered");
          display.display();

          delay(1000);

          display.clearDisplay();
          display.println(CompanyName);
          display.println("Read Student");
          display.display();
        }
      }
      Step7 += "/Arduino/Endattendance?data={\"AttendanceId\":\""; Step7 += AttendanceId; Step7 += "\"}";
      Request(Step7, 7);
      display.clearDisplay();
      display.println(CompanyName);
      display.println("Record :");
      display.setTextColor(WHITE, BLACK);
      display.println(Status);
      display.setTextColor(BLACK);
      display.display();

      Serial.print ("Attendance Record : "); Serial.println(Status);
      Serial.println("Attendance done!");
      delay(2500);
      display.clearDisplay();
      display.println(CompanyName);
      display.println("Enter Master");
      display.display();
    }
    else {
      display.clearDisplay();
      display.println(CompanyName);
      display.println("Master not found");
      display.display();
      Serial.print("Master not found Data=");
      Serial.println(Data);
      delay(1000);
      display.clearDisplay();
      display.println(CompanyName);
      display.println("Enter Master ");
      display.display();

    }
    yoklama = true;
    for (int i = 0; i <= arrayread; i++) {
      Courselist[i][0] = "";
      Courselist[i][1] = "";
    }
    arrayread = 0;

  }

  delay(50);
}


