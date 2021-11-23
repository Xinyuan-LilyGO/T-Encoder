#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

/*
connecting Rotary encoder

Rotary encoder side    MICROCONTROLLER side  
-------------------    ---------------------------------------------------------------------
CLK (A pin)            any microcontroler intput pin with interrupt -> in this example pin 36
DT (B pin)             any microcontroler intput pin with interrupt -> in this example pin 37
SW (button pin)        any microcontroler intput pin with interrupt -> in this example pin 38
GND - to microcontroler GND
VCC                    microcontroler VCC (then set ROTARY_ENCODER_VCC_PIN -1) 

*/
#define LED_GREEG						25
#define LED_BLUE					26
#define LED_RED  					27
#define BUZZER 						19
#define ROTARY_ENCODER_A_PIN 		36
#define ROTARY_ENCODER_B_PIN    	37
#define ROTARY_ENCODER_BUTTON_PIN 	38
#define ROTARY_ENCODER_VCC_PIN 		-1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

//depending on your encoder - try 1,2 or 4 to get expected behaviour
//#define ROTARY_ENCODER_STEPS 1
#define ROTARY_ENCODER_STEPS 2
//#define ROTARY_ENCODER_STEPS 4

//instead of changing here, rather change numbers above
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

int16_t test_limits = 2;

int led_red();
int led_green();
int led_blue();
int buzzer();

void rotary_loop();
void rotary_onButtonClick();


// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     2000

bool on_Button_down = false;
void setup() {
  Serial.begin(115200);

  // initialize digital pin output.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEG, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(BUZZER, LEDC_CHANNEL_0);


  led_blue(); // blue led 
 

  //we must initialize rotary encoder
    rotaryEncoder.begin();

    rotaryEncoder.setup(
        [] { rotaryEncoder.readEncoder_ISR(); },
        [] { on_Button_down=true; });
    //optionally we can set boundaries and if values should cycle or not
    bool circleValues = false;
    rotaryEncoder.setBoundaries(0, 1000, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    /*Rotary acceleration introduced 25.2.2021.
   * in case range to select is huge, for example - select a value between 0 and 1000 and we want 785
   * without accelerateion you need long time to get to that number
   * Using acceleration, faster you turn, faster will the value raise.
   * For fine tuning slow down.
   */
    //rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
    rotaryEncoder.setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration



}

// the loop function runs over and over again forever
void loop() {

//in loop call your custom function which will process rotary encoder values
    if (millis() > 20000)
        rotaryEncoder.enable();
 
    rotary_loop();
    delay(50);

}

void rotary_onButtonClick()
{ 

    if(on_Button_down){
     
    static unsigned long lastTimePressed = 0;
    if (millis() - lastTimePressed < 500)
        return; //ignore multiple press in that time milliseconds
    lastTimePressed = millis();


    buzzer();
    Serial.print(" buzzer");

    rotaryEncoder.setBoundaries(-test_limits, test_limits, false);
    Serial.print("new boundaries are between minimumn value ");
    Serial.print(-test_limits);
    Serial.print(" and maximum value");
    Serial.println(-test_limits);
    rotaryEncoder.reset();

    if (test_limits >= 2048)
        test_limits = 2;
    test_limits *= 2;

    on_Button_down = false;
    }
}

void rotary_loop()
{
    rotary_onButtonClick();
    //lets see if anything changed
    int16_t encoderDelta = rotaryEncoder.encoderChanged();

    //optionally we can ignore whenever there is no change
    if (encoderDelta == 0)
        return;

    //for some cases we only want to know if value is increased or decreased (typically for menu items)
    if (encoderDelta > 0)
    {
        led_green();
        Serial.print("+");
    }
        
   else if (encoderDelta < 0)
    {
        led_red();
         Serial.print("-");

    }
       

    //for other cases we want to know what is current value. Additionally often we only want if something changed
    //example: when using rotary encoder to set termostat temperature, or sound volume etc

    //if value is changed compared to our last read
    if (encoderDelta != 0)
    {
        //now we need current value
        int16_t encoderValue = rotaryEncoder.readEncoder();
        //process new value. Here is simple output.
        Serial.print("Value: ");
        Serial.println(encoderValue);
    }
}
int buzzer(){

    ledcWriteTone(LEDC_CHANNEL_0, LEDC_BASE_FREQ);
    ledcWrite(LEDC_CHANNEL_0, 150);
    delay(300);
    ledcWrite(LEDC_CHANNEL_0, 0);

    return 1;
}

int led_red(){
  digitalWrite(LED_RED, LOW); 
  digitalWrite(LED_GREEG, HIGH); 
  digitalWrite(LED_BLUE, HIGH); 
  return 1;
}
int led_green(){
  digitalWrite(LED_RED, HIGH); 
  digitalWrite(LED_GREEG, LOW); 
  digitalWrite(LED_BLUE, HIGH); 
}
int led_blue(){
  digitalWrite(LED_RED, HIGH); 
  digitalWrite(LED_GREEG, HIGH); 
  digitalWrite(LED_BLUE, LOW); 
  return 1;
}