#include <SoftwareSerial.h>
#include "libraries/eepromManager.h"
#include "libraries/utils.h"

#include "color_manager.h"

// Communication pins ------------//
#define GREEN_SX 4
#define GREEN_DX 5
#define CONFIG_RX 3
#define CONFIG_TX 2
//--------------------------------//

//------ Pins ------//
#define DX_SEL 8
#define DX_LED 12
#define SX_SEL 9
#define SX_LED 13
//------------------//

#define UPDATE_INTERVAL 150

struct config
{
    bool ready = false;
    bool aluminium = false;

    bool lightsEnabled = true;
    bool leftColorEnabled = true;
    bool rightColorEnabled = true;

    bool calibrateWhite = false;
    bool calibrateGreen = false;
    bool calibrateBlack = false;
    bool calibrateAluminium = false;

    hsv leftColorData = {0, 0, 0};
    hsv rightColorData = {0, 0, 0};
} cfg;

extern HardwareSerial Serial;
ColorManager colorManager(SX_SEL, SX_LED, DX_SEL, DX_LED);
SoftwareSerial configSerial(CONFIG_RX, CONFIG_TX);
unsigned long lastUpdate;

void setup()
{
    Serial.begin(115200);
    configSerial.begin(9600);

    pinMode(GREEN_SX, OUTPUT);
    pinMode(GREEN_DX, OUTPUT);
    digitalWrite(GREEN_SX, LOW);
    digitalWrite(GREEN_DX, LOW);

    // eepromManager.clear();
    eepromManager.init();
    colorManager.init();

    lastUpdate = millis() % (UPDATE_INTERVAL * 2);
}

void loop()
{
    if (configSerial.available())
    {
        readConfiguration();
    }

    colorManager.measure();

    if (cfg.ready)
    {
        if (cfg.calibrateWhite)
        {
            cfg.calibrateWhite = false;
            colorManager.calibrateWhite();
        }
        if (cfg.calibrateGreen)
        {
            cfg.calibrateGreen = false;
            colorManager.calibrateGreen();
        }
        if (cfg.calibrateBlack)
        {
            cfg.calibrateBlack = false;
            colorManager.calibrateBlack();
        }
        if (cfg.calibrateAluminium)
        {
            cfg.calibrateAluminium = false;
            colorManager.calibrateAluminium();
        }

        if (cfg.lightsEnabled)
        {
            colorManager.lightsOn();
        }
        else
        {
            colorManager.lightsOff();
        }

        if ((millis() % (UPDATE_INTERVAL * 2)) - lastUpdate > UPDATE_INTERVAL)
        {
            lastUpdate = millis() % (UPDATE_INTERVAL * 2);
            cfg.leftColorData = colorManager.getLeftSensor().getColor().getHSV();
            cfg.rightColorData = colorManager.getRightSensor().getColor().getHSV();
            cfg.aluminium = colorManager.checkAluminium();
            writeConfiguration();
        }
    }

    digitalWrite(GREEN_SX, colorManager.checkGreenSx());
    digitalWrite(GREEN_DX, colorManager.checkGreenDx());
}

void writeConfiguration()
{
    configSerial.write((uint8_t *)&cfg, sizeof(config));
}
void readConfiguration()
{
    configSerial.readBytes((uint8_t *)&cfg, sizeof(config));
}
void printConfiguration()
{
    Serial.println(F("Configuration: { "));
    Serial.print(F("\tready: "));
    Serial.print(cfg.ready);
    Serial.print(F("\n\taluminium: "));
    Serial.print(cfg.aluminium);

    Serial.print(F("\n\tightsEnabled: "));
    Serial.print(cfg.lightsEnabled);
    Serial.print(F("\n\tleftColorEnabled: "));
    Serial.print(cfg.leftColorEnabled);
    Serial.print(F("\n\trightColorEnabled: "));
    Serial.print(cfg.rightColorEnabled);

    Serial.print(F("\n\tcalibrateWhite: "));
    Serial.print(cfg.calibrateWhite);
    Serial.print(F("\n\tcalibrateGreen: "));
    Serial.print(cfg.calibrateGreen);
    Serial.print(F("\n\tcalibrateBlack: "));
    Serial.print(cfg.calibrateBlack);
    Serial.print(F("\n\tcalibrateAluminium: "));
    Serial.print(cfg.calibrateAluminium);
    Serial.println(F("\n}"));
}

void printCalibrations()
{
    // Serial.println(F("Left sensor:"));
    // Serial.print("\t");
    // colorManager.getLeftSensor().printWhiteRef();
    // Serial.print("\t");
    // colorManager.getLeftSensor().printWhiteRng();
    // Serial.print("\t");
    // colorManager.getLeftSensor().printGreenRef();
    // Serial.print("\t");
    // colorManager.getLeftSensor().printGreenRng();
    // Serial.println(F("\nRight sensor:"));
    // Serial.print("\t");
    // colorManager.getRightSensor().printWhiteRef();
    // Serial.print("\t");
    // colorManager.getRightSensor().printWhiteRng();
    // Serial.print("\t");
    // colorManager.getRightSensor().printGreenRef();
    // Serial.print("\t");
    // colorManager.getRightSensor().printGreenRng();
    // Serial.println();
}