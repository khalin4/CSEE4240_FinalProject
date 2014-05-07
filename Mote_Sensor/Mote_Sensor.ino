#include <SoftwareSerial.h>

//goal is to send an xbee api packet to our connected xbee radio, 
//such that the radio transmits data to another node on our network.  

SoftwareSerial xbee(2,3);

int supplyVoltage = 5; // 5V
int analogPin = 0; // reading temp from pin A0
int reading; // reading from analog pin
float voltage; // reading convertered into voltage
float tempC; // temperature in Celsius
float tempF; // temperature in Fahrenheit
int temp=0;
unsigned long sensorTime = 5000; //how often to send data
unsigned long sensorLast = 0;

void setup()
{
  //xbee radio rx tx can be attached to pins 2 and 3 (software serial)
  //, or to pins 0 and 1 (hardward serial)
  
  //we'll use pins 2 and 3, because we'll want to debug over usb
  Serial.begin(9600); //for debugging
  xbee.begin(9600); //to talk to the xbee
}


//changes PAN ID and then broadcasts 
void sendPacket()
{
  unsigned char packet[20] = {0};  //actual size will change (initialize to all zeros)
  unsigned int packetLength = 16;
  
  packet[0] = 0x7E;
  packet[1] = (packetLength >> 8) & 0xFF; //get MSB
  packet[2] = (packetLength >> 0) & 0xFF; //get LSB
  
  //frame specific data
  packet[3] = 0x10; //send data identifier (tx request)
  packet[4] = 0x01; //packet id
  
  //64-bit address of destination
  /*
  packet[5] = 0x00;
  packet[6] = 0x13;
  packet[7] = 0xA2;
  packet[8] = 0x00;
  packet[9] = 0x40;
  packet[10] = 0x90;
  packet[11] = 0x2D;
  packet[12] = 0xF4;
  */
  
  //broadcast
  packet[5] = 0x00;
  packet[6] = 0x00;
  packet[7] = 0x00;
  packet[8] = 0x00;
  packet[9] = 0x00;
  packet[10] = 0x00;
  packet[11] = 0xFF;
  packet[12] = 0xFF;
  
  //16 bit address, set to 0xFFFE if not known
  packet[13] = 0xFF;
  packet[14] = 0xFE;
  
  packet[15] = 0x00; //10 hops (maximum)
  packet[16] = 0x01; //disable ack
  
  //now comes the actual data
  Serial.println(temp);
  packet[17] = (temp >> 8) & 0xFF; //MSB
  packet[18] = (temp >> 0) & 0xFF;  //LSB
  
  
  //checksum
  unsigned long byteSum = 0;
  
  for(int i=3;i < 19;i++)
  {
    byteSum += packet[i];
  }
  
  packet[19] = 0xFF - (byteSum & 0xFF);
  
  Serial.println();
  
  //actually write the byte data to software serial
  for(int i=0;i<20;i++)
  {
    xbee.write(packet[i]);
  }
}

  
void getSensorData()
{
   reading = analogRead(analogPin);
  
  // Convert reading into voltage
  float voltage = (reading * supplyVoltage) / 1024.0;
  
  // Convert milliVolt reading to temp based on datasheet
  // Scale factor 10 mV/C and 500 mV offset (750mV = 25C)
  tempC = (voltage - 0.5) * 100;
  tempF = (tempC * 9.0 / 5.0) + 32.0;
  
   temp = (int) tempF;
  // Output values over Serial
  Serial.print("Celsius: " );
  Serial.println(tempC);
  Serial.print("Fahrenheit: " );
  Serial.println(tempF);
  Serial.println(temp);
  
  delay(1000); // output once every second
}

void loop()
{
  if(millis()-sensorLast > sensorTime)
  {
    getSensorData();
    sendPacket();
    sensorLast = millis(); //wait until next time period passes
  }
}

