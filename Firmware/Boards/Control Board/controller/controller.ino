// #include <EEPROM.h>

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
#define CAL_IR 0x00
#define CAL_COLOR 0x01
#define CAL_COLOR_ABORT 0x02
#define ENABLE_COLOR 0x03
//-------------------------------//

// EEPROM Addresses --------------//
//--------------------------------//

#include "libraries/utils.h"

#include "devices.h"
#include "gui_manager.h"
#include "movement_interface.h"
#include "spi_interface_master.h"

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

ListMenu testing("testing");
ListMenu calibration("calibration");
ListMenu settings("settings");

ListMenu ledSettings("led-settings");
ListMenu pidSettings("pid-settings");

Gui gui;

SPIMasterInterface spi;
MovementInterface movement;

void setup()
{
    Serial.begin(115200);

    led.init();
    keyboard.init();
    buzzer.init();

    movement.init();
    spi.init();

    // US_N.init();
    // US_NW.init();
    // US_NE.init();
    // US_SW.init();
    // US_SE.init();

    movement.attachLeftMotor(SERVO_LEFT);
    movement.attachRightMotor(SERVO_RIGHT);

    mainMenu.addItem(MenuItem("Start", icons::start, []() { followLine(); }));
    mainMenu.addItem(MenuItem("Calib", icons::sliders, []() { gui.setActiveMenu("calibration"); }));
    mainMenu.addItem(MenuItem("Setup", icons::gear, []() { gui.setActiveMenu("settings"); }));
    gui.addMenu(&mainMenu);

    calibration.addItem(MenuItem("Back", []() { gui.setActiveMenu("main-menu"); }));
    calibration.addItem(MenuItem("Line", []() {
        spi.execAction(CAL_IR);
        gui.drawLoadingBar("Calibrating", 5000);
    }));
    calibration.addItem(MenuItem("Color", []() { gui.colorCalibrationWizard(spi, keyboard); }));
    gui.addMenu(&calibration);

    settings.addItem(MenuItem("Back", []() { gui.setActiveMenu("main-menu"); }));
    settings.addItem(MenuItem("Enable color", []() { spi.execAction(ENABLE_COLOR); }));
    settings.addItem(MenuItem("System Test", []() { gui.setActiveMenu("testing"); }));
    settings.addItem(MenuItem("PID Settings", []() { gui.setActiveMenu("pid-settings"); }));
    settings.addItem(MenuItem("RGB LED Settings", []() { gui.setActiveMenu("led-settings"); }));
    gui.addMenu(&settings);

    pidSettings.addItem(MenuItem("Speed", []() {
        movement.setSpeed(gui.numberDialog(movement.getSpeed(), 0.0, 1.0, 0.01, keyboard));
    }));

    ledSettings.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    ledSettings.addItem(MenuItem("Set Hue", []() {
        led.setH(gui.numberDialog<float>(led.getColor().getH(), 0, 360, 1, keyboard, [](float h) { led.setH(h); }));
    }));
    ledSettings.addItem(MenuItem("Set Saturation", []() {
        led.setS(gui.numberDialog<float>(led.getColor().getS(), 0, 1, 0.01, keyboard, [](float s) { led.setS(s); }));
    }));
    ledSettings.addItem(MenuItem("Set Brightness", []() {
        led.setV(gui.numberDialog<float>(led.getColor().getV(), 0, 1, 0.01, keyboard, [](float v) { led.setV(v); }));
    }));
    gui.addMenu(&ledSettings);

    testing.addItem(MenuItem("Back", []() { gui.setActiveMenu("settings"); }));
    testing.addItem(MenuItem("Color", []() {
        while (!keyboard.pressedOnce(1))
        {
            keyboard.update();
            byte colorData = spi.requestData<byte>(COLOR);
            byte color_sx = colorData & 0x01;
            byte color_dx = (colorData & 0x02) >> 1;
            byte aluminium = (colorData & 0x04) >> 2;
            gui.printColorData(color_sx, color_dx, aluminium);
        }
    }));
    testing.addItem(MenuItem("Left Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, [](float n) { movement.getLeftMotor()->setSpeed(n); });
        movement.getLeftMotor()->setSpeed(0);
    }));
    testing.addItem(MenuItem("Right Servo", []() {
        gui.numberDialog<float>(0, -1, 1, 0.01, keyboard, [](float n) { movement.getRightMotor()->setSpeed(n); });
        movement.getRightMotor()->setSpeed(0);
    }));
    gui.addMenu(&testing);

    gui.setActiveMenu("main-menu");
    gui.init();
}

void loop()
{
    keyboard.update();

    if (!keyboard.isConnected())
    {
        followLine();
    }

    if (keyboard.pressedRepeat(0))
    {
        gui.selectPreviousItem() ? buzzer.actionTone() : buzzer.disabledTone();
    }
    else if (keyboard.pressedOnce(1))
    {
        buzzer.actionTone();
        gui.execSelectedItemAction();
    }
    else if (keyboard.pressedRepeat(2))
    {
        gui.selectNextItem() ? buzzer.actionTone() : buzzer.disabledTone();
    }

    gui.drawActiveMenu();
}

void followLine()
{
    keyboard.update();
    while (!keyboard.pressedOnce(1))
    {
        keyboard.update();

        float line = spi.requestData<float>(LINE);
        movement.setLinePosition(line / 3500.0 - 1.0);
        movement.followLine();

        byte color = spi.requestData<byte>(COLOR);
        byte color_sx = color & B01;
        byte color_dx = (color & B10) >> 1;
        byte aluminium = (color & B100) >> 2;

        // if (color_sx)
        // {
        // }
        // if (color_dx)
        // {
        // }

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