// Interfaces with the AVRMotionController to move the servos to their
// target positions. Uses sensors to create feedback loop to correct
// for error in servo motion

#include <algorithm>
#include <cstring>

#include <Servo.h>

// Constants
static constexpr uint8_t VERT_SERVO_NUM{0};
static constexpr uint8_t VERT_SERVO_PIN{9};
static constexpr uint16_t VERT_SERVO_DEFAULT_US{1500};
static constexpr uint16_t VERT_SERVO_MIN_US{860};
static constexpr uint16_t VERT_SERVO_MAX_US{2120};
static constexpr double VERT_SERVO_MOTION_RANGE_DEG{180};
static constexpr double VERT_SERVO_US_PER_DEG{(VERT_SERVO_MAX_US - VERT_SERVO_MIN_US) 
                                                         / VERT_SERVO_MOTION_RANGE_DEG};

static constexpr uint8_t HORIZ_SERVO_NUM{1};
static constexpr uint8_t HORIZ_SERVO_PIN{10};
static constexpr uint8_t HORIZ_SERVO_FEEDBACK_PIN{11};
static constexpr uint16_t HORIZ_SERVO_DEFAULT_US{1500};
static constexpr uint16_t HORIZ_SERVO_MIN_US{1280};
static constexpr uint16_t HORIZ_SERVO_MAX_US{1720};
static constexpr double HORIZ_SERVO_AVG_TEN_US{(HORIZ_SERVO_MAX_TEN_US - HORIZ_SERVO_MIN_TEN_US) / 2.0 + HORIZ_SERVO_MIN_TEN_US};
static constexpr double HORIZ_SERVO_CW_DEADZONE_TEN_US{148}; 
static constexpr double HORIZ_SERVO_CCW_DEADZONE_TEN_US{152};

static constexpr uint8_t FOCUS_SERVO_NUM{2};
static constexpr uint8_t FOCUS_SERVO_PIN{12};
static constexpr uint16_t FOCUS_SERVO_DEFAULT_US{1500};

static constexpr uint8_t COMMAND_LEN{5};

// Globals
Servo gVertServo;
Servo gHorizServo;
Servo gFocusServo;
byte gCommand[COMMAND_LEN];

int currHorizAngle{0};
int targetHorizAngle{0};
int Kp{1}; // Proportionality constant

void measureHorizAngle()
{
   // Taken from Paralax 360 Feedback Angle Control code
   
   // Constants
   static constexpr int unitsFC{360};       // Units in a full circle
   static constexpr int dutyScale{1000};    // Scale duty cycle to 1/1000ths
   static constexpr int dcMin{29};          // Min duty cycle
   static constexpr int dcMax{971};         // Max duty cycle
   static constexpr int q2min{unitsFC / 4}; // For checking if in 1st quadrant
   static constexpr int q3max{q2min * 3};   // For checking if in 4th quadrant

   static int turns{0};      // For tracking turns
   static int thetaP{0}; // For tracking angle 

   // Measure low and high times, making sure to take only valid cycle times
   // (Check for jumps across 0 to 359 degree boundary)
   int tCycle{0};
   int tHigh{0};
   int tLow{0};
   int dc{0};
   while(true)
   {
      tHigh = pulseIn(HORIZ_SERVO_FEEDBACK_PIN, HIGH);
      tLow = pulseIn(HORIZ_SERVO_FEEDBACK_PIN, LOW);
      tCycle = tHigh + tLow;
      if ((tCycle > 1000) && tCycle < 1200) // Check for valid
      {
         break;
      }
      dc = (dutyScale * tHigh) / tCycle;
   }

   // Calculate angle
   int theta = (unitsFC - 1) - ((dc - dcMin) * unitsFC) / (dcMax - dcMin + 1);

   // Clamp theta to positive
   theta = std::clamp(theta, 0, unitsFC - 1);

   // If we transition from quadrant 4 to quadrant 1, increase the turn count
   if ((theta < q2min) && (thetaP > q3max)) ++turns;
   // If we transition from quadrant 1 to quadrant 4, decrease the turn count
   if ((thetaP < q2min) && (theta > q3max)) --turns;

   // Construct current angle from turns count and theta
   if (turns >= 0)
   {
      currHorizAngle = (turns * unitsFC) + theta;
   }
   else
   {
      currHorizAngle = ((turns + 1) * unitsFC) - (unitsFC - theta);
   }

   thetaP = theta;
}

void controlHorizServo()
{
   int errorAngle{targetHorizAngle - currHorizAngle};
   int output{errorAngle * Kp};
   int offset{0};

   output = std::clamp(output, -200, 200);
   if (errorAngle > 0)
   {
      offset = 30;
   }
   else if (errorAngle < 0)
   {
      offset = -30;
   }
   else
   {
      offset = 0;
   }

   gHorizServo.writeMicroseconds(output + offset);
}

void setup()
{
   // Attach the servos and move to default positions
   gVertServo.attach(VERT_SERVO_PIN);
   gVertServo.writeMicroseconds(VERT_SERVO_DEFAULT_US);
   
   gHorizServo.attach(HORIZ_SERVO_PIN);
   gHorizServo.writeMicroseconds(HORIZ_SERVO_DEFAULT_US);
   pinMode(HORIZ_SERVO_FEEDBACK_PIN, INPUT);

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
            {
               float theta{0.0f};
               std::memcpy(&theta, gCommand+1, sizeof(float));

               const int numUs{static_cast<int>(std::clamp(theta * VERT_SERVO_US_PER_DEG + VERT_SERVO_MIN_US,
                                                           static_cast<double>(VERT_SERVO_MIN_US),
                                                           static_cast<double>(VERT_SERVO_MAX_US)))};

               gVertServo.writeMicroseconds(numUs);
               break;
            }               
            case HORIZ_SERVO_NUM:
               float theta{0.0f};
               std::memcpy(&theta, gCommand+1, sizeof(float));

               targetHorizAngle = theta;
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
