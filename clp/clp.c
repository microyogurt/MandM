#include <avr/io.h>
#include <avr/interrupt.h> 
#include <stdio.h>
#include <stdlib.h>
#define F_CPU 8000000UL
#include <util/delay.h> 

void initLCD();
void putchLCD(char ch);
void putsLCD(char* ch);
void gotoLC(char line, char col);
void clrLCD();
char kbscan();
void sysinit();
unsigned char rd_LCDreg(unsigned char vRS);
void wr_LCDreg(unsigned char vRS, unsigned char data);
void ckbf();

#define IReg 0
#define SReg 0
#define DReg 1

#define clrbit(var,bit) var &= ~(1<<bit)
#define setbit(var,bit) var |= 1<<bit

#define NOKEY 0x7f

#define C1 1
#define C2 2
#define C3 3

#define OC0 3

#define OCIE2 7
#define IF 7

volatile unsigned char kbhit = 0;
volatile char kbcode; 

int main(){

   unsigned char pit = 0;
   unsigned char stare = C1;
   char buf_pit[4];
   unsigned long int a,b,c;

   sysinit();
   
   TCCR2 = 0b00001110;
   OCR2 = 250-1;
   
   //          __ __Fast PWM
   //         |  |
   TCCR0 = 0b01101100;
   //          || |||
   //          || ------ clk(i/o)/256
   //          ||
   //          ---------Clear OC0 on compare match, set OC0 at bottom
   OCR0 = 128;
   
   setbit(DDRB, OC0);
   
   //demascheaza cererea de intrerupere OCF2 de la timerul 2
   setbit(TIMSK, OCIE2);

   //seteaza IF in SREG 
   setbit(SREG, IF);


   while(1){
      

	  //c pitagoreic start
      if( kbhit && kbcode>='0' && kbcode<='9'){
         kbhit=0;
         switch(stare){
         	case C1:   //cifra 0
               gotoLC(1,1);
               putsLCD("                ");
               buf_pit[0]=kbcode;
               gotoLC(1,1);
               putchLCD(kbcode);
               stare=C2;
               break;

            case C2:
               buf_pit[1]=kbcode;
               gotoLC(1,2);
               putchLCD(kbcode);
               stare=C3;
               break;
         
            case C3:
               buf_pit[2]=kbcode;
               buf_pit[3]='\0';
               gotoLC(1,3);
               putchLCD(kbcode);
               gotoLC(1,5);
               putsLCD("Busy...");
               c = atoi(buf_pit);
               
               //scrie "Busy..." pe linia 1, din coloana 5
               //calculeaza c

               for(a = 1;a<=999;a++){
                  for(b = 1;b<=999;b++){
                     if((a*a + b*b) == c*c ){
                        pit = 1;
                        goto FINISH;
                     }
                  }
               }

               FINISH : gotoLC(1,5);
          
      		   if(pit){
      		      putsLCD("       PYTH");
                  pit = 0;
      		   }
      		   else{
      		      putsLCD("      NPYTH");
      		   }
               stare=C1;
              
               break;
         }//end switch     
     }
       // end c pitagoreic      

   }//end while
}//end main

ISR(TIMER2_COMP_vect){ 
   //ceas
   static unsigned char cycles=0;
   static unsigned char s = 0;
   static unsigned char m = 24;
   static unsigned char h = 9;
   unsigned char digit;
   //

   //tastatura
   static unsigned char code_now=0x7f;
   static unsigned char loop_cnt=0;
   unsigned char code_ante;
   //
   
   //luminozitatea
   static int lumi=50;  
   char buf_lumi[4];
   //
   unsigned char AC;
   
   ckbf();
   _delay_us(8);
   AC = rd_LCDreg(SReg) & 0x7F;


   gotoLC(2,1);
   if(h>9){
      digit = h/10 + '0';
      putchLCD(digit);
      digit = h%10 + '0';
      putchLCD(digit);
   } else {
      putchLCD(' ');
      digit = h%10 + '0';
      putchLCD(digit);
   }
   putchLCD(':');
   digit = m/10 + '0';
   putchLCD(digit);
   digit = m%10 + '0';
   putchLCD(digit);

   putchLCD(':');
   digit = s/10 + '0';
   putchLCD(digit);
   digit = s%10 + '0';
   putchLCD(digit);

   if(loop_cnt==4){ //4 sau alta valoare
      loop_cnt=0;
      code_ante = code_now;
      code_now = kbscan();
      if( code_ante == NOKEY && code_now != NOKEY){
         kbhit=1;
         kbcode=code_now;
      }
   } 
   

   //PWM start
   if(kbhit  && (kbcode == 'C' || kbcode == 'D')){
      kbhit = 0;
      if(kbcode == 'C'){
         lumi += 5;
         if(lumi > 100)
            lumi = 100;
      }
      if(kbcode == 'D'){
         lumi -= 5;
         if(lumi < 0)
            lumi = 0;
      }
      OCR0 = lumi*255/100;
      gotoLC(2,12);
      putsLCD("     ");
      gotoLC(2,11);
      putsLCD("L=");
      itoa(lumi,buf_lumi,10);
      putsLCD(buf_lumi);
      putsLCD("%");
   }
   //PWM end

   cycles++;
   
   //ceas start
   if(cycles == 125){
      cycles = 0;
      s++;
      if(s==60){
         s = 0;
         m++;
      }
      if(m==60){
         m = 0;
         h++; 
      }
      if(h==24){
         h=0;
      }
   } 
   if(cycles == 67) {
      gotoLC(2,3);
      putchLCD(' ');
      gotoLC(2,6);
      putchLCD(' ');
   }

 

   if(TIFR & 1<<OCF2){
      clrLCD();
      putsLCD("ERROR");
      while(1){}
   }
   //ceas end
   
   setbit(AC, 7);
   wr_LCDreg(IReg, AC);
   ckbf();
   _delay_us(8);
   loop_cnt++;


}//end ISR
