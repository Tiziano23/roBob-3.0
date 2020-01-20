// Servo Pins --------------------//
#define SERVO_LEFT 7
#define SERVO_RIGHT 6
//--------------------------------//

// Keyboard Buttons Pins ---------//
#define KB_BTN_LEFT 30
#define KB_BTN_CENTER 32
#define KB_BTN_RIGHT 34
#define KB_CONNECTED 36
//--------------------------------//

// Buzzer Pin --------------------//
#define BUZ_PIN 2
//--------------------------------//

// SR-04 Ultrasonic sensos pins --//
#define US_N_T 0
#define US_N_E 2

#define US_NW_T 3
#define US_NW_E 4

#define US_NE_T 45
#define US_NE_E 47

#define US_SW_T 6
#define US_SW_E 7

#define US_SE_T 22
#define US_SE_E 24
//--------------------------------//

// RGB Led pins ------------------//
#define LED_R 5
#define LED_G 4
#define LED_B 3
//--------------------------------//

// SPI Addresses -----------------//
#define NONE 0x00
//------ Data --------------------//
#define LINE 0x00
#define COLOR 0x01
//------ Functions ---------------//
#define CAL_LINE 0x00
#define CAL_COLOR 0x01
#define CAL_COLOR_ABORT 0x02
#define TOGGLE_LEFT_COLOR 0x03
#define TOGGLE_RIGHT_COLOR 0x04
//--------------------------------//

// EEPROM Addresses --------------//
//--------------------------------//

#include "devices.h"
#include "gui_manager.h"
#include "movement_interface.h"
#include "libraries/spi/spi_interface_master.h"

extern HardwareSerial Serial;

RGBLed led(LED_R, LED_G, LED_B);
Buzzer buzzer = Buzzer(BUZ_PIN);
Keyboard keyboard(KB_BTN_LEFT, KB_BTN_CENTER, KB_BTN_RIGHT, KB_CONNECTED);
// SR_04 US_N  = SR_04(US_N_T, US_N_E);
// SR_04 US_NW = SR_04(US_NW_T, US_NW_E);
// SR_04 US_NE = SR_04(US_NE_T, US_NE_E);
// SR_04 US_SW = SR_04(US_SW_T, US_SW_E);
// SR_04 US_SE = SR_04(US_SE_T, US_SE_E);
// SR_04 sensors[5] = {US_N, US_NW, US_NE, US_SW, US_SE};

MainMenu mainMenu("main-menu");

// main-menu
ListMenu calibration("calibration");
ListMenu settings("settings");

// main-menu --> settings
ListMenu testing("settings:system-test");
ListMenu servoSettings("settings:servo-settings");
ListMenu ledSettings("settings:led-settings");
ListMenu pidSettings("settings:pid-settings");

// main-menu --> settings --> servoSettings
ListMenu leftServoSettings("settings:servo-settings:left-servo");
ListMenu rightServoSettings("settings:servo-settings:right-servo");

Gui gui;
MovementInterface movement;
SPIMasterInterface spi;
GyroscopeAccelerometer accelGyro;

