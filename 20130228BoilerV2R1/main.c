/*				CTC BOILER
				AUTOMATIC CONTROLLER
            Programmer: Sajjad Hussain S.E.
            Crystal Frequency	=	11.0592 MHz
            Micocontroller		= 	AT89C51RC2
            New PCB DESIGN	ESDG-07-2k13
            Dated: 	17-01-2013
            O U T P U T
            ===========
            1.	Main Gas Valve
            2.	PILOT	Valve
            3.	BURNER Valve
            4.	BLOWER Motor
            5.	DAMPER	CW
            6.	DAMPER	CCW
            7.	IGNITER
            8.	SPARE BUZZER
            I N P U T
            ===========
            1.	START			default= open contact (pull-up)
            2.	STOP 			default= open contact (pull-up)
            3.	TEMPH			default= close contact (ground)
            4.	TEMPL			default= close contact (ground)
            5.	DAMPER0		default= open contact (pull-up)
            6.	DAMPER5		default= open contact (pull-up)
            7.	DAMPER100	default= open contact (pull-up)
            8.	FLOW			
            9.	MAINGAS		default= open contact (pull-up)*/
#include "at89c51rc2.h"
#include "boiler.h"
// Main Program Starts Here
void main()
{
	// Initilize Microcontroller
	P2 = 0x00;
	TMOD =0x21;// Timer 1= 8-bit Auto Reload, Timer 0= 16-bit
	CCON = 0x00;// Set PCA Module and PCA Interrupt for Stop Button
	CMOD = 0x00;
	CH = 0;
	CL = 0;
	//CCAPM0 = 0x11;	// Capture -ve Edge  for Stop Button
	CCON |= 0x40; //CR = 1;
   // Initial Screen
	DelayXms(100);

	LcdInit();
	LcdGotoXY(1,1);
	LcdWriteStr("  ESDG-PD-ICCC");
	LcdGotoXY(2,1);
	LcdWriteStr(" BOILER CONTROL");
	
	DelayXms(100);
	LcdGotoXY(1,1);
	LcdWriteStr(" BOILER CONTROL ");
	LcdGotoXY(2,1);
	LcdWriteStr("By Sajjad SE-PD ");
	
	DelayXms(100);
	LcdGotoXY(1,1);
	LcdWriteStr(" BOILER CONTROL ");
	LcdGotoXY(2,1);
	LcdWriteStr("Checking Default");

	// Setting default conditions
	DelayXms(100);
	LcdGotoXY(2,1);
	LcdWriteStr("Err: START=ON   ");
	while(iSTOP==1)
		oSPARE = 1;
	
	oSPARE = 0;
	DelayXms(50);
	LcdGotoXY(2,1);
	LcdWriteStr("Err: HI TEMP=ON ");
	while(iTEMPH==1)
		oSPARE = 1;

	
	oSPARE = 0;
	DelayXms(50);
	LcdGotoXY(2,1);
	LcdWriteStr("Err: LO TEMP=ON ");
	while(iTEMPL==1)
		oSPARE = 1;
	
	oSPARE = 0;
	DelayXms(50);
	LcdGotoXY(2,1);
	LcdWriteStr("Err: MG VALVE=ON");
	while(iMAINGAS==0)
		oSPARE = 1;
	
	EC = 1;
	EA = 1;
	
	LcdGotoXY(2,1);
	DelayXms(50);
	LcdWriteStr("   Damper =     ");
	LcdGotoXY(2,13);
	Putc(ReadDamper()+48);
	
	DelayXms(100);
	state = init_damper;
	nstate = state;

	// Default 
 	isInit = 1;
	//Running Forever
	while(isInit)
	{	
		switch(state)
		{
		 	case init_damper:
		 		if(ReadDamper() == 7) //off
	 		 		nstate = damper_is_off;
		 		else//4 6 0
		 		{
	 		 		oDAMPERCCW = 1;	// start motor ccw to bring it to off position
	 		 		nstate = wait_for_off;
		 		}
		 		state = idle_state;
		 		break;// outer case
		 	case wait_for_off:
		 		if(ReadDamper() == 7)
		 			nstate = damper_is_off;
		 		state = idle_state;
		 		break;
		 	case damper_is_off:
		 		oDAMPERCCW = 0;
		 		DelayXms(100);
		 		// start motor cw to bring it to just at 0%
		 		oDAMPERCW = 1;
				nstate = wait_for_damper0;
				state = idle_state;
				break;
			case wait_for_damper0:
				if (ReadDamper() == 6)
					nstate = damper_is_at_0;
				state = idle_state;
		 		break;
		 	case damper_is_at_0:
		 		oDAMPERCW = 0;
		 		nstate = wait_for_start;
		 		state = idle_state;
		 		LcdGotoXY(2,1);
				LcdWriteStr("Waiting to Start");
		 		break;
		 	case wait_for_start:
		 		if(iSTART == 0)
		 			nstate = set_damper_to_100;
		 		state = idle_state;
		 		break;
		 	case set_damper_to_100:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Purging.       ");
				CCAPM0 = 0x11;
		 		oDAMPERCW = 1;	// start motor cw, bring damper from 0 to 100%
				nstate = wait_for_damper100;
				state = idle_state;
				break;
			case wait_for_damper100:
		 		if (ReadDamper() == 0)	//position 100%		 		 	
		 		 		nstate = damper_is_at_100;
		 		 state = idle_state;
 		 		break;
		 	case damper_is_at_100:
		 		oDAMPERCW = 0;
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Purging..      ");//Blower is ON
		 		nstate = set_blower_on;
		 		state = idle_state;
		 		break;
		 	case set_blower_on:
		 		oBLOWER = 1;//for 45 Sec
				DelayXms(2000);
				nstate = set_blower_off;
				state = idle_state;
		 		break;
		 	case set_blower_off:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Purging...     ");//Blower is OFF
		 		oBLOWER = 0;//for 30 Sec
		 		DelayXms(2000);
				nstate = set_damper_to_5;
				state = idle_state;
		 		break;
		 	case set_damper_to_5:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Igniting       ");//Opening 5%
		 		oDAMPERCCW = 1;	// start motor ccw, to bring damper from 100 to 5%
				nstate = wait_for_damper5;
				state = idle_state;
				break;
			case wait_for_damper5:
		 		if (ReadDamper() == 6)	//position just below 5%
		 		{		 		 	
		 		 		nstate = damper_is_at_5;
		 		 		oDAMPERCCW = 0;// 4-11-2013
 		 		}
 		 		state = idle_state;
 		 		break;
		 	case damper_is_at_5:
		 		oDAMPERCCW = 0;
		 		TR2 = 0; ET2 = 0;
		 		nstate = set_blower_on2;
		 		state = idle_state;
		 		break;
		 	case set_blower_on2:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Igniting.      ");//Blower is ON
		 		oBLOWER = 1;
		 		nstate = main_gas_valve_on;
		 		state = idle_state;
		 		break;
		 	case main_gas_valve_on:
		 		oMGV = 1;
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Igniting..     ");//MGV is ON
				nstate = check_mgv;
				state = idle_state;
		 		break;
		 	case check_mgv:
		 		if(iMAINGAS==0)
		 			nstate = pilot_on;
		 		state = idle_state;
		 		break;
		 	case pilot_on:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Igniting...    ");//Pilot is ON
		 		oPILOT = 1;
		 		DelayXms(1000);
		 		nstate = igniter_on;
		 		state = idle_state;
		 		break;
		 	case igniter_on:
		 		oIGNITER = 1;
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Igniting....   ");//Igniter is ON
				DelayXms(500);
				nstate = set_damper_to_100_again;
				state = idle_state;
		 		break;
		 	case set_damper_to_100_again:
				oIGNITER = 0; //after 10sec
				oBURNER = 1;
		 		oDAMPERCW = 1;	// start motor cw, bring damper from 5 to 100%
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Burning...     ");//Opening 100%
				nstate = wait_for_damper100_again;
				state = idle_state;
				break;
			case wait_for_damper100_again:
		 		if (ReadDamper() == 0)	//position 100%		 		 	
		 		 		nstate = damper_is_at_100_again;
 		 		state = idle_state;
 		 		break;
		 	case damper_is_at_100_again:
		 		oDAMPERCW = 0;
		 		nstate = check_temp_high;
		 		state = idle_state;
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Running...     ");//Wait HIGH TEMP
		 		break;
		 	case check_temp_high:
		 		//if((iTEMPH == 1) || (iMAINGAS==1))
		 		if(iTEMPH == 1)
		 		{
					DelayXms(50);//500ms	to secure from glitch
					//if((iTEMPH == 1) || (iMAINGAS==1))//Re Check
					if(iTEMPH == 1)
						nstate = close_all;
		 		}
		 		state = idle_state;
		 		break;
		 	case close_all:
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Closing...     ");
		 		oMGV = 0;
		 		oPILOT = 0;
		 		oIGNITER = 0;
		 		oBLOWER = 0;
		 		oBURNER = 0;
		 		oDAMPERCW = 0;
		 		oSPARE = 0;
		 		oDAMPERCCW = 1;	// start motor ccw, to bring Damper from 100% to 0%
				nstate = wait_for_damper_off;
				state = idle_state;
				break;
			case wait_for_damper_off:
		 		if (ReadDamper() == 7)	//position off
		 		 		nstate = damper_at_off;
 		 		state = idle_state;
 		 		break;
		 	case damper_at_off:
		 		oDAMPERCCW = 0;
	 			nstate = check_temp_low;
	 			state = idle_state;
		 		LcdGotoXY(2,1);
				LcdWriteStr(" Closed!!!      ");//Wait LOW TEMP  
		 		break;
		 	case check_temp_low:
		 		if(iTEMPL == 0)
		 		{
		 			DelayXms(50);//500ms	to secure from glitch
		 			if(iTEMPL == 0)//Re Check
		 				nstate = damper_is_at_0;
		 		}
		 		state = idle_state;
		 		break;
		 	case idle_state:
				if(stopped)
		 		{
		 			state = close_all;
		 			stopped = 0;
		 		}
		 		else
		 			state = nstate;
		 		break;
		}//switch(state)
	}//while
}//main
// Interrupt Routines
void PCATimers() interrupt 6  using 1
{
	if(CCF0 & isInit)// Stop
	{
		CCF0 = 0;
		CCAPM0 = 0x00;
		stopped = 1;
		state = idle_state;// forcefully for perging and hi temp
	}
}





































