/*
 * PPM generator originally written by David Hasko
 * on https://code.google.com/p/generate-ppm-signal/ 
 */
                                                                  // TESTED ON BETAFLIGHT 
//////////////////////CONFIGURATION///////////////////////////////
#define CHANNEL_NUMBER 6  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1500 //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 10  //set PPM signal output pin on the arduino

/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[CHANNEL_NUMBER];
String a ="900,1000";
int throttile,aux1;
void setup(){  

  //initiallize default ppm values
  for(int i=0; i<CHANNEL_NUMBER; i++){
       ppm[0]= 1486; // pitch
       ppm[1]= 1486; //roll
       ppm[2]= 1000;  //throttile
       ppm[3]= 1486;  //yew
       ppm[4]= 1000;  //aux1
       ppm[5]= 1500;  //aux2
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();

  Serial.begin(9600);
}

void loop(){
  if(Serial.available()>0){
    a=Serial.readString();       // recive string
  }
String part01 = getValue(a,',',0);   //splt string
String part02 = getValue(a,',',1);
throttile = part01.toInt();          //convert string to int
aux1= part02.toInt();
  ppm[2]= throttile;                          
    ppm[4]= aux1;                   // set value to ppm
Serial.print(throttile);            // print in serial monitor
Serial.print("--");
Serial.println(aux1);
delay(1);
  
}

ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PULSE_LENGTH * 2;
    state = false;
  } else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      OCR1A = (FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PULSE_LENGTH) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}



String getValue(String data, char separator, int index)    //string spliter fuction
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
