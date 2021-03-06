
#include <WiFi.h>
#include <Wire.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;

const char* ssid     = "****"; // Your SSID (Name of your WiFi)
const char* password = "****"; //Your Wifi password

const char* host = "api.thingspeak.com";
String api_key = "****"; // Your API Key provied by thingspeak

//Device addresss
const int TMP117_Address = 0x48;

//Hexadecimal addresses for various TMP117 registers
const int Temp_Reg = 0x00;
const int Config_Reg = 0x01;
const int High_Lim_Reg = 0x02;
const int Low_Lim_Reg = 0x03;
const int EEPROM_Unlock_Reg = 0x04;
const int Device_ID_Reg = 0x0F;

//TMP117 Offset Reg
const int Offset_Reg = 0x07;
const int offsetH = B00001100;
const int offsetL = B10000000;

//set temp threshold
const uint8_t highlimH = B00001101; // high byte of high limit
const uint8_t highlimL = B10000000; // low byte of high lim- high 27 C
const uint8_t lowlimH = B00001100; // high byte of low lim
const uint8_t lowlimL = B00000000; // low byte of low limit-low 24 C

 double temp;

void setup(){
   Wire.begin(); 
Connect_to_Wifi();
  Serial.begin(115200);

  I2Cwrite(TMP117_Address, High_Lim_Reg, highlimH, highlimL);
  I2Cwrite(TMP117_Address, Low_Lim_Reg, lowlimH, lowlimL);
  I2Cwrite(TMP117_Address, Config_Reg, 0x02, 0x20);
  //    I2Cwrite(TMP117_Address,Offset_Reg,offsetH,offsetL);
  I2Cwrite(TMP117_Address, Offset_Reg, 0x00, 0x00);
}

void loop(){

  
temp = ReadTempSensor();
Serial.println(temp);
delay(500);

// call function to send data to Thingspeak
Send_Data();

delay(5000);
 
}

void Connect_to_Wifi()
{

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void Send_Data()
{


  WiFiClient client;

  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  else
  {
    String data_to_send = api_key;
    data_to_send += "&field1=";
    data_to_send += String(temp);

    data_to_send += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data_to_send.length());
    client.print("\n\n");
    client.print(data_to_send);

    delay(1000);
  }

  client.stop();

}


double I2Cwrite(int dev, int reg, int H, int L) {
  // int H, int L: high and low bytes of data to transmit
  Wire.beginTransmission(dev);
  Wire.write(reg);
  Wire.write(H);
  Wire.write(L);
  Wire.endTransmission();
  delay(10);

}

double ReadTempSensor(void) {
  //data array to store 2-bytes from i2c line
  uint8_t data[2];
  //combination of 2-byte data into 16-bit data
  uint16_t datac;

  //points to device and begins transmission
  Wire.beginTransmission(TMP117_Address);
  //points to temperature register to read/write data
  Wire.write(Temp_Reg);
  //ends data transfer and transmits data from register
  Wire.endTransmission();
  //delay to allow sufficient conversion time
  delay(10);
  //regquest 2 byte temp data from device
  Wire.requestFrom(TMP117_Address, 2);
  //checks if data received matches the reequested  2-bytes
  if (Wire.available() <= 2) {
    //store each byte of data from temp register
    data[0] = Wire.read();
    data[1] = Wire.read();
    //combines data to make . 16 bit binary number
    datac = ((data[0] << 8) | data[1]);
    //convert to celciud (7.8125 mC resolution) and return
    return datac * 0.0078125;
  }


}