void setup()
{
    Serial.begin(115200);

    spi.init();
    led.init();
    buzzer.init();
    keyboard.init();
    movement.init();
    accelGyro.init();

    // US_N.init();
    // US_NW.init();
    // US_NE.init();
    // US_SW.init();
    // US_SE.init();

    movement.attachLeftMotor(SERVO_LEFT);
    movement.attachRightMotor(SERVO_RIGHT);
    movement.setGyroscopeAccelerometer(accelGyro);

    mainMenu.addItem(MenuItem("Start", icons::start, []() { followLine(); }));
    mainMenu.addItem(MenuItem("Calib", icons::sliders, []() { gui.setActiveMenu("calibration"); }));
    mainMenu.addItem(MenuItem("Setup", icons::gear, []() { gui.setActiveMenu("settings"); }));

    calibration.addItem(MenuItem("Back", []() { gui.setActiveMenu("main-menu"); }));
    calibration.addItem(MenuItem("Line", []() {
        spi.execAction(CAL_LINE);
        gui.drawLoadingBar("Calibrating", 10000);
    }));
    calibration.addItem(MenuItem("Color", []() { gui.colorCalibrationWizard(spi, keyboard); }));

    settings.addItem(MenuItem("Back", []() { gui.setActiveMenu("main-menu"); }));
    settings.addItem(MenuItem("System Test", []() { gui.setActiveMenu("settings:system-test"); }));
    settings.addItem(MenuItem("Servo Settings", []() { gui.setActiveMenu("settings:servo-settings"); }));
    settings.addItem(MenuItem("PID Settings", []() { gui.setActiveMenu("settings:pid-settings"); }));
    settings.addItem(MenuItem("LED Settings", []() { gui.setActiveMenu("settings:led-settings"); }));

    pidSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    pidSettings.addItem(MenuItem("Set speed", []() {
        movement.setSpeed(gui.numberDialog(movement.getSpeed(), 0.0, 1.0, 0.01, keyboard, Percentual));
    }));
    pidSettings.addItem(MenuItem("Set Kp", []() {
        movement.setKp(gui.numberDialog(movement.getKp(), 0.0, 10.0, 0.001, keyboard, Real3));
    }));
    pidSettings.addItem(MenuItem("Set Ki", []() {
        movement.setKi(gui.numberDialog(movement.getKi(), 0.0, 10.0, 0.001, keyboard, Real3));
    }));
    pidSettings.addItem(MenuItem("Set Kd", []() {
        movement.setKd(gui.numberDialog(movement.getKd(), 0.0, 10.0, 0.001, keyboard, Real3));
    }));

    ledSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    ledSettings.addItem(MenuItem("Set Hue", []() {
        led.setH(gui.numberDialog<int>(led.getColor().getH(), 0, 360, 1, keyboard, Integer, [](int h) { led.setH(h); }));
    }));
    ledSettings.addItem(MenuItem("Set Saturation", []() {
        led.setS(gui.numberDialog<float>(led.getColor().getS(), 0, 1, 0.01, keyboard, Real2, [](float s) { led.setS(s); }));
    }));
    ledSettings.addItem(MenuItem("Set Brightness", []() {
        led.setV(gui.numberDialog<float>(led.getColor().getV(), 0, 1, 0.01, keyboard, Real2, [](float v) { led.setV(v); }));
    }));

    testing.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    testing.addItem(MenuItem("Color", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            byte colorData = spi.requestData<byte>(COLOR);
            byte color_sx = colorData & B1;
            byte color_dx = (colorData & B10) >> 1;
            byte aluminium = (colorData & B100) >> 2;
            gui.printColorData(color_sx, color_dx, aluminium);
        }
    }));
    testing.addItem(MenuItem("Accelerometer", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            accelGyro.update();
            math::Vector3f accel = accelGyro.getAcceleration();
            d.clearDisplay();
            d.setTextSize(1);
            d.setCursor(0, 0);
            d.println("Accel: {");
            d.print("x: ");
            d.println(accel.x);
            d.print("y: ");
            d.println(accel.y);
            d.print("z: ");
            d.println(accel.z);
            d.println(" }");
            d.display();
        }
    }));
    testing.addItem(MenuItem("Gyroscope", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            accelGyro.update();
            math::Vector3f rot = accelGyro.getRotation();
            d.clearDisplay();
            d.setTextSize(1);
            d.setCursor(0, 0);
            d.println("Gyro: {");
            d.print("x: ");
            d.println(rot.x);
            d.print("y: ");
            d.println(rot.y);
            d.print("z: ");
            d.println(rot.z);
            d.println(" }");
            d.display();
        }
    }));
    testing.addItem(MenuItem("Left Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, Percentual, [](float n) { movement.getLeftMotor().setSpeed(n); });
        movement.getLeftMotor().setSpeed(0);
    }));
    testing.addItem(MenuItem("Right Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, Percentual, [](float n) { movement.getRightMotor().setSpeed(n); });
        movement.getRightMotor().setSpeed(0);
    }));

    servoSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    servoSettings.addItem(MenuItem("Move Forward", []() { movement.moveForward(1); }));
    servoSettings.addItem(MenuItem("Left Servo", []() { gui.setActiveMenu("settings:servo-settings:left-servo"); }));
    servoSettings.addItem(MenuItem("Right Servo", []() { gui.setActiveMenu("settings:servo-settings:right-servo"); }));

    leftServoSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings:servo-settings"); }));
    leftServoSettings.addItem(MenuItem("Zero", []() {
        gui.numberDialog<unsigned int>(movement.getLeftMotor().getZeroValue(), ServoMotor::MIN_VALUE, ServoMotor::MAX_VALUE, 1, keyboard, Integer, [](unsigned int val) {
            movement.getLeftMotor().setZeroValue(val);
            movement.getLeftMotor().setSpeed(0);
        });
    }));
    leftServoSettings.addItem(MenuItem("Range", []() {
        gui.numberDialog<unsigned int>(movement.getLeftMotor().getRange(), ServoMotor::MIN_RANGE, ServoMotor::MAX_RANGE, 1, keyboard, Integer, [](unsigned int val) {
            movement.getLeftMotor().setRange(val);
        });
    }));

    rightServoSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings:servo-settings"); }));
    rightServoSettings.addItem(MenuItem("Zero", []() {
        gui.numberDialog<unsigned int>(movement.getRightMotor().getZeroValue(), ServoMotor::MIN_VALUE, ServoMotor::MAX_VALUE, 1, keyboard, Integer, [](unsigned int val) {
            movement.getRightMotor().setZeroValue(val);
            movement.getRightMotor().setSpeed(0);
        });
    }));
    rightServoSettings.addItem(MenuItem("Range", []() {
        gui.numberDialog<unsigned int>(movement.getRightMotor().getRange(), ServoMotor::MIN_RANGE, ServoMotor::MAX_RANGE, 1, keyboard, Integer, [](unsigned int val) {
            movement.getRightMotor().setRange(val);
        });
    }));

    gui.addMenu(&mainMenu);
    gui.addMenu(&calibration);
    gui.addMenu(&settings);
    gui.addMenu(&pidSettings);
    gui.addMenu(&ledSettings);
    gui.addMenu(&testing);
    gui.addMenu(&servoSettings);
    gui.addMenu(&leftServoSettings);
    gui.addMenu(&rightServoSettings);

    gui.setActiveMenu("main-menu");
    gui.init();
}

