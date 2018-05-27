#include <avr/io.h>

#define F_CPU 8000000UL
#include <util/delay.h> 

#define clrbit(var,bit) var &= ~(1<<bit)
#define setbit(var,bit) var |= 1<<bit

#define E  7
#define RW 6
#define RS 5

#define IReg 0
#define SReg 0
#define DReg 1

#define NOKEY 0x7f 

#define nop asm("nop"::);

volatile unsigned long int delay;

void wait(unsigned long int val){
   for(delay=0;delay<val;delay++){}
}

unsigned char rd_LCDreg(unsigned char vRS){
   unsigned char data;
   DDRD = 0x00;

   if(vRS==0)
      clrbit(PORTB,RS);
   else
      setbit(PORTB,RS);
   setbit(PORTB,RW);
   nop;
   setbit(PORTB,E);
   nop; nop; nop; nop; nop;
   data = PIND;
   clrbit(PORTB,E);
   nop;

   DDRD = 0xff;
   return data;
}

void wr_LCDreg(unsigned char vRS, unsigned char data){
   DDRD = 0xff;
   if(vRS==0)
      clrbit(PORTB,RS);
   else
      setbit(PORTB,RS);
   clrbit(PORTB,RW);
   PORTD=data;
   nop;
   setbit(PORTB,E);
   nop; nop; nop;
   clrbit(PORTB,E);
   nop;
}


void ckbf(){
   while(rd_LCDreg(SReg) & 1<<7){} 
}

void initLCD(){
   ckbf();
   // Function set
   //bit 7 6 5 4   3 2 1 0
   //    0 0 1 DL  N F — — 
   //    0 0 1 1   1 1 1 1
   wr_LCDreg(IReg, 0x3f);

   ckbf();
   // entry mode set
   //bit 7 6 5 4   3 2  1  0
   //    0 0 0 0   0 1 I/D S 
   //    0 0 0 0   0 1  1  0
   wr_LCDreg(IReg, 0x06);

   ckbf();
   // Display on/off control
   //bit 7 6 5 4   3 2 1 0
   //    0 0 0 0   1 D C B
   //    0 0 0 0   1 1 0 0
   wr_LCDreg(IReg, 0x0c);

   ckbf();
   //Clear display
   wr_LCDreg(IReg, 0x01);
}

void insertSmiley(){
   unsigned char smiley[16] = {
      0x00, //1
      0x0A, //2
      0x00, //3
      0x04, //4
      0x00, //5
      0x11, //6
      0x0A, //7
      0x04, //8
      0x00, //9
      0x00, //10
      0x00, //11
      0x00, //12
      0x00, //13
      0x00, //14
      0x00, //15
      0x00, //16
   };
   
   ckbf();
   wr_LCDreg(IReg, 0x50);
   for(int i = 0; i<15; i++){
      ckbf();
      wr_LCDreg(DReg, smiley[i]);
   }
   ckbf();
   wr_LCDreg(IReg, 0x80);
}

void sysinit(){
   PORTB = 0x00;
   DDRB  = 0xe2; //1110 0010
   DDRD  = 0xff;
   wait(120000UL);
   initLCD();
   insertSmiley();
}


void gotoLC(unsigned char line, unsigned char col){

   if( line > 0 && line <3 && col > 0 && col < 17 ){
      ckbf();
      wr_LCDreg(IReg, 0x80+(line-1)*0x40+col-1);
   }

}

void putchLCD(char ch){
   ckbf();
   wr_LCDreg(DReg, ch);
   ckbf();
   _delay_us(8);
   if((rd_LCDreg(SReg) & ~(1<<7) ) == 0x10)
      gotoLC(2,1);
   else if((rd_LCDreg(SReg) & ~(1<<7)) == 0x50)
      gotoLC(1,1);
}

void putsLCD(char* ch){
   for(int i=0; ch[i] != '\0'; i++)
      putchLCD(ch[i]);
}

void clrLCD(){
   ckbf();
   wr_LCDreg(IReg, 0x01);
}

char kbscan(){
   unsigned char temp;
   unsigned char La=0, Ca=0;
   unsigned char cod_intern;
   unsigned char cols;
   char cod_extern;
   char tabela_trans[]="123A456B789C*0#D";

   for( DDRA = 1<<4; DDRA != 0; DDRA = DDRA = DDRA << 1 ) { 
      nop; nop; nop; nop; nop; nop;
      cols=~PINA;
      cols = cols & 0x0F;
      if(cols != 0)
         break;
   }

   if(cols == 0)
      return NOKEY;

   temp=DDRA;
   if(temp == 0x10){
      La = 0;
   } else if (temp == 0x20) {
      La = 1;
   } else if (temp == 0x40) {
      La = 2;
   } else if (temp == 0x80) {
      La = 3;
   }
   
   if(cols & 0x01){
      Ca = 0;
   } else if (cols & 0x02) {
      Ca = 1;
   } else if (cols & 0x04) {
      Ca = 2;
   } else if (cols & 0x08) {
      Ca = 3;
   }
   
   cod_intern = La * 4 + Ca;
   cod_extern = tabela_trans[cod_intern];

   return cod_extern;
}



 
