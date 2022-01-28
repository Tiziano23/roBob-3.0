#pragma once
#include <Servo.h>
#include <PID_v1.h>
#include <Arduino.h>

#include "mathUtils.h"
#include "array.h"
#include "devices.h"

// Servos Costants ---------------//
#define M_PER_SECOND_FULL_SPEED 0.210526316      // m/s
#define DEG_PER_SECOND_AT_FULL_SPEED 90.04783149 // °/s
//--------------------------------//

enum ServoMode
{
    NORMAL,
    INVERTED
};

class ServoMotor
{
public:
    static const unsigned int MIN_VALUE = 1400;
    static const unsigned int MAX_VALUE = 1600;

    static const unsigned int MIN_RANGE = 1;
    static const unsigned int MAX_RANGE = 100;

    ServoMotor(ServoMode mode = NORMAL) : mode(mode) {}
    ServoMotor(int pin, ServoMode mode = NORMAL) : mode(mode)
    {
        attach(pin);
    }

    void attach(int pin)
    {
        pin = pin;
        motor.attach(pin);
        motor.writeMicroseconds(zero);
    }
    void setSpeed(double val)
    {
        speed = constrain(mode == INVERTED ? -val : val, -1, 1);
        motor.writeMicroseconds(math::fmap(speed, -1, 1, zero - range, zero + range));
    }
    double getSpeed()
    {
        return speed;
    }
    void setZeroValue(unsigned int val)
    {
        zero = val;
    }
    unsigned int getZeroValue()
    {
        return zero;
    }
    void setRange(unsigned int val)
    {
        range = val;
    }
    unsigned int getRange()
    {
        return range;
    }
    void setMode(ServoMode mode_)
    {
        mode = mode_;
    }
    ServoMode getMode()
    {
        return mode;
    }

private:
    int pin;
    unsigned int zero = 1500;
    unsigned int range = 45;
    double speed = 0;
    Servo motor = Servo();
    ServoMode mode = NORMAL;
};

class MovementInterface
{
public:
    enum ManeuverType
    {
        MOVE_FORWARD,
        MOVE_BACKWARDS,
        TURN_ANGLE_LEFT,
        TURN_ANGLE_RIGHT
    };
    struct ManeuverData
    {
        ManeuverType type;
        uint16_t duration;
        uint32_t startTime;
    };

    void init()
    {
        pidController.SetMode(AUTOMATIC);
        pidController.SetOutputLimits(-1, 1);
        leftMotor.setZeroValue(1500);
        rightMotor.setZeroValue(1504);
        loadDataFromEEPROM();
    }

    void attachLeftMotor(int pin)
    {
        leftMotor.attach(pin);
    }
    void attachRightMotor(int pin)
    {
        rightMotor.attach(pin);
    }
    void setGyroscopeAccelerometer(GyroscopeAccelerometer &gyroscope)
    {
        accelGyro = &gyroscope;
    }

    // Maneuvers
    int queuedManeuvers()
    {
        return maneuverQueue.size();
    }
    bool isManeuverQueued()
    {
        return maneuverQueue.size() > 0;
    }
    bool isManeuverRunning()
    {
        if (!currentManeuverAvailable)
            return false;
        else
            return (millis() - currentManeuver.startTime) < currentManeuver.duration;
        return false;
    }
    void updateManeuver()
    {
        if (isManeuverQueued() && !currentManeuverAvailable)
            startNextManeuver();

        if (hasCurrentManeuverEnded())
        {
            stop();
            currentManeuverAvailable = false;
            if (isManeuverQueued())
                startNextManeuver();
        }
    }
    void clearManeuverQueue()
    {
        maneuverQueue.clear();
    }
    void queueManeuver(ManeuverType type, uint16_t duration)
    {
        ManeuverData m;
        m.type = type;
        m.duration = duration;
        maneuverQueue.put(m);
    }

