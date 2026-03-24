/**************************************************************************/
/*!
	@file     FRAM_MB85RC_I2C.cpp
	@author   SOSAndroid (E. Ha.)
	@license  BSD (see license.txt)

	Driver for the MB85RC I2C FRAM from Fujitsu.

	@section  HISTORY

	v1.0 - First release
	v1.0.1 - Robustness enhancement
	v1.0.2 - fix constructor, introducing byte move in memory
	v1.0.3 - fix writeLong() function
	v1.0.4 - fix constructor call error
	v1.0.5 - Enlarge density chip support by making check more flexible, Error codes not anymore hardcoded, add connect example, add Cypress FM24 & CY15B series comment.
	v1.1.0b - adding support for devices without device IDs + 4K & 16 K devices support
	v1.1.0b1 - Fixing checkDevice() + end of range memory map check
	v1.2.0 - Uses reinterpret_cast instead of bit shift / masking for performance. Breaks backward compatibility with previous code - See PR#6
	v1.2.1 - Fix comment line #76 (issue #11), max address define statement for 512K & 1M chips (issue 13), 0b000XXXXXXXX on <64kb device (issue #10)
	v1.3.0 - Fix access to las byte of memory map by @marmik18 - Commit 690a9ac
*/
/**************************************************************************/

/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *

Connection using I2C with Wire2:
SCL2 pin 24
SDA2 pin 25

*/

#include <stdlib.h>
#include "LillaFRAM_MB85RC_I2C.h"

/*========================================================================*/
/*                            CONSTRUCTOR                               */
/*========================================================================*/

