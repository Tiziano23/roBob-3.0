// MISO-MOSI Communication pins --//
//------ MEGA -------- NANO ------//
/*   50 -> MISO      12 -> MISO   */
/*   51 -> MOSI      11 -> MOSI   */
/*   52 -> SCK       13 -> SCK    */
/*   53 -> SS        10 -> SS     */
//--------------------------------//

#include <SPI.h>

#define NONE 0x00
#define SS_INT 2

class SPISlaveInterface
{
private:
	static const byte startAddressOffset = 1;
	static const byte actionBufferSize = 16;
	static const byte actionAddressOffset = startAddressOffset;
	static const byte dataBufferSize = 128;
	static const byte dataAddressOffset = startAddressOffset + actionBufferSize;

	volatile byte currentByte = 0;
	volatile void (*actionBuffer[actionBufferSize])();
	volatile byte dataBuffer[dataBufferSize][4];

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
		pinMode(SS_INT, INPUT);

		attachInterrupt(digitalPinToInterrupt(SS_INT), SPI_SS_FALLING, FALLING);
	}

	template <typename T>
	void setValue(byte address, T value)
	{
		union U {
			T value;
			byte bytes[sizeof(T)];
		} conversion;
		conversion.value = value;
		for (int i = 0; i < sizeof(T); i++)
		{
			dataBuffer[address][i] = conversion.bytes[i];
		}
	}
	byte getByte(byte address, byte byteIndex)
	{
		return dataBuffer[address - dataAddressOffset][byteIndex];
	}
	template <typename Function>
	void setAction(byte address, Function function)
	{
		actionBuffer[address] = function;
	}
	void execAction(byte address)
	{
		actionBuffer[address - actionAddressOffset]();
	}
	void onTransmissionCompleted()
	{
		byte address = SPDR;
		if (address == NONE)
		{
			SPDR = 0;
			currentByte = 0;
		} else {
			if (address < actionAddressOffset + actionBufferSize) execAction(address);
			else if (address < dataAddressOffset + dataBufferSize) {
				SPDR = getByte(address, currentByte);
				currentByte++;
			} else {
				SPDR = 0;
				currentByte = 0;
			}
		}
	}
};
