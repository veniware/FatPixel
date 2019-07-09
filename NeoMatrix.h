/*
  NeoMatrix - Library for driving WS2812B (aka NeoPixel).
  Created by Andreas Venizelou, December 2017.
  Released into the public domain.
*/

#pragma once 

#include "Arduino.h"

class NeoMatrix { 
  
    public:
        NeoMatrix(int width, int height);
        NeoMatrix(int width, int height, int secondary);
        ~NeoMatrix();

        void setPixel(int x, int y, byte r, byte g, byte b);
        void setPixel(byte* buff, int x, int y, byte r, byte g, byte b);
        byte* getPixel(byte* buff, int x, int y) const;
        void multiply(double v);
        void add(byte v);
        void subtract(byte v);
        bool isNotBlack(byte* buff, int x, int y) const;
        bool isLineFull(byte* buff, int y) const;
        void refresh() const;
        void pushBuffer(byte* buff) const;

        byte* screen_buffer;

    private:
        int width;
        int height;
        int length;

};