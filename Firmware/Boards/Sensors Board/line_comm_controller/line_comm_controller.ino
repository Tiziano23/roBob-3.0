#include <SoftwareSerial.h>

#include "math.h"
#include "line_sensor_manager.h"
#include "spi_interface_slave.h"

// Communication pins -------------//
#define GREEN_SX 0  //BIT_1
#define GREEN_DX 3  //BIT_2
#define ALUMINIUM 4 //BIT_3
#define CONFIG_RX 5 //BIT_4
#define CONFIG_TX 6 //BIT_5
//--------------------------------//

// Pins --------------------------//
#define QTR_LED_PIN 7
//--------------------------------//

// Addresses ---------------------//
//------ Data --------------------//
#define LINE 0x00
#define COLOR 0x01
//------ Functions ---------------//
#define CAL_IR 0x00
#define TOGGLE_LEFT_COLOR 0x01
#define TOGGLE_RIGHT_COLOR 0x02
#define CAL_COLOR 0x03
#define CAL_COLOR_ABORT 0x04
//--------------------------------//
extern HardwareSerial Serial;

struct config
{
    bool color_sx;
    bool color_dx;
} cfg;

int sensor_pins[8] = {A5, A4, A3, A2, A1, A0, 8, 9};

SoftwareSerial configSerial(CONFIG_RX, CONFIG_TX);
QTR_Controller qtr(sensor_pins, QTR_LED_PIN);
SPISlaveInterface spi;

void setup()
{
    Serial.begin(115200);
    configSerial.begin(115200);

    pinMode(GREEN_SX, INPUT);
    pinMode(GREEN_DX, INPUT);
    pinMode(ALUMINIUM, INPUT);

    pinMode(CONFIG_RX, INPUT);
    pinMode(CONFIG_TX, OUTPUT);

    qtr.init();
    spi.init();

    spi.setAction(CAL_IR, []() {
        qtr.calibrate();
    });
    spi.setAction(TOGGLE_LEFT_COLOR, []() {
        cfg.color_sx = !cfg.color_sx;
        configSerial.println(
            String("{\"sx\":") + String(cfg.color_sx) + String(",") +
            String("\"dx\":") + String(cfg.color_dx) +
            String("}"));
    });
    spi.setAction(TOGGLE_RIGHT_COLOR, []() {
        cfg.color_dx = !cfg.color_dx;
        configSerial.println(
            String("{\"sx\":") + String(cfg.color_sx) + String(",") +
            String("\"dx\":") + String(cfg.color_dx) +
            String("}"));
    });
    spi.setAction(CAL_COLOR, []() {
    });
    spi.setAction(CAL_COLOR_ABORT, []() {
    });
}

void loop()
{
    spi.setValue(LINE, (float)qtr.getLine());
    spi.setValue(COLOR, decodeColorData());
}

byte decodeColorData()
{
    byte data;
    data |= (digitalRead(GREEN_SX) & 1);
    data |= (digitalRead(GREEN_DX) & 1) << 1;
    data |= (digitalRead(ALUMINIUM) & 1) << 2;
    return data;
}

//-- SPI Interrupt --//
ISR(SPI_STC_vect)
{
    spi.onTransmissionCompleted();
}
