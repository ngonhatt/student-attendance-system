#include <Keypad_I2C.h>          // Sử dụng thư viện Keypad_I2C cho keypad qua giao thức I2C
#include <LiquidCrystal_I2C.h>  // LCD
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>    // RFID
#include <HTTPSRedirect.h>
#include <RTClib.h>               // Sử dụng thư viện RTClib cho đồng hồ thời gian thực

#define I2CADDR 0x20   // Gán địa chỉ I2C



#define ROW_NUM     4 // Số hàng
#define COLUMN_NUM  4 // Số cột

// Định nghĩa chân cho BUTTON
#define BUTTON_PIN  33 // Chân nút

// Định nghĩa chân cho RFID
#define SS_PIN    5  // ESP32 pin GPIO32 
#define RST_PIN  4 // ESP32 pin GPIO5

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbyB_PFtjJ3xrzeLXIRD_XXUoRjPZexSNz-ad50bFuq5HgshAxHE8O19EjhBF8C6Upev";
//---------------------------------------------------------------------------------------------------------
// Enter network credentials:
const char* ssid     = "Uchihahahaha";
const char* password = "Minhtien@16";
//---------------------------------------------------------------------------------------------------------
// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Điểm danh\", \"values\": ";
String payload = "";
//---------------------------------------------------------------------------------------------------------
// Google Sheets setup (do not edit)
const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;
//------------------------------------------------------------
// Declare variables that will be published to Google Sheets
String student_id;
String student_name;
//------------------------------------------------------------
int blocks[] = {4, 5};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))
//------------------------------------------------------------

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM]       = {0, 1, 2, 3};
byte pin_columns[COLUMN_NUM] = {4, 5, 6, 7};

Keypad_I2C keypad(makeKeymap(keys), pin_rows, pin_columns, ROW_NUM, COLUMN_NUM, I2CADDR, PCF8574);

LiquidCrystal_I2C lcd(0x27, 20, 4); // Địa chỉ I2C 0x27, màn hình 20 cột và 4 dòng

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
//------------------------------------------------------------
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Length of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];
// RTC
RTC_DS1307 rtc;


// Mảng chứa tên các ngày trong tuần
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

byte authorizedUID[4] = {0xBB, 0x8E, 0xB8, 0x89};
byte authorizedUID2[4] = {0xA1, 0x3F, 0x9A, 0x8A};

bool mode = false; // Lựa chọn chế độ Nhập MSSV hay quét mã
String input_password; // Chuỗi lưu MSSV
const String password_1 = "21119112";
const String password_2 = "21119140";
String new_password[10] ;
int i=0;
int cursorColumn = 0; // Vị trí cột con trỏ
int Buzzer    = 32;   
int flag1=0;
int flag2=0;

void setup() {
  Serial.begin(9600);
  lcd.init(); // Khởi tạo màn hình LCD
  lcd.backlight(); // Bật đèn nền
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  keypad.begin(makeKeymap (keys));
 // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("WiFi...");
  //----------------------------------------------------------
  // Connect to WiFi
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  //----------------------------------------------------------
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  //----------------------------------------------------------
  lcd.clear();
  lcd.setCursor(0,0); //col=0 row=0
  lcd.print("Connecting to");
  lcd.setCursor(0,1); //col=0 row=0
  lcd.print("Google ");
  delay(5000);
  //----------------------------------------------------------
  Serial.print("Connecting to ");
  Serial.println(host);
  //----------------------------------------------------------
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for(int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    //*************************************************
    if (retval == 1){
      flag = true;
      String msg = "Connected. OK";
      Serial.println(msg);
      lcd.clear();
      lcd.setCursor(0,0); //col=0 row=0
      lcd.print(msg);
      delay(2000);
      break;
    }
    //*************************************************
    else
      Serial.println("Connection failed. Retrying...");
    //*************************************************
  }
  //----------------------------------------------------------
  if (!flag){
    //____________________________________________
    lcd.clear();
    lcd.setCursor(0,0); //col=0 row=0
    lcd.print("Connection fail");
    //____________________________________________
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    delay(5000);
    return;
    //____________________________________________
  }
  //----------------------------------------------------------
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
  //----------------------------------------------------------
  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Cấu hình nút là INPUT_PULLUP để sử dụng nút nhấn có đầu vào nội kéo lên
  pinMode(Buzzer, OUTPUT); // Cấu hình nút là INPUT_PULLUP để sử dụng nút nhấn có đầu vào nội kéo lên
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  begin();

}

