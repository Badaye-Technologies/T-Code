// --- Secondary Core --- //
#include <EEPROM.h>
#include <Wire.h>
volatile char buffer[40];
volatile boolean receiveFlag = false;

//void receive(int howMany){  Wire.readBytes(buffer, howMany); receiveFlag = true;}

#include <SPI.h>
#include <SD.h>
const int CS = A0;
File local_storage;
String dataFile = "data.txt";

//#include "DFRobotDFPlayerMini.h"

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


SoftwareSerial mySoftwareSerial(10, 9); // RX, TX

DFRobotDFPlayerMini MP3;

const byte Previous = A1;
const byte PlayPause = A2;
const byte Next = A3;
byte Vol = 10;


boolean isPlaying = false;

const byte trigger = 7, echo = 8; //7 8
const byte sopa = 5, wota = 6;
int sopa_delay = 2000;
int water_del = 5000;

int flash = 3;
unsigned int x = 0;// complete_handwash ... 62k
unsigned int y = 0; // incomplete_handwash 62k

void setup() {
  delay(1000);
  Wire.begin(8);                // join i2c bus with address #8
  Serial.begin(9600);           // start serial for output

pinMode(PlayPause, INPUT);//digitalWrite(buttonPause,HIGH);
pinMode(Next, INPUT);//digitalWrite(buttonNext,HIGH);
pinMode(Previous, INPUT);//digitalWrite(buttonPrevious,HIGH);

pinMode(sopa, OUTPUT); 
pinMode(wota, OUTPUT); 

pinMode(flash, OUTPUT); digitalWrite(flash, HIGH);

//digitalWrite(wota, HIGH); digitalWrite(sopa, HIGH); 

mySoftwareSerial.begin(9600);
//MP3.begin (9600);

//playFirst(); //boot sound
//isPlaying = true;


  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!MP3.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  MP3.volume(Vol);  //Set volume value. From 0 to 30
  
  
 pinMode(trigger, OUTPUT); // Sets the trigPin as an OUTPUT
 pinMode(echo, INPUT); // Sets the echoPin as an INPUT
 
 if (!SD.begin(CS)) {
    Serial.println("Sd Card initialization failed!");
    while (1);
  }
    Serial.println("Card initialization done.");

  delay(100);

  //Wire.onReceive(receiveEvent); // register event
   //delay(500);
}


// function that executes whenever data is received from master
// this function is registered as an event, see setup()
String stringie = "Received: ";
void receiveEvent(int howMany) {
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    
    Serial.print(c);         // print the character
  }
   
  int waya = Wire.read();    // receive byte as an integer
  stringie += waya;
  Serial.println(waya);         // print the integer
  Serial.println(stringie);
}


byte distance1 = 0, distance2 = 0;
bool sonar_disabled = false;

int z = 0; byte currentMenu=1;
void loop() {
 
  x++; 
  /*** Cellular Network time synchronization
  * 1. look for the hands
  * 2. trigger soap pump
  * 3. play audio file
  * 4. trigger water pump 
  * 5. log data on EEPROM && SD Card
  * 6. Bind & Transmit Data when time is right
  * 7. communicate with the other IC
  ***/
  distance1 = 0; 
  
      distance1 = sonar(trigger, echo);

      if(distance1 == 0){ return; }
else{ 
      if(distance1 <= 15){ analogWrite(flash, 64); } //Serial.print(" :: Too Close :: ");  //too near
else  if(distance1 >  30) {if(x<100) analogWrite(flash, 254); else if(x>=100 && x<150) analogWrite(flash, 0); else x=0; }//Serial.print(" :: Too far!");  // too far      
else  if(distance1 <= 30){ //15 - 30 cm
    //sonar_disabled = true;
    analogWrite(flash, 0);
                          Serial.print("MP3 State: "); Serial.println(MP3.readState());
      MP3.play(1);  //Play the first mp3
                          Serial.print("MP3 State: "); Serial.println(MP3.readState());
      //delay(100);
                          Serial.print("MP3 State: "); Serial.println(MP3.readState());
      trigger_soap(sopa_delay);  delay(4500);//range
     Serial.println("Soaping finished!");
    
      //1. call them back 
     //2. detect if they have come
        //3. if(they _have)trigger water
       //4 && log complete handwash
      //if not summon them back up to third time
     //log it as an incompleete handwash
      int callBack = 0;
      //while(callBack<3){callBack++; 
      MP3.play(2); //call them back
      
      int count = 0; distance2 = 0;
    while(count < 240){ Serial.println(count);
          distance2 = sonar(trigger, echo); Serial.print("Distance: "); Serial.println(distance2);
         if(distance2 >= 5 && distance2 <= 50) { Serial.println("Watering Now!");
           analogWrite(flash, 0);
           trigger_wota(water_del);
            x++; //abamaze
            log_handwash();
            break; 
           }

           count++;
       }
      // if(callBack>1)delay(100);
     //} // close call_back
        
    }
  
}

 


currentMenu = buttonScan();
  
Serial.print("Current Menu Selection: "); Serial.println(currentMenu);
Serial.print("Water Mode: "); Serial.println(water_del);
Serial.print("Soap Mode: "); Serial.println(sopa_delay);
Serial.print("Volume Mode: "); Serial.println(Vol);
Serial.println();
z++;
}//loop close


