#define IO7 (13)              // Input/Output to payload plus
#define IO6 (12)              // Input/Output to paylsoad plus
#define IO5 (11)              // Input/Output to payload plus
#define IO4 (10)              // Input/Output to payload plus
#define IO3 (9)              // Input/Output to payload plus
#define IO2 (4)              // Input/Output to payload plus
#define IO1 (3)              // Input/Output to payload plus
#define IO0 (A6)              // Input/Output to payload plus

//#define TRUESTATE ((uint8_t) 1)
//#define EVENT_FLAGADRESS (FRAM_ADRESS)
//#define PUMP_FLAG_ADRESS_SIZE 8

/*
****************************************************************************************
****************************************************************************************

Files Required to make a complete program - 
  CLI_V1.0, Quest_CLI.h, Quest_Flight.h, Quest_flight.cpp
      Important Functions and their use:
          cmd_takeSphot
              This function will use the serial camera (the one with the cable) to take a photo (jpg)
              on the transfer of this photo to the Host controller it will create a text file (txt)
              with the same file name as the jpg file of the conditions of when the jpg was taken.
              NOTE: add2text can be used with this command
          cmd_takeSpiphoto
              This function is for the SPI camera (the one that plugges directly on the microlab board)
              it will take a jpg the same as the serial camera, also creating a txt file.  Also NOTE: the
              add2text function can be used on the photo text file.
          nophotophoto
              if a camera is not being used, then this will simulate the camera photo operation. The text file 
              is still created, so the add2text function can be used the get data downloaded in the text file
          nophoto30K
              this dois not use a camera, instead it uses the file space for the photo the containe ascii data
              using dataappend function defined below.  The data will still have a jpg etension, but the file
              will contain ascii of the data put in buy the dataappend unction. Plus a text file is generated,
              and the add2text function will add more data.
          -----
          add2text(int value1,int value2,int value3)
              this function will take the three varables and place them at the end of the text file (the file is
              limited the 1024 charators),  if you look at the function the text output can be formatted is almost
              any way.  Look at it and change it to acccomadate your data...
          dataappend(int counts,int ampli,int SiPM,int Deadtime)
              this function will add ascii data to the 30K byte data buffer that was used a a jpg photo.  look at
              the data formating and change it necessary for you project information.  To view the data, you can
              use a hex/ascii file viewer or change the ext to a txt.  then it should be viewable with a text exitor.
.
******************************************************************************************                                   
******************************************************************************************                  
*/

#include "Quest_Flight.h"
#include "Quest_CLI.h"

//////////////////////////////////////////////////////////////////////////
//    This defines the timers used to control flight operations
//////////////////////////////////////////////////////////////////////////
//  Fast clock --- 1 hour = 5 min = 1/12 of an  hour
//     one millie -- 1ms
//
#define SpeedFactor 1  // = times faster
//
//
//////////////////////////////////////////////////////////////////////////
//
#define one_sec 1000           //one second = 1000 millis
#define one_min 60 * one_sec   // one minute of time
#define one_hour 60 * one_min  // one hour of time
#define one_day 24 * one_hour  //one day of time
//
//
#define TimeEvent1_time ((one_day * 2) / SpeedFactor)  //main time event
#define TimeEvent2_time ((one_sec * 18.5))  // time to pump fluid
#define TimeEvent3_time ((one_sec * 2))  // time to pump CO2 FIND THISS
#define TimeEvent4_time ((one_sec * 20))  // time to suck fluid FIND THIS
#define TimeEvent5_time ((one_sec * 2))  // time between photo
                                                        //
int sensor1count = 0;                                   //counter of times the sensor has been accessed
int sensor2count = 0;                                   //counter of times the sensor has been accessed
int State = 0;                                          //FOR TESTING ONLY WILL SWITCH FROM SPI CAMERA TO SERIAL CAMERA EVERY HOUR
int fieldInc = 0;
//
///////////////////////////////////////////////////////////////////////////
/**
   @brief Flying function is used to capture all logic of an experiment during flight.
*/
//************************************************************************
//   Beginning of the flight program setup

