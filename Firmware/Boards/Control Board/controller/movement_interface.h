#pragma once
#include <Servo.h>
#include <PID_v1.h>

#include "libraries/math.h"
#include "devices.h"

// Servos Costants ---------------//
#define TURN_SPEED 14.28 // °/s
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
        motor.writeMicroseconds(math::fMap(speed, -1, 1, zero - range, zero + range));
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
    unsigned int range = 70;
    double speed = 0;
    Servo motor = Servo();
    ServoMode mode = NORMAL;
};

class MovementInterface
{
public:
    void init()
    {
        pidController.SetOutputLimits(-1, 1);
        pidController.SetMode(AUTOMATIC);

        leftMotor.setZeroValue(1500);
        rightMotor.setZeroValue(1504);
    }

    void attachLeftMotor(int pin)
    {
        leftMotor.attach(pin);
    }
    void attachRightMotor(int pin)
    {
        rightMotor.attach(pin);
    }

    void stop()
    {
        pidController.SetMode(MANUAL);
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
    }
    void followLine()
    {
        pidController.SetMode(AUTOMATIC);
        PIDInput = linePosition;
        pidController.Compute();
        leftMotor.setSpeed(calcLeftMotorSpeed(PIDOutput, movementSpeed));
        rightMotor.setSpeed(calcRightMotorSpeed(PIDOutput, movementSpeed));
    }

    ServoMotor &getLeftMotor()
    {
        return leftMotor;
    }
    ServoMotor &getRightMotor()
    {
        return rightMotor;
    }

    void setLinePosition(double position)
    {
        linePosition = position;
    }

    void setSpeed(double speed)
    {
        movementSpeed = speed;
    }
    double getSpeed()
    {
        return movementSpeed;
    }

    void setKp(double _Kp)
    {
        Kp = _Kp;
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKp()
    {
        return Kp;
    }
    void setKi(double _Ki)
    {
        Ki = _Ki;
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKi()
    {
        return Ki;
    }
    void setKd(double _Kd)
    {
        Kd = _Kd;
        pidController.SetTunings(Kp, Ki, Kd);
    }
    double getKd()
    {
        return Kd;
    }

    void moveForward(double distance)
    {
        double distanceTravelled = 0;
        double deltaTime = 0, lastTime = micros();
        leftMotor.setSpeed(movementSpeed);
        rightMotor.setSpeed(movementSpeed);
        while (distanceTravelled < distance)
        {
            distanceTravelled += ga.getAcceleration().x * MATH_g * deltaTime;
            lastTime = micros();

        }
        stop();
    }
    void moveBackwards(double distance)
    {
        leftMotor.setSpeed(-movementSpeed);
        rightMotor.setSpeed(-movementSpeed);
        
        stop();
    }
    void turnAngle(float angle)
    {
        unsigned long startTime = millis();
        leftMotor.setSpeed(0.7 * math::sign(angle));
        rightMotor.setSpeed(0.7 * math::sign(angle));
        while (millis() - startTime < abs(angle) * TURN_SPEED)
            ;
        stop();
    }
    void turnGreenSX()
    {
        stop();
        moveForward(1250);
        turnAngle(45);
        // if (lastGreenSX)
        // {
        //     turnAngle(15);
        //     moveForward(1000);
        //     reset();
        // }
        // else
        // {
        //     moveForward(300);
        //     setVelocity(17);
        //     setTurnVelocity(35);
        //     setOffset(-25);
        // }
        // lastGreenSX = !lastGreenSX;
        // lastGreenTime = millis();
    }
    void turnGreenDX()
    {
        stop();
        moveForward(1250);
        turnAngle(-45);
        // if (lastGreenDX)
        // {
        //     turnAngle(-15);
        //     moveForward(1250);
        //     reset();
        // }
        // else
        // {
        //     moveForward(300);
        //     setVelocity(17);
        //     setTurnVelocity(35);
        //     setOffset(20);
        // }
        // lastGreenDX = !lastGreenDX;
        // lastGreenTime = millis();
    }
    void turnGreenBoth()
    {
        stop();
        turnAngle(180);
        moveBackwards(300);
    }

private:
    double movementSpeed = 0.50;
    double linePosition = 0;

    double PIDInput = 0;
    double PIDOutput = 0;
    double PIDSetpoint = 0;

    double Kp = 2.650;
    double Ki = 2.000;
    double Kd = 0.350;

    PID pidController = PID(&PIDInput, &PIDOutput, &PIDSetpoint, Kp, Ki, Kd, DIRECT);
    ServoMotor leftMotor = ServoMotor(INVERTED);
    ServoMotor rightMotor = ServoMotor();

    GyroscopeAccelerometer ga = GyroscopeAccelerometer();

    double calcLeftMotorSpeed(double x, double v)
    {
        if (x >= 0)
            return -pow(x, 1) * (1 + v) + v;
        else
            return pow(-x, 1) * (1 - v) + v;
    }
    double calcRightMotorSpeed(double x, double v)
    {
        if (x >= 0)
            return pow(x, 1) * (1 - v) + v;
        else
            return -pow(-x, 1) * (1 + v) + v;
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
//     void turnAngle(float angle) {
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
//       this->turnAngle(45);
//       if (this->lastGreenSX) {
//         this->turnAngle(15);
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
//       this->turnAngle(-45);
//       if (this->lastGreenDX) {
//         this->turnAngle(-15);
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
//       this->turnAngle(180);
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
