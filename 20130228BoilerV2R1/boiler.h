#define Putc LcdWriteChar
#define  LED_ON			1
#define  LED_OFF		0

#define  init_damper							1
#define  damper_is_off						2
#define  wait_for_damper0					3
#define  damper_is_at_0						4
#define  wait_for_start						5
#define  set_damper_to_100					6
#define  wait_for_damper100				7
#define  damper_is_at_100					8
#define  set_blower_on						9
#define  set_blower_off						10
#define  set_damper_to_5					11
#define  wait_for_damper5					12
#define  damper_is_at_5						13
#define  set_blower_on2						14
#define  main_gas_valve_on					15
#define  check_mgv							16
#define  pilot_on								17
#define  igniter_on							18
#define  set_damper_to_100_again			19
#define  wait_for_damper100_again		20
#define  damper_is_at_100_again			21
#define  check_temp_high					22
#define  close_all							23
#define  wait_for_damper_off				24
#define  damper_at_off						25
#define  check_temp_low						26
#define  idle_state							27
#define  wait_for_off						28

sbit BS  = P0^7;			

sbit RS = P1^0;
sbit RW = P1^1;
sbit EN =	P1^2;
sbit iSTOP = P1^3;//Input, Feedback
sbit iSTART = P1^4;//Input, Feedback
sbit iTEMPH = P1^5;//Input, Feedback
sbit iTEMPL = P1^6;//Input, Feedback


sbit oIGNITER = P2^7;//Output, Relay
sbit oBLOWER = P2^6;//Output, Relay
sbit oSPARE = P2^5;//Output, Relay
sbit oMGV = P2^4;//Output, Relay
sbit oBURNER = P2^3;//Output, Relay
sbit oPILOT = P2^2;//Output, Relay
sbit oDAMPERCCW = P2^1;//Output, Relay
sbit oDAMPERCW = P2^0;//Output, Relay

sbit iRX = P3^0;
sbit oTX = P3^1;
sbit iDAMPER5 = P3^2;//Input, Feedback
sbit iDAMPER0 = P3^3;//Input, Feedback
sbit iDAMPER100 = P3^4;//Input, Feedback
sbit iMAINGAS = P3^5;//Input, Feedback




bit isInit=0;
bit stopped;
int state = 100;
int nstate;

//Function Prototypes
void LcdInit();
void DelayUs(void);
void Delay10ms(void);
void LcdClear(void);
void LcdWriteStr(char *var);
void LcdWriteCmd(unsigned char c);
void LcdWriteChar(unsigned char var);
void LcdGotoXY(unsigned char r,unsigned char c);
unsigned char ReadDamper(void)
{
	bit bt;
	unsigned char dmp;
	unsigned char temp;
	
	bt = iDAMPER100;
	temp = (unsigned char)bt;
	dmp = temp;
	
	bt = iDAMPER5;
	temp = (unsigned char)bt;
	dmp = (dmp<<1) + temp;
	
	bt = iDAMPER0;
	temp = (unsigned char)bt;
	dmp = (dmp<<1) + temp;
	
	return(dmp);
} 

void DelayXms(unsigned int cnt)
{
 unsigned int h;
 for(h=0;h<cnt;h++)
 {
	if(stopped) break;
	Delay10ms();
 }
}
void Delay10ms(void)
{
	// Timer 0 in 16 Bit Mode @ 11059200Hz, For 10msec delay
	TL0 = 0x00;
	TH0 = 0xDC;
	TR0  = 1;
	while(!TF0);
	TR0  = 0;
	TF0  = 0;
}

void DelayUs(void)
{
	TL0 = 0x48;
	TH0 = 0xFF;//200 usec at 11059200Hz
	TR0  = 1;
	while(!TF0);
	TR0  = 0;
	TF0  = 0;
}
// 8bit LCD Interface
void LcdInit()
{
   LcdWriteCmd(0x38);	//Function Set 0x38
   LcdWriteCmd(0x38);	//Function Set
   LcdWriteCmd(0x38);	//Function Set
	LcdWriteCmd(0x06);	//Entry Mode Set 0x06
   LcdWriteCmd(0x0C);	//Display On  Off Control 0x0C
}

void LcdBusy()
{
	BS   = 1;			//Make D7th bit of LCD as i/p
   EN   = 1;         //Make port pin as o/p
   RS   = 0;         //Selected command register
   RW   = 1;         //We are reading
   while(BS)
   {   					//read busy flag again and again till it becomes 0 Enable H->L
   	EN   = 0;
      EN   = 1;
   }
}
void LcdWriteCmd(unsigned char var)
{
	P0 = var;      	//Commands to be Written
   RS   = 0;        	//Selected command register
   RW   = 0;        	//We are writing in instruction register
   EN   = 1;        	//Enable H->L
   EN   = 0;
   LcdBusy();      //Wait for LCD to process the command
}
void LcdWriteChar(unsigned char var)
{
	P0 = var;      	//Data/Character to be Written
   RS   = 1;         //Selected data register
   RW   = 0;         //We are writing
   EN   = 1;         //Enable H->L
   EN   = 0;
   LcdBusy();      //Wait for LCD to process the command
}
void LcdWriteStr(char *var)
{
	while(*var)       //till string ends send characters one by one
   	LcdWriteChar(*var++);
}

void LcdGotoXY(unsigned char row, unsigned char col)
{
	switch (row)
	{
		case 1: LcdWriteCmd(0x80 + col - 1); break;
		case 2: LcdWriteCmd(0xc0 + col - 1); break;
		default: break;
		Delay10ms();
	}
}

void LcdClear()
{
 LcdWriteCmd(0x01);
 Delay10ms();
}