//
//
void Flying() {
  //
  Serial.println("\n\rRun flight program\n\r");
  //
  uint32_t TimeEvent1 = millis();  //set TimeEvent1 to effective 0
  uint32_t TimeEvent2 = millis();  //set TimeEvent1 to effective 0
  uint32_t TimeEvent3 = millis();  //set TimeEvent1 to effective 0
  uint32_t TimeEvent4 = millis();  //set TimeEvent1 to effective 0
  uint32_t TimeEvent5 = millis();  //set TimeEvent1 to effective 0


  uint32_t Sensor1Timer = millis();       //clear sensor1Timer to effective 0
  uint32_t Sensor2Timer = millis();       //clear sensor1Timer to effective 0
  uint32_t Sensor2Deadmillis = millis();  //clear mills for difference
  //
  uint32_t one_secTimer = millis();  //set happens every second
  uint32_t sec60Timer = millis();    //set minute timer

  //*****************************************************************
  //   Here to set up flight conditions i/o pins, atod, and other special condition
  //   of your program
  //
  //
  //
  //******************************************************************

  //pump liquid, pump co2, electric field on/off, take photo

  //------------ flying -----------------------

  Serial.println("Flying NOW  -  x=abort");              //terminal output for abort back to test
  Serial.println("Terminal must be reset after abort");  //terminal reset requirement upon soft reset

  missionMillis = millis();  //Set mission clock millis, you just entered flight conditions
  //
  //
  /////////////////////////////////////////////////////
  //----- Here to start a flight from a reset ---------
  /////////////////////////////////////////////////////
  //
  DateTime now = rtc.now();                  //get time now
  currentunix = (now.unixtime());            //get current unix time, don't count time not flying
  writelongfram(currentunix, PreviousUnix);  //set fram Mission time start now counting seconds unix time
                                             //
                                             //***********************************************************************
                                             //***********************************************************************
                                             //  All Flight conditions are now set up,  NOW to enter flight operations
                                             //
                                             //***********************************************************************
                                             //***********************************************************************
                                             //

  analogWrite(A0, 0);

  digitalWrite(IO1, LOW);
  digitalWrite(IO2, LOW);
  digitalWrite(IO3, LOW);
  digitalWrite(IO4, LOW);
  digitalWrite(IO5, LOW);
  digitalWrite(IO6, LOW);
  digitalWrite(IO7, LOW);  

  delay(one_day / SpeedFactor);  //24 hour wait before project
  

  while (1) {
    //
    //----------- Test for terminal abort command (x) from flying ----------------------
    //
    while (Serial.available()) {  //Abort flight program progress
      byte x = Serial.read();     //get the input byte
      if (x == 'x') {             //check the byte for an abort x
        return;                   //return back to poeration sellection
      }                           //end check
    }                             //end abort check
                                  //------------------------------------------------------------------
                                  //
                                  //*********** Timed Event 1 test ***************************************
                                  //
    //  this test if TimeEvent1 time has come
    //  See above for TimeEvent1_time settings between this event
    //
    if ((millis() - TimeEvent1) > TimeEvent1_time) {  
      TimeEvent1 = millis();                          //yes is time now reset TimeEvent1
                                                      //  Take a photo using the serial c329 camera and place file name in Queue
      Serial.println("time event 1 started");
      fieldInc++;
      Serial.println("analog increment");
      if      (fieldInc == 1) analogWrite(A0, 20339);
      else if (fieldInc == 2) analogWrite(A0, 19831);
      else if (fieldInc == 3) analogWrite(A0, 19267);
      else if (fieldInc == 4) analogWrite(A0, 18756);
      else if (fieldInc == 5) analogWrite(A0, 18305);
      else if (fieldInc == 6) analogWrite(A0, 17882);
      else if (fieldInc == 7) analogWrite(A0, 17439);
      else if (fieldInc == 8) analogWrite(A0, 16781); 
      //pump fluid

      digitalWrite(IO1, LOW);
      digitalWrite(IO2, LOW);
      digitalWrite(IO3, LOW);
      digitalWrite(IO4, LOW);
      digitalWrite(IO5, LOW);
      digitalWrite(IO6, HIGH);
      digitalWrite(IO7, HIGH);    //left tube pump right suck

      Serial.println("first liquid pump");

      TimeEvent2 = millis();
      while (1) { 
        if((millis() - TimeEvent2) > TimeEvent2_time ) { //amnt of time to pump
          TimeEvent2 = 0;
          Serial.println("time event 2 hit");
          digitalWrite(IO6, LOW);
          digitalWrite(IO7, LOW);    //stop pumping fluid
          break;
        }
        if ((millis() - one_secTimer) > one_sec) {  //one sec counter
          one_secTimer = millis();                  //reset one second timer
          DotStarYellow();                          //turn on Yellow DotStar to Blink for running
          //
          //****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
          // DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
          //    Mission clock timer
          //    FRAM keep track of cunlitive power on time
          //    and RTC with unix seconds
          //------------------------------------------------------------------------------
          DateTime now = rtc.now();        //get the time time,don't know how long away
          currentunix = (now.unixtime());  //get current unix time
          Serial.print(currentunix);
          Serial.print(" ");                                                        //testing print unix clock
          uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix));  //get delta sec of unix time
          uint32_t cumunix = readlongFromfram(CumUnix);                             //Get cumulative unix mission clock
          writelongfram((cumunix + framdeltaunix), CumUnix);                        //add and Save cumulative unix time Mission
          writelongfram(currentunix, PreviousUnix);                                 //reset PreviousUnix to current for next time
                                                                                    //
                                                                                    //********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
          //
          //  This part prints out every second
          //
          Serial.print(": Mission Clock = ");       //testing print mission clock
          Serial.print(readlongFromfram(CumUnix));  //mission clock
          Serial.print(" is ");                     //spacer
          //
          //------Output to the terminal  days hours min sec
          //
          getmissionclk();
          Serial.print(xd);
          Serial.print(" Days  ");
          Serial.print(xh);
          Serial.print(" Hours  ");
          Serial.print(xm);
          Serial.print(" Min  ");
          Serial.print(xs);
          Serial.println(" Sec");
          //
          //
          DotStarOff();
        } 

      }
      
      //activate electric field

      //analogWrite(A0, 4750); //increment voltage

      // take photo

      Serial.println("first photo");

      digitalWrite(IO5, HIGH);
      //delay(1000);  TEST TO SEE IF THIS NEEDS A DELAYY
      cmd_takeSpiphoto();
      digitalWrite(IO5, LOW);

      Serial.println("pump CO2");

      digitalWrite(IO1, HIGH);                          // replace with CO2 pin
      digitalWrite(IO4, HIGH);                          // replace with CO2 pin

      TimeEvent3 = millis();
      while (1) { 
        if((millis() - TimeEvent3) > TimeEvent3_time ) { //FIND TIME TO PUMP CO2;
          TimeEvent3 = 0;
          Serial.println("time event 3 hit");
          digitalWrite(IO1, LOW);
          digitalWrite(IO4, LOW);    //stop pumping fluid
          break;
        }
        if ((millis() - one_secTimer) > one_sec) {  //one sec counter
          one_secTimer = millis();                  //reset one second timer
          DotStarYellow();                          //turn on Yellow DotStar to Blink for running
          //
          //****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
          // DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
          //    Mission clock timer
          //    FRAM keep track of cunlitive power on time
          //    and RTC with unix seconds
          //------------------------------------------------------------------------------
          DateTime now = rtc.now();        //get the time time,don't know how long away
          currentunix = (now.unixtime());  //get current unix time
          Serial.print(currentunix);
          Serial.print(" ");                                                        //testing print unix clock
          uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix));  //get delta sec of unix time
          uint32_t cumunix = readlongFromfram(CumUnix);                             //Get cumulative unix mission clock
          writelongfram((cumunix + framdeltaunix), CumUnix);                        //add and Save cumulative unix time Mission
          writelongfram(currentunix, PreviousUnix);                                 //reset PreviousUnix to current for next time
                                                                                    //
                                                                                    //********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
          //
          //  This part prints out every second
          //
          Serial.print(": Mission Clock = ");       //testing print mission clock
          Serial.print(readlongFromfram(CumUnix));  //mission clock
          Serial.print(" is ");                     //spacer
          //
          //------Output to the terminal  days hours min sec
          //
          getmissionclk();
          Serial.print(xd);
          Serial.print(" Days  ");
          Serial.print(xh);
          Serial.print(" Hours  ");
          Serial.print(xm);
          Serial.print(" Min  ");
          Serial.print(xs);
          Serial.println(" Sec");
          //
          //
          DotStarOff();
        } 
      }

      //loop 40 iteration
        //take photo
        // wait 3 sec

        //suck fluid at 20
          //pump fluid
          //pump co2
      
      for (int i = 1; i < 20; i++) {
        //sync is failing here
        digitalWrite(IO5, HIGH);

        delay(1000);

        cmd_takeSpiphoto();
        digitalWrite(IO5, LOW);

        

        if (i % 10 == 10) {
          Serial.println("suck fluid"); //SWITCH

          digitalWrite(IO2, HIGH);
          digitalWrite(IO3, HIGH);
          digitalWrite(IO6, LOW);
          digitalWrite(IO7, LOW);  //suck fluid from chamber

          TimeEvent4 = millis();
          while (1) { 
            if((millis() - TimeEvent4) > TimeEvent4_time ) { //FIND TIME TO SUCJ FLUID;
              
              TimeEvent4 = 0;
              digitalWrite(IO2, LOW);
              digitalWrite(IO3, LOW); //off
              Serial.println("done sucking fluid");
              break;
            }
            if ((millis() - one_secTimer) > one_sec) {  //one sec counter
              one_secTimer = millis();                  //reset one second timer
              DotStarYellow();                          //turn on Yellow DotStar to Blink for running
              //
              //****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
              // DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
              //    Mission clock timer
              //    FRAM keep track of cunlitive power on time
              //    and RTC with unix seconds
              //------------------------------------------------------------------------------
              DateTime now = rtc.now();        //get the time time,don't know how long away
              currentunix = (now.unixtime());  //get current unix time
              Serial.print(currentunix);
              Serial.print(" ");                                                        //testing print unix clock
              uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix));  //get delta sec of unix time
              uint32_t cumunix = readlongFromfram(CumUnix);                             //Get cumulative unix mission clock
              writelongfram((cumunix + framdeltaunix), CumUnix);                        //add and Save cumulative unix time Mission
              writelongfram(currentunix, PreviousUnix);                                 //reset PreviousUnix to current for next time
                                                                                        //
                                                                                        //********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
              //
              //  This part prints out every second
              //
              Serial.print(": Mission Clock = ");       //testing print mission clock
              Serial.print(readlongFromfram(CumUnix));  //mission clock
              Serial.print(" is ");                     //spacer
              //
              //------Output to the terminal  days hours min sec
              //
              getmissionclk();
              Serial.print(xd);
              Serial.print(" Days  ");
              Serial.print(xh);
              Serial.print(" Hours  ");
              Serial.print(xm);
              Serial.print(" Min  ");
              Serial.print(xs);
              Serial.println(" Sec");
              //
              //
              DotStarOff();
            } 
          }

          delay(1000);

          //delay(5000); need a delay ?

          Serial.println("pumping fluid");
          digitalWrite(IO6, HIGH);
          digitalWrite(IO7, HIGH);  //pump fluid into chamber

          TimeEvent2 = millis();
          while (1) { 
            if((millis() - TimeEvent2) > TimeEvent2_time ) { //amnt of time to pump
              TimeEvent2 = 0;
              digitalWrite(IO6, LOW);
              digitalWrite(IO7, LOW);    //stop pumping fluid
              Serial.println("done pumping");
              break;
            }
            if ((millis() - one_secTimer) > one_sec) {  //one sec counter
              one_secTimer = millis();                  //reset one second timer
              DotStarYellow();                          //turn on Yellow DotStar to Blink for running
              //
              //****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
              // DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
              //    Mission clock timer
              //    FRAM keep track of cunlitive power on time
              //    and RTC with unix seconds
              //------------------------------------------------------------------------------
              DateTime now = rtc.now();        //get the time time,don't know how long away
              currentunix = (now.unixtime());  //get current unix time
              Serial.print(currentunix);
              Serial.print(" ");                                                        //testing print unix clock
              uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix));  //get delta sec of unix time
              uint32_t cumunix = readlongFromfram(CumUnix);                             //Get cumulative unix mission clock
              writelongfram((cumunix + framdeltaunix), CumUnix);                        //add and Save cumulative unix time Mission
              writelongfram(currentunix, PreviousUnix);                                 //reset PreviousUnix to current for next time
                                                                                        //
                                                                                        //********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
              //
              //  This part prints out every second
              //
              Serial.print(": Mission Clock = ");       //testing print mission clock
              Serial.print(readlongFromfram(CumUnix));  //mission clock
              Serial.print(" is ");                     //spacer
              //
              //------Output to the terminal  days hours min sec
              //
              getmissionclk();
              Serial.print(xd);
              Serial.print(" Days  ");
              Serial.print(xh);
              Serial.print(" Hours  ");
              Serial.print(xm);
              Serial.print(" Min  ");
              Serial.print(xs);
              Serial.println(" Sec");
              //
              //
              DotStarOff();
            }             
          } 
        }
      }

      // deactivate electric feild
      analogWrite(A0, 0);
      
    }  //end of TimeEvent1_time
    //------------------------------------------------------------------
    //
    //*********** Timed Event 1 test ***************************************
    //
    //  this test if TimeEvent2 time has come
    //  See above for TimeEvent2_time settings between this event
    //
    // if ((millis() - TimeEvent2) > TimeEvent2_time ) {  //FLUID PUMP TIME

    //   //if(readbyteFromfram(EVENT_FLAGADRESS) != ON_TRUE){

    //   //}
    //   Serial.print("time event 2 started");
    //   TimeEvent2 = millis();                          //yes is time now reset TimeEvent2

    //   digitalWrite(IO0, LOW);                          // replace with CO2 pin
    //   digitalWrite(IO1, LOW);                          // replace with CO2 pin
    //   //writebytefram(ON_TRUE, ADRESS);
    //   for (int i = 0; i < 20; i++) {                  //num of photos taken
    //     cmd_takeSphoto();
    //     delay(12000);  // time taken to pump CO2
    //   }
    //   digitalWrite(IO0, HIGH);                          // replace with CO2 pin
    //   digitalWrite(IO1, HIGH);  
    // }  //end of TimeEvent2_time
    //------------------------------------------------------------------
    // if ((millis() - TimeEvent3) > TimeEvent3_time) {  //Liquid Pump Time Event
    //   Serial.print("time event 3 started");
    //   TimeEvent3 = millis();                          //yes is time now reset TimeEvent3
    //   digitalWrite(1, HIGH);                          // replace with pump pin
    //   delay(1000);                                    // time taken to pump
    //   //analogWrite(A0, 4750);
    // }

    // if ((millis() - TimeEvent4) > TimeEvent4_time) {  //liquid suck event
    //   Serial.print("time event 4 started");
    //   TimeEvent4 = millis();                          //yes is time now reset TimeEvent4
    //   //analogWrite(A0, 0);
    //   delay(1000);
    //   digitalWrite(IO2, HIGH);
    //   digitalWrite(IO3, HIGH);
    //   digitalWrite(IO6, LOW);                          // replace with fluid pin
    //   digitalWrite(IO7, LOW);                          // replace with fluid pin -- IO6,7 change to high/low to change directin
    //   delay(1000);            //time taken to suck
    //   digitalWrite(IO2, LOW);
    //   digitalWrite(IO3, LOW);
    // }

    //*******************************************************************************
    //*********** One second counter timer will trigger every second ****************
    //*******************************************************************************
    //  Here one sec timer - every second
    //
    if ((millis() - one_secTimer) > one_sec) {  //one sec counter
      one_secTimer = millis();                  //reset one second timer
      DotStarYellow();                          //turn on Yellow DotStar to Blink for running
      //
      //****************** NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_NO_ *************************
      // DO NOT TOUCH THIS CODE IT IS NECESARY FOR PROPER MISSION CLOCK OPERATIONS
      //    Mission clock timer
      //    FRAM keep track of cunlitive power on time
      //    and RTC with unix seconds
      //------------------------------------------------------------------------------
      DateTime now = rtc.now();        //get the time time,don't know how long away
      currentunix = (now.unixtime());  //get current unix time
      Serial.print(currentunix);
      Serial.print(" ");                                                        //testing print unix clock
      uint32_t framdeltaunix = (currentunix - readlongFromfram(PreviousUnix));  //get delta sec of unix time
      uint32_t cumunix = readlongFromfram(CumUnix);                             //Get cumulative unix mission clock
      writelongfram((cumunix + framdeltaunix), CumUnix);                        //add and Save cumulative unix time Mission
      writelongfram(currentunix, PreviousUnix);                                 //reset PreviousUnix to current for next time
                                                                                //
                                                                                //********* END_NO_END_NO_END_NO_END_NO_END_NO_END_NO_ **************************
      //
      //  This part prints out every second
      //
      Serial.print(": Mission Clock = ");       //testing print mission clock
      Serial.print(readlongFromfram(CumUnix));  //mission clock
      Serial.print(" is ");                     //spacer
      //
      //------Output to the terminal  days hours min sec
      //
      getmissionclk();
      Serial.print(xd);
      Serial.print(" Days  ");
      Serial.print(xh);
      Serial.print(" Hours  ");
      Serial.print(xm);
      Serial.print(" Min  ");
      Serial.print(xs);
      Serial.println(" Sec");
      //
      //
      DotStarOff();
    }  // end of one second routine
       //
       //**********************************************************************
       //*********** Read Sensor1 Event read and add to text buffer************
       //**********************************************************************
    //
    // if ((millis() - Sensor1Timer) > Sensor1time) {    //Is it time to read?
    //   Sensor1Timer = millis();                        //Yes, lets read the sensor1
    //   sensor1count++;
    //   int value1 = sensor1count;              //sensor count number up from zero
    //   int value2 = 55000;                     //SIMULATED SENSOR VALUE,need to calculate real value
    //   int value3 = 14;                        //SIMULATED SENSOR VALUE,need to calculate real value
    //   //
    //   add2text(value1, value2, value3);       //add the values to the text buffer
    //   //
    // }     // End of Sensor1 time event
    //
    //**********************************************************************
    //*********** Read Sensor2 Event read and add to text buffer************
    //*********** Test of filling the 30K data buffer for lots of data *****
    //**********************************************************************
    //  If it is event driven then remove the Sensor2Timer evvvvvend
    //  here to get the  data for the event
    //
    //   if ((millis() - Sensor2Timer) > Sensor2time) {    //Is it time to read?
    //     Sensor2Timer = millis();                        //Yes, lets read the sensor1
    //     sensor2count++;
    //     //
    //     //  Here to calculate and store data
    //     //
    //     int Deadtime = millis()-Sensor2Deadmillis;      //time in millis sence last visit
    //     Sensor2Deadmillis = millis();                   //set millis this visit
    //     //
    //     //**** now get ampli and SiPM *****
    //     int ampli = 555;              //SIMULATED
    //     int SiPM  = 888;              //SIMULATED
    //     //***** end simulated *************
    //     //
    //     dataappend(sensor2count, ampli, SiPM, Deadtime);
    //   }     // End of Sensor2Timer
  }  // End of while
}  //End nof Flighting
//
//
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//    This is a function to adds three values to the user_text_buffer
//    Written specificy for 2023-2024 Team F, Team B,
//    Enter the function with "add2text(1st interger value, 2nd intergre value, 3rd intergervalue);
//    the " - value1 " text can be changed to lable the value or removed to same space
//    ", value2 " and ", value 3 " masy also be removed or changed to a lable.
//    Space availiable is 1024 bytes, Note- - each Data line has a ncarrage return and a line feed
//
//example of calling routine:
//       //
//      int value1 = 55;
//      int value2 = 55000;
//      int value3 = 14;
//      add2text(value1, value2, value3);
//      //
//EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
//
void add2text(int value1, int value2, int value3) {                //Add value to text file
  if (strlen(user_text_buf0) >= (sizeof(user_text_buf0) - 100)) {  //Check for full
    Serial.println("text buffer full");                            //yes, say so
    return;                                                        //back to calling
  }
  char temp[11];                    // Maximum number of digits for a 32-bit integer
  int index = 10;                   //Start from the end of the temperary buffer
  char str[12];                     //digits + null terminator
                                    //--------- get time and convert to str for entry into text buffer ----
  DateTime now = rtc.now();         //get time of entry
  uint32_t value = now.unixtime();  //get unix time from entry
  do {
    temp[index--] = '0' + (value % 10);  // Convert the least significant digit to ASCII
    value /= 10;                         // Move to the next digit
  } while (value != 0);
  strcpy(str, temp + index + 1);  // Copy the result to the output string
                                  //---------- end of time conversion uni time is now in str -------------
  strcat(user_text_buf0, (str));  //write unix time
  //
  // unit time finish entry into this data line
  //
  strcat(user_text_buf0, (" - count= "));  // seperator
  strcat(user_text_buf0, (itoa(value1, ascii, 10)));
  strcat(user_text_buf0, (", value2= "));
  strcat(user_text_buf0, (itoa(value2, ascii, 10)));
  strcat(user_text_buf0, (", value3= "));
  strcat(user_text_buf0, (itoa(value3, ascii, 10)));
  strcat(user_text_buf0, ("\r\n"));

  //Serial.println(strlen(user_text_buf0));  //for testing
}
//------end of Function to add to user text buffer ------
//
//=============================================================================
//
////FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  Function to write into a 30K databuffer
//    char databuffer[30000];         // Create a character buffer with a size of 2KB
//    int databufferLength = 0;       // Initialize the buffer length
//  Append data to the large data buffer buffer always enter unit time of data added
//  enter: void dataappend(int counts, int ampli, int SiPM, int Deadtime) (4 values)
//
void dataappend(int counts, int ampli, int SiPM, int Deadtime) {  //entry, add line with values to databuffer
  //----- get and set time to entry -----
  DateTime now = rtc.now();                     //get time of entry
  String stringValue = String(now.unixtime());  //convert unix time to string
  const char* charValue = stringValue.c_str();  //convert to a C string value
  appendToBuffer(charValue);                    //Sent unix time to databuffer
  //----- add formated string to buffer -----
  String results = " - " + String(counts) + " " + String(ampli) + " " + String(SiPM) + " " + String(Deadtime) + "\r\n";  //format databuffer entry
  const char* charValue1 = results.c_str();                                                                              //convert to a C string value
  appendToBuffer(charValue1);                                                                                            //Send formated string to databuff
  //
  //  Serial.println(databufferLength);                                   //print buffer length for testing only
}
//-----------------------                                               //end dataappend
//----- sub part od dataappend -- append to Buffer -----
//-----------------------
void appendToBuffer(const char* data) {                      //enter with charator string to append
  int dataLength = strlen(data);                             //define the length of data to append
                                                             // ----- Check if there is enough space in the buffer                           //enough space?
  if (databufferLength + dataLength < sizeof(databuffer)) {  //enouth space left in buffer
    // ----- Append the data to the buffer
    strcat(databuffer, data);        //yes enough space, add data to end of buffer
    databufferLength += dataLength;  //change to length of the buffer
  } else {
    Serial.println("Buffer is full. Data not appended.");  //Not enough space, say so on terminal
  }                                                        //end not enough space
}  //end appendToBuffer
//

//=================================================================================================================
//
