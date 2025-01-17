/* TW20FirmWare
 * 1. Keep Time
 * 2. i2C Transmit
 * 3. GSM send/receive
 * 4. Manage Power

*   task 1 Read battery voltage 
*   task 2 Display updated battery voltage in % in status bar and in V on power 
*   task 3 Disconnect loads if 30% and beep when < 20% 
*   task 4 Read solar voltage and take appropriate action  
*/


int loadLine = 9;
int batteryPin = A3;
int solarPin = A2;
int UMEME_PIN = A1;


String battMess = ""; bool bat_has_power = false;
String solMess = ""; 
bool startCharging = false; bool solarNow = false;


int indicator = 5;
bool flip = false;



//#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>


SoftwareSerial SIM(3, 2); // RX, TX

String registered;
char Kit_ID[] = "TW20 Master Kit";

void updateSerial();
unsigned long track_time(); 


String all_data = "=> ";
char read_byte; 

char acquired[70];
char stolen[17];

void updateSerial(){  delay(500);

  //while (Serial.available()){SIM.write(Serial.read()); } //Forward what Serial received to Software Serial Port
  int i=0;
  while(SIM.available())   { /*Serial.write(SIM.read());*/  read_byte = SIM.read(); acquired[i] = read_byte;  all_data += read_byte; i++;}
  //else {Serial.println("SIM not available!");}
}    




uint8_t sound_fx = 25;

// -- clock code
uint8_t pulsePin = 10;
unsigned int pulse;

int second=0, minute=0, hour=0, day=1, month=1, year=2021, Y1=20, Y2=21; 
//uint16_t year = 0;
const String zero = "0"; /*char Month[8] = "Null";*/ String Month;
uint8_t time_slot = EEPROM.read(1000);; 


void setup() {
pinMode(indicator, OUTPUT);
pinMode(batteryPin, INPUT); 
pinMode(solarPin, INPUT); 
pinMode(UMEME_PIN, INPUT);
pinMode(loadLine, OUTPUT);

digitalWrite(indicator, 1);

Wire.begin(); // join i2c bus (address optional for master)
Serial.begin(9600);
SIM.begin(9600);

 SIM.println("AT"); delay(500);
 SIM.println("AT+CCLK?");delay(500); // +CCLK: "04/01/01,00:14:12+22"
 SIM.println("AT+CLTS=1"); delay(500);// Enable auto network time sync : 
 SIM.println("AT+CLTS?");  delay(500);// Check if value is set : 
 SIM.println("AT&W"); delay(500); // Save the setting to permanent memory so that module enables sync on restart also  :  
 SIM.println("AT+CCLK?"); updateSerial(); //delay(500);// Read current Time

     //Serial.print("Last Character => "); Serial.println(read_byte);
     Serial.print("Captured from GSM => "); Serial.print(all_data);


for(int i=0; i<70; i++){
  //Serial.print(acquired[i]);
  if(acquired[i] == '"' && (acquired[i+1] == '2' || acquired[i+1] == '0')){//first occurence
    for(int j=0; j<17; j++){
      stolen[j] = acquired[i+j+1];
    }
   }
  }
digitalWrite(indicator, 0);
String Batt_Notification;
       Batt_Notification = power();

  // turn ON notification //
//for(int i=0; i<5; i++){}
Serial.println("Reporting...");

SIM.println("AT"); delay(200);
                    SIM.println("AT+CMGF=1\r"); delay(200);// Configuring TEXT mode
                    SIM.println("AT+CMGS=\"+256705551779\""); delay(200);  //Baidhe: 256705551779 
   
                    SIM.println(Kit_ID); SIM.println(" has just been switched ON."); SIM.println();
                    SIM.print("System Time => "); SIM.println(stolen); SIM.println();
                    SIM.print("System Power=> "); SIM.print(Batt_Notification); delay(200); updateSerial(); 
                      
                    SIM.write(26); delay(200);

SIM.println("AT"); delay(200);
                    SIM.println("AT+CMGF=1\r"); delay(200);// Configuring TEXT mode
                    SIM.println("AT+CMGS=\"+256701464155\""); delay(200);  //Muutu: 256758643906 / 256753958232, Mugaga: 256701464155, Muwonge: 256703564800    --- Recipient number (s) --- MasterLine 
   
                    SIM.println(Kit_ID); SIM.println(" has just been switched ON."); SIM.println();
                    SIM.print("System Time => "); SIM.println(stolen); SIM.println();
                    SIM.print("System Power=> "); SIM.print(Batt_Notification); delay(200); updateSerial(); 
                      
                    SIM.write(26); delay(200);

SIM.println("AT"); delay(200);
                    SIM.println("AT+CMGF=1\r"); delay(200);// Configuring TEXT mode
                    SIM.println("AT+CMGS=\"+256759991993\""); delay(200);  //Baidhe: 256705551779, Muutu: 256758643906 / 256753958232, Mugaga: 256701464155, Muwonge: 256703564800    --- Recipient number (s) --- MasterLine 
   
                    SIM.println(Kit_ID); SIM.println(" has just been switched ON."); SIM.println();
                    SIM.print("System Time => "); SIM.println(stolen); SIM.println();
                    SIM.print("System Power=> "); SIM.print(Batt_Notification); delay(200); updateSerial(); 
                      
                    SIM.write(26); delay(200);

}//close setup

