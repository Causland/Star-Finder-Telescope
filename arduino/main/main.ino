// Interfaces with the AVRMotionController to move the servos to their
// target positions. Uses sensors to create feedback loop to correct
// for error in servo motion

// Uncomment for debug logging of PID controllers
//#define DEBUG

#include <stdint.h>
#include <string.h>
#include <Servo.h>

// Constants
static constexpr uint8_t VERT_SERVO_NUM{0};
static constexpr uint8_t VERT_SERVO_PIN{9};
static constexpr uint16_t VERT_SERVO_MIN_US{860};
static constexpr uint16_t VERT_SERVO_MAX_US{1490};
static constexpr double VERT_SERVO_MOTION_RANGE_DEG{90.0};

static constexpr uint8_t HORIZ_SERVO_NUM{1};
static constexpr uint8_t HORIZ_SERVO_PIN{10};
static constexpr uint8_t HORIZ_SERVO_FEEDBACK_PIN{11};
static constexpr uint16_t HORIZ_SERVO_STOP_US{1500};
static constexpr uint16_t HORIZ_SERVO_MIN_SPEED_OFFSET_US{30};
static constexpr uint16_t HORIZ_SERVO_MAX_SPEED_OFFSET_US{220};
static constexpr uint16_t HORIZ_SERVO_MAX_SPEED_DPS{840};
static constexpr uint16_t HORIZ_SERVO_DEFAULT_US{HORIZ_SERVO_STOP_US};
static constexpr uint16_t HORIZ_SERVO_REFRESH_RATE_MS{10};

static constexpr uint8_t FOCUS_SERVO_NUM{2};
static constexpr uint8_t FOCUS_SERVO_PIN{12};
static constexpr uint16_t FOCUS_SERVO_DEFAULT_US{1500};

static constexpr uint8_t COMMAND_LEN{5};

/*!
 * Servo wrapper class to provide some default properties and state to servos.
 */ 
class CustomServo
{
public:
  /*!
   * Create a CustomServo with a dedicated output pin and uS range.
   * \param[in] pinNum the physical pin number.
   * \param[in] defaultUs the default microseconds for the servo.
   * \param[in] minUs the minimum microseconds for the servo.
   * \param[in] maxUs the maximum microseconds for the servo.
   */
  CustomServo(const uint8_t pinNum, const uint16_t defaultUs, 
              const uint16_t minUs, const uint16_t maxUs) :
                pinNum(pinNum), defaultUs(defaultUs), minUs(minUs), maxUs(maxUs) {}

  /*!
   * Initializes the default state of the servo and attaches to the underlying resource
   */
  void init()
  {
    servo.attach(pinNum);
    servo.writeMicroseconds(defaultUs);
  }

  virtual ~CustomServo() = default;
  CustomServo(const CustomServo&) = delete;
  CustomServo& operator=(const CustomServo&) = delete;
  CustomServo(CustomServo&&) = delete;
  CustomServo& operator=(CustomServo&&) = delete;

  /*!
   * Wrapper function to write a certain microsecond value to the servo object.
   * \param[in] us microseconds to write.
   */
  void writeMicroseconds(const int& us) { servo.writeMicroseconds(us); }

  /*!
   * Stop any motion.
   */
  virtual void stop() = 0;

  /*!
   * Measure the current position of the servo.
   * \return the position of the servo in degrees.
   */
  virtual double measurePosition() = 0;

  double getCurrPos() { return currAngle; }
  double getPrevPos() { return prevAngle; }

  uint8_t pinNum{0}; //!< The physical pin number.