    // General movement function
    void stop()
    {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
    }
    void moveForward(double distance)
    {
        queueManeuver(MOVE_FORWARD, distance / (movementSpeed * M_PER_SECOND_FULL_SPEED) * 1000);
    }
    void moveBackwards(double distance)
    {
        queueManeuver(MOVE_BACKWARDS, distance / (movementSpeed * M_PER_SECOND_FULL_SPEED) * 1000);
    }
    void turnAngleLeft(float angle)
    {
        queueManeuver(TURN_ANGLE_LEFT, angle / (0.7 * DEG_PER_SECOND_AT_FULL_SPEED) * 1000);
    }
    void turnAngleRight(float angle)
    {
        queueManeuver(TURN_ANGLE_RIGHT, angle / (0.7 * DEG_PER_SECOND_AT_FULL_SPEED) * 1000);
    }

    // Line following
    void followLine()
    {
        PIDInput = linePosition;
        pidController.Compute();
        leftMotor.setSpeed(calcLeftMotorSpeed(PIDOutput, movementSpeed * speedMultiplier, 1));
        rightMotor.setSpeed(calcRightMotorSpeed(PIDOutput, movementSpeed * speedMultiplier, 1));
    }
    void setLinePosition(double position)
    {
        linePosition = position;
    }
    void setSpeed(double speed)
    {
        movementSpeed = speed;
        saveDataToEEPROM();
    }
    double getSpeed() { return movementSpeed; }
    void setSpeedMultiplier(double multiplier)
    {
        speedMultiplier = multiplier;
    }
    double getSpeedMultiplier() { return speedMultiplier; }
    void setKp(double _Kp)
    {
        Kp = _Kp;
        saveDataToEEPROM();
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKp() { return Kp; }
    void setKi(double _Ki)
    {
        Ki = _Ki;
        saveDataToEEPROM();
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKi() { return Ki; }
    void setKd(double _Kd)
    {
        Kd = _Kd;
        saveDataToEEPROM();
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKd() { return Kd; }

    // Green
    void checkGreenIgnoreTimeout()
    {
        if (ignoreGreenLeft || ignoreGreenRight)
        {
            if (millis() - lastIgnoreTime > greenIgnoreDelay)
            {
                ignoreGreenLeft = false;
                ignoreGreenRight = false;
            }
        }
    }
    bool shouldTurnLeft(bool greenLeft)
    {
        if (greenLeft && !ignoreGreenLeft)
        {
            ignoreGreenRight = !ignoreGreenRight;
            lastIgnoreTime = millis();
            return true;
        }
        else
            return false;
    }
    void turnLeft()
    {
        moveForward(0.075);
        turnAngleLeft(65);
    }
    bool shouldTurnRight(bool greenRight)
    {
        if (greenRight && !ignoreGreenRight)
        {
            ignoreGreenLeft = !ignoreGreenLeft;
            lastIgnoreTime = millis();
            return true;
        }
        else
            return false;
    }
    void turnRight()
    {
        moveForward(0.075);
        turnAngleRight(65);
    }
    bool shouldInverse(bool greenLeft, bool greenRight)
    {
        return greenLeft && greenRight;
    }
    void inverse()
    {
        turnAngleRight(180);
    }

    // Obstacle
    bool isAvoidingObstacle()
    {
        return avoidingObstacle;
    }
    void startAvoidingObstacleLeft(SR_04 *distanceSensor)
    {
        turnAngleLeft(90);
        obstacleDistanceSensor = distanceSensor;
        avoidingObstacle = true;
    }
    void startAvoidingObstacleRight(SR_04 *distanceSensor)
    {
        turnAngleRight(90);
        obstacleDistanceSensor = distanceSensor;
        avoidingObstacle = true;
    }
    void followObstacle()
    {
        if (obstacleDistanceSensor)
        {
            leftMotor.setSpeed(obstacleSpeed);
            rightMotor.setSpeed(obstacleSpeed);
        }
    }

    ServoMotor &getLeftMotor()
    {
        return leftMotor;
    }
    ServoMotor &getRightMotor()
    {
        return rightMotor;
    }

private:
    ServoMotor leftMotor = ServoMotor(INVERTED);
    ServoMotor rightMotor = ServoMotor();
    GyroscopeAccelerometer *accelGyro;

    // Maneuvers
    Queue<ManeuverData> maneuverQueue;
    ManeuverData currentManeuver;
    bool currentManeuverAvailable = false;
    double turnSpeed = 0.5;

    // Obstacle avoiding
    SR_04 *obstacleDistanceSensor;
    bool avoidingObstacle = false;
    double obstacleSpeed = 0.3;
    unsigned long obstacleStartTime;
    unsigned long obstacleTimeout = 10000; //ms

    // Green turn indicators
    bool ignoreGreenLeft = false;
    bool ignoreGreenRight = false;
    unsigned long lastIgnoreTime = 0;
    unsigned long greenIgnoreDelay = 10000; //ms

    // Line following
    double movementSpeed = 0.35;
    double speedMultiplier = 1;
    double linePosition = 0;
    // - PID Controller
    double Kp = 1.000;
    double Ki = 0.000;
    double Kd = 0.000;
    double PIDInput = 0;
    double PIDOutput = 0;
    double PIDSetpoint = 0;
    PID pidController = PID(&PIDInput, &PIDOutput, &PIDSetpoint, Kp, Ki, Kd, DIRECT);

    double calcLeftMotorSpeed(double x, double v, double n)
    {
        if (x >= 0)
            return -pow(x, n) * (1 + v) + v;
        else
            return pow(-x, n) * (1 - v) + v;
    }
    double calcRightMotorSpeed(double x, double v, double n)
    {
        return calcLeftMotorSpeed(-x, v, n);
    }

    // EEPROM data
    void loadDataFromEEPROM()
    {
        eepromManager.get(0, movementSpeed);
        eepromManager.get(1, Kp);
        eepromManager.get(2, Ki);
        eepromManager.get(3, Kd);
    }
    void saveDataToEEPROM()
    {
        eepromManager.put(0, movementSpeed);
        eepromManager.put(1, Kp);
        eepromManager.put(2, Ki);
        eepromManager.put(3, Kd);
    }

    // Maneuvers methods
    void moveForwardHandler()
    {
        leftMotor.setSpeed(movementSpeed);
        rightMotor.setSpeed(movementSpeed);
    }
    void moveBackwardsHandler()
    {
        leftMotor.setSpeed(-movementSpeed);
        rightMotor.setSpeed(-movementSpeed);
    }
    void turnAngleLeftHandler()
    {
        leftMotor.setSpeed(-turnSpeed);
        rightMotor.setSpeed(turnSpeed);
    }
    void turnAngleRightHandler()
    {
        leftMotor.setSpeed(turnSpeed);
        rightMotor.setSpeed(-turnSpeed);
    }
    bool hasCurrentManeuverEnded()
    {
        if (!currentManeuverAvailable)
            return false;
        return (millis() - currentManeuver.startTime) >= currentManeuver.duration;
    }
    void startNextManeuver()
    {
        currentManeuverAvailable = true;
        currentManeuver = maneuverQueue.get();
        switch (currentManeuver.type)
        {
        case MOVE_FORWARD:
            moveForwardHandler();
            break;
        case MOVE_BACKWARDS:
            moveBackwardsHandler();
            break;
        case TURN_ANGLE_LEFT:
            turnAngleLeftHandler();
            break;
        case TURN_ANGLE_RIGHT:
            turnAngleRightHandler();
            break;
        }
        currentManeuver.startTime = millis();
    }
};

// class ServoInterface {
//   private:
//     int velocity = 30;
//     int turnVelocity = 85;
//     int multiplier = 2;
//     int lineOffset = 0;

//     bool lastGreenDX = false;
//     bool lastGreenSX = false;
//     unsigned long lastGreenTime = 0;

//     int leftMotorPin;
//     int rightMotorPin;
//     int direction = 0;
//     Servo leftMotor;
//     Servo rightMotor;
//   public:
//     ServoInterface(int left, int right) {
//       this->leftMotorPin = left;
//       this->rightMotorPin = right;
//     }
//     void init() {
//       this->leftMotor = Servo();
//       this->rightMotor = Servo();
//       this->leftMotor.attach(leftMotorPin);
//       this->rightMotor.attach(rightMotorPin);
//       this->leftMotor.writeMicroseconds(1500);
//       this->rightMotor.writeMicroseconds(1500);
//       EEPROM.get(2, this->velocity);
//       EEPROM.get(4, this->multiplier);
//     }
//     void reset() {
//       int vel = 0;
//       EEPROM.get(2, vel);
//       this->setVelocity(vel);
//       this->setTurnVelocity(85);
//       this->setOffset(0);
//     }

//     //-- Getters & Setters --//
//     void setVelocity(int newVelocity) {
//       this->velocity = newVelocity;
//     }
//     int getVelocity() {
//       return this->velocity;
//     }
//     void setTurnVelocity(int newVelocity) {
//       this->turnVelocity = newVelocity;
//     }
//     int setTurnVelocity() {
//       return this->turnVelocity;
//     }
//     void setMultiplier(int newMultiplier) {
//       this->multiplier = newMultiplier;
//     }
//     int getMultiplier() {
//       return this->multiplier;
//     }
//     void setDirection(int newDir) {
//       this->direction = newDir;
//     }
//     int getDirection() {
//       return this->direction;
//     }
//     void setOffset(int newOffset) {
//       this->lineOffset = newOffset;
//     }
//     int getOffset() {
//       return this->lineOffset;
//     }

//     //-- Movement Functions --//
//     void stop() {
//       this->leftMotor.writeMicroseconds(1500);
//       this->rightMotor.writeMicroseconds(1500);
//     }
//     void setLeftSpeed(int speed) {
//       this->leftMotor.writeMicroseconds(1500 + map(speed, -100, 100, 70, -70));
//     }
//     void setRightSpeed(int speed) {
//       this->rightMotor.writeMicroseconds(1500 + map(speed, -100, 100, -70, 70));
//     }
//     void moveForward(int time) {
//       this->setLeftSpeed(this->velocity);
//       this->setRightSpeed(this->velocity);
//       delay(time);
//       this->stop();
//     }
//     void moveBackwards(int time) {
//       this->setLeftSpeed(-this->velocity);
//       this->setRightSpeed(-this->velocity);
//       delay(time);
//       this->stop();
//     }
//     void turnByAngle(float angle) {
//       int sign = angle > 0 ? 1 : -1;
//       unsigned long startTime = millis();
//       this->leftMotor.writeMicroseconds(1500 + (sign * 70));
//       this->rightMotor.writeMicroseconds(1500 + (sign * 70));
//       while (millis() - startTime < abs(angle) * 14.28);
//       this->leftMotor.writeMicroseconds(1500);
//       this->rightMotor.writeMicroseconds(1500);
//     }
//     void turnRightAngle(int dir, SPI_Controller spic) {
//       int line = 0;
//       unsigned long startTime = millis();
//       if (dir == 1) {
//         this->leftMotor.writeMicroseconds(1440);
//         this->rightMotor.writeMicroseconds(1430);
//       } else if (dir == -1) {
//         this->leftMotor.writeMicroseconds(1570);
//         this->rightMotor.writeMicroseconds(1560);
//       }
//       while (millis() - startTime < 1286) { // 1286 = 14.28(SERVO_CONST) * 90deg
//         d.clearDisplay();
//         d.setTextSize(2);
//         d.setCursor(0,0);
//         d.print("Right Angle: ");
//         d.println(dir);
//         d.display();
//         spic.request(LINE, 2);
//         line = spic.getResponse(LINE);
//         if (3450 <= line && line <= 3550)break;
//       }
//       this->setDirection(line / 35 - 100);
//     }
//     void turnGreenSX() {
//       if (this->lastGreenDX)return;
//       this->stop();
//       this->moveForward(1250);
//       this->turnByAngle(45);
//       if (this->lastGreenSX) {
//         this->turnByAngle(15);
//         this->moveForward(1000);
//         this->reset();
//       } else {
//         this->moveForward(300);
//         this->setVelocity(17);
//         this->setTurnVelocity(35
//         );
//         this->setOffset(-25);
//       }
//       this->lastGreenSX = !this->lastGreenSX;
//       this->lastGreenTime = millis();
//     }
//     void turnGreenDX() {
//       if (this->lastGreenSX)return;
//       this->stop();
//       this->moveForward(1250);
//       this->turnByAngle(-45);
//       if (this->lastGreenDX) {
//         this->turnByAngle(-15);
//         this->moveForward(1250);
//         this->reset();
//       } else {
//         this->moveForward(300);
//         this->setVelocity(17);
//         this->setTurnVelocity(35);
//         this->setOffset(20);
//       }
//       this->lastGreenDX = !this->lastGreenDX;
//       this->lastGreenTime = millis();
//     }
//     void turnGreenBoth() {
//       this->stop();
//       this->turnByAngle(180);
//       this->moveBackwards(300);
//       if (this->lastGreenSX || this->lastGreenDX) {
//         this->reset();
//       }
//     }

//     void update(SPI_Controller spic) {
//       //-- Obstacle --//
//       //int obstacleDist = ;
//       //--------------//

//       if (millis() - lastGreenTime > GREEN_TIMEOUT){
//         this->lastGreenSX = false;
//         this->lastGreenDX = false;
//         this->reset();
//       }
//       if(this->lineOffset != 0){
//         this->multiplier = 12;
//         /*if(this->direction == -100)this->turnRightAngle(-1,spic);
//         if(this->direction == 100)this->turnRightAngle(1,spic);*/
//       } else {
//         this->multiplier = map(abs(this->direction), 0, 100, 2, 15);
//       }

//       int leftSpeed = 0;
//       int rightSpeed = 0;
//       //int val = 0;
//       if (this->direction <= 0) {
//         leftSpeed  = map(this->direction, -100, 0 + this->lineOffset, -this->turnVelocity * this->multiplier /** this->multiplier*/, this->velocity);
//         rightSpeed = map(this->direction, -100, 0 + this->lineOffset, this->turnVelocity * this->multiplier /** this->multiplier*/, this->velocity);

//         /*val = constrain(this->multiplier * this->direction, -100, 0);
//           leftSpeed  = map(val, -100, 0, -this->turnVelocity, this->velocity);
//           rightSpeed = map(val, -100, 0, this->turnVelocity, this->velocity);*/
//       } else if (this->direction > 0) {
//         leftSpeed  = map(this->direction, 0 + this->lineOffset, 100, this->velocity, this->turnVelocity * this->multiplier /** this->multiplier*/);
//         rightSpeed = map(this->direction, 0 + this->lineOffset, 100, this->velocity, -this->turnVelocity * this->multiplier /** this->multiplier*/);

//         /*val = constrain(this->multiplier * this->direction, 0, 100);
//           leftSpeed  = map(val, 0, 100, this->velocity, this->turnVelocity);
//           rightSpeed = map(val, 0, 100, this->velocity, -this->turnVelocity);*/
//       }
//       leftSpeed = constrain(leftSpeed, -100, 100);
//       rightSpeed = constrain(rightSpeed, -100, 100);
//       this->leftMotor.writeMicroseconds(1500 + map(leftSpeed , -100, 100, 70, -70));
//       this->rightMotor.writeMicroseconds(1500 + map(rightSpeed, -100, 100, -70, 70));
//     }
// };
