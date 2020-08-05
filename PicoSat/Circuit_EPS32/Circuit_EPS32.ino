//Includes libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <mySD.h>




#include <WiFi.h>
#include "time.h"

const char* ssid       = "UbaLink";
const char* password   = "casaqwe159";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;
const int   daylightOffset_sec = 3600;



//Definitions
// Definition BME280
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C


// Definition MPU6050
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;

// Select SDA and SCL pins for I2C communication
const uint8_t scl = 22;
const uint8_t sda = 21;

// sensitivity scale factor respective to full scale setting provided in datasheet
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;


// Definition micro SD
File myFile;
const int chipSelect = 5; // GPIO 5


// Functions
//MPU6050 Functions
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

//configure MPU6050
void MPU6050_Init() {
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read() << 8) | Wire.read());
  AccelY = (((int16_t)Wire.read() << 8) | Wire.read());
  AccelZ = (((int16_t)Wire.read() << 8) | Wire.read());
  Temperature = (((int16_t)Wire.read() << 8) | Wire.read());
  GyroX = (((int16_t)Wire.read() << 8) | Wire.read());
  GyroY = (((int16_t)Wire.read() << 8) | Wire.read());
  GyroZ = (((int16_t)Wire.read() << 8) | Wire.read());
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%H:%M:%S");
}




// Start Program
void setup() {
  delay(5000);// delay boot

  Serial.begin(115200);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  
  // BME280 init
  bool status;
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);
  if (!status) {
    while (1);
  }

  // MPU6050 init
  Wire.begin(sda, scl);
  MPU6050_Init();

  // Micro SD init
  pinMode(SS, OUTPUT);
  if (!SD.begin(5, 23, 19, 18)) {
    return;
  }

  
  // Creating file
  myFile = SD.open("data.csv", FILE_WRITE);
  if (myFile) {
    Serial.println("time,Ax,Ay,Az,Temperature_mpu,Gx,Gy,Gz,Temperature_bme,Pressure,Altitude,Humidity");
    myFile.println("time,Ax,Ay,Az,Temperature_mpu,Gx,Gy,Gz,Temperature_bme,Pressure,Altitude,Humidity");
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    ESP.restart();
  }

}

void loop() {


  //save Values
  myFile = SD.open("data.csv", FILE_WRITE);
  if (myFile)
  {
    while (touchRead(4) > 15)
    {
      struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  
  double Ax, Ay, Az, T, Gx, Gy, Gz;

  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);

  //divide each with their sensitivity scale factor
  Ax = (double)AccelX / AccelScaleFactor;
  Ay = (double)AccelY / AccelScaleFactor;
  Az = (double)AccelZ / AccelScaleFactor;
  T = (double)Temperature / 340 + 36.53; //temperature formula
  Gx = (double)GyroX / GyroScaleFactor;
  Gy = (double)GyroY / GyroScaleFactor;
  Gz = (double)GyroZ / GyroScaleFactor;

  
      Serial.println(touchRead(4));
      Serial.print(&timeinfo, "%H:%M:%S");
      myFile.print(&timeinfo, "%H:%M:%S");
      Serial.print(",");
      myFile.print(",");
      Serial.print(Ax);
      myFile.print(Ax);
      Serial.print(",");
      myFile.print(",");
      Serial.print(Ay);
      myFile.print(Ay);
      Serial.print(",");
      myFile.print(",");
      Serial.print(Az);
      myFile.print(Az);
      Serial.print(",");
      myFile.print(",");
      Serial.print(T);
      myFile.print(T);
      Serial.print(",");
      myFile.print(",");
      Serial.print(Gx);
      myFile.print(Gx);
      Serial.print(",");
      myFile.print(",");
      Serial.print(Gy);
      myFile.print(Gy);
      Serial.print(",");
      myFile.print(",");
      Serial.print(Gz);
      myFile.print(Gz);
      Serial.print(",");
      myFile.print(",");
      Serial.print(bme.readTemperature());
      myFile.print(bme.readTemperature());
      Serial.print(",");
      myFile.print(",");
      Serial.print(bme.readPressure() / 100.0F);
      myFile.print(bme.readPressure() / 100.0F);
      Serial.print(",");
      myFile.print(",");
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      myFile.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      Serial.print(",");
      myFile.print(",");
      Serial.println(bme.readHumidity());
      myFile.println(bme.readHumidity());
    
      delay(1000);
    }
    myFile.close();
    while (1) {}
  }
  else
  {
    // if the file didn't open, print an error:
    while (1) {}
  }
}