  uint16_t defaultUs{0}; //!< The default microseconds for the servo.
  uint16_t minUs{0}; //!< The minimum microseconds for the servo.
  uint16_t maxUs{UINT16_MAX}; //!< The maximum microseconds for the servo.

protected:
  /*!
   * Measure the duty cycle of a pin using a +-100 window around the period.
   * \param[in] pin the pin to read the duty cycle.
   * \param[in] periodUs the period of the full cycle in microseconds.
   * \param[in] average the number of samples to average the duty cycle over.
   * \param[in] timeoutMs the duration to wait for a valid duty cycle calculation in milliseconds.
   * \return the duty cycle. -1.0 if invalid.
   */
  static double measureDutyCycle(const int& pin, const int& periodUs, 
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

  double currAngle{0.0}; //!< Current angle of the servo. Most recent measurement.
  double prevAngle{0.0}; //!< Previous angle of the servo.
  
  Servo servo; //!< The underlying Servo object to control the servo.
};

/*!
 * A generic 360 degree continuous servo.
 */
class ContinuousServo : public CustomServo
{
public:
  /*!
   * Create a ContinuousServo with a dedicated output pin, uS range, and speed properties.
   * \param[in] pinNum the physical pin number.
   * \param[in] defaultUs the default microseconds for the servo.
   * \param[in] minSpeedOffsetUs the minimum microsecond offset to cause the servo to rotate.
   * \param[in] maxSpeedOffsetUs the maximum microsecond offset the servo can be set (max speed).
   * \param[in] maxSpeedDps the calculated maximum speed in deg per second the servo can operate.
   *                        Used to help filter sporadic jumps in sensors.
   * \param[in] feedbackPinNum the physical pin number of the feedback sensor.
   */
  ContinuousServo(const uint8_t pinNum, const uint16_t defaultUs,
                   const uint16_t minSpeedOffsetUs, const uint16_t maxSpeedOffsetUs,
                   const uint16_t maxSpeedDps, const uint8_t feedbackPinNum) : 
                    CustomServo(pinNum, defaultUs, defaultUs - maxSpeedOffsetUs, defaultUs + maxSpeedOffsetUs),
                    minSpeedOffsetUs(minSpeedOffsetUs), maxSpeedOffsetUs(maxSpeedOffsetUs), 
                    maxSpeedDps(maxSpeedDps), feedbackPinNum(feedbackPinNum) 
  {
    pinMode(feedbackPinNum, INPUT);
  }

  ~ContinuousServo() override = default;
  ContinuousServo(const ContinuousServo&) = delete;
  ContinuousServo& operator=(const ContinuousServo&) = delete;
  ContinuousServo(ContinuousServo&&) = delete;
  ContinuousServo& operator=(ContinuousServo&&) = delete;

  /*!
   * Stop rotation by setting to default microseconds.
   */
  void stop() override { servo.writeMicroseconds(defaultUs); }

  /*!
   * Get the true measured angle 0-360 degrees of the sensor. This value is not
   * adjusted for the number of turns.
   * \return the measured angle from 0-360 degrees.
   */
  double getMeasuredAngle() { return currMeasuredAngle; }

  uint16_t minSpeedOffsetUs{0}; //!< Microsecond offset to cause rotation of servo.
  uint16_t maxSpeedOffsetUs{UINT16_MAX}; //!< Microsecond offset to rotate servo at max speed.
  uint16_t maxSpeedDps{UINT16_MAX}; //!< Maximum speed of the servo.
  int turns{0}; //!< The number of turns the servo has made since startup.

protected:
  /*!
   * Adjust the number of turns based on the previous and current measured angle.
   * Uses quadrants to figure out 0-360 crossover
   */
  void calcTurns()
  {
    // Use quadrants to figure out turn
    if (prevMeasuredAngle >= 270.0 && currMeasuredAngle <= 90.0)
    {
      ++turns;
    }
    else if (prevMeasuredAngle <= 90.0 && currMeasuredAngle >= 270.0) 
    {
      --turns;
    }
  }

  uint8_t feedbackPinNum{0}; //!< The physical pin number of the feedback sensor.

  double currMeasuredAngle{0.0}; //!< The real 0-360 measured angle of the servo.
                                 //!< CustomServo::currAngle holds the angle adjusted for turns.
  double prevMeasuredAngle{0.0}; //!< The real 0-360 previous measured angle of the servo.
                                 //!< CustomServo::prevAngle holds the angle adjusted for turns.
};

/*!
 * A ContinuousServo implementation for a Parallax 360 servo.
 */
class Parallax360Servo : public ContinuousServo
{
public:
  /*!
   * Create a ContinuousServo with a dedicated output pin, uS range, and speed properties.
   * \param[in] pinNum the physical pin number.
   * \param[in] defaultUs the default microseconds for the servo.
   * \param[in] minSpeedOffsetUs the minimum microsecond offset to cause the servo to rotate.
   * \param[in] maxSpeedOffsetUs the maximum microsecond offset the servo can be set (max speed).
   * \param[in] maxSpeedDps the calculated maximum speed in deg per second the servo can operate.
   *                        Used to help filter sporadic jumps in sensors.
   * \param[in] feedbackPinNum the physical pin number of the feedback sensor.
   */
  Parallax360Servo(const uint8_t pinNum, const uint16_t defaultUs,
                   const uint16_t minSpeedOffsetUs, const uint16_t maxSpeedOffsetUs,
                   const uint16_t maxSpeedDps, const uint8_t feedbackPinNum) : 
                     ContinuousServo(pinNum, defaultUs,
                                     minSpeedOffsetUs, maxSpeedOffsetUs, 
                                     maxSpeedDps, feedbackPinNum) {}
  
