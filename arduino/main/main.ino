// Interfaces with the AVRMotionController to move the servos to their
// target positions. Uses sensors to create feedback loop to correct
// for error in servo motion

#include <string.h>
#include <Servo.h>

// Constants
static constexpr uint8_t VERT_SERVO_NUM{0};
static constexpr uint8_t VERT_SERVO_PIN{9};
static constexpr uint16_t VERT_SERVO_DEFAULT_US{1200};
static constexpr uint16_t VERT_SERVO_MIN_US{860};
static constexpr uint16_t VERT_SERVO_MAX_US{1490};
static constexpr double VERT_SERVO_MOTION_RANGE_DEG{90.0};
static constexpr double VERT_SERVO_US_PER_DEG{(VERT_SERVO_MAX_US - VERT_SERVO_MIN_US) 
                                                         / VERT_SERVO_MOTION_RANGE_DEG};

static constexpr uint8_t HORIZ_SERVO_NUM{1};
static constexpr uint8_t HORIZ_SERVO_PIN{10};
static constexpr uint8_t HORIZ_SERVO_FEEDBACK_PIN{11};
static constexpr uint16_t HORIZ_SERVO_STOP_US{1500};
static constexpr int HORIZ_SERVO_MIN_SPEED_OFFSET_US{30};
static constexpr int HORIZ_SERVO_MAX_SPEED_OFFSET_US{220};
static constexpr uint16_t HORIZ_SERVO_DEFAULT_US{HORIZ_SERVO_STOP_US};
static constexpr uint16_t HORIZ_SERVO_REFRESH_RATE_MS{10};

static constexpr uint8_t FOCUS_SERVO_NUM{2};
static constexpr uint8_t FOCUS_SERVO_PIN{12};
static constexpr uint16_t FOCUS_SERVO_DEFAULT_US{1500};

static constexpr uint8_t COMMAND_LEN{5};

static constexpr int UNITS_FC{360};

// Globals
Servo gVertServo;
Servo gHorizServo;
Servo gFocusServo;
byte gCommand[COMMAND_LEN];

double gCurrHorizAngle{0.0}; //!< Current angle of horizontal servo
double gTargetHorizAngle{0.0}; //!< Target angle of horizontal servo from latest command
unsigned long gCurrHorizUpdateMs{0}; //!< Current time of the latest horizontal servo measurement and update
unsigned long gPrevHorizUpdateMs{0}; //!< Previous time the horizontal servo was measured and updated


/*!
 * Measure the duty cycle of a pin using a +-100 window around the period.
 * \param[in] pin the pin to read the duty cycle.
 * \param[in] periodUs the period of the full cycle in microseconds.
 * \param[in] average the number of samples to average the duty cycle over.
 * \param[in] timeoutMs the duration to wait for a valid duty cycle calculation in milliseconds.
 * \return the duty cycle. -1.0 if invalid.
 */
double measureDutyCycle(const int& pin, const int& periodUs, 
                        const int& average, const int& timeoutMs)
{
   const auto startTime{millis()};
   auto sampleCount{0};
   double dcSum{0.0};

   while(millis() - startTime < timeoutMs)
   {
      const auto tHigh{pulseIn(pin, HIGH)};
      const auto tLow{pulseIn(pin, LOW)};
      const auto tCycle{tHigh + tLow};
      if (tCycle > periodUs - 100 && tCycle < periodUs + 100)
      {
         dcSum += static_cast<double>(tHigh) / tCycle;
         ++sampleCount;
         if (sampleCount == average)
         {
            return dcSum / average;
         }
      }
   }

   return -1.0;
}

/*!
 * Calculate the position of the horizontal servo using feedback sensor data.
 */
void calcHorizAngle()
{
   static constexpr double DC_MIN{0.029}; //!< Min duty cycle
   static constexpr double DC_MAX{0.971}; //!< Max duty cycle
   static constexpr int CYCLE_PERIOD_US{1100}; //!< The approx. period of one complete cycle
   static constexpr int Q1_MAX{UNITS_FC / 4}; //!< The max angle of the first quadrant
   static constexpr int Q4_MIN{Q1_MAX * 3}; //!< The min angle of the fourth quadrant

   static int turns{0};
   static double prevMeasuredAngle{0.0};

   double dc{measureDutyCycle(HORIZ_SERVO_FEEDBACK_PIN, 
                              CYCLE_PERIOD_US,
                              5,
                              500)};
   if (dc < 0.0) return; // The calculation failed, nothing to do anymore

   // Clamp measured angle into the 0 - UNITS_FC range
   double measuredAngle = ((dc - DC_MIN) * UNITS_FC) / (DC_MAX - DC_MIN);
   if (measuredAngle < 0) measuredAngle = 0;
   // Subtract very small number to be just under UNITS_FC
   else if (measuredAngle >= UNITS_FC) measuredAngle = UNITS_FC - 0.000001;

   // Check for origin crossover and adjust turn count. Note - This calculation assumes
   // the servo will not move over 50% FC within update frequency
   if (measuredAngle > Q4_MIN && prevMeasuredAngle < Q1_MAX) --turns;
   else if (measuredAngle < Q1_MAX && prevMeasuredAngle > Q4_MIN) ++turns;

   // Calculate the current position based on turn count
   if (turns >= 0) gCurrHorizAngle = (turns * UNITS_FC) + measuredAngle;
   else gCurrHorizAngle = ((turns + 1) * UNITS_FC) - (UNITS_FC - measuredAngle);

   prevMeasuredAngle = measuredAngle;
}

