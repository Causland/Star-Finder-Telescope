// Interfaces with the AVRMotionController to move the servos to their
// target positions. Uses sensors to create feedback loop to correct
// for error in servo motion

#include <Servo.h>

// Constants
constexpr uint8_t VERT_SERVO_NUM{0};
constexpr uint8_t VERT_SERVO_PIN{9};
constexpr uint16_t VERT_SERVO_DEFAULT_US{1500};

constexpr uint8_t HORIZ_SERVO_NUM{1};
constexpr uint8_t HORIZ_SERVO_PIN{10};
constexpr uint16_t HORIZ_SERVO_DEFAULT_US{1500};

constexpr uint8_t FOCUS_SERVO_NUM{2};
constexpr uint8_t FOCUS_SERVO_PIN{11};
constexpr uint16_t FOCUS_SERVO_DEFAULT_US{1500};

constexpr uint8_t COMMAND_LEN{2};

// Globals
Servo gVertServo;
Servo gHorizServo;
Servo gFocusServo;
byte gCommand[COMMAND_LEN];

void setup()
{
   // Attach the servos and move to default positions
   gVertServo.attach(VERT_SERVO_PIN);
   gVertServo.writeMicroseconds(VERT_SERVO_DEFAULT_US);
   gHorizServo.attach(HORIZ_SERVO_PIN);
   gHorizServo.writeMicroseconds(HORIZ_SERVO_DEFAULT_US);
   gFocusServo.attach(FOCUS_SERVO_PIN);
   gFocusServo.writeMicroseconds(FOCUS_SERVO_DEFAULT_US);

   Serial.begin(9600);
}

void loop()
{
   // Check for new serial data
   if (Serial.available() > 0)
   {
      size_t numBytes = Serial.readBytes(gCommand, COMMAND_LEN);

      if (numBytes >= COMMAND_LEN)
      {
         // We received a command. Check for which servo and set
         // the new microseconds. Note, the commanded value is
         // in ten microseconds units
         switch(gCommand[0])
         {
            case VERT_SERVO_NUM:
               gVertServo.writeMicroseconds(gCommand[1] * 10);
               break;
            case HORIZ_SERVO_NUM:
               gHorizServo.writeMicroseconds(gCommand[1] * 10);
               break;
            case FOCUS_SERVO_NUM:
               gFocusServo.writeMicroseconds(gCommand[1] * 10);
               break;
            default: // Nothing here
               break;
         }
      }
   }
}