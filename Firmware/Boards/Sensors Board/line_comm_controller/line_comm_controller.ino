#include <SoftwareSerial.h>
#include "libraries/spi/spi_interface_slave.h"
#include "libraries/utils.h"

#include "line_sensor_manager.h"

// Communication pins ------------//
#define GREEN_SX 5
#define GREEN_DX 6
#define CONFIG_RX 3
#define CONFIG_TX 4
//--------------------------------//

// QTR Pins ----------------------//
#define QTR_FRONT_PIN A6
#define QTR_LED_PIN 7
//--------------------------------//

// SPI Addresses -----------------//
#define NONE 0x00
//------ Data --------------------//
#define LINE_DATA 0x00
#define COLOR_DATA 0x01
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

extern HardwareSerial Serial;

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

uint8_t sensor_pins[8] = {A5, A4, A3, A2, A1, A0, 8, 9};
QTR_Controller qtr = QTR_Controller(sensor_pins, QTR_FRONT_PIN, QTR_LED_PIN);
SPISlaveInterface spi;

SoftwareSerial configSerial(CONFIG_RX, CONFIG_TX);

void setup()
{
    Serial.begin(115200);
    configSerial.begin(9600);

    eepromManager.init();

    pinMode(GREEN_SX, INPUT);
    pinMode(GREEN_DX, INPUT);

    qtr.init();
    spi.init();

    spi.setAction(CAL_LINE, []() {
        qtr.calibrate();
    });
    spi.setAction(CAL_WHITE, []() {
        cfg.calibrateWhite = true;
        writeConfiguration();
        cfg.calibrateWhite = false;
    });
    spi.setAction(CAL_GREEN, []() {
        cfg.calibrateGreen = true;
        writeConfiguration();
        cfg.calibrateGreen = false;
    });
    spi.setAction(CAL_BLACK, []() {
        cfg.calibrateBlack = true;
        writeConfiguration();
        cfg.calibrateBlack = false;
    });
    spi.setAction(CAL_ALUMI, []() {
        cfg.calibrateAluminium = true;
        writeConfiguration();
        cfg.calibrateAluminium = false;
    });
    spi.setAction(TOGGLE_LEFT_COLOR, []() {
        cfg.leftColorEnabled = !cfg.leftColorEnabled;
        writeConfiguration();
    });
    spi.setAction(TOGGLE_RIGHT_COLOR, []() {
        cfg.rightColorEnabled = !cfg.rightColorEnabled;
        writeConfiguration();
    });
    spi.setAction(TOGGLE_LIGHTS, []() {
        cfg.lightsEnabled = !cfg.lightsEnabled;
        writeConfiguration();
    });

    cfg.ready = true;
    writeConfiguration();
}

void loop()
{
    while (spi.pendingActions())
    {
        spi.execAction();
    }

    if (configSerial.available())
    {
        readConfiguration();
    }

    qtr.printValues();

    spi.setValue<double>(LINE_DATA, qtr.getLine());
    spi.setValue<byte>(COLOR_DATA, encodeColorData());
    spi.setValue<hsv>(LEFT_COLOR_DATA, cfg.leftColorData);
    spi.setValue<hsv>(RIGHT_COLOR_DATA, cfg.rightColorData);
}

byte encodeColorData()
{
    byte data = 0;
    data |= digitalRead(GREEN_SX) & 1;
    data |= (digitalRead(GREEN_DX) & 1) << 1;
    data |= (cfg.aluminium & 1) << 2;
    return data;
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

    Serial.print(F("\n\tlightsEnabled: "));
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

// SPI Interrupt -----------------//
ISR(SPI_STC_vect)
{
    spi.onTransmissionCompleted();
}