/*!
 * Move the horizontal servo towards the target angle.
 */
void controlHorizServo()
{
   static constexpr double ERROR_TOLERANCE{0.5};
   static constexpr double K_P{0.8};
   static constexpr double K_I{0.005};
   static constexpr double K_D{45};
   
   static constexpr uint8_t ERROR_HISTORY_LEN{5};
   static double errorHistory[ERROR_HISTORY_LEN]{0.0};
   static double errorSum{0.0};
   static uint8_t errorIndex{0};
   static double prevAvgError{0.0};
   static double prevIntegral{0.0};

   errorSum -= errorHistory[errorIndex];
   errorHistory[errorIndex] = gTargetHorizAngle - gCurrHorizAngle;
   errorSum += errorHistory[errorIndex];
   ++errorIndex;
   if (errorIndex >= ERROR_HISTORY_LEN) errorIndex = 0;

   const double avgError{errorSum / ERROR_HISTORY_LEN};

   if (avgError < ERROR_TOLERANCE && avgError > -1 * ERROR_TOLERANCE)
   {
      // We are within tolerance, stop the servo
      gHorizServo.writeMicroseconds(HORIZ_SERVO_STOP_US);
      prevAvgError = avgError;
      prevIntegral = 0.0;
      return;
   }

   auto deltaMs{gCurrHorizUpdateMs - gPrevHorizUpdateMs};
   const double integral{prevIntegral + avgError * deltaMs};
   double integralPortion{integral * K_I};
   if (integralPortion < -1 * HORIZ_SERVO_MIN_SPEED_OFFSET_US) 
      integralPortion = -1 * HORIZ_SERVO_MIN_SPEED_OFFSET_US;
   else if (integralPortion > HORIZ_SERVO_MIN_SPEED_OFFSET_US)
      integralPortion = HORIZ_SERVO_MIN_SPEED_OFFSET_US;

   // Determine output offset of the PID controller
   int offset{avgError * K_P + 
              integralPortion +
              (avgError - prevAvgError) / deltaMs * K_D};

   if (offset < -1 * HORIZ_SERVO_MAX_SPEED_OFFSET_US) offset = -1 * HORIZ_SERVO_MAX_SPEED_OFFSET_US;
   else if (offset > HORIZ_SERVO_MAX_SPEED_OFFSET_US) offset = HORIZ_SERVO_MAX_SPEED_OFFSET_US;

   gHorizServo.writeMicroseconds(HORIZ_SERVO_STOP_US - offset);

   prevAvgError = avgError;
   prevIntegral = integral;
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
               memcpy(&theta, gCommand+1, sizeof(float));

               int numUs{theta * VERT_SERVO_US_PER_DEG + VERT_SERVO_MIN_US};
               if (numUs < VERT_SERVO_MIN_US) numUs = VERT_SERVO_MIN_US;
               else if (numUs > VERT_SERVO_MAX_US) numUs = VERT_SERVO_MAX_US;

               gVertServo.writeMicroseconds(numUs);
               break;
            }               
            case HORIZ_SERVO_NUM:
            {
               float theta{0.0f};
               memcpy(&theta, gCommand+1, sizeof(float));

               gTargetHorizAngle = theta;
               break;
            }
            case FOCUS_SERVO_NUM:
            {
               gFocusServo.writeMicroseconds(gCommand[1] * 10);
               break;
            }
            default: // Nothing here
               break;
         }
      }
   }

   gCurrHorizUpdateMs = millis();
   if (gCurrHorizUpdateMs - gPrevHorizUpdateMs > HORIZ_SERVO_REFRESH_RATE_MS)
   {
      calcHorizAngle();
      controlHorizServo();
      gPrevHorizUpdateMs = gCurrHorizUpdateMs;
   }
}
