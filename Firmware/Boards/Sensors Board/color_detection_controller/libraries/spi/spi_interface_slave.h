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
	static const uint8_t dataBufferSize = 128;
	static const uint8_t dataAddressOffset = startAddressOffset + actionBufferSize;

	Queue<uint8_t> actionAddressQueue;
	volatile uint8_t currentByte = 0;
	volatile void (*actionBuffer[actionBufferSize])();
	volatile uint8_t dataBuffer[dataBufferSize][4];

	static void SPI_SS_FALLING()
	{
	}

public:
	void init()
	{
		SPCR |= bit(SPE);
		SPCR |= bit(SPIE);

		pinMode(SS, INPUT);
		pinMode(MOSI, INPUT);
		pinMode(MISO, OUTPUT);

		// pinMode(SS_INT, INPUT);
		// attachInterrupt(digitalPinToInterrupt(SS_INT), SPI_SS_FALLING, FALLING);
	}

	template <typename T>
	void setValue(uint8_t address, T value)
	{
		union U {
			T value;
			uint8_t bytes[sizeof(T)];
		} conversion;
		conversion.value = value;
		for (int i = 0; i < sizeof(T); i++)
		{
			dataBuffer[address][i] = conversion.bytes[i];
		}
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
			{
				SPDR = getByte(address, currentByte);
				currentByte++;
			}
			else
			{
				SPDR = 0;
				currentByte = 0;
			}
		}
	}
};