void begin() {
  lcd.setCursor(0,2);
  lcd.print("                    ");
  lcd.setCursor(0,3);
  lcd.print("                    ");
  RTC();
  lcd.setCursor(0, 2);
  lcd.print("Chon che do:");
  lcd.setCursor(0, 3);

  lcd.print("1.Nhap MSSV |2.RFID");
  delay(2000);
}

void pass(){
  String values = "";
char key = keypad.getKey();
    if (key) {
      if (key == '*') { // Nếu nhấn phím *
        if (cursorColumn > 0) {
        digitalWrite (Buzzer, HIGH);                            
        delay (150);                                            
        digitalWrite (Buzzer, LOW);
        cursorColumn--; // Di chuyển con trỏ về phía trước
        lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ LCD
        lcd.print(" "); // In một ký tự trống để xóa
        lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ trở lại vị trí ban đầu
        if (!input_password.isEmpty()) {
          input_password.remove(input_password.length() - 1); // Xóa ký tự cuối cùng khỏi chuỗi nhập
        }
      } 
          } else if (key == 'D') { // If key pressed is 'D'
        
        bool passwordMatched = false;
        digitalWrite (Buzzer, HIGH);                            
        delay (150);                                            
        digitalWrite (Buzzer, LOW);
        // Check if the input password matches the predefined passwords
        if (input_password == password_1) {
            flag1 = 1;
            student_name = "N.H.Minh Nhat";
            passwordMatched = true;
           
        } else if (input_password == password_2) {
            flag1 = 1;
            student_name = "P.Minh Tien";
            passwordMatched = true;
            
        } else {
            // Check if the input password matches any of the new passwords
            for (int j = 0; j < i; j++) {
                if (input_password == new_password[j]) {
                    flag1 = 1;
                    student_name = "New Student";
                    passwordMatched = true;
                   
                    break;
                }
            }
        }
        
        if (passwordMatched) {
            cursorColumn = 0;
            input_password = "";
            mode = false;

        } else {
            flag2 = 1;
            student_name = "UNKNOWN";
            student_id = "UNKNOWN";
            
            cursorColumn = 0;
            input_password = "";
            mode=false;
            delay(2000);
        }
    } else if (cursorColumn < 8) { // Nếu độ dài chuỗi nhập chưa vượt quá maxCodeLength
          input_password += key;
          student_id = input_password;
          lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ đến vị trí (cursorColumn, 0)
          lcd.print(key); // In ký tự được nhấn vào màn hình LCD
          digitalWrite (Buzzer, HIGH);                            
          delay (150);                                            
          digitalWrite (Buzzer, LOW);
          cursorColumn++; // Di chuyển con trỏ đến vị trí tiếp theo 
    }
    if (flag1)
    {
    values = "\"" + student_id + ","+ student_name + "\"}";
    payload = payload_base + values;

    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,2);
    lcd.print("Publishing data...");
    
    if(client->POST(url, host, payload)){ 
      // do stuff here if publish was successful
      
    } else {
      // do stuff here if publish was not successful
    }
    //----------------------------------------------------------------
    // a delay of several seconds is required before publishing again    
    delay(100);
    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
    lcd.print("Ma so SV: "); // In một ký tự trống để xóa
    lcd.setCursor(10, 2); // Di chuyển con trỏ LCD
    lcd.print(student_id); // In một ký tự trống để xóa
    lcd.setCursor(0,3); // Di chuyển con trỏ LCD
    lcd.print("Diem danh thanh cong"); // In một ký tự trống để xóa
    digitalWrite (Buzzer, HIGH);                            
    delay (500);                                            
    digitalWrite (Buzzer, LOW);
    delay(2000); // Hiển thị thông báo trong 2 giây
    lcd.clear(); // Xóa màn hình sau khi hiển thị thông báo
    flag1=0;
    begin();
    }
    if (flag2)
    {
    values = "\"" + student_id + ","+ student_name + "\"}";
    payload = payload_base + values;
    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,2);
    lcd.print("Publishing data...");
    
    if(client->POST(url, host, payload)){ 
      // do stuff here if publish was successful
      
    } else {
      // do stuff here if publish was not successful
    }
    //----------------------------------------------------------------
    // a delay of several seconds is required before publishing again    
    delay(100);
    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
    lcd.print("MSSV khong hop le"); // In một ký tự trống để xóa
    lcd.setCursor(0,3); // Di chuyển con trỏ LCD
    lcd.print("Diem danh that bai"); // In một ký tự trống để xóa
    digitalWrite (Buzzer, HIGH);                            
    delay (1000);                                            
    digitalWrite (Buzzer, LOW);
    delay(2000); // Hiển thị thông báo trong 2 giây
    lcd.setCursor(0,2);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("                    ");
    flag2=0;
    lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
    lcd.print("Muon DK MSSV moi ? "); // In một ký tự trống để xóa
    lcd.setCursor(0,3); // Di chuyển con trỏ LCD
    lcd.print("1: Co | 0: Khong"); // Hiển thị lựa chọn

  char key = keypad.getKey();
  while (key != '1' && key != '0') {   // Vòng lặp sẽ tiếp tục cho đến khi một phím '1' hoặc '0' được nhấn
    key = keypad.getKey();
  }


  if (key == '1') { 
  digitalWrite(Buzzer, HIGH);
  delay(150);
  digitalWrite(Buzzer, LOW);
  lcd.setCursor(0, 2);
  lcd.print("                    "); // Xóa dòng 2
  lcd.setCursor(0, 3);
  lcd.print("                    "); // Xóa dòng 3
    while (true) {
      key = keypad.getKey();
      lcd.setCursor(0, 2); // Di chuyển con trỏ đến vị trí (cursorColumn, 0)
      lcd.print("Moi nhap MSSV moi: "); // In ký tự được nhấn vào màn hình LCD
      if (key == '*') { // Nếu nhấn phím *
        if (cursorColumn > 0) {
          digitalWrite(Buzzer, HIGH);
          delay(150);
          digitalWrite(Buzzer, LOW);
          cursorColumn--; // Di chuyển con trỏ về phía trước
          lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ LCD
          lcd.print(" "); // In một ký tự trống để xóa
          lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ trở lại vị trí ban đầu
          if (!input_password.isEmpty()) {
            input_password.remove(input_password.length() - 1); // Xóa ký tự cuối cùng khỏi chuỗi nhập
          }
        }
      } else if (cursorColumn < 8 && key && key != 'D') { // Nếu độ dài chuỗi nhập chưa vượt quá maxCodeLength
        input_password += key;
        student_id = input_password;
        lcd.setCursor(cursorColumn, 3); // Di chuyển con trỏ đến vị trí (cursorColumn, 0)
        lcd.print(key); // In ký tự được nhấn vào màn hình LCD
        digitalWrite(Buzzer, HIGH);
        delay(150);
        digitalWrite(Buzzer, LOW);
        cursorColumn++; // Di chuyển con trỏ đến vị trí tiếp theo
      } else if ( key == 'D' ) {
        digitalWrite(Buzzer, HIGH);
        delay(150);
        digitalWrite(Buzzer, LOW);
        new_password[i++] = input_password;
        student_id = input_password;
        
        lcd.setCursor(0, 2);
        lcd.print("                    "); // Xóa dòng 3 
        lcd.setCursor(0, 3);
        lcd.print("                    "); // Xóa dòng 3 
        lcd.setCursor(0, 2); // Di chuyển con trỏ đến vị trí (cursorColumn, 0)
        lcd.print("MSSV da duoc cap"); // In ký tự được nhấn vào màn hình LCD
        lcd.setCursor(0, 3); // Di chuyển con trỏ đến vị trí (cursorColumn, 0)
        lcd.print("nhat!"); // In ký tự được nhấn vào màn hình LCD
        delay(2000);
        student_name= "";
        cursorColumn = 0; // Đặt lại vị trí con trỏ về 0
        input_password="";
        mode = false; // Quay lại chế độ chọn sau khi đăng nhập thành công
        break;

      }
    }
  } else if (key == '0') {
    digitalWrite(Buzzer, HIGH);
    delay(150);
    digitalWrite(Buzzer, LOW);
    lcd.clear();
    begin();
  }
  begin();
}
}
}

