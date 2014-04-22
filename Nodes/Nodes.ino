
#include <SoftwareSerial.h>

//goal is to send an xbee api packet to our connected xbee radio, 
//such that the radio transmits data to another node on our network.  

SoftwareSerial xbee(2,3);

unsigned int analogReading0 = 0;

double REF_VOLTAGE = 3.3;

unsigned long sensorTime = 5000; //how often to send data
unsigned long sensorLast = 0;
unsigned char newPANID1 = 0xD0, newPANID2 = 0x0F; //will change when given specific PAN ID to connect to


unsigned char receiveBuffer[100];
int currentReceiveIndex = 0; //where to write the next character

void setup()
{
  //xbee radio rx tx can be attached to pins 2 and 3 (software serial)
  //, or to pins 0 and 1 (hardward serial)
  
  //we'll use pins 2 and 3, because we'll want to debug over usb
  Serial.begin(9600); //for debugging
  
  xbee.begin(9600); //to talk to the xbee
  setPANID();
  writeID();
  resetNetwork();
}


//changes PAN ID and then broadcasts 
void sendPacket()
{
  unsigned char packet[20] = {0};  //actual size will change (initialize to all zeros)
  unsigned int packetLength = 26;
  
  packet[0] = 0x7E;
  packet[1] = (packetLength >> 8) & 0xFF; //get MSB
  packet[2] = (packetLength >> 0) & 0xFF; //get LSB
  
  //frame specific data
  packet[3] = 0x10; //send data identifier (tx request)
  packet[4] = 0x01; //packet id
  
  //64-bit address of destination
  
  packet[5] = 0x00;
  packet[6] = 0x13;
  packet[7] = 0xA2;
  packet[8] = 0x00;
  packet[9] = 0x40;
  packet[10] = 0x90;
  packet[11] = 0x2D;
  packet[12] = 0xF4;
  
  //leave alone for destination coordinator
  
  //broadcast
  /*
  packet[5] = 0x00;
  packet[6] = 0x00;
  packet[7] = 0x00;
  packet[8] = 0x00;
  packet[9] = 0x00;
  packet[10] = 0x00;
  packet[11] = 0xFF;
  packet[12] = 0xFF;
  */
  
  //16 bit address, set to 0xFFFE if not known
  packet[13] = 0xFF;
  packet[14] = 0xFE;
  
  packet[15] = 0x00; //10 hops (maximum)
  packet[16] = 0x00; //disable ack
  
  //now comes the actual data
  packet[17] = (analogReading0 >> 8) & 0xFF; //MSB
  packet[18] = (analogReading0 >> 0) & 0xFF;  //LSB
  
  //checksum
  unsigned long byteSum = 0;
  
  for(int i=3;i < 18;i++)
  {
    byteSum += packet[i];
  }
  
  packet[19] = 0xFF - (byteSum & 0xFF);
   
  for(int i=0;i<20;i++)
  {
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  
  Serial.println();
  
  //actually write the byte data to software serial
  for(int i=0;i<30;i++)
  {
    xbee.write(packet[i]);
  }
}

void setPANID()
{
  unsigned char newPANID[10];
  unsigned int PANIDlength = 6;
  
  newPANID[0] = 0x7E;
  newPANID[1] = (PANIDlength >> 8) & 0xFF; //get MSB
  newPANID[2] = (PANIDlength >> 0) & 0xFF; //get LSB 
  newPANID[3]=0x08;//API identifier
  newPANID[4]=0x53;//Frame ID
  newPANID[5]=0x49;//ASCII value = 'I'=====|-----> command for setting PAN ID;
  newPANID[6]=0x44;//ASCII value = 'D'=====|
  newPANID[7]=newPANID1;  //'D0'
  newPANID[8]=newPANID2;  //0F
  
  unsigned int byteSum = 0;
  
  for(int i=3;i<9;i++)
  {
    byteSum += newPANID[i];
  }
  
  newPANID[9]=0xFF-(byteSum & 0xFF);
  
  for(int i=0;i<10;i++)
  {
   xbee.write(newPANID[i]); 
  }
}

void writeID()
{
  unsigned char writePANID[8];
  unsigned int writeIDlength = 4;
  
  writePANID[0]=0x7E;
  writePANID[1]= (writeIDlength >> 8) & 0xFF;
  writePANID[2] = (writeIDlength >> 0) & 0xFF;
  writePANID[3]=0x08;//API identifier
  writePANID[4]=0x55;//Frame ID
  writePANID[5]=0x57;// 'W' - Actually writes PAN ID
  writePANID[6]=0x52; //'R'
  
  //checksum
  unsigned int byteSum = 0;
  for(int i=3;i<7;i++)
  {
    byteSum += writePANID[i];
  }
  
  writePANID[7]=0xFF-(byteSum & 0xFF);
  
  for(int i=0;i<8;i++)
  {
   xbee.write(writePANID[i]); 
  }
}

/*
void parsePacket()
{
  if(receiveBuffer[0] != 0x7E){
    currentReceiveIndex = 0;
    return;
  }
  if(currentReceiveIndex < 3){
    return;
  }
  unsigned int length = ((receiveBuffer[1] << 8) & 0xFF00) | receiveBuffer[2];
  
  if(currentReceiveIndex < 4 + length){
    return;
  }
  
  unsigned char * frameData = receiveBuffer + 3;
  int checksum = frameData[length+1];
  
  switch(frameData[0]){
      case 0x90: //receive packet
        Serial.println("received an rx packet");
        currentReceiveIndex = 0;
        break;
      
      default:
        Serial.println("Unknown packet received");
        currentReceiveIndex = 0;
        break;
  }  
}
*/ 
 
void resetNetwork()
{
  unsigned char reset[9];
  unsigned int resetlength = 5;
  
  reset[0] = 0x7E;
  reset[1] = (resetlength >> 8) & 0xFF;
  reset[2] = (resetlength >> 0) & 0xFF;
  reset[3] = 0x08; //api id
  reset[4] = 0x56; //frame id
  reset[5] = 0x4E;  //'N'
  reset[6] = 0x52;  //'R'
  reset[7] = 0x00;
  
  //checksum
  unsigned int byteSum = 0;
  for (int i=3;i<8;i++)
  {
    byteSum += reset[i];
  }
  
  reset[8] = 0xFF - (byteSum & 0xFF);
  
  for(int i=0;i<8;i++)
  {
    xbee.write(reset[i]);
  }
}

/*
void readSerial()
{
    if(xbee.available())
    {
      int b = xbee.read();
      if(b >= 0)
      {
        receiveBuffer[currentReceiveIndex++] = (unsigned char) b;
        parsePacket();
      }
      if(currentReceiveIndex >= 100){
        Serial.println("Error, receive buffer overflowed");
        currentReceiveIndex = 0; //reset our buffer
      }
      //Serial.print(b,HEX);
     // Serial.print(" ");
    }
}
*/

void getSensorData()
{ 
  analogReading0 = analogRead(0);  
  // converting that reading to voltage
  float voltage = analogReading0 * REF_VOLTAGE / 1024.0;  
  // convert the voltage to a temperature
  float celsius = (voltage - 0.5) * 100 ;  
  // print it on the serial output
  Serial.print(celsius);
}

void loop()
{
  
  if(millis()-sensorLast > sensorTime)
  {
    getSensorData();
    sendPacket();
    sensorLast = millis(); //wait until next time period passes
  }
  
  //readSerial();
}

