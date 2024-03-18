/*
  SerialBroadcast.h - Library for board to board serial transmission code
  Created by Jake Gardner, November, 11th 2018
  Released into the public domain.
*/

#ifndef SerialBroadcast_h
#define SerialBroadcast_h

#include "Arduino.h"
#include "State.h"
#include "Executor.h"
#include "Timer.h"

class SerialBroadcast
{
  public:
    SerialBroadcast(HardwareSerial* ptrSerial, long baud, bool ShowDiag);
    void listenAndUpdate(State states[], int numOfStates); //
	void checkAndSend(Executor executor); //
	void setSendNextFlag();
    bool isError; //state bits
	
  protected:
    HardwareSerial* inSerial;
	Timer timerSend;
	bool showDiag;
	void send();
	void addToQueue(String queueString);
    char inChar;
	String inString,locationString,stepString,sentString;
    int queueIndex, sendIndex;
	bool codeFlag,receiptFlag,sendNextFlag,resendFlag;
	const int CODE_LENGTH=10,RECEIPT_LENGTH=6;
};

#endif