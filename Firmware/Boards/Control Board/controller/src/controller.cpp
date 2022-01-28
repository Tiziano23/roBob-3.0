// Servo Pins -------------------//
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

// SR-04 Ultrasonic sensors pins -//
#define US_NE_T 23
#define US_NE_E 25

#define US_N_T 35
#define US_N_E 37

#define US_NW_T 45
#define US_NW_E 47

#define US_SW_T 39
#define US_SW_E 41

#define US_SE_T 27
#define US_SE_E 29
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
#define LEFT_COLOR_DATA 0x02
#define RIGHT_COLOR_DATA 0x03
//------ Functions ---------------//
#define CAL_LINE 0x00
#define CAL_WHITE 0x01
#define CAL_GREEN 0x02
#define CAL_BLACK 0x03
#define CAL_ALUMI 0x04
#define TOGGLE_LIGHTS 0x05
#define TOGGLE_LEFT_COLOR 0x06
#define TOGGLE_RIGHT_COLOR 0x07
//--------------------------------//

#include "spi/spi_interface_master.h"
#include "eepromManager.h"

#include "navigation_manager.h"
#include "devices.h"
#include "gui_manager.h"
#include "movement_interface.h"

extern HardwareSerial Serial;

RGBLed led(LED_R, LED_G, LED_B);
Buzzer buzzer = Buzzer(BUZ_PIN);
Keyboard keyboard(KB_BTN_LEFT, KB_BTN_CENTER, KB_BTN_RIGHT, KB_CONNECTED);

MainMenu mainMenu("main-menu");

// main-menu
ListMenu calibration("calibration");
ListMenu settings("settings");

// main-menu --> calibration
ListMenu colorCalibration("calibration:color");

// main-menu --> settings
ListMenu systemTest("settings:system-test");
ListMenu servoSettings("settings:servo-settings");
ListMenu ledSettings("settings:led-settings");
ListMenu pidSettings("settings:pid-settings");

// main-menu --> settings --> systemTest
ListMenu colorTest("settings:system-test:color");

// main-menu --> settings --> servoSettings
ListMenu leftServoSettings("settings:servo-settings:left-servo");
ListMenu rightServoSettings("settings:servo-settings:right-servo");

Gui gui;
MovementInterface movement;
SPIMasterInterface spi;
GyroscopeAccelerometer accelGyro;

