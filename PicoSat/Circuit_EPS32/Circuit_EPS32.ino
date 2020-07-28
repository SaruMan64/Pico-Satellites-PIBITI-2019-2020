/*  
 *   PicoSat circuit with ESP32
 *   
 *   Circuit demonstration with ESP32.
 *   Program originally added 28 abr 2020.
 *   by Yeté A M Labarca.
 *   
 */

//Include the libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <MQUnifiedsensor.h> // More information <https://github.com/miguel5612/MQSensorsLib>



/*************************Definition of MQ135 for ESP32************************/
/*************************ESP32 Hardware Related Macros************************/
#define Board ("ESP-32") // Board ESP32.
#define Pin (25)  // Pinout here.
/*************************ESP32 Software Related Macros************************/
#define Type ("MQ-135") // Type sensor.
#define Voltage_Resolution (5) // <- IMPORTANT, Voltage sensor.
#define ADC_Bit_Resolution (12) // ESP-32 bit resolution.
#define RatioMQ135CleanAir (3.6) // Ratio of sensor,RS / R0 = 3.6 ppm 
/*************************Globals**********************************************/
MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);


/*************************Definition of GY-521 for ESP32***********************/
const int MPU_ADDR = 0x68;
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;
char tmp_str[7];

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}


/*************************Definition of BME280 for ESP32***********************/
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
unsigned long delayTime;
float vTemperature, vPressure, vAltitude, vHumidity;


/*************************Another configuration for I2C communication**********/
// If you need to change the I2C communication pin, you can use the configuration below.
//const int sda_pin = 5; // pin definition I2C SDA
//const int scl_pin = 6; // pin definition I2C SCL

//void initI2C() {
//  Serial.println("---inside initI2C");
//  Wire.begin(sda_pin, scl_pin);
//}

void setup() {
  
  Serial.begin(9600);
  
  //initI2C(); //Start another configuration for I2C communication
  
  while(!Serial);    // time to get serial running
  Serial.println(F("BME280 test"));
  
  startMQ135();
  startGY521();
  startBME280();
  while(!Serial);    // time to get serial running
  Serial.println(F("BME280 test"));
  delay(1000);
}

void loop() {

  print_MQ135(); 
  delay(100);
  printGY521();
  delay(100);
  printBME280();
  delay(500);
}


/**************************Start sensors***************************************/

void startMQ135(){
  /*************************Definition MQ135***********************************/
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.init(); 
    Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*************************End of definition MQ135****************************/
}

void startGY521(){
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void startBME280() {
    unsigned status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        while (1);
    }
    Serial.println("-- Default Test --");

}


/**************************Print sensor values*********************************/
void print_MQ135(){
  /*************************Definition MQ135***********************************/
  MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
  
  MQ135.setA(605.18); MQ135.setB(-3.937); // Configurate the ecuation values to get CO concentration
  float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

  MQ135.setA(77.255); MQ135.setB(-3.18); // Configurate the ecuation values to get Alcohol concentration
  float Alcohol = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

  MQ135.setA(110.47); MQ135.setB(-2.862); // Configurate the ecuation values to get CO2 concentration
  float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

  MQ135.setA(44.947); MQ135.setB(-3.445); // Configurate the ecuation values to get Tolueno concentration
  float Tolueno = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configurate the ecuation values to get NH4 concentration
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

  MQ135.setA(34.668); MQ135.setB(-3.369); // Configurate the ecuation values to get Acetona concentration
  float Acetona = MQ135.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  
  Serial.println("** Lectures from MQ-135 ****");
  Serial.println("|    CO   |  Alcohol |   CO2  |  Tolueno  |  NH4  |  Acteona  |");
  Serial.print("|   "); Serial.print(CO); 
  Serial.print("   |   "); Serial.print(Alcohol); 
  Serial.print("   |   "); Serial.print(CO2); 
  Serial.print("   |   "); Serial.print(Tolueno); 
  Serial.print("   |   "); Serial.print(NH4); 
  Serial.print("   |   "); Serial.print(Acetona);
  Serial.println("   |");
  /*************************End of definition MQ135****************************/
}

void printBME280() {
    Serial.print("Temperature = ");
    vTemperature = bme.readTemperature();
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");
    vPressure = bme.readPressure() / 100.0F;
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    vAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    vHumidity = bme.readHumidity();
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}

void printGY521(){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
  Serial.println();
}
