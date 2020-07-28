
#include <SPI.h>
#include <mySD.h>

File myFile;

const int chipSelect = 5;

long randNumber;

void setup() {
  delay(10000); //program startup delay
  
  Serial.begin(115200);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }  
  //star sd card, SD_Star(pin CS, pin MOSI, pin MISO, pin SCK)
  SD_Start(5, 23, 19, 18); 
  
  myFile = SD.open("data.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("a,b,c,d,e");
    myFile.println("a,b,c,d,e");
    
  // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening archive.");
  }
  
  randomSeed(34);
}
 
void loop() {
  Serial.println(touchRead(4));  // get value of Touch 0 pin = GPIO 4
  myFile = SD.open("data.txt", FILE_WRITE);
  delay(1000);

  if (myFile)
  {
    while(touchRead(4) > 15)
    {
      Serial.print("Valor Touch: ");
      Serial.println(touchRead(4));
      for(int i = 0; i < 5; i++)
      {
        if(i == 4)
        {
          Serial.println(random(4096));
         myFile.println(randNumber);
         Serial.println();
          break;
        }
        Serial.print(random(4096));
        Serial.print(",");
        myFile.println(randNumber);
        myFile.print(",");
      }
      delay(1000);
    }
      myFile.close();
      while(1){}
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening archive.");
  }
  
}

void SD_Start(int CS, int SCK, int MOSI, int MISO){
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  if (!SD.begin(CS, MOSI, MISO, SCK)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}




/*
 * CO Alcohol CO2 Tolueno NH4 Acteona Pressure  Altitude  Humidity  Temperature_BME280  Temperature_GY-521  Ax  Ay  Az  Gx  Gy  Gz
 */