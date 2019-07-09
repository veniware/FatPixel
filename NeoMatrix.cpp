/*
  NeoMatrix - Library for driving WS2812B (aka NeoPixel).
  Created by Andreas Venizelou, December 2017.
  Released into the public domain.
*/

#include "NeoMatrix.h"

NeoMatrix::NeoMatrix(int width, int height)
    : width(width), height(height), length(width * height) {

    screen_buffer = (byte*) calloc(this->length*3, 1);

    pinMode(8, OUTPUT);
}

NeoMatrix::NeoMatrix(int width, int height, int secondary) //more size allocated for 2nd screen
    : width(width), height(height), length(width * height + secondary) {
    screen_buffer = (byte*) calloc((this->length + secondary) * 3, 1);

    pinMode(8, OUTPUT);
}

NeoMatrix::~NeoMatrix() {
    free(screen_buffer);
}

void NeoMatrix::setPixel(int x, int y, byte r, byte g, byte b) {
    int index = (y%2 == 0)? (width*y + x) * 3 : (width*y + width - x - 1) * 3;
    screen_buffer[index]     = g;
    screen_buffer[index + 1] = r;
    screen_buffer[index + 2] = b;
}

void NeoMatrix::setPixel(byte* buff, int x, int y, byte r, byte g, byte b) {
    int index = (y%2 == 0)? (width*y + x) * 3 : (width*y + width - x - 1) * 3;
    buff[index]     = g;
    buff[index + 1] = r;
    buff[index + 2] = b;
}

byte* NeoMatrix::getPixel(byte* buff, int x, int y) const {
    int index = (y%2 == 0)? (width*y + x) * 3 : (width*y + width - x - 1) * 3;
    byte* result = (byte*) malloc(3);
    result[0] = buff[index + 1];
    result[1] = buff[index];
    result[2] = buff[index + 2];
    return result;
}

void NeoMatrix::multiply(double v) {
    for (int i = 0; i < length*3; i++) screen_buffer[i] *= v;
}

void NeoMatrix::add(byte v) {
    for (int i = 0; i < length*3; i++)
        if (screen_buffer[i] <= 255-v) screen_buffer[i] += v;
}

void NeoMatrix::subtract(byte v) {
    for (int i = 0; i < length*3; i++)
        if (screen_buffer[i] >= v) screen_buffer[i] -= v;
}

bool NeoMatrix::isNotBlack(byte* buff, int x, int y) const {
    int index = (y%2 == 0)? (width*y + x) * 3 : (width*y + width - x - 1) * 3;
    if (buff[index] > 0) return true;
    if (buff[index+1] > 0) return true;
    if (buff[index+2] > 0) return true;
    return false;
}

bool NeoMatrix::isLineFull(byte* buff, int y) const {
    for (int x=0; x<10; x++) {
        int index = (y%2 == 0)? (width*y + x) * 3 : (width*y + width - x - 1) * 3;
        if (buff[index] == 0 && buff[index+1] == 0 && buff[index+2] == 0) return false;
    }

    return true;
}

void NeoMatrix::refresh() const {
    pushBuffer(screen_buffer);
}

void NeoMatrix::pushBuffer(byte* buff) const {
    noInterrupts(); //kill the interrupts while we send the bit stream out...
    
    byte initialValue = PORTB; //save the status of the entire PORT B - let's us write to the entire port without messing up the other pins on that port
    unsigned int len = length * 3; //total bytes in the LED string

    for (int i = 0; i < len; i++) {
        //here's the tricky part, check if the bit in the byte is high/low then right that status to the pin
        // (RGB[i] & B10000000) will strip away the other bits in RGB[i], so here we'll be left with B10000000 or B00000000
        // then it's easy to check if the bit is high or low by AND'ing that with the bit mask ""&& B10000000)"" this gives 1 or 0
        // if it's a 1, we'll OR that with the Existing port, thus keeping the pin HIGH, if 0 the pin is written LOW

        PORTB = 1; //bit 7  first, set the pin HIGH - it always goes high regardless of a 0/1
        PORTB = ((buff[i] & B10000000) && B10000000) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"); //these are NOPS - these let us delay clock cycles for more precise timing
        PORTB = initialValue; //okay, here we know we have to be LOW regardless of the 0/1 bit state
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");//minimum LOW time for pin regardless of 0/1 bit state

        //then do it again for the next bit and so on... see the last bit though for a slight change

        PORTB = 1; //bit 6
        PORTB = ((buff[i] & B01000000) && B01000000) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 5
        PORTB = ((buff[i] & B00100000) && B00100000) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 4
        PORTB = ((buff[i] & B00010000) && B00010000) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 3
        PORTB = ((buff[i] & B00001000) && B00001000) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 2
        PORTB = ((buff[i] & B00000100) && B00000100) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 1
        PORTB = ((buff[i] & B00000010) && B00000010) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        PORTB = initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        PORTB = 1; //bit 0
        __asm__("nop\n\t"); //on this last bit, the check is much faster, so had to add a NOP here
        PORTB = ((buff[i] & B00000001) && B00000001) | initialValue;
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        //note there are no NOPs after writing the pin LOW, this is because the FOR Loop uses clock cycles that we can use instead of the NOPS
        
        PORTB = initialValue;
    }

    interrupts();
}
