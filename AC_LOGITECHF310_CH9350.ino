/*
   CH9350 + Logitech F310
   Arduino Nano

   CH9350 TX -> D10
   CH9350 RX -> D11

   Baud CH9350 = 57600 (maximum recomended for SoftwareSerial Arduino Nano)
   Configuration Dip switch 2 BA1 = 0 .All others = 1
   Code developed by acjacques. November 2026 . contact:  acj@infolink.com.br  Whatsapp: 55 21 992148116
*/

#include <SoftwareSerial.h>

#define CH9350_RX 10
#define CH9350_TX 11

SoftwareSerial CH9350(CH9350_RX, CH9350_TX);

struct Gamepad
{
  uint8_t ID;

  uint8_t LX;
  uint8_t LY;

  uint8_t RX;
  uint8_t RY;

  uint8_t HAT;

  uint8_t A;
  uint8_t B;
  uint8_t X;
  uint8_t Y;

  uint8_t LB;
  uint8_t RB;

  uint8_t LT;
  uint8_t RT;

  uint8_t BACK;
  uint8_t START;

  uint8_t L3;
  uint8_t R3;

  uint8_t MODE;
};

Gamepad pad;
Gamepad lastPad;

bool gamepadConnected = false;
bool newFrame = false;

////////////////////////////////////////////////////////

const char* hatName(uint8_t h)
{
  switch(h)
  {
    case 0: return "UP";
    case 1: return "UP_RIGHT";
    case 2: return "RIGHT";
    case 3: return "DOWN_RIGHT";
    case 4: return "DOWN";
    case 5: return "DOWN_LEFT";
    case 6: return "LEFT";
    case 7: return "UP_LEFT";
    case 8: return "CENTER";
  }

  return "?";
}

////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);

  CH9350.begin(57600);

  memset(&pad, 0, sizeof(Gamepad));
  memset(&lastPad, 0xFF, sizeof(Gamepad));

  Serial.println();
  Serial.println("CH9350 + Logitech F310");
  Serial.println();
}

////////////////////////////////////////////////////////

void loop()
{
  serialEvent();

  if(newFrame)
  {
    newFrame = false;

    printFrame();
  }
}
////////////////////////////////////////////////////////
void printFrame()
{
  Serial.print("ID=");
  Serial.print(pad.ID);

  Serial.print(" LX=");
  Serial.print(pad.LX);

  Serial.print(" LY=");
  Serial.print(pad.LY);

  Serial.print(" RX=");
  Serial.print(pad.RX);

  Serial.print(" RY=");
  Serial.print(pad.RY);

  Serial.print(" HAT=");
  Serial.print(pad.HAT);
  //Serial.print(hatName(pad.HAT));  // uncomment if you want see Hat names

  Serial.print(" A=");
  Serial.print(pad.A);

  Serial.print(" B=");
  Serial.print(pad.B);

  Serial.print(" X=");
  Serial.print(pad.X);

  Serial.print(" Y=");
  Serial.print(pad.Y);

  Serial.print(" LB=");
  Serial.print(pad.LB);

  Serial.print(" RB=");
  Serial.print(pad.RB);

  Serial.print(" LT=");
  Serial.print(pad.LT);

  Serial.print(" RT=");
  Serial.print(pad.RT);

  Serial.print(" BACK=");
  Serial.print(pad.BACK);

  Serial.print(" START=");
  Serial.print(pad.START);

  Serial.print(" L3=");
  Serial.print(pad.L3);

  Serial.print(" R3=");
  Serial.print(pad.R3);

  Serial.print(" MODE=");
  Serial.print(pad.MODE);

  Serial.print(" CONNECT=");
  Serial.println(gamepadConnected);
}
////////////////////////////////////////////

void decodeReport(uint8_t *r)
{
  pad.ID = r[0];

  pad.LX = r[1];
  pad.LY = r[2];

  pad.RX = r[3];
  pad.RY = r[4];

  pad.HAT = r[5] & 0x0F;

  pad.X = (r[5] & 0x10) ? 1 : 0;
  pad.A = (r[5] & 0x20) ? 1 : 0;
  pad.B = (r[5] & 0x40) ? 1 : 0;
  pad.Y = (r[5] & 0x80) ? 1 : 0;

  pad.LB = (r[6] & 0x01) ? 1 : 0;
  pad.RB = (r[6] & 0x02) ? 1 : 0;

  pad.LT = (r[6] & 0x04) ? 1 : 0;
  pad.RT = (r[6] & 0x08) ? 1 : 0;

  pad.BACK  = (r[6] & 0x10) ? 1 : 0;
  pad.START = (r[6] & 0x20) ? 1 : 0;

  pad.L3 = (r[6] & 0x40) ? 1 : 0;
  pad.R3 = (r[6] & 0x80) ? 1 : 0;

  pad.MODE = (r[7] & 0x08) ? 1 : 0;
}

////////////////////////////////////////////////////////

void serialEvent()
{
  static uint8_t frame[32];

  static uint8_t state = 0;
  static uint8_t index = 0;
  static uint8_t frameSize = 0;

  while(CH9350.available())
  {
    uint8_t b = CH9350.read();

    switch(state)
    {
      //----------------------------
      // Header 57
      //----------------------------

      case 0:

        if(b == 0x57)
        {
          frame[0] = b;
          state = 1;
        }

        break;

      //----------------------------
      // Header AB
      //----------------------------

      case 1:

        if(b == 0xAB)
        {
          frame[1] = b;
          state = 2;
        }
        else
        {
          state = 0;
        }

        break;

      //----------------------------
      // CMD
      //----------------------------

      case 2:

        frame[2] = b;
        state = 3;

        break;

      //----------------------------
      // LEN
      //----------------------------

      case 3:

        frame[3] = b;

        //----------------------------------
        // HEARTBEAT
        //----------------------------------

        if(frame[2] == 0x82)
        {
          state = 0;
          break;
        }

        //----------------------------------
        // CONECTADO
        //----------------------------------

        if(frame[2] == 0x80)
        {
          gamepadConnected = true;

          Serial.println("GAMEPAD CONNECTED");

          state = 0;
          break;
        }

        //----------------------------------
        // DESCONECTADO
        //----------------------------------

        if(frame[2] == 0x86)
        {
          gamepadConnected = false;

          Serial.println("GAMEPAD DISCONNECTED");

          state = 0;
          break;
        }

        //----------------------------------

        frameSize = b + 4;

        index = 4;
        state = 4;

        break;

      //----------------------------------
      // Payload
      //----------------------------------

      case 4:

        frame[index++] = b;

        if(index >= frameSize)
        {
          state = 0;

          //----------------------------------
          // HID REPORT
          //----------------------------------

          if(frame[2] == 0x88)
          {
            decodeReport(&frame[4]);

            if(memcmp(&pad,
                      &lastPad,
                      sizeof(Gamepad)))
            {
              memcpy(&lastPad,
                     &pad,
                     sizeof(Gamepad));

              newFrame = true;
            }
          }
        }

        break;
    }
  }
}