void RFID(){
 if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.println();
      Serial.println(F("Reading last data from RFID..."));  
      //----------------------------------------------------------------
      String values = "", data;
     
      //creating payload - method 2 - More efficient
      for (byte i = 0; i < total_blocks; i++) {
        ReadDataFromBlock(blocks[i], readBlockData);
        //*************************************************
        if(i == 0){
          data = String((char*)readBlockData);
          data.trim();
          student_id = data;
          values = "\"" + (data.length() == 0 ? "UNKNOWN" : data) + ",";
        }
        //*************************************************
        else if(i == total_blocks-1){
          data = String((char*)readBlockData);
          data.trim();
          values += (data.length() == 0 ? "UNKNOWN" : data) + "\"}";
        }
        //*************************************************
        else{
          data = String((char*)readBlockData);
          data.trim();
          values += (data.length() == 0 ? "UNKNOWN" : data) + ",";
        }
      }
      //----------------------------------------------------------------
      // Create json object string to send to Google Sheets
      // values = "\"" + value0 + "," + value1 + "," + value2 + "\"}"
      payload = payload_base + values;
      //----------------------------------------------------------------

      //----------------------------------------------------------------
      // Publish data to Google Sheets
      Serial.println("Publishing data...");
      Serial.println(payload);
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("Publishing data...");
      
      if(client->POST(url, host, payload)){ 
        // do stuff here if publish was successful

      }
      //----------------------------------------------------------------
      else{
        // do stuff here if publish was not successful
       
      }
      //----------------------------------------------------------------
      // a delay of several seconds is required before publishing again    
      // delay(100);
    
      if(data.length() == 0){
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
      lcd.print("MSSV khong hop le"); // In một ký tự trống để xóa
      lcd.setCursor(0,3); // Di chuyển con trỏ LCD
      lcd.print("Diem danh that bai"); // In một ký tự trống để xóatCursor(0,2);
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
      digitalWrite (Buzzer, HIGH);                            
      delay (1000);                                            
      digitalWrite (Buzzer, LOW);
      delay(2000);
      }
      else
      {
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
      lcd.print("Ma so SV: "); // In một ký tự trống để xóa
      lcd.setCursor(10, 2); // Di chuyển con trỏ LCD
      lcd.print(student_id); // In một ký tự trống để xóa
      lcd.setCursor(0,3); // Di chuyển con trỏ LCD
      lcd.print("Diem danh thanh cong"); // In một ký tự trống để xóa
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
      digitalWrite (Buzzer, HIGH);                            
      delay (1000);                                            
      digitalWrite (Buzzer, LOW);
      delay(1000);
      }
      

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
      delay(1000);
      lcd.clear();
      begin();
    }
  }
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void RTC(){
  DateTime now = rtc.now();
  int year = now.year();
  int month = now.month();
  int day = now.day();
  int hour = now.hour();
  int minute = now.minute();
  int second = now.second();

  lcd.setCursor(0, 0);  // start to print at the first row
  lcd.print(day);
  lcd.print("/");
  lcd.print(month);
  lcd.print("/");
  lcd.print(year);
  lcd.print(" ");
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);

  lcd.setCursor(0, 1);  // start to print at the second row
  
  if(now.hour()<=9)
    {
      lcd.print("0");
      lcd.print(hour);
    }
    else {
     lcd.print(hour); 
    }
    lcd.print(':');
    if(now.minute()<=9)
    {
      lcd.print("0");
      lcd.print(minute);
    }
    else {
     lcd.print(minute); 
    }
    lcd.print(':');
    if(now.second()<=9)
    {
      lcd.print("0");
      lcd.print(second);
    }
    else {
     lcd.print(second); 
    }
}