void setup()
{
    Wire.begin();
    Serial.begin(115200);
    
    eepromManager.init();

    gui.init();
    gui.boot();

    spi.init();
    led.init();
    buzzer.init();
    keyboard.init();
    movement.init();

    gui.printGyroscopeCalibrationMessage();    
    accelGyro.init();

    led.setColor(RGBLed::BLUE);

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
    calibration.addItem(MenuItem("Color", []() { gui.setActiveMenu("calibration:color"); }));

    colorCalibration.addItem(MenuItem("Back", []() { gui.setActiveMenu("calibration"); }));
    colorCalibration.addItem(MenuItem("White", []() {
        gui.colorCalibrationGui(keyboard, spi, CAL_WHITE, 6000);
    }));
    colorCalibration.addItem(MenuItem("Green", []() {
        gui.colorCalibrationGui(keyboard, spi, CAL_GREEN, 6000);
    }));
    colorCalibration.addItem(MenuItem("Black", []() {
        gui.colorCalibrationGui(keyboard, spi, CAL_BLACK, 6000);
    }));
    colorCalibration.addItem(MenuItem("Aluminium", []() {
        gui.colorCalibrationGui(keyboard, spi, CAL_ALUMI, 6000);
    }));

    settings.addItem(MenuItem("Back", []() { gui.setActiveMenu("main-menu"); }));
    settings.addItem(MenuItem("System Test", []() { gui.setActiveMenu("settings:system-test"); }));
    settings.addItem(MenuItem("Servo Settings", []() { gui.setActiveMenu("settings:servo-settings"); }));
    settings.addItem(MenuItem("PID Settings", []() { gui.setActiveMenu("settings:pid-settings"); }));
    settings.addItem(MenuItem("LED Settings", []() { gui.setActiveMenu("settings:led-settings"); }));
    settings.addItem(MenuItem("Accelerometer", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            accelGyro.update();
            d.clearDisplay();
            d.setTextSize(1);
            d.setCursor(0, 0);
            d.println("Accel: {");
            d.print("x: ");
            d.print(accelGyro.getWorldAcceleration().x / 1000);
            d.print("m/ss\ny: ");
            d.print(accelGyro.getWorldAcceleration().y / 1000);
            d.print("m/ss\nz: ");
            d.print(accelGyro.getWorldAcceleration().z / 1000);
            d.println("m/ss }");
            d.display();
        }
    }));
    settings.addItem(MenuItem("Gyroscope", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            accelGyro.update();

            if (keyboard.pressedOnce(RIGHT))
                accelGyro.registerOffset();

            d.clearDisplay();
            d.setTextSize(1);
            d.setCursor(0, 0);
            d.println("Gyro: {");
            d.print("x: ");
            d.print((int)(accelGyro.getRotation().x * RAD_TO_DEG));
            d.print("deg\ny: ");
            d.print((int)(accelGyro.getRotation().y * RAD_TO_DEG));
            d.print("deg\nz: ");
            d.print((int)(accelGyro.getRotation().z * RAD_TO_DEG));
            d.println("deg }");

            d.setCursor(90, 24);
            d.print("Calib.");

            d.display();
        }
    }));

    systemTest.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    systemTest.addItem(MenuItem("Line Sensors", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            double line = spi.requestData<double>(LINE);
            d.clearDisplay();
            d.setTextSize(2);
            d.setCursor(40, 0);
            d.println(line, 3);
            d.display();
        }
    }));
    systemTest.addItem(MenuItem("Color Sensors", []() { gui.setActiveMenu("settings:system-test:color"); }));
    systemTest.addItem(MenuItem("Left Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, Percentual, [](float n) { movement.getLeftMotor().setSpeed(n); });
        movement.getLeftMotor().setSpeed(0);
    }));
    systemTest.addItem(MenuItem("Right Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, Percentual, [](float n) { movement.getRightMotor().setSpeed(n); });
        movement.getRightMotor().setSpeed(0);
    }));

    colorTest.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings:system-test"); }));
    colorTest.addItem(MenuItem("Toggle lights", []() {
        spi.execAction(TOGGLE_LIGHTS);
        gui.drawActionCompleted();
    }));
    colorTest.addItem(MenuItem("Green recognition", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            byte data = spi.requestData<byte>(COLOR);
            bool sx, dx, aluminium;
            decodeColorData(data, sx, dx, aluminium);
            gui.printColorData(sx, dx, aluminium);
        }
    }));
    colorTest.addItem(MenuItem("Left color", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            hsv data = spi.requestData<hsv>(LEFT_COLOR_DATA);
            led.setH(data.h - 0.1);
            led.setS(math::fmap(data.s, 0.46, 0.51, 0, 1));
            led.setV(0.1);

            d.clearDisplay();
            d.setCursor(0, 0);
            d.setTextSize(1);
            d.print("h: ");
            d.print(data.h * 100., 5);
            d.print("%\ns: ");
            d.print(data.s * 100., 5);
            d.print("%\nv: ");
            d.print(data.v * 100., 5);
            d.println("%");
            d.display();
        }
        led.off();
    }));
    colorTest.addItem(MenuItem("Right color", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            hsv data = spi.requestData<hsv>(RIGHT_COLOR_DATA);
            led.setH(data.h - 0.1);
            led.setS(math::fmap(data.s, 0.46, 0.51, 0, 1));
            led.setV(0.05);

            d.clearDisplay();
            d.setCursor(0, 0);
            d.setTextSize(1);
            d.print("h: ");
            d.print(data.h * 100, 5);
            d.print("%\ns: ");
            d.print(data.s * 100, 5);
            d.print("%\nv: ");
            d.print(data.v * 100, 5);
            d.println("%");
            d.display();
        }
        led.off();
    }));

    servoSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    servoSettings.addItem(MenuItem("Move Forward", []() { movement.moveForward(1); }));
    servoSettings.addItem(MenuItem("Calibrate turn", []() {
        while (!keyboard.pressedOnce(MIDDLE))
        {
            keyboard.update();
            movement.getLeftMotor().setSpeed(1);
            movement.getRightMotor().setSpeed(-1);
        }
        movement.stop();
    }));
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
        led.setH(gui.numberDialog<int>(led.getColor().getH() * 360, 0, 360, 1, keyboard, Integer, [](int h) { led.setH((double)h / 360.); }));
    }));
    ledSettings.addItem(MenuItem("Set Saturation", []() {
        led.setS(gui.numberDialog<float>(led.getColor().getS(), 0, 1, 0.01, keyboard, Real2, [](float s) { led.setS(s); }));
    }));
    ledSettings.addItem(MenuItem("Set Brightness", []() {
        led.setV(gui.numberDialog<float>(led.getColor().getV(), 0, 1, 0.01, keyboard, Real2, [](float v) { led.setV(v); }));
    }));

    gui.addMenu(&mainMenu);

    gui.addMenu(&calibration);
    gui.addMenu(&colorCalibration);

    gui.addMenu(&settings);
    gui.addMenu(&systemTest);
    gui.addMenu(&colorTest);
    gui.addMenu(&servoSettings);
    gui.addMenu(&leftServoSettings);
    gui.addMenu(&rightServoSettings);
    gui.addMenu(&pidSettings);
    gui.addMenu(&ledSettings);

    gui.setActiveMenu("main-menu");
    gui.drawActiveMenu();
}

void loop()
{
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
        accelGyro.update();

        double line = spi.requestData<double>(LINE);
        movement.setLinePosition(line);

        byte colorData = spi.requestData<byte>(COLOR);
        bool greenLeft, greenRight, aluminium;
        decodeColorData(colorData, greenLeft, greenRight, aluminium);
        
        movement.followLine();

        if (greenLeft && greenRight)
        {
            led.setColor(RGBLed::GREEN);
            movement.turnGreenBoth();
        }
        else if (greenLeft)
        {
            led.setColor(RGBLed::GREEN);
            movement.turnGreenLeft();
        }
        else if (greenRight)
        {
            led.setColor(RGBLed::GREEN);
            movement.turnGreenRight();
        }
        else
        {
            led.setColor(RGBLed::BLUE);
        }

        if (10 * DEG_TO_RAD <= abs(accelGyro.getRotation().y) && abs(accelGyro.getRotation().y) <= 30 * DEG_TO_RAD)
        {
            if (math::sign(accelGyro.getRotation().y) > 0 && movement.getSpeedMultiplier() != 2)
                movement.setSpeedMultiplier(2);
            if (math::sign(accelGyro.getRotation().y) < 0 && movement.getSpeedMultiplier() != 0.5)
                movement.setSpeedMultiplier(0.5);
        }
        else if (movement.getSpeedMultiplier() != 1)
        {
            movement.setSpeedMultiplier(1);
        }

        gui.lineFollowerGui(line, greenLeft, greenRight);
    }
    movement.stop();
}

void decodeColorData(byte data, bool &sx, bool &dx, bool &aluminium)
{
    sx = data & 1;
    dx = (data & 2) >> 1;
    aluminium = (data & 4) >> 2;
}