  ~Parallax360Servo() override = default;
  Parallax360Servo(const Parallax360Servo&) = delete;
  Parallax360Servo& operator=(const Parallax360Servo&) = delete;
  Parallax360Servo(Parallax360Servo&&) = delete;
  Parallax360Servo& operator=(Parallax360Servo&&) = delete;

  /*!
   * Measure the position of the servo and adjust on turn count
   * \return the adjusted turn based position
   */
  double measurePosition() override
  {
    static constexpr double DC_MIN{0.029}; //!< Min duty cycle
    static constexpr double DC_MAX{0.971}; //!< Max duty cycle
    static constexpr int CYCLE_PERIOD_US{1100}; //!< The approx. period of one complete cycle

    const double dc{measureDutyCycle(feedbackPinNum,
                                     CYCLE_PERIOD_US,
                                     3,
                                     100)};
    if (dc < 0.0) return prevAngle; // The calculation failed, nothing to do anymore

    // Clamp measured angle into the 0 - 360 range
    prevMeasuredAngle = currMeasuredAngle;
    currMeasuredAngle = ((dc - DC_MIN) * 360.0) / (DC_MAX - DC_MIN);
    if (currMeasuredAngle < 0 || currMeasuredAngle >= 360.0) currMeasuredAngle = 0;

    calcTurns();

    prevAngle = currAngle;
    currAngle = 360.0 * turns + currMeasuredAngle;
    return currAngle;    
  }
};

/*! 
 * A generic position controlled servo.
 */
class PositionalServo : public CustomServo
{
public:
  /*!
   * Create a PositionalServo with a dedicated output pin and uS range.
   * \param[in] pinNum the physical pin number.
   * \param[in] defaultUs the default microseconds for the servo.
   * \param[in] minUs the minimum microseconds for the servo.
   * \param[in] maxUs the maximum microseconds for the servo.
   * \param[in] rangeDeg the motion range of the servo in degrees.
   */
  PositionalServo(const uint8_t pinNum,
                  const uint16_t defaultUs, const uint16_t minUs, const uint16_t maxUs,
                  const double& rangeDeg) : CustomServo(pinNum, defaultUs, minUs, maxUs),
                    rangeDeg(rangeDeg), usPerDeg(rangeDeg > 0.0 ? (maxUs - minUs) / rangeDeg : 0.0) {}
  
  ~PositionalServo() override = default;
  PositionalServo(const PositionalServo&) = delete;
  PositionalServo& operator=(const PositionalServo&) = delete;
  PositionalServo(PositionalServo&&) = delete;
  PositionalServo& operator=(PositionalServo&&) = delete;

  /*!
   * Stop the servo by writing the current position
   */
  void stop() override { servo.writeMicroseconds(servo.readMicroseconds()); }

  /*!
   * Estimate the position of the servo by finding current 
   * servo microseconds.
   */
  double measurePosition() override
  {
    prevAngle = currAngle;
    currAngle = servo.readMicroseconds() / usPerDeg;
    return currAngle;
  }

  double rangeDeg{0.0}; //!< The range of motion of the servo.
  double usPerDeg{0.0}; //!< The number of microseconds for each degree of motion in the range.
};

/*! 
 * A basic implementation of a generic PID controller for a continuous servo.
 */
class PIDController
{
public:
  /*!
   * Create a PIDController with the provided ContinousServo and PID constants.
   * \param[in] servo a pointer to a ContinuousServo object for position, properties, and control.
   * \param[in] settlingTimeSec the duration in seconds in which to control the servo before stopping PID.
   * \param[in] P the P constant.
   * \param[in] I the I constant.
   * \param[in] D the D constant.
   */
  PIDController(ContinuousServo* servo, const int& settlingTimeSec=-1,
                const double& P=0.0, const double& I=0.0, const double& D=0.0) : 
    servo(servo), settlingTimeSec(settlingTimeSec), K_P(P), K_I(I), K_D(D) 
    {
      if (servo == nullptr) 
      {
        Serial.println("ERROR - Servo cannot be null for PID controller");
        while (1) 
        {
          // Wait forever
        }
      }
    }

