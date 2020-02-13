// MISO-MOSI Communication pins --//
//------ MEGA -------- NANO ------//
/*   50 -> MISO      12 -> MISO   */
/*   51 -> MOSI      11 -> MOSI   */
/*   52 -> SCK       13 -> SCK    */
/*   53 -> SS        10 -> SS     */
//--------------------------------//
#include <SPI.h>
#include "../array.h"

#define NONE 0x00
#define SS_INT 2

class SPISlaveInterface
{
private:
	static const uint8_t startAddressOffset = 1;
	static const uint8_t actionBufferSize = 16;
	static const uint8_t actionAddressOffset = startAddressOffset;
	static const uint8_t dataBufferSize = 32;
	static const uint8_t dataAddressOffset = startAddressOffset + actionBufferSize;
	static const uint8_t dataSize = 16;

	Queue<uint8_t> actionAddressQueue;
	volatile uint8_t currentByte = 0;
	volatile void (*actionBuffer[actionBufferSize])();
	volatile uint8_t dataBuffer[dataBufferSize][dataSize];

	static void SPI_SS_FALLING()
	{
	}

public:
	void init()
	{
		SPCR |= _BV(SPE);
		SPCR |= _BV(SPIE);

		pinMode(SS, INPUT);
		pinMode(SCK, INPUT);
		pinMode(MOSI, INPUT);
		pinMode(MISO, OUTPUT);

		// pinMode(SS_INT, INPUT);
		// attachInterrupt(digitalPinToInterrupt(SS_INT), SPI_SS_FALLING, FALLING);
	}

	template <typename T>
	void setValue(uint8_t address, T value)
	{
		if (sizeof(T) <= dataSize)
			for (int i = 0; i < sizeof(T); i++)
				dataBuffer[address][i] = ((uint8_t *)&value)[i];
	}
	uint8_t getByte(uint8_t address, uint8_t byteIndex)
	{
		return dataBuffer[address - dataAddressOffset][byteIndex];
	}
	template <typename Function>
	void setAction(uint8_t address, Function function)
	{
		actionBuffer[address] = function;
	}
	void registerActionExecution(uint8_t address)
	{
		actionAddressQueue.put(address - actionAddressOffset);
	}
	int pendingActions()
	{
		return actionAddressQueue.size();
	}
	void execAction()
	{
		actionBuffer[actionAddressQueue.get()]();
	}
	void onTransmissionCompleted()
	{
		uint8_t address = SPDR;
		if (address == NONE)
		{
			SPDR = 0;
			currentByte = 0;
		}
		else
		{
			if (address < actionAddressOffset + actionBufferSize)
				registerActionExecution(address);
			else if (address < dataAddressOffset + dataBufferSize)
				SPDR = getByte(address, currentByte++);
			else
			{
				SPDR = 0;
				currentByte = 0;
			}
		}
	}
};