int hand_wash = 0;

boolean send_unlocked = false;
boolean sent = false;

//the charging science //
float charging_voltage; 
bool isCharging = false, can_charge = true;

void loop(){ flip = !flip; digitalWrite(indicator, flip);


SIM.println("AT+CCLK?"); updateSerial();

for(int i=0; i<70; i++){
  //Serial.print(acquired[i]);
  if(acquired[i] == '"' && (acquired[i+1] == '2' || acquired[i+1] == '0')){//first occurence
    for(int j=0; j<17; j++){
      stolen[j] = acquired[i+j+1];
    }
   }
  }
//delay(500);

char concatenate[2];

for(int i=0; i<17; i++){

 if(stolen[i] == ','){ Serial.println();
    concatenate[0] = stolen[i+1]; //Serial.print("Char 1: "); Serial.println(concatenate[0]);
    concatenate[1] = stolen[i+2]; //Serial.print("Char 2: "); Serial.println(concatenate[1]);
    hour = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

    concatenate[0] = stolen[i+4];
    concatenate[1] = stolen[i+5];
    minute = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

    concatenate[0] = stolen[i+7];
    concatenate[1] = stolen[i+8];
    second = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

    //backward to date... up to i-7 && i-8
    concatenate[0] = stolen[i-2];
    concatenate[1] = stolen[i-1];
    day = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

    concatenate[0] = stolen[i-5];
    concatenate[1] = stolen[i-4];
    month = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

    concatenate[0] = stolen[i-8];
    concatenate[1] = stolen[i-7];
    year = 10 * (int(concatenate[0]) - 48)  + (int(concatenate[1]) - 48);

  break;    
  }
}
Serial.println();
Serial.print("Extracted time :: "); Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
Serial.println();

calendar();
Serial.print("Extracted Date => "); Serial.print(day); Serial.print("th "); Serial.print(Month);  Serial.print(", "); Serial.print(year);
Serial.println();

  
  //String send_data = "";
         //send_data = char(hour) + ":" + char(minute);
  /*
  Wire.beginTransmission(8); // transmit time to device #8
  Wire.read(); 
  Wire.endTransmission();
  */
  

String Data = "";

if(hand_wash==0) Data += "Not hand-wash yet";
else {Data += String(hand_wash);}
       Data += power();;

  if(hour == 13) send_unlocked = true;
  else {send_unlocked = false; sent = false;}


  if(send_unlocked && !sent){//if the time is right and message is not yet sent ... and update EEPROM send flag
    transmit(Data);
    }

Serial.println();Serial.println();
/*
Wire.beginTransmission(8); // transmit to device #8 to keep to the SD CARD
Wire.write("Battery Percent: ");
Wire.write(buttPercent);
Wire.write("%");
Wire.endTransmission();
*/    
}



void calendar(){
year += 2000;
    // shift to the display class{};
      if(month==1) Month = "January";    else if(month==2) Month = "February"; else if(month==3) Month = "March";
 else if(month==4) Month = "April";      else if(month==5) Month = "May";      else if(month==6) Month = "June";
 else if(month==7) Month = "July";       else if(month==8) Month = "August";   else if(month==9) Month = "September";
 else if(month==10)Month = "October";    else if(month==11) Month = "November";     else if(month==12) Month = "December"; 
//if day has changed 
/*
if(month == 2){ 
        if(year%4 != 0){if(day == 29){day=0; month++;}} // lunar
        else {if(day == 30){ day=1; month++; }} // leap year
      }
  else  if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12){
        if(day == 31){month++; day = 1;}
    }
  else { //month == April || month == June || month == September || month == November
    if(day == 32){month++; day=1;}
    }

  if(month == 13){year++; month = 1; }
*/
 
}

String power(){
  String PowerMessage = "";
  
if(!isCharging){

int sanya = 0, UMEME = 0;
for(int i=0; i<5; i++){
sanya = analogRead(UMEME_PIN);
UMEME += sanya;
//Serial.print("Each UMEME: "); Serial.println(sanya);
}
UMEME /= 5;

float UMEME_OC = 0.00;
UMEME_OC = UMEME * (13.00/478.00);
Serial.print("UMEME VOLT = "); Serial.println(UMEME_OC);


int sana = 0, Solar = 0;
for(int i=0; i<5; i++){
  sana = analogRead(solarPin);
  Solar += sana;

  //Serial.print("One Sol = "); Serial.println(sana);
  //Serial.print("Total Solar = "); Serial.println(Solar);
}
Solar /= 5;

float Solar_OC = 0.00;
Solar_OC = (13.00/478.00) * Solar;  
Serial.print("Solar Voltage = "); Serial.println(Solar_OC); 

       if(UMEME_OC > 14.00 && UMEME_OC >= Solar_OC){// --- divert load to UMEME if 15 or more...
            digitalWrite(loadLine, HIGH); 
            Serial.println("Line Routed to UMEME");
          }
  else if(Solar_OC > 15.00 && Solar_OC > UMEME){// - divert load to Solar if big enuf to run pumps ()
            digitalWrite(loadLine, LOW);
            Serial.println("Line Routed to Solar");

          }

  int readBattery = 0;

  int total_battery = 0, average_battery = 0;
  float loaded_battery = 0.00;   

int buttPercent = 0; float Butt_Percent = 0.00;

    for(int i=0; i<10; i++){ // till reading stabilizes
      readBattery   = analogRead(batteryPin); 
      total_battery += readBattery;
      //Serial.print("Battery => "); Serial.print(i+1); Serial.print(" "); Serial.println(readBattery);
      }
  average_battery = total_battery/10;
//Serial.print("Average Battery: ");  Serial.println(average_battery); 
  loaded_battery = float(average_battery);
    
    if(readBattery <= 0){bat_has_power = false; battMess = "Battery Removed!"; } // to avoid isnan() --- just in case
    else{
          if(!isCharging){
      
        loaded_battery = (13.00/478.00) * loaded_battery; //ceil(); floor();//rounds to int round(); 
        Serial.print("Voltage: "); Serial.println(loaded_battery); 

    if(loaded_battery < 11.00){   
      
           if(loaded_battery <= 6.99){battMess = "Battery Removed! V = " + (String)loaded_battery; bat_has_power = false;}
        
     else  if(loaded_battery>=7.00 && loaded_battery <=9.00){ //unacceptable battery level
                 battMess = "Battery Voltage Dangerously Low! V = " + (String)loaded_battery; bat_has_power = false;}
            
        else{ //battery present but critically low btn 9 to 11V
          battMess = "Battery Voltage Extremely Low! V = " + (String)loaded_battery; bat_has_power = false;
          Butt_Percent = map(loaded_battery, 9.000, 11.00, 0.000, 10.00);
           }
    }
   
  else {  // if atleast 11.00000001V still available on battery, its okay keep on...
               if(loaded_battery >= 12.70){ Butt_Percent = (loaded_battery/13.00)*100; bat_has_power = true; battMess = "Battery V = " + (String)loaded_battery + "V Power Excellent!";} // --- full battery
           
          else if(loaded_battery >= 12.20 && loaded_battery < 12.70){ Butt_Percent  =  (50*(loaded_battery-12.20)/(13.00-12.20))+50;   bat_has_power = true; battMess = "Battery V = "+(String)loaded_battery +"V Power Available!";} // --- linear interpolation
           
          else if(loaded_battery >= 11.70 && loaded_battery < 12.20){ Butt_Percent  = (25*(loaded_battery-11.70)/(12.20-11.70))+25;   bat_has_power = true; startCharging = true; battMess = "Power Running Low! V = " + (String)loaded_battery;}
           
          else if(loaded_battery >= 11.00 && loaded_battery < 11.70){ Butt_Percent  = (25*(loaded_battery-11.0)/(11.70-11.00));  bat_has_power = false; startCharging = true;  battMess = "Power Low! V = "+(String)loaded_battery;} //empty

                  Butt_Percent += 0.50;  // ---  0.5 to not truncate
          
          
        } //close critically low battery
       buttPercent = int(Butt_Percent);
    } // close if is not currently charging
}//close else of battery read




Serial.print("Battery => "); Serial.print(buttPercent); Serial.println("%");
Serial.println();
Serial.println(battMess);


//if(UMEME > 50){} Route Lod Line to UMEME
//if(Solar > 30 && UMEME < 50){} Route Load Line to Solar

/*
Serial.print("Solar => ");   Serial.println(ann);
Serial.print("UMEME => ");   Serial.println(ane);
Serial.println();
*/
String UMEME_Prompt = "";

    if(UMEME_OC < 5.00){UMEME_Prompt += "Not Connected";}
else {UMEME_Prompt += String(UMEME_OC);}

            PowerMessage = battMess;
            PowerMessage += "("+ String(Butt_Percent) +")";
            PowerMessage += "\n\n"; //"Battery : " + String(buttPercent) + "%";
            PowerMessage += "Solar = " + String(Solar_OC);
            PowerMessage += "V";
            PowerMessage += "UMEME: " + UMEME_Prompt;
  }


  return PowerMessage;
}

void transmit(String datum){
  Serial.println("Sending via GSM Network...");
                    SIM.println("AT"); //Initiate data transmission when a certain number of handwashs has been bundled up
                      updateSerial();
       
                    SIM.println("AT+CMGF=1\r"); // Configuring TEXT mode
                      updateSerial();

            
                    SIM.println("AT+CMGS=\"+256759991993\"");   //Baidhe: 256705551779, Muutu: 256758643906 / 256753958232, Mugaga: 256701464155, Muwonge: 256703564800    --- Recipient number (s) --- MasterLine 
                      updateSerial();
                      
                      SIM.print(Kit_ID); SIM.print(" Hand-washes Today:"); SIM.println();  updateSerial();
                      SIM.print(datum); updateSerial();    
                        
                    SIM.write(26);

                    sent = true;
}
