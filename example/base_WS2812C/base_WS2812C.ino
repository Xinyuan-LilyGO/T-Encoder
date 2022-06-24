#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN        25// On Trinket or Gemma, suggest changing this to 1
#define BUZZER                      19
#define ROTARY_ENCODER_A_PIN        36
#define ROTARY_ENCODER_B_PIN        37
#define ROTARY_ENCODER_BUTTON_PIN   38
#define ROTARY_ENCODER_VCC_PIN      -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

//depending on your encoder - try 1,2 or 4 to get expected behaviour
//#define ROTARY_ENCODER_STEPS 1
#define ROTARY_ENCODER_STEPS 2
//#define ROTARY_ENCODER_STEPS 4

//instead of changing here, rather change numbers above
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

int16_t test_limits = 2;


#include <Adafruit_NeoPixel.h>

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 2 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels


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
void setup()
{
    Serial.begin(115200);

    // initialize digital pin output.
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER, OUTPUT);


    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(BUZZER, LEDC_CHANNEL_0);


    led_blue(); // blue led

    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

    //we must initialize rotary encoder
    rotaryEncoder.begin();

    rotaryEncoder.setup(
        [] { rotaryEncoder.readEncoder_ISR(); },
        [] { on_Button_down = true; });
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
void loop()
{

//in loop call your custom function which will process rotary encoder values
    if (millis() > 20000)
        rotaryEncoder.enable();

    rotary_loop();
    delay(50);


}

void rotary_onButtonClick()
{

    if (on_Button_down) {

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
    if (encoderDelta > 0) {
        led_green();
        Serial.print("+");
    }

    if (encoderDelta < 0) {
        led_red();
        Serial.print("-");

    }


    //for other cases we want to know what is current value. Additionally often we only want if something changed
    //example: when using rotary encoder to set termostat temperature, or sound volume etc

    //if value is changed compared to our last read
    if (encoderDelta != 0) {
        //now we need current value
        int16_t encoderValue = rotaryEncoder.readEncoder();
        //process new value. Here is simple output.
        Serial.print("Value: ");
        Serial.println(encoderValue);
    }
}
int buzzer()
{

    ledcWriteTone(LEDC_CHANNEL_0, LEDC_BASE_FREQ);
    ledcWrite(LEDC_CHANNEL_0, 150);
    delay(300);
    ledcWrite(LEDC_CHANNEL_0, 0);

    return 1;
}

int led_red()
{
    pixels.clear(); // Set all pixel colors to 'off'
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(150, 0, 0));
        pixels.show();   // Send the updated pixel colors to the hardware.
        delay(1); // Pause before next pass through loop
    }

    return 1;
}
int led_green()
{
    pixels.clear(); // Set all pixel colors to 'off'
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
        pixels.show();   // Send the updated pixel colors to the hardware.
        delay(1); // Pause before next pass through loop
    }
    return 1;
}
int led_blue()
{

    pixels.clear(); // Set all pixel colors to 'off'
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(0, 0, 150));
        pixels.show();   // Send the updated pixel colors to the hardware.
        delay(1); // Pause before next pass through loop
    }
    return 1;
}