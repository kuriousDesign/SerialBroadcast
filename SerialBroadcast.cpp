/*
  SerialBroadcast.cpp - Library for board to board serial transmissions code
  Created by Jake Gardner, November 23rd, 2018
  Released into the public domain.
*/

#include "Arduino.h"
#include "State.h"
#include "Executor.h"
#include "SerialBroadcast.h"
#include "Timer.h"

  const int BUFF_SIZE=11;
  String stringBuffer[BUFF_SIZE];


//Constructor
SerialBroadcast::SerialBroadcast(HardwareSerial* ptrSerial, long baud, bool ShowDiag)
{
  inSerial = ptrSerial;
  inSerial->begin(baud);
  showDiag=ShowDiag;
  queueIndex=0;
  sendIndex=0;
  isError=0;
  receiptFlag=0;
  codeFlag=0;
  sendNextFlag=1;
  timerSend.reset();
}


//listenAndUpdate Method
void SerialBroadcast::listenAndUpdate(State states[], int numOfStates)
{

  while (inSerial->available())// & !codeFlag & !receiptFlag)
  {
     inChar=inSerial->read();
     if(inChar=='\r')
	   {
        if(inString.substring(0,2)=="tx" & inString.length()==CODE_LENGTH){
           codeFlag=1;
		 }
		else if (inString.substring(0,2)=="rx" & inString.length()==RECEIPT_LENGTH){
		   	receiptFlag=1;
		}
		else {
        inString="";
        }
      }
	else if (inChar!='\n'){
       inString+=inChar;
	 }
   


	//Incoming Code Detected
    if(codeFlag)
	{
	  if(showDiag){
	  	Serial.println("Incoming code detected:  " + inString);
	  }
	  inString=inString.substring(2);
      locationString=inString.substring(0,4);
	  if (locationString.charAt(3)=='_'){
	  	locationString=locationString.substring(0,3);
	  }
	  //Serial.println("location String: " + locationString);
	   stepString=inString.substring(4);
	   //Serial.println("Step String: " + stepString);
	   //Serial.println("Size String: " + (String)sizeof(states));
	  for (int i=0;i<numOfStates;i++ ){
	  //Serial.println(locationString + " = " + states[i].handle);
	  	if(states[i].handle==locationString){
			states[i].update(stepString.toInt());
			//addToQueue("rx" + inString.substring(0,4));
			inSerial->println("rx" + inString.substring(0,4));
			//Serial.println(locationString +"state has been updated to: " + (String)states[i].isStepNumber);
			break;
		}
	  }
	 
      inString="";
      codeFlag=0;
    }
	
	
	
	//Incoming Reply from Outgoing Command Detected: reset send flag
	if(receiptFlag){
		if(showDiag){
			Serial.println("Receipt code detected:  " + inString);
		}
		if(inString.substring(2)== sentString.substring(2,6)){
			sendNextFlag=1;
			timerSend.reset();
			sendIndex++;
			resendFlag=0;
			if(sendIndex>=BUFF_SIZE){
	        		sendIndex=0;
			}
		}
		else{
			resendFlag=1;
			Serial.println("mismatch: " + inString);
		}
	  inString="";
      receiptFlag=0;
	}
    
	}//while
}//END listenAndUpdate



//checkAndSend Method
void SerialBroadcast::checkAndSend(Executor executor)
{
	
    if (executor.isFirstScanOfStep)
	{
			String codeString="____"; //initialize code string
			codeString.setCharAt(0,executor.handle.charAt(0));
			codeString.setCharAt(1,executor.handle.charAt(1));
			codeString.setCharAt(2,executor.handle.charAt(2));
			if(executor.handle.length()>3)
			{
					codeString.setCharAt(3,executor.handle.charAt(3));
			}
			
			//Serial.println("codeString: " + codeString);
			String stepString=(String)executor.isStepNumber;
			int L=stepString.length();

			  switch (L) {

	           default:
			   		isError=1;
				break;
				
				case 1:
					codeString+="000"+stepString;
				break;
				
				case 2:
					codeString+="00"+stepString;
				break;
				
				case 3:
					codeString+="0"+stepString;
				break;
				
				case 4:
					codeString+=stepString;
				break;
				
			}
		addToQueue("tx" + codeString);
		//Serial.println("code string is: "+ codeString);
		
	}
	
send();

}//checkAndSend



//addToQueue Method
void SerialBroadcast::addToQueue(String queueString)
{
	stringBuffer[queueIndex]=queueString;
	if(showDiag){
		Serial.println("queue index is: " + (String)queueIndex);
	}
    queueIndex++;
	if(queueIndex>=BUFF_SIZE){
		queueIndex=0;
	}
}//addToQueue



//setSendNextFlag Method
void SerialBroadcast::setSendNextFlag()
{
	sendNextFlag=1;
	sendIndex++;
	resendFlag=0;
}//



//Send Method
void SerialBroadcast::send()
{
	if (sendNextFlag){
  		if(stringBuffer[sendIndex]!=""){
   			sentString=stringBuffer[sendIndex];
   			inSerial->println(sentString);
   			stringBuffer[sendIndex]="";
   			//sendIndex++;

   			sendNextFlag=0;
			timerSend.reset();
			timerSend.set(1000);
			if(showDiag){
				Serial.println("send index is: " + (String)sendIndex);
			}
		}
  	}
	else{
	   timerSend.call();
		if(timerSend.isDone){
			resendFlag=1;
		}
	}
  
   if(resendFlag){
   	 inSerial->println(sentString);
	 Serial.println("resent" );
	 resendFlag=0;
	 timerSend.reset();
	 timerSend.set(2000);
    }
 }
   
 