    /*!
     * Update the PID controller and move the servo. Applies filtering to input and velocity before
     * calculating microsecond offset for motion.
     */
    void move()
    {
      const auto currUpdateMs{millis()};

      // If we are out of the settling window, do not move
      if (settlingTimeSec > 0 &&
          currUpdateMs - targetTimeMs > settlingTimeSec * 1000)
      {
        servo->stop();
        return;
      }

      // Find time between this and last call

      const double deltaS{(currUpdateMs - prevUpdateMs) / 1000.0};

      // Update position
      const double currAngle{servo->measurePosition()};
      const double prevAngle{servo->getPrevPos()};

      // Ignore sporadic jumps greater than max possible speed
      const auto dist{currAngle > prevAngle ? currAngle - prevAngle : 
                                              prevAngle - currAngle};
      if (dist > servo->maxSpeedDps * deltaS) return;

      // Filter the input current angle
      static constexpr double ANGLE_FILTER_FREQ{100}; // Very light filtering. This should be ok from sensor
      const double filteredCurrAngle{prevFilteredCurrAngle + 
                                     (ANGLE_FILTER_FREQ * deltaS / (1.0 + ANGLE_FILTER_FREQ * deltaS)) * 
                                     (currAngle - prevFilteredCurrAngle)};
  
      // Calculate error based on filtered position
      double error{filteredCurrAngle - targetAngle};

      // Calculate integral portion
      double integral{prevIntegral + error * deltaS};
      if (integral < -1 * (servo->minSpeedOffsetUs / K_I)) 
          integral = -1 * (servo->minSpeedOffsetUs / K_I);
      else if (integral > servo->minSpeedOffsetUs / K_I)
          integral = servo->minSpeedOffsetUs / K_I;

      // Calculate derivative portion
      static constexpr double VEL_FILTER_FREQ{120};
      const double vel{(filteredCurrAngle - prevFilteredCurrAngle) / deltaS};
      const double filteredVel{prevFilteredVel + 
                               (VEL_FILTER_FREQ * deltaS / (1.0 + VEL_FILTER_FREQ * deltaS)) * 
                               (vel - prevFilteredVel)};
      
      const double propPortion{error * K_P};
      const double integPortion{integral * K_I};
      const double derivPortion{filteredVel * K_D};
      const int offset{propPortion + integPortion + derivPortion};

#ifdef DEBUG
      static char buf[128]{};
      static char str[16]{};
      dtostrf(currUpdateMs / 1000.0, 0, 3, str); strncpy(buf, str, 15); strcat(buf, ", ");
      itoa(servo->turns, str, 10);               strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(currAngle, 0, 3, str);             strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(filteredCurrAngle, 0, 3, str);     strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(targetAngle, 0, 3, str);           strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(error, 0, 3, str);                 strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(propPortion, 0, 3, str);           strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(integPortion, 0, 3, str);          strncat(buf, str, 15); strcat(buf, ", ");
      dtostrf(derivPortion, 0, 3, str);          strncat(buf, str, 15); strcat(buf, ", ");
      itoa(offset, str, 10);                     strncat(buf, str, 15); strcat(buf, "\n");
      Serial.print(buf);
#endif

      servo->writeMicroseconds(servo->defaultUs + offset);

      // Setup for next function call
      prevFilteredCurrAngle = filteredCurrAngle;
      prevIntegral = integral;
      prevFilteredVel = filteredVel;
      prevUpdateMs = currUpdateMs;
    }

    /*!
     * Update the target for the controller.
     * \param[in] newTargetDeg the new target in degrees.
     */
    void updateTarget(const double& newTargetDeg) 
    { 
      targetTimeMs = millis();
      targetAngle = newTargetDeg;
      prevIntegral = 0.0; 
    }

private:
    double K_P{0.0}; //!< The proportional constant.
    double K_I{0.0}; //!< The integration constant.
    double K_D{0.0}; //!< The derivative constant.

