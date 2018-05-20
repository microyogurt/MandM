#include <avr/io.h>

#define S2 250000UL 
#define NOKEY 0x7f 
#define DELAY 200 

void sysinit();
void putchLCD(char ch);
void putsLCD (char* ch);
void wait(unsigned long int val);
void clrLCD();
void gotoLC(unsigned char line, unsigned char col);
char kbscan();

int main(){
   
   char code_now=0x7f, code_ante;
   unsigned char kbhit=0;
   char kbcode = 0x7f;
   unsigned char flagA = 0;
   unsigned char countA = 0;
    

   unsigned char loop_cnt = 0;
   unsigned char cycles=0;
   unsigned char s = 0;
   unsigned char m = 24;
   unsigned char h = 9;
   unsigned char digit;
   
   TCCR2 = 0b00001110;
   OCR2 = 250-1;

   sysinit();
   
   while(1){ //bucla principala

      if(loop_cnt==100){
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
                  gotoLC(1,1);
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
               gotoLC(1,3);
               putchLCD(' ');
               gotoLC(1,6);
               putchLCD(' ');
            }

            if(TIFR & 1<<OCF2){
               clrLCD();
               putsLCD("ERROR");
               while(1){}
            }
         }
      
      
      
      loop_cnt++;
   } //end while 
}