LillaFRAM_MB85RC_I2C::LillaFRAM_MB85RC_I2C(void)
{
	_framInitialised = false;
	_manualMode = false;
	i2c_addr = MB85RC_DEFAULT_ADDRESS;
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

void LillaFRAM_MB85RC_I2C::begin(void)
{
	LillaFRAM_MB85RC_I2C::checkDevice();
}

byte LillaFRAM_MB85RC_I2C::checkDevice(void)
{
	byte result;

	result = getDeviceIDs();

	if ((result == ERROR_0) && ((manufacturer == FUJITSU_MANUFACT_ID) || (manufacturer == CYPRESS_MANUFACT_ID) || (manufacturer == MANUALMODE_MANUFACT_ID)) && (maxaddress != 0))
	{
		_framInitialised = true;
	}
	else
	{
		result = ERROR_7;
		_framInitialised = false;
	}
	return result;
}

/**************************************************************************/
/*!
	@brief  Writes an array of bytes from a specific address

	@params[in] i2cAddr
				The I2C address of the FRAM memory chip (1010+A2+A1+A0)
	@params[in] framAddr
				The 16-bit address to write to in FRAM memory
	@params[in] items
				The number of items to write from the array
	@params[in] values[]
				The array of bytes to write
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::writeArray(uint16_t framAddr, byte items, uint8_t values[])
{
	if ((framAddr > maxaddress) || ((framAddr + (uint16_t)items - 1) > maxaddress))
		return ERROR_11;

	LillaFRAM_MB85RC_I2C::I2CAddressAdapt(framAddr);
	for (byte i = 0; i < items; i++)
	{
		Wire2.write(values[i]);
	}
	return Wire2.endTransmission();
}

/**************************************************************************/
/*!
	@brief  Writes a single byte to a specific address

	@params[in] i2cAddr
				The I2C address of the FRAM memory chip (1010+A2+A1+A0)
	@params[in] framAddr
				The 16-bit address to write to in FRAM memory
	@params[in] value
				One byte to write
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/

byte LillaFRAM_MB85RC_I2C::writeByte(uint16_t framAddr, uint8_t value)
{
	uint8_t buffer[] = {value};
	return LillaFRAM_MB85RC_I2C::writeArray(framAddr, 1, buffer);
}

/**************************************************************************/
/*!
	@brief  Reads an array of bytes from the specified FRAM address

	@params[in] i2cAddr
				The I2C address of the FRAM memory chip (1010+A2+A1+A0)
	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] items
				number of items to read from memory chip
	@params[out] values[]
				array to be filled in by the memory read
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::readArray(uint16_t framAddr, byte items, uint8_t values[])
{
	if ((framAddr > maxaddress) || ((framAddr + (uint16_t)items - 1) > maxaddress))
		return ERROR_11;

	byte result;
	if (items == 0)
	{
		result = ERROR_8; // number of bytes asked to read null
	}
	else
	{
		LillaFRAM_MB85RC_I2C::I2CAddressAdapt(framAddr);
		result = Wire2.endTransmission();

		Wire2.requestFrom(i2c_addr, (uint8_t)items);
		for (byte i = 0; i < items; i++)
		{
			values[i] = Wire2.read();
		}
	}
	return result;
}

/**************************************************************************/
/*!
	@brief  Reads one byte from the specified FRAM address

	@params[in] i2cAddr
				The I2C address of the FRAM memory chip (1010+A2+A1+A0)
	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[out] *values
				data read from memory
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::readByte(uint16_t framAddr, uint8_t *value)
{
	uint8_t buffer[1];
	byte result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 1, buffer);
	*value = buffer[0];
	return result;
}
/**************************************************************************/
/*!
	@brief  Copy a byte from one address to another in the memory scope

	@params[in] i2cAddr
				The I2C address of the FRAM memory chip (1010+A2+A1+A0)
	@params[in] origAddr
				The 16-bit address to read from in FRAM memory
	@params[in] destAddr
				The 16-bit address to write in FRAM memory
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::copyByte(uint16_t origAddr, uint16_t destAddr)
{
	uint8_t buffer[1];
	byte result = LillaFRAM_MB85RC_I2C::readByte(origAddr, buffer);
	result = LillaFRAM_MB85RC_I2C::writeByte(destAddr, buffer[0]);
	return result;
}

/**************************************************************************/
/*!
	@brief  Reads one bit from the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] bitNb
				The bit position to read
	@params[out] *bit
				value of the bit: 0 | 1
	@returns
				return code of Wire2.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::readBit(uint16_t framAddr, uint8_t bitNb, byte *bit)
{
	byte result;
	if (bitNb > 7)
	{
		result = ERROR_9;
	}
	else
	{
		uint8_t buffer[1];
		result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 1, buffer);
		*bit = bitRead(buffer[0], bitNb);
	}
	return result;
}

/**************************************************************************/
/*!
	@brief  Set one bit to the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] bitNb
				The bit position to set
	@returns
				return code of Wire2.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::setOneBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7)
	{
		result = ERROR_9;
	}
	else
	{
		uint8_t buffer[1];
		result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 1, buffer);
		bitSet(buffer[0], bitNb);
		result = LillaFRAM_MB85RC_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
	@brief  Clear one bit to the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] bitNb
				The bit position to clear
	@returns
				return code of Wire2.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::clearOneBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7)
	{
		result = ERROR_9;
	}
	else
	{
		uint8_t buffer[1];
		result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 1, buffer);
		bitClear(buffer[0], bitNb);
		result = LillaFRAM_MB85RC_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
	@brief  Toggle one bit to the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] bitNb
				The bit position to toggle
	@returns
				return code of Wire2.endTransmission()
				return code 9 if bit position is larger than 7
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::toggleBit(uint16_t framAddr, uint8_t bitNb)
{
	byte result;
	if (bitNb > 7)
	{
		result = ERROR_9;
	}
	else
	{
		uint8_t buffer[1];
		result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 1, buffer);

		if ((buffer[0] & (1 << bitNb)) == (1 << bitNb))
		{
			bitClear(buffer[0], bitNb);
		}
		else
		{
			bitSet(buffer[0], bitNb);
		}
		result = LillaFRAM_MB85RC_I2C::writeArray(framAddr, 1, buffer);
	}
	return result;
}
/**************************************************************************/
/*!
	@brief  Reads a 16bits value from the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[out] value
				16bits word
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::readWord(uint16_t framAddr, uint16_t *value)
{
	uint8_t buffer[2];
	byte result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 2, buffer);
	*value = *reinterpret_cast<uint16_t *>(buffer);
	return result;
}

/**************************************************************************/
/*!
	@brief  Write a 16bits value from the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from in FRAM memory
	@params[in] value
				16bits word
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::writeWord(uint16_t framAddr, uint16_t value)
{
	uint8_t *buffer = reinterpret_cast<uint8_t *>(&value);
	return LillaFRAM_MB85RC_I2C::writeArray(framAddr, 2, buffer);
}
/**************************************************************************/
/*!
	@brief  Read a 32bits value from the specified FRAM address

	@params[in] framAddr
				The 16-bit address to read from FRAM memory
	@params[in] value
				32bits word
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::readLong(uint16_t framAddr, uint32_t *value)
{
	uint8_t buffer[4];
	byte result = LillaFRAM_MB85RC_I2C::readArray(framAddr, 4, buffer);
	*value = *reinterpret_cast<uint32_t *>(buffer);
	return result;
}
/**************************************************************************/
/*!
	@brief  Write a 32bits value to the specified FRAM address

	@params[in] framAddr
				The 16-bit address to write to FRAM memory
	@params[in] value
				32bits word
	@returns
				return code of Wire2.endTransmission()
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::writeLong(uint16_t framAddr, uint32_t value)
{
	uint8_t *buffer = reinterpret_cast<uint8_t *>(&value);
	return LillaFRAM_MB85RC_I2C::writeArray(framAddr, 4, buffer);
}
/**************************************************************************/
/*!
	@brief  Reads the Manufacturer ID and the Product ID frm the IC

	@params[in]   idtype
				  1: Manufacturer ID, 2: ProductID, 3:density code, 4:density
	@params[out]  *id
				  The 16 bits ID value
	@returns
				  0: success
				  1: error
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::getOneDeviceID(uint8_t idType, uint16_t *id)
{
	byte result;
	const uint8_t manuf = 1;
	const uint8_t prod = 2;
	const uint8_t densc = 3;
	const uint8_t densi = 4;

	switch (idType)
	{
	case manuf:
		*id = manufacturer;
		result = ERROR_0;
		break;
	case prod:
		*id = productid;
		result = ERROR_0;
		break;
	case densc:
		*id = densitycode;
		result = ERROR_0;
		break;
	case densi:
		*id = density;
		result = ERROR_0;
		break;
	default:
		*id = 0;
		result = ERROR_5;
		break;
	}
	return result;
}

boolean LillaFRAM_MB85RC_I2C::isReady(void)
{
	return _framInitialised;
}

byte LillaFRAM_MB85RC_I2C::eraseDevice(void)
{
	byte result = 0;
	uint16_t i = 0;
	while ((i < maxaddress) && (result == 0))
	{
		result = LillaFRAM_MB85RC_I2C::writeByte(i, 0x00);
		++i;
	}
	return result;
}

/*========================================================================*/
/*                           PRIVATE FUNCTIONS                            */
/*========================================================================*/

/**************************************************************************/
/*!
	Reads the Manufacturer ID and the Product ID from the IC and populate class' variables for devices supporting that feature

	@params[in]   none
	@params[out]  manufacturerID
				  The 12-bit manufacturer ID (Fujitsu = 0x00A)
	@params[out]  productID
				  The memory density (bytes 11..8) and proprietary
				  Product ID fields (bytes 7..0). Should be 0x510 for
				  the MB85RC256V for instance.
	@param[out]	  The memory densitycode (bytes 11..8)
				  from 0x03 (64K chip) to 0x07 (1M chip)
	@param[out]	  The memory density got from density code
				  from 64 to 1024K
	@param[out]	  The memory max address of storage slot
	@returns
				  return code of Wire2.endTransmission() or interpreted error.
*/
/**************************************************************************/
byte LillaFRAM_MB85RC_I2C::getDeviceIDs(void)
{
	uint8_t localbuffer[3] = {0, 0, 0};
	uint8_t result;

	/* Get device IDs sequence 	*/
	/* 1/ Send 0xF8 to the I2C bus as a write instruction. bit 0: 0 => 0xF8 >> 1 */
	/* Send 0xF8 to 12C bus. Bit shift to right as beginTransmission() requires a 7bit. beginTransmission() 0 for write => 0xF8 */
	/* Send device address as 8 bits. Bit shift to left as we are using a simple write()                                        */
	/* Send 0xF9 to I2C bus. By requesting 3 bytes to read, requestFrom() add a 1 bit at the end of a 7 bits address => 0xF9    */
	/* See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RC-DS501-00017-3v0-E.pdf             */

	Wire2.beginTransmission(MASTER_CODE >> 1);
	Wire2.write((byte)(i2c_addr << 1));
	result = Wire2.endTransmission(false);

	Wire2.requestFrom(MASTER_CODE >> 1, 3);
	localbuffer[0] = (uint8_t)Wire2.read();
	localbuffer[1] = (uint8_t)Wire2.read();
	localbuffer[2] = (uint8_t)Wire2.read();

	/* Shift values to separate IDs */
	manufacturer = (localbuffer[0] << 4) + (localbuffer[1] >> 4);
	densitycode = (uint16_t)(localbuffer[1] & 0x0F);
	productid = ((localbuffer[1] & 0x0F) << 8) + localbuffer[2];

	if (manufacturer == FUJITSU_MANUFACT_ID)
	{
		switch (densitycode)
		{
		case DENSITY_MB85RC04V:
			density = 4;
			maxaddress = MAXADDRESS_04;
			break;
		case DENSITY_MB85RC64TA:
			density = 64;
			maxaddress = MAXADDRESS_64;
			break;
		case DENSITY_MB85RC256V:
			density = 256;
			maxaddress = MAXADDRESS_256;
			break;
		case DENSITY_MB85RC512T:
			density = 512;
			maxaddress = MAXADDRESS_512;
			break;
		case DENSITY_MB85RC1MT:
			density = 1024;
			maxaddress = MAXADDRESS_1024;
			break;
		default:
			density = 0;	/* means error */
			maxaddress = 0; /* means error */
			if (result == 0)
				result = ERROR_7; /*device unidentified, comminication ok*/
			break;
		}
	}
	else if (manufacturer == CYPRESS_MANUFACT_ID)
	{
		switch (densitycode)
		{
		case DENSITY_CY15B128J:
			density = 128;
			maxaddress = MAXADDRESS_128;
			break;
		case DENSITY_CY15B256J:
			density = 256;
			maxaddress = MAXADDRESS_256;
			break;
		case DENSITY_FM24V05:
			density = 512;
			maxaddress = MAXADDRESS_512;
			break;
		case DENSITY_FM24V10:
			density = 1024;
			maxaddress = MAXADDRESS_1024;
			break;
		default:
			density = 0;	/* means error */
			maxaddress = 0; /* means error */
			if (result == 0)
				result = ERROR_7; /*device unidentified, comminication ok*/
			break;
		}
	}
	else
	{
		density = 0;	/* means error */
		maxaddress = 0; /* means error */
		if (result == 0)
			result = ERROR_7; /*device unidentified, comminication ok*/
	}

	return result;
}

/**************************************************************************/
/*!
	@brief 	Adapts the I2C calls (chip address + memory pointer) according to chip datasheet
			4K chips : 1 MSB of memory address as LSB of device address + 8 bits memory address
			16K chips : 3 MSB of memory address as LSB of device address + 8 bits memory address
			64K and more chips : full chipp address & 16 bits memory address


	@params[in]  address : memory address
	@param[out]	 none
	@returns	 void
*/
/**************************************************************************/
void LillaFRAM_MB85RC_I2C::I2CAddressAdapt(uint16_t framAddr)
{
	uint8_t chipaddress;

	switch (density)
	{
	case 4:
		// chipaddress = (i2c_addr | ((framAddr >> 8) & 0x1)); //Issue #10
		i2c_addr = ((i2c_addr & 0b11111110) | ((framAddr >> 8) & 0b00000001));
		break;
	case 16:
		// chipaddress = (i2c_addr | ((framAddr >> 8) & 0x7)); 	//Issue #10
		i2c_addr = ((i2c_addr & 0b11111000) | ((framAddr >> 8) & 0b00000111));
		break;
	default:
		chipaddress = i2c_addr;
		break;
	}

	if (density < 64)
	{
		Wire2.beginTransmission(i2c_addr);
		Wire2.write(framAddr & 0xFF);
	}
	else
	{
		Wire2.beginTransmission(i2c_addr);
		Wire2.write(framAddr >> 8);
		Wire2.write(framAddr & 0xFF);
	}
	return;
}