    unsigned long prevUpdateMs{millis()}; //!< The last time the move() function was called.
    double prevFilteredCurrAngle{0.0}; //!< The previous filtered angle of the servo.
    double prevFilteredVel{0.0}; //!< The previous filtered velocity of the servo.
    double prevIntegral{0.0}; //!< The previous integral value.

    unsigned long targetTimeMs{millis()}; //!< Time of the last target update.
    double targetAngle{0.0}; //!< The target angle in degrees.

    int settlingTimeSec{-1}; //!< The duration in seconds in which to let the PID controller operate before stopping.
                             //!< Important for stopping drift of servo due to I.

    ContinuousServo* servo; //!< A pointer to the ContinousServo for position and control.
};

// Globals
PositionalServo gVertServo{VERT_SERVO_PIN, (VERT_SERVO_MIN_US + VERT_SERVO_MAX_US) / 2, 
                           VERT_SERVO_MIN_US, VERT_SERVO_MAX_US, VERT_SERVO_MOTION_RANGE_DEG};
Parallax360Servo gHorizServo{HORIZ_SERVO_PIN, HORIZ_SERVO_STOP_US, 
                             HORIZ_SERVO_MIN_SPEED_OFFSET_US, HORIZ_SERVO_MAX_SPEED_OFFSET_US,
                             HORIZ_SERVO_MAX_SPEED_DPS, HORIZ_SERVO_FEEDBACK_PIN};
PIDController gHorizPID{&gHorizServo, 5, 1.275, 3.0, 0.425};
Servo gFocusServo;

void setup()
{
  Serial.begin(9600);

#ifdef DEBUG
  Serial.println("Time (s), Turns, Measured Theta (deg), Current Theta (deg), Target Theta (deg), Error (deg), P, I, D, Offset (10us)");
#endif

  gVertServo.init();
  gHorizServo.init();

  // Move to known angle
  gHorizPID.updateTarget(20);
}

void loop()
{
  // Check for new serial data
  if (Serial.available() > 0)
  {
    byte command[COMMAND_LEN];
    const size_t numBytes{Serial.readBytes(command, COMMAND_LEN)};

    if (numBytes >= COMMAND_LEN)
    {
      // We received a command for a servo. Update its position
      switch(command[0])
      {
        case VERT_SERVO_NUM:
        {
          float theta{0.0f};
          memcpy(&theta, command+1, sizeof(float));

          int numUs{theta * gVertServo.usPerDeg + gVertServo.minUs};
          if (numUs < gVertServo.minUs) numUs = gVertServo.minUs;
          else if (numUs > gVertServo.maxUs) numUs = gVertServo.maxUs;

          gVertServo.writeMicroseconds(numUs);
          break;
        }               
        case HORIZ_SERVO_NUM:
        {
          float theta{0.0f};
          memcpy(&theta, command+1, sizeof(float));

          double targetAngle{theta};

          // Calculate the next target based on the current real
          // angle adjusted for turns
          gHorizServo.measurePosition();
          const auto measuredAngle{gHorizServo.getMeasuredAngle()};
          const auto& turns{gHorizServo.turns};
          if (targetAngle > measuredAngle && 
              targetAngle - measuredAngle > 180.0)
          {
            gHorizPID.updateTarget(targetAngle + 360.0 * (turns - 1));
          }
          else if (measuredAngle > targetAngle &&
                   measuredAngle - targetAngle > 180)
          {
            gHorizPID.updateTarget(targetAngle + 360.0 * (turns + 1));
          }
          else
          {
            gHorizPID.updateTarget(targetAngle + 360.0 * turns);
          }
          break;
        }
        case FOCUS_SERVO_NUM:
        {
          gFocusServo.writeMicroseconds(command[1] * 10);
          break;
        }
        default: // Nothing here
          break;
      }
    }
  }

  static auto prevHorizUpdateMs{millis() - HORIZ_SERVO_REFRESH_RATE_MS};
  const auto now{millis()};
  if (now - prevHorizUpdateMs > HORIZ_SERVO_REFRESH_RATE_MS)
  {
    gHorizPID.move();
    prevHorizUpdateMs = now;
  }
}