void loop()
{
    accelGyro.update();
    keyboard.update();
    if (!keyboard.isConnected())
        followLine();

    if (keyboard.pressedRepeat(LEFT))
        gui.selectPreviousItem() ? buzzer.actionTone() : buzzer.disabledTone();
    else if (keyboard.pressedOnce(MIDDLE))
    {
        keyboard.update();
        gui.execSelectedItemAction() ? buzzer.actionTone() : buzzer.disabledTone();
    }
    else if (keyboard.pressedRepeat(RIGHT))
        gui.selectNextItem() ? buzzer.actionTone() : buzzer.disabledTone();

    gui.drawActiveMenu();
}

void followLine()
{
    keyboard.update();
    while (!keyboard.pressedOnce(MIDDLE))
    {
        keyboard.update();

        double line = spi.requestData<double>(LINE);
        movement.setLinePosition(line);
        movement.followLine();

        byte color = spi.requestData<byte>(COLOR);
        byte color_sx = color & B1;
        byte color_dx = (color & B10) >> 1;
        byte aluminium = (color & B100) >> 2;

        if (color_sx)
        {
        }
        if (color_dx)
        {
        }

        d.clearDisplay();
        d.setTextSize(2);
        d.setCursor(40, 0);
        d.println(line);
        d.print("SX:");
        d.print(color_sx);
        d.print("  DX:");
        d.println(color_dx);
        d.display();
    }
    movement.stop();
}