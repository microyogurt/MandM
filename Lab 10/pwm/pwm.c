#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

void initLCD();
void putchLCD(char ch);
void putsLCD(char* ch);
void gotoLC(char line, char col);
void clrLCD();
char kbscan();
void sysinit();

#define clrbit(var,bit) var &= ~(1<<bit)
#define setbit(var,bit) var |= 1<<bit

#define C1 1
#define C2 2
#define C3 3

#define OC0 3;

int main(){
   unsigned char code_now=0x7f, code_ante;
   unsigned char kbhit=0,kbcode = 0x7f, loop_cnt=0;
   
   //ceas
   unsigned char cycles=0;
   unsigned char s = 0;
   unsigned char m = 24;
   unsigned char h = 9;
   unsigned char digit;
   //

   unsigned char pit = 0;

	int lumi=50;  //luminozitatea
   char buf_lumi[4];

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


   while(1){
      if(loop_cnt==200){
         code_ante=code_now;
         code_now = kbscan();
         if(code_now!=0x7f && code_ante==0x7f){
            kbhit=1;
            kbcode=code_now;
         }
         loop_cnt=0;
      }

      if(TIFR & 1<<OCF2){
         TIFR |= 1<<OCF2;
         cycles++;

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
            if(h==24)
               h=0;
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
         gotoLC(2,11);
         putsLCD("L=");
         itoa(lumi,buf_lumi,10);
         putsLCD(buf_lumi);
         putsLCD("% " );
      }
      //PWM end

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
                     if((a*a + b*b) == c*c )
                        pit = 1;
                  }
               }

               gotoLC(1,5);
          
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

      kbhit=0;
      loop_cnt++;
   }//end while
}//end main

