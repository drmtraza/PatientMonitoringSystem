#ifndef WiFiEspNTPClient_h
#define WiFiEspNTPClient_h




#include <WiFiEspUdp.h>




class WifiNTPClient : public WiFiEspUDP
{
	private:
		uint8_t m_nDOM, m_nMonth, m_nHour, m_nMinute, m_nSecond, m_nDOW;
		uint16_t m_nYear, m_nDOY;


		// Send the request to the specified NTP server
		void sendNTPRequest(const char *strNTPServer, uint8_t* arrayBuffer, const uint8_t nBuffSize);
		uint8_t getMonthDays(const uint16_t nMonth, const bool bLeapYear);
		bool isLeapYear(const uint16_t nYear);
		uint8_t calcDOW();
		void CopyString(char *strDest, const __FlashStringHelper *strSrc);

	public:
		WifiNTPClient();  // Constructor

		uint8_t begin()
		{
			return WiFiEspUDP::begin(2390);
		}

		// Get current UTC time
		bool getUTCTime(const char* strNTPServer);
		bool getUTCTime();

		uint16_t getDOW()
		{
			return m_nDOW;
		};

		uint16_t getDOY()
		{
			return m_nDOY;
		};

		uint8_t getDOM()
		{
			return m_nDOM;
		};

		uint8_t getMonth()
		{
			return m_nMonth;
		};

		uint16_t getYear()
		{
			return m_nYear;
		};

		uint8_t getHour()
		{
			return m_nHour;
		};

		uint8_t getMinute()
		{
			return m_nMinute;
		};

		uint8_t getSecond()
		{
			return m_nSecond;
		};

};

#endif
