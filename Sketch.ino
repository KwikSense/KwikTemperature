//This skecth runs on the ATTiny85 to:
//1.collect Temperature Data
//2.Collect GPS co-ordinates
//3.communicate with the A7 GPRS+GPS chip
//4.Store an offline backup incase of communication failure
//5.sleep


//Commands: (in order)
//collect temperature
//collect coordinates
//prepare request

//AT
//ATE1
//AT+CGACT=1
//AT+CGDCONT=1,"IP"."*APN*"
//AT+CGACT=1,1
//AT+CIPSTART="TCP","*IP address*",*port no*
//AT+CIPSEND
//SEND  request (GET...) NL;CR
//AT+CIPCLOSE

//in case of failure
//AT+CCLK? (to get time)
//add temperature to Array + TimeStamp

//sleep for 30min

//INCLUDE LIBRARIES:
//communicate with A7 chip
#include <SoftwareSerial.h>

//collect data
#include <DallasTemperature.h>
#include <OneWire.h>

//for power managment
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "LowPower.h"

#define ONE_WIRE_BUS 2  // sensors pin
#define sleep_stage 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
SoftwareSerial mySerial(10, 11); // RX, TX
SoftwareSerial GPS(9) //RX for GPS 

//MY VARIABLES:
int stage = 0; //at what stage is the program
int retry = 0; //number of complete retries (from stage 3 onwards)

//request components
String request = "GET /public/api/v1/SendSMSAlert?camp_id=15&sensortype_id=";
String GETgeolat = "&sensorpin=1&geolat=";
String geolat = "";
String GETgeolong = "&geolong=";
String geolong = "";
String GETval = "&val=";
String val = "";
String GETend = "&is_active=t&users_id=1&is_raise_alarm=N";
String GETdate = "&date=";
String date = "";
String URL; //to store everything
//sample request:
// GET /public/api/v1/SendSMSAlert?camp_id=15&sensortype_id=1&sensorpin=1&geolat=34.38516



&geolong=35.781543&val=12&is_active=t&users_id=1&is_raise_alarm=N

int offlineVal[12]; //array to store unsent data
String offlineTime[12];

void backup() // to store data offline incase of internet failure
{
  int valueToSave = val;
  String dateToSave;
  //get date
  if (offlineRequest == true){ //if this is a failed attempt to send a previously stroed data point then it's date is saved already
     dateToSave = date;   
   }
   else { //if this is a failed attempt to send an acquired data point , find date of failure to save
    
  //to recieve data from A7 chip
  String content = "";
  char character;
  boolean startRecording = false;

  mySerial.println("AT+CCLK");//ask for time

  delay(1);
  while (mySerial.available())
  {
    character = mySerial.read();
    if (character == 34) { //if '"' found start saving; if found again stop saving
      startRecording != startRecording;
    };

    if (startRecording || character == 34)
    {
      content.concat(character);
    }
  };
   }


stage = sleep_stage;
}



void sendCommand(String command, int nextStep)
{
  byte tries = 0;
  byte repeat = 1;
  while (repeat == 1) {
    //to recieve data from A7 chip
    String content = "";
    char character;
    mySerial.println(command);

    delay(1);
    while (mySerial.available())
    {
      character = mySerial.read();
      content.concat(character);
    };

    if (content == "OK")
    {
      stage = nextStep;
      content = "";
      repeat = 0;
    }
    else // so it trys again
    {
      delay(10);
      tries++;
    };

    if (tries >= 5 && retry == 1) //if operation is already repeating and no successful tires
    {
      backup(); //resort to backup
      retry = 0; //clear flag since action already has been taken
      repeat = 0;//to break the loop
      stage = 12; //go to sleep
      break;
    }
    else if (tries >= 5) //if it tried 5 times with no success
    {
      stage = 3; // start the complete operation once more
      retry = 1; // set flag that we are doing a complete retry
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  mySerial.begin(115200);
  GPS.begin(9600);
  // Start up the temperature sensor library
  sensors.begin();
}



void loop() {
  switch (stage) {
    case 1:    //get Tempertaure
      sensors.requestTemperatures(); //issue request to get data
      val = sensors.getTempCByIndex(0); //set Val to temperature value


      stage = 2;
      break;

    case 2: //get GPS co-ordinates
      sendCommand("AT+GPS", 3);
      
    String content = "";
    char character;
    mySerial.println(command);
    delay(1);
    while (mySerial.available())
    {
      character = mySerial.read();
      content.concat(character);
    };
    
      URL = request + GETgeolat + geolat + GETgeolong + geolong + GETval + val + GETend; // complete URL
      break;

    case 3: //communication with A7 chip
      //AT
      sendCommand("AT", 4);
      break;

    case 4: //communication with A7 chip
      //ATE1
      sendCommand("ATE1", 5);
      break;

    case 5: //communication with A7 chip
      //AT+CGACT=1
      sendCommand("AT+CGACT=1", 6);
      break;

    case 6: //communication with A7 chip
      //AT+CGDCONT=1,"IP","Data.mic1.com.lb"
      sendCommand("AT+CGDCONT=1,\"IP\",\"Data.mic1.com.lb", 7);
      break;

    case 7: //communication with A7 chip
      //AT+CGACT=1,1
      sendCommand("AT+CGACT=1,1", 8);
      break;

    case 8: //communication with A7 chip
      //AT+CIPSTART="TCP","54.186.61.242",80
      sendCommand("AT+CIPSTART=\"TCP\",\"54.186.61.242\",80", 9);
      break;

    case 9: //communication with A7 chip
      //AT+CIPSEND
      sendCommand("AT+CIPSEND", 10);
      break;

    case 10: //communication with A7 chip
      //SEND  request (GET...) NL;CR

      sendCommand(URL, 12);
      retry = 0; // success of operation, so retry flag is set down
      break;


    case 12: //set A7+ATMega328 chip to sleep
    offlineRequest = false; //so that next request failure will acquire new date instead of using stored/older one
    while (s < 225)
    {
    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,SPI_OFF, USART0_OFF, TWI_OFF); //switches off all internal scicuits for max power saving
    s++
    }     
      break;

  }
}
//
//void sleep(){
//activate wdt
//lower pin for A7 sleep
//}
//
//ISR(WDT)
//{
//  higher pin to deactivate low power mode for A7
//  RESET A7 to wake up.
//  continue loop
//}