void loop() {
  RTC();
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected())
      {client->connect(host, httpsPort);}
  }
  else{Serial.println("Error creating client object!");}
  //---------------------------------------------------------------
  int buttonState = digitalRead(BUTTON_PIN);
  

  if (buttonState == LOW) { // Nếu nút được nhấn
    digitalWrite (Buzzer, HIGH);                            
    delay (150);                                            
    digitalWrite (Buzzer, LOW);
    mode = !mode; // Chuyển đổi giữa chế độ nhập mã và chế độ quét RFID
    lcd.clear();
    if (mode) {
      RTC();
      lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
      lcd.print("Moi nhap ma so SV: ");
      input_password = ""; // Xóa chuỗi MSSV khi bắt đầu nhập
  
    } else {
      RTC();
      lcd.setCursor(0, 2); // Di chuyển con trỏ LCD
      lcd.print("Moi quet the RFID: ");
      
    }
    delay(5); // Đợi một chút để tránh nhận nhiều lần với một lần nhấn nút
  }
  if (mode) { // Nếu đang ở chế độ nhập mã
    pass(); // Gọi hàm xử lý nhập mã

  } 
  else { // Nếu đang ở chế độ quét RFID
    RFID();
  }

}
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  //----------------------------------------------------------------------------
  /* Prepare the key for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  //----------------------------------------------------------------------------
  /* Authenticating the desired data block for Read access using Key A */
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
  //----------------------------------------------------------------------------
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(rfid.GetStatusCodeName(status));
     return;
  }
  //----------------------------------------------------------------------------
  else {
    Serial.println("Authentication success");
  }
  //----------------------------------------------------------------------------
  /* Reading data from the Block */
  status = rfid.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  //----------------------------------------------------------------------------
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
  //----------------------------------------------------------------------------
}
