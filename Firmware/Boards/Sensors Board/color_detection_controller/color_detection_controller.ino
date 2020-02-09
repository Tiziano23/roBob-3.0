#include <NeoSWSerial.h>
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

// #define UPDATE_INTERVAL 1500
// #define CLEAR_EEPROM

extern HardwareSerial Serial;

struct config
{
    bool aluminium = false;
    bool leftColorEnabled = true;
    bool rightColorEnabled = true;
    bool calibrateWhite = false;
    bool calibrateGreen = false;
    bool calibrateBlack = false;
    bool calibrateAluminium = false;
    bool readyForNextRead = false;

    hsv leftColorData = {0, 0, 0};
    hsv rightColorData = {0, 0, 0};
} cfg;

bool update = false;

ColorManager colorManager(SX_SEL, SX_LED, DX_SEL, DX_LED);
NeoSWSerial configSerial(CONFIG_RX, CONFIG_TX);

void setup()
{
    Serial.begin(115200);
    configSerial.begin(9600);

    pinMode(GREEN_SX, OUTPUT);
    pinMode(GREEN_DX, OUTPUT);
    digitalWrite(GREEN_SX, LOW);
    digitalWrite(GREEN_DX, LOW);

#ifdef CLEAR_EEPROM
    eepromManager.clear();
#endif
    eepromManager.init();
    colorManager.init();

    digitalWrite(SX_LED, HIGH);
    digitalWrite(DX_LED, HIGH);

    printCalibrations();
}

void loop()
{
    if (configSerial.available() >= sizeof(config))
    {
        readConfiguration();
    }

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

    colorManager.measure();
    digitalWrite(GREEN_SX, colorManager.checkGreenSx());
    digitalWrite(GREEN_DX, colorManager.checkGreenDx());
    cfg.aluminium = colorManager.checkAluminium();

    cfg.leftColorData = colorManager.getLeftSensor().getColor().getHSV();
    cfg.rightColorData = colorManager.getLeftSensor().getColor().getHSV();

    if (cfg.readyForNextRead)
    {
        cfg.readyForNextRead = false;
        writeConfiguration();
    }
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
    Serial.print(F("\twhite: "));
    Serial.print(cfg.calibrateWhite);
    Serial.print(F("\n\tgreen: "));
    Serial.print(cfg.calibrateGreen);
    Serial.print(F("\n\tblack: "));
    Serial.print(cfg.calibrateBlack);
    Serial.print(F("\n\taluminium: "));
    Serial.print(cfg.calibrateAluminium);
    Serial.println(F("\n}"));
}
void printCalibrations()
{
    Serial.println(F("Left sensor:"));
    Serial.print("\t");
    colorManager.getLeftSensor().printWhiteRef();
    Serial.print("\t");
    colorManager.getLeftSensor().printWhiteRng();
    Serial.print("\t");
    colorManager.getLeftSensor().printGreenRef();
    Serial.print("\t");
    colorManager.getLeftSensor().printGreenRng();
    Serial.println(F("\nRight sensor:"));
    Serial.print("\t");
    colorManager.getRightSensor().printWhiteRef();
    Serial.print("\t");
    colorManager.getRightSensor().printWhiteRng();
    Serial.print("\t");
    colorManager.getRightSensor().printGreenRef();
    Serial.print("\t");
    colorManager.getRightSensor().printGreenRng();
    Serial.println();
}