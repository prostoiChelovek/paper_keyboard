#include <Arduino.h>

#define L1 2
#define L2 3
#define L3 4
#define L4 5
#define L5 6
#define L6 7

void setup() {
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(L3, OUTPUT);
    pinMode(L4, OUTPUT);
    pinMode(L5, OUTPUT);
    pinMode(L6, OUTPUT);

    Serial.begin(9600);
}

void inv(int pin)
{
    digitalWrite(pin, digitalRead(pin) == HIGH ? LOW : HIGH);
}

byte data;
void loop() {
    data = Serial.read();
    if(data != 255)
    {
        Serial.println(data);
        switch(data)
        {
        case 49: // 1
            inv(L1);
            break;
        case 50: // 2
            inv(L2);
            break;
        case 51: // 3
            inv(L3);
            break;
        case 52: // 4
            inv(L4);
            break;
        case 53: // 5
            inv(L5);
            break;
        case 54: // 6
            inv(L6);
            break;
        };
    }
}