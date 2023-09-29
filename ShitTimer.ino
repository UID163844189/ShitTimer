// 井inculde 《SP1。h》
#include <SPI.h>
#include <SoftwareSerial.h>

// #define Key1 PA13 // run
// #define Key2 PA14 // trans
// #define Key3 PB2  // set
// #define Key4 PA0
#define runKey PA13	  // run
#define transKey PA14 // trans
#define setKey PB2	  // set
const int slaveSelect = PA4;
const int scanLimit = 7;
int hours, minutes, seconds;
//SoftwareSerial ssr(/*rx =*/PA1, /*tx =*/PA0);
HardwareSerial Serial2(PA1, PA0);

int secEta = 0; // 手动设置的倒计时时长

void mp3Command(byte command, unsigned short data);
void setup()
{
	Serial.begin(115200);
	Serial2.begin(9600);
	mp3Command(0x09, 1);
	pinMode(runKey, INPUT);
	pinMode(transKey, INPUT);
	pinMode(setKey, INPUT);
	// pinMode(Key4, INPUT);

	SPI.begin();
	pinMode(slaveSelect, OUTPUT);
	// pinMode(beep, OUTPUT);
	digitalWrite(slaveSelect, LOW);
	sendCommand(12, 1);			// Shutdown,open
	sendCommand(15, 0);			// DisplayTest,no
	sendCommand(10, 15);		// Intensity,15(max)
	sendCommand(11, scanLimit); // ScanLimit,8-1=7
	sendCommand(9, 255);		// DecodeMode,Code B decode for digits 7-0
	digitalWrite(slaveSelect, HIGH);
	initdisplay();
	Serial.println("LED Ready");
}
void loop()
{
	if (!digitalRead(setKey))
	{
		Serial.println("set time");
		adjustTime();
	}

	// delay(50);
	if (!digitalRead(runKey))
	{
		delay(250);
		while (secEta > 0)
		{
			secEta--;
			displayTime(secEta);
			if (!digitalRead(runKey))
			{
				delay(250);
				break;
			}
			if (secEta == 0)
			{
				alarm();
			}
			delay(1000);
		}
	}
}
void alarm()
{
	mp3Command(0x03, 1);
}
void sendCommand(int command, int value)
{
	digitalWrite(slaveSelect, LOW);
	SPI.transfer(command);
	SPI.transfer(value);
	digitalWrite(slaveSelect, HIGH);
}
void initdisplay()
{
	sendCommand(8, 0xa);
	sendCommand(7, 0xa);
	sendCommand(6, 0xa);
	sendCommand(5, 0xa);
	sendCommand(4, 0xa);
	sendCommand(3, 0xa);
	sendCommand(2, 0xa);
	sendCommand(1, 0xa);
}

void mp3Command(byte command, unsigned short data = 0x0000) // 控制mp3芯片的函数
{
	unsigned int CheckTemp = 0;
	delay(10);
	byte buffer[10];
	buffer[0] = 0x7e;
	buffer[1] = 0xff;
	buffer[2] = 0x06;
	buffer[3] = command;
	buffer[4] = 0x00;
	buffer[5] = data >> 8;
	buffer[6] = data;

	for (int i = 1; i < 7; i++)
		CheckTemp += buffer[i];
	CheckTemp = 0 - CheckTemp;
	buffer[7] = CheckTemp >> 8;
	buffer[8] = CheckTemp;
	buffer[9] = 0xef;

	for (int i = 0; i < 10; i++)
	Serial2.write(buffer[i]);
}

void adjustTime() // 设定时间函数
{
	delay(200);
	secEta = 0;
	int temp = 0;

	int k = 8;					// 从右到左显示屏上的第几位
	for (int j = 6; j > 0; j--) // 从右到左显示屏上的第几位数字
	{
		Serial.print("It's adjusting ");
		Serial.println(j);
		for (;;) // 此处针对每一位进行操作，按下确认后跳转到 confirm
		{
			sendCommand(k, 0xf); // 关闭第一位
			for (int i = 0; i < 10; i++)
			{
				if (!digitalRead(transKey))
				{
					temp++;
					if (temp >= 10) // 防止溢出
						temp = 0;
					Serial.print("set to");
					Serial.println(temp);
					delay(200);
					goto display;
				}
				if (!digitalRead(setKey))
					goto confirm;
				delay(50);
			}
		display:
			sendCommand(k, temp); // 显示第一位
			for (int i = 0; i < 10; i++)
			{
				if (!digitalRead(transKey))
				{
					temp++;
					if (temp >= 10) // 防止溢出
						temp = 0;
					Serial.print("set to");
					Serial.println(temp);
					delay(200);
					goto display;
				}
				if (!digitalRead(setKey))
					goto confirm;
				delay(50);
			}
		}
	confirm:
		sendCommand(k, temp); // 显示第一位

		switch (j)
		{
		case 6:
			secEta += 36000 * temp;
			k = 7;
			break;
		case 5:
			secEta += 3600 * temp;
			k = 5;
			break;
		case 4:
			secEta += 600 * temp;
			k = 4;
			break;
		case 3:
			secEta += 60 * temp;
			k = 2;
			break;
		case 2:
			secEta += 10 * temp;
			k = 1;
			break;
		case 1:
			secEta += 1 * temp;
			break;
		default:
			break;
		}
		delay(250);
	}

	displayTime(secEta);
}

void displayTime(int input) // 将秒数显示在屏幕上
{
	secToHms(input);
	sendCommand(8, hours / 10);
	sendCommand(7, hours % 10);
	sendCommand(6, 0xa);
	sendCommand(5, minutes / 10);
	sendCommand(4, minutes % 10);
	sendCommand(3, 0xa);
	sendCommand(2, seconds / 10);
	sendCommand(1, seconds % 10);

	// Serial.print(hours);
	// Serial.print(":");
	// Serial.print(minutes);
	// Serial.print(":");
	// Serial.println(seconds);

	Serial.print("Scrn should display: ");
	Serial.print(hours / 10);
	Serial.print(hours % 10);
	Serial.print("-");
	Serial.print(minutes / 10);
	Serial.print(minutes % 10);
	Serial.print("-");
	Serial.print(seconds / 10);
	Serial.print(seconds % 10);
	Serial.println();
}

void secToHms(int input) // 将秒数转换为时分秒并存储在 hours minutes seconds 里
{
	// 计算小时数、分钟数和剩余的秒数
	hours = input / 3600;
	minutes = (input % 3600) / 60;
	seconds = input % 60;
	// 输出结果
	Serial.print("secToHms: ");
	Serial.print(input);
	Serial.print("s = ");
	Serial.print(hours);
	Serial.print(":");
	Serial.print(minutes);
	Serial.print(":");
	Serial.print(seconds);
	Serial.println();

	// return 0;
}