byte buttonScan(){
int Play, Nxt, Prev; int buttonPressed = 0;
  Play = analogRead(PlayPause);
  Nxt = analogRead(Next);
  Prev = analogRead(Previous);

if(Play <= 50 || Nxt <= 50 || Prev <= 50){
  delay(500);
}

if(Play <= 50 && currentMenu <= 5){
  currentMenu++;
}

if(Play <= 50 && currentMenu > 5){
  currentMenu = 1;
}

if(currentMenu == 1){
  if(Nxt <= 50){ if(Vol<30){Vol+=5; MP3.volume(Vol);} //store the setting to the SD card
  Serial.print("Volume Up: "); Serial.println(Vol); Serial.println();}
  if(Prev <= 50){if(Vol>5){Vol-=5; MP3.volume(Vol);}
  Serial.print("Volume Down: "); Serial.println(Vol);Serial.println();}
  }

else if(currentMenu == 2){
  if(Nxt <= 50){ if(sopa_delay<3000) sopa_delay+=100;  //store the setting to the SD card
    Serial.print("Added Delay"); Serial.println(sopa_delay);}
  if(Prev <= 50){ if(sopa_delay>2000) sopa_delay-=100;
    Serial.print("Reduced Delay: "); Serial.println(sopa_delay);}
  }

else if(currentMenu == 3){
  if(Nxt <= 50){ if(water_del<6000) water_del+=100;  //store the setting to the SD card
    Serial.print("Added Delay"); Serial.println(water_del);}
  if(Prev <= 50){ if(water_del>5000) water_del-=100;
    Serial.print("Reduced Delay: "); Serial.println(water_del);}
  }

else if(currentMenu == 4){
  
}

  z=0;

// === Serial.print("Current Menu"); Serial.println(currentMenu); === //
return currentMenu;
}


bool logged = false;
bool log_handwash(){
      String Message;
      Serial.println("Saving Hand-wash");
  if(!SD.exists(dataFile)) {Serial.print(dataFile); Serial.println(" doesn't exist"); //dataFile = "/" + dataFile; //create it  
                             if(SD.mkdir(dataFile)) Serial.println("Created!"); else {Serial.println("Failed to create File: "); Serial.print(dataFile); }}
  
   local_storage = SD.open(dataFile, FILE_WRITE);

  // if the file opened wello wello, jiwandikemu:
  if (local_storage) {
    Serial.print("Writing to "); Serial.println(dataFile); 

        local_storage.println();
        local_storage.println("Kit: BTL001 2021");
        local_storage.println("Location: Badaye HQ");
        local_storage.println("Log Date: Sun 4 Oct, 2021");
        local_storage.println("Log Time: 12:12pm");

        local_storage.print("Complete Hand-washes: "); local_storage.println(x);
        local_storage.print("InComplete Hand-washes: "); local_storage.println(y);
        /*
    for(int i=0; i<dataPoints; i++){
      for(int j=0; j<2; j++){
          local_storage.print(binded_for_datalog[i][j]);
        }
        local_storage.println();
      }
    */
        local_storage.println();
        
        local_storage.close(); Message = "Handwash Logged!";
    Serial.println(Message);
        logged = true;

  } else {
    // if the file didn't open, print an error:
    Message = "Error Opening " + dataFile;
    Serial.println(Message);
    }

  return logged;
}


void trigger_soap(int duration){
  Serial.println("Releasing soap...");
  digitalWrite(sopa, HIGH); delay(duration); digitalWrite(sopa, LOW);
}

void trigger_wota(int duration){
  Serial.println("Releasing Water...");
digitalWrite(wota, HIGH);
    delay(duration);//range
digitalWrite(wota, LOW);
}


unsigned long sonar(byte trigPin, byte echoPin){
//if(sonar_disabled) return 0;
//else{
// defines variables
long duration; // variable for the duration of sound wave travel
float distance; // variable for the distance measurement
 
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds

  digitalWrite(trigPin, HIGH);  delayMicroseconds(10);  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds

  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = 0.034 * duration / 2.00; // Speed of sound wave divided by 2 (go and back)

  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");  Serial.print(distance);  Serial.println(" cm");

  return (long) distance;
//}
}


  
