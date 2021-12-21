#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
LiquidCrystal_I2C lcd(0x3F, 20, 4);
const int nb_bouton = 36;

const int BUTTON_ARDUINO_PIN[nb_bouton]=   {9,51,10,50,11,12,49,13,48,52,47,53  ,46,39,45,38,44,43,37,42,36,41,35,40, 34,8,33,26,32,31,25,30,24,29,23,28};
const int listeDiese[20]={1,3,99,6,8,10, 99,13,15,99,18,20,22, 99,25,27,99,30,32,34};
const int listeNormal[20]={0,2,4,5,7,9,11 ,12,14,16,17,19,21,23 ,24,26,28,29,31,33};
int buttonCState[nb_bouton] = {};        // Valeur Actuel /Pas les fachos ;
int buttonPState[nb_bouton] = {};        // Valeur Precedente
unsigned long lastDebounceTime[nb_bouton] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 50; 


// POTENTIOMETERS
const int N_POTS = 12; //* total numbers of pots (slide & rotary)
const int POT_ARDUINO_PIN[N_POTS] = {A0,A1,A2,A3,A4,A5,A6,A8,A9,A10,A11,A12}; //* pins of each pot connected straight to the Arduino
int potCState[N_POTS] = {0}; // Current state of the pot
int potPState[N_POTS] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot
int midiCState[N_POTS] = {0}; // Current state of the midi value
int midiPState[N_POTS] = {0}; // Previous state of the midi value
const int TIMEOUT = 300; //* Amount of time the potentiometer will be read after it exceeds the varThreshold
const int varThreshold = 10; //* Threshold for the potentiometer signal variation
boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[N_POTS] = {0}; // Previously stored time
unsigned long timer[N_POTS] = {0}; // Stores the time that has elapsed since the timer was reset


//MIDI
byte midiCh = 1; //* MIDI channel to be used
byte note = 12; //* Lowest note to be used
byte cc = 12; //* Lowest MIDI CC to be used
byte velocity = 127; // VELOCITY 


void setup() {
  // put your setup code here, to run once:
  // 31250 for MIDI class compliant Possible que ca ne marche pas donc go 31250
  //
  //Serial.begin(115200);
  MIDI.begin();  
  lcd.init();
  lcd.backlight();
  //lcd.setCursor(1, 0);
  //lcd.print("HELLO");
  //initialisation des boutons
  //void LCDInitPrintpot();
  for (int i = 0; i < nb_bouton; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT);
  }
  void LCDInitPrintpot();
 

//initprintPiano();
}
void loop() {
  readPotard();
  readBouton();
}

///////////////////////////////////////////////////////////////
//Lecture des 12 potards + Envoie midi + Actualisation MIDI // 
/////////////////////////////////////////////////////////////
void readPotard(){
  for (int i = 0; i < N_POTS; i++) { // Loops through all the potentiometers
    potCState[i] = analogRead(POT_ARDUINO_PIN[i]); // reads the pins from arduino
    midiCState[i] = map(potCState[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi
    potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot
    if (potVar > varThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis(); // Stores the previous time
    }
    timer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms
    if (timer[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving = true;
    }
    else {
      potMoving = false;
    }
    if (potMoving == true) { // If the potentiometer is still moving, send the change control
      if (midiPState[i] != midiCState[i]) {
        if(i==0){
          note=map(midiCState[0],0,127,0,10)*12;
          LCDPrintPot(i,map(midiCState[i],0,127,0,100));
        }
        else if(i==1){
          velocity = midiCState[1];
          LCDPrintPot(i,map(midiCState[i],0,127,0,100));
        }
        else{
            MIDI.sendControlChange(cc + i, midiCState[i], midiCh); // cc number, cc value, midi channel
            LCDPrintPot(i,map(midiCState[i],0,127,0,100));
            potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
            midiPState[i] = midiCState[i];
        }
      }
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//Lecture des Boutons avec Debouce ON / OFF + et envoie de la note midi + Actualisation du LCD//
///////////////////////////////////////////////////////////////////////////////////////////////
void readBouton(){
  for (int i = 0; i < nb_bouton; i++) {
    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);  // read pins from arduino
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();
        if (buttonCState[i] == LOW) {
         MIDI.sendNoteOn(note + i, velocity , midiCh); // note, velocity, channel
          LCDOnPiano(i);
        }
        else {         
          MIDI.sendNoteOff(note + i, 0, midiCh); // note, velocity, channel
          LCDOffPiano(i);
        }
        buttonPState[i] = buttonCState[i];
      }
    }
  }
}


///////////////////////////
//Gestion de l'ecran LCD//
/////////////////////////
void LCDOnPiano(int note){
  //Check if #
  for(int i=0; i<20; i++){
    if (note == listeDiese[i]){
      lcd.setCursor(i,0);
      lcd.print("|");
    }
  }
  for(int i=0; i<20; i++){
    if (note==listeNormal[i]){
      //Serial.print("ListeNormal Hit");
      lcd.setCursor(i,1);
      lcd.print("|");
    }
  }
}
void LCDOffPiano(int note){ 
  //Check if #
  for(int i=0; i<20; i++){
    if (note==listeDiese[i]){
      lcd.setCursor(i,0);
      lcd.print("O");
    }
  }
  for(int i=0; i<20; i++){
    if (note==listeNormal[i]){
      lcd.setCursor(i,1);
      lcd.print("O");
    }
  }
}
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//Affichage des Potards sur le LCD Cette fonction est très malecrite mais tant pis dans les fais pas plus couteuse que ca juste moche//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!// 
void LCDPrintPot(int n_pot, int val){
  if(n_pot==0){
      if(val==100){
        val=10;
      }
    lcd.setCursor(0,3); 
    lcd.print("Off");
    lcd.print(val);
  }
  if (n_pot==2){
       if(val==100){
        val=10;
      }
    lcd.setCursor(6,3);
    lcd.print(val);
  }
  if (n_pot==5){
      if(val==100){
        val=10;
      }
    lcd.setCursor(9,3);
    lcd.print(val);
  }
  if (n_pot==6){
      if(val==100){
        val=10;
      }
    lcd.setCursor(12,3);
    lcd.print(val);
  }
  if (n_pot==8){
      if(val==100){
        val=10;
      }
    lcd.setCursor(15,3);
    lcd.print(val);
  }
    if (n_pot==11){
      if(val==100){
        val=10;
      }
    lcd.setCursor(18,3);
    lcd.print(val);
  }
//////////////////
//Ligne du haut//
////////////////
  if(n_pot==1){
      if(val==100){
        val=10;
      }
    lcd.setCursor(0,2);
    lcd.print("Vel");
    lcd.print(val);
  }
  if (n_pot==3){
      if(val==100){
        val=10;
      }
    lcd.setCursor(6,2);
    lcd.print(val);
  }
  if (n_pot==4){
      if(val==100){
        val=10;
      }
    lcd.setCursor(9,2);
    lcd.print(val);
  }
  if (n_pot==7){
      if(val==100){
        val=10;
      
    }
    lcd.setCursor(12,2);
    lcd.print(val);
  }
  if (n_pot==9){
      if(val==100){
        val=10;
      }
    lcd.setCursor(15,2);
    lcd.print(val);
  }
    if (n_pot==10){
    lcd.setCursor(18,2);
    lcd.print(val);
  }
}
void LCDInitPrintpot(){
  lcd.setCursor(0,2);
  lcd.print("Vel:TU ES PA VR AI ME");
  lcd.setCursor(0,2);
  lcd.print("OFF:NT TR ES BE AU :)");
}
