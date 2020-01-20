// Communication pins -------------//
#define GREEN_SX 2  //BIT_1
#define GREEN_DX 3  //BIT_2
#define ALUMINIUM 4 //BIT_3
#define CONFIG_RX 5 //BIT_4
#define CONFIG_TX 6 //BIT_5
//--------------------------------//

// QTR Pins ----------------------//
#define QTR_LED_PIN 7
//--------------------------------//

// SPI Addresses -----------------//
#define NONE 0x00
//------ Data --------------------//
#define LINE_DATA 0x00
#define COLOR_DATA 0x01
//------ Functions ---------------//
#define CAL_LINE 0x00
#define CAL_COLOR 0x01
#define CAL_COLOR_ABORT 0x02
#define TOGGLE_LEFT_COLOR 0x03
#define TOGGLE_RIGHT_COLOR 0x04
//--------------------------------//

#include <SoftwareSerial.h>

#include "line_sensor_manager.h"
#include "libraries/spi/spi_interface_slave.h"

extern HardwareSerial Serial;

struct config
{
    bool color_sx;
    bool color_dx;
} cfg;

const uint8_t sensor_pins[8] = {A5, A4, A3, A2, A1, A0, 8, 9};
QTR_Controller qtr(sensor_pins, QTR_LED_PIN);
SoftwareSerial configSerial(CONFIG_RX, CONFIG_TX);
SPISlaveInterface spi;

void setup()
{
    Serial.begin(115200);
    // configSerial.begin(115200);

    pinMode(GREEN_SX, INPUT);
    pinMode(GREEN_DX, INPUT);
    pinMode(ALUMINIUM, INPUT);

    pinMode(CONFIG_RX, INPUT);
    pinMode(CONFIG_TX, OUTPUT);

    qtr.init();
    spi.init();

    spi.setAction(CAL_LINE, []() {
        qtr.calibrate();
    });
    spi.setAction(CAL_COLOR, []() {
    });
    spi.setAction(CAL_COLOR_ABORT, []() {
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
}

void loop()
{
    while (spi.pendingActions() > 0)
        spi.execAction();

    spi.setValue<double>(LINE_DATA, qtr.getLine());
    spi.setValue<byte>(COLOR_DATA, encodeColorData());
}

byte encodeColorData()
{
    byte data;
    data |= (digitalRead(GREEN_SX) & B1);
    data |= (digitalRead(GREEN_DX) & B1) << 1;
    data |= (digitalRead(ALUMINIUM) & B1) << 2;
    return data;
}

// SPI Interrupt -----------------//
ISR(SPI_STC_vect)
{
    spi.onTransmissionCompleted();
}
