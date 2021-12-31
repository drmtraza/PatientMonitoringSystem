/*--------------------------------------------------------------------
This file is part of the Arduino WiFiEsp library.

The Arduino WiFiEsp library is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

The Arduino WiFiEsp library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with The Arduino WiFiEsp library.  If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------*/

#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>
#include "WiFiEspUdp.h"
#include "WiFiEspNTPClient.h"




#define NTP_PACKET_SIZE 48




/* Constructor */
WifiNTPClient::WifiNTPClient()
{
  m_nDOM = m_nMonth = m_nHour = m_nMinute = m_nSecond = m_nDOW = 0;
  m_nYear = m_nDOY = 0;
}

void WifiNTPClient::sendNTPRequest(const char *strNTPServer, uint8_t* arrayBuffer, const uint8_t nBuffSize)
{
  Serial.print(F("Trying '"));
  Serial.print(strNTPServer);
  Serial.print(F("'..."));
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)

  arrayBuffer[0] = 0b11100011;   // LI, Version, Mode
  arrayBuffer[1] = 0;     		 // Stratum, or type of clock
  arrayBuffer[2] = 6;     		 // Polling Interval
  arrayBuffer[3] = 0xEC;  		 // Peer Clock Precision

  // 8 bytes of zero for Root Delay & Root Dispersion
  arrayBuffer[12]  = 49;
  arrayBuffer[13]  = 0x4E;
  arrayBuffer[14]  = 49;
  arrayBuffer[15]  = 52;
    
  // All NTP fields have been given values, now you can send a packet requesting a timestamp:
  beginPacket(strNTPServer, 123); //NTP requests are to port 123
  write(arrayBuffer, nBuffSize);
  endPacket();
}

bool WifiNTPClient::isLeapYear(const uint16_t nYear)
{
  return ((nYear % 4) == 0) || (((nYear % 100) == 0) && ((nYear % 400) == 0));
}

uint8_t WifiNTPClient::calcDOW()
{
  static int arrayTable[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
   
  uint16_t nYear = m_nYear - m_nMonth < 3;

  return ((nYear + nYear / 4 - nYear / 100 + nYear / 400 + arrayTable[m_nMonth - 1] + m_nDOM) % 7);
}

uint8_t WifiNTPClient::getMonthDays(const uint16_t nMonth, const bool bLeapYear)
{
  uint8_t nMaxDays = 0;

  switch (nMonth)
  {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12: nMaxDays = 31;
             break;
    case 4:
    case 6:
    case 9:
    case 11: nMaxDays = 30;
             break;
	case 2: 
            if (bLeapYear)
              nMaxDays = 29;
            else
              nMaxDays = 28;
  }
  return nMaxDays;
}

void WifiNTPClient::CopyString(char *strDest, const __FlashStringHelper *strSrc)
{
	memset(strDest, 0, sizeof strDest);
	strcpy_P(strDest, (PGM_P)strSrc);
}

bool WifiNTPClient::getUTCTime()
{
  char strNTPServer[25];
  bool bResult = false;
  
  CopyString(strNTPServer, F("0.uk.pool.ntp.org"));
  if (!(bResult = getUTCTime(strNTPServer)))
  {
	CopyString(strNTPServer, F("1.uk.pool.ntp.org"));
	  if (!(bResult = getUTCTime(strNTPServer)))
	  {
		  CopyString(strNTPServer, F("0.us.pool.ntp.org"));
		  if (!(bResult = getUTCTime(strNTPServer)))
		  {
			  CopyString(strNTPServer, F("1.us.pool.ntp.org"));
			  if (!(bResult = getUTCTime(strNTPServer)))
			  {
				  CopyString(strNTPServer, F("2.us.pool.ntp.org"));
				  if (!(bResult = getUTCTime(strNTPServer)))
				  {
					  CopyString(strNTPServer, F("3.us.pool.ntp.org"));
					  if (!(bResult = getUTCTime(strNTPServer)))
					  {
						  CopyString(strNTPServer, F("0.au.pool.ntp.org"));
						  if (!(bResult = getUTCTime(strNTPServer)))
						  {
							  CopyString(strNTPServer, F("1.au.pool.ntp.org"));
							  if (!(bResult = getUTCTime(strNTPServer)))
							  {
								  CopyString(strNTPServer, F("2.au.pool.ntp.org"));
								  if (!(bResult = getUTCTime(strNTPServer)))
								  {
									  CopyString(strNTPServer, F("3.au.pool.ntp.org"));
									  if (!(bResult = getUTCTime(strNTPServer)))
									  {
										Serial.println(F("FAILED!"));
										flush();
									  } 
								  } 
							  } 
						  } 
					  } 
				  } 
			  } 
		  } 
	  } 
  }
  return bResult;
}
 

bool WifiNTPClient::getUTCTime(const char* strNTPServer)
{
  bool bResult = false;
  uint16_t nTimeout = millis() + 5000;
  uint32_t nStartMillis = millis();
  uint8_t arrayBuffer[NTP_PACKET_SIZE];

  // Set all bytes in the buffer to 0
  memset(arrayBuffer, 0, NTP_PACKET_SIZE);
  sendNTPRequest(strNTPServer, arrayBuffer, NTP_PACKET_SIZE);
  
  while (!available() && (((int32_t)millis() - nTimeout) < 0));
  if (parsePacket()) 
  {
    read(arrayBuffer, NTP_PACKET_SIZE);

    uint32_t nHighWord = word(arrayBuffer[32], arrayBuffer[33]),
			 nLowWord = word(arrayBuffer[34], arrayBuffer[35]),
			 nSecsSince1900 = 0, nEpoch = 0;

    // Combine the four bytes (two words) into a long integer this is NTP time (seconds since Jan 1 1900):
    nSecsSince1900 = nHighWord << 16 | nLowWord;

    // Now convert NTP time into every day or Unix time
    // Unix time starts on Jan 1 1970 - in seconds, that's 2208988800
    const uint32_t nSeventyYears = 2208988800UL;

    // Subtract seventy m_nYears:
    nEpoch = nSecsSince1900 - nSeventyYears;

	m_nDOW = weekday(nEpoch);
	m_nDOM = day(nEpoch);
	m_nMonth = month(nEpoch);
	m_nYear = year(nEpoch);
	m_nHour = (uint8_t)((nEpoch  % 86400L/*60 * 60 * 24*/) / 3600);
	m_nMinute = (uint8_t)((nEpoch % 3600/*60 * 60*/) / 60);
	m_nSecond = (uint8_t)(nEpoch % 60);
	
	bResult = true;
	Serial.print(F(" server responded on port "));
	Serial.print(remotePort());
	Serial.println(F("!"));
  }
  else
  {
    Serial.println(F("no response from server!"));
  }
  return bResult;
}
