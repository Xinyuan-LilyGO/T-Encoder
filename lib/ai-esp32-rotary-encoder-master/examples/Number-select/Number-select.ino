#include "AiEsp32RotaryEncoder.h"
#include "AiEsp32RotaryEncoderNumberSelector.h"
#define ROTARY_ENCODER_A_PIN 32
#define ROTARY_ENCODER_B_PIN 21
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder *rotaryEncoder = new AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
AiEsp32RotaryEncoderNumberSelector numberSelector = AiEsp32RotaryEncoderNumberSelector();
/*
In this example an additional functionality is used.

AiEsp32RotaryEncoderNumberSelector is that additional helper which will hide calculation for a rotary encoder.

In example 1 - you can select some temperature between -12 and +31.5 degrees in steps by 0.5 degrees; using precision of 1 decimal
In example 2 - you can select some frequency between 6999000 and 7000000 Hz in steps by 10 Hz; using precision of 0 decimal (integer)

Internally AiEsp32RotaryEncoderNumberSelector will do the math and set the most apropriate acceleration, min and max values for you

use setRange to set parameters
use setValue for a default/initial value
and finally read the value with getValue

So, this value is actually value you need

In code bellow comment / uncomment example 1 or 2
*/

void rotary_onButtonClick()
{
    static unsigned long lastTimePressed = 0;
    if (millis() - lastTimePressed < 200)
        return;
    lastTimePressed = millis();

    Serial.print("Selected value is ");
    Serial.print(numberSelector.getValue(), 1);
    Serial.println(" ***********************");
}

void setup()
{
    Serial.begin(115200);
    rotaryEncoder->begin();
    rotaryEncoder->setup(
        [] { rotaryEncoder->readEncoder_ISR(); },
        [] { rotary_onButtonClick(); });

    numberSelector.attachEncoder(rotaryEncoder);
    //example 1
    //numberSelector.setRange(-12.0, 31.5, 0.5, false, 1);
    //numberSelector.setValue(24.3);
    //example 2
    numberSelector.setRange(6999000.0, 7000000.0, 10, false, 0);
    numberSelector.setValue(6999500.0);
}

void loop()
{
    if (rotaryEncoder->encoderChanged())
    {
        Serial.print(numberSelector.getValue());
        Serial.println(" ");
    }
}