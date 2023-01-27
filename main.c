#include <xc.h>
#include <stdint.h>
/*
 Group number: 2
 Participants:
 Amer Abu Adas - 2416220
 Nansy Ibrahim - 2415305
 Hana Elboghdady - 2455566
 AA plss
 */
volatile int counter = 0;
volatile int counter1 = 0;
volatile int cTimer0 = 0;
volatile int cTimer1 = 0;
volatile int displaySegmentSelector = 0;
volatile int victorySegmentSelector = 0;
volatile int loseSegmentSelector = 0;
volatile int passedNotes = 0;
#define nTimer0_500ms (19531);
#define nTimer0_1s (39062); //
#define nTimer0_400ms (15625);
#define nTimer0_300ms (11719);
#define nTimer1_500ms (76);

/*
 Timer0 8 bit 2^8 = 256 instruction cycle

Osc 40MHz 

to find period =>  1/4 Osc = 10MHZ = 0.1 microseconds 

256 * 0.1 microseconds = 25.6 milliseconds

(500ms)/25.6 microseconds = 19531 counts

16 bit = up to 65536 clock cycles 

65536 * 0.1 microseconds = 6.5536 milliseconds is the period

(500 ms) / 6.5536 microsec = 76.29 counts
 */
int healthFlag = 0;

int maxTimer0 = nTimer0_500ms;
//int maxTimer1 = nTimer1_500ms;
volatile int health = 9;
int level = 1;
#define nLevel1Notes (5);
#define nLevel2Notes (10);
#define nLevel3Notes (15);
int maxNotes = nLevel1Notes;
int nNotes = 0;
int win =0;


int startButtonPressSumming = 0;
int iSum = 0;
int N_SUM = 100;
int sumRG0 = 0;
int sumRG1 = 0;
int sumRG2 = 0, sumRG3 = 0, sumRG4 = 0;
int sumRC0;

unsigned char a = 0x00;
unsigned char row = 0x00;
int b = 0x00;

int last3Bits = 0;
int mask = 0x07;
int cycleCount = 0;

int showHealth = 1;
int gameStarted = 0;
int resetBool = 0;
#pragma config OSC = HSPLL
#pragma config WDT = OFF // Watchdog Timer
/*Datahsheet, p.33: The HSPLL mode makes use of the HS mode oscillator for frequencies up to 10 MHz. A PLL then multiplies the
oscillator output frequency by 4 to produce an internal clock frequency up to 40 MHz.*/
typedef enum GameState{
PREGAME,
L1,
L2,
L3,
POSTGAME_VICTORY,
POSTGAME_LOSS};

enum GameState current = PREGAME;



void updateMatrix() {
    LATF = LATE;
    LATE = LATD;
    LATD = LATC;
    LATC = LATB;
    LATB = LATA;
    LATA = 0x0;
}

int gethealth(){
    int x;
    switch (health) {
        case 1:
            x=6;
            break;
        case 2:
            x=91;
            break;
        case 3:
            x=79;
            break;
        case 4:
            x=102;
            break;
        case 5:
            x=109;
            break;
        case 6:
            x=125;
            break;
        case 7:
            x=7;
            break;
        case 8:
            x=127;
            break;
        case 9:
            x=111;
            break;            
    }
    return x;      
}

int getlevel(){
    int x;
    switch (current) {
        case L1:
            x=6;
            break;
        case L2:
            x=91;
            break;
        case L3:
            x=79;
            break;
    }
    return x;  
}

void update7SegmentDisplay(void) {
    TRISJ = 0x00;
    TRISH = 0x00;
    PORTJ = 0x00;
    PORTH = 0x00;
   
    if(current == POSTGAME_LOSS){  //DISPLAY LOSE  // && !win
        switch (loseSegmentSelector++ % 4) {
            case 0:
                PORTH = 8;
                PORTJ = 0x79;
                break;
            case 1:
                PORTH = 4;
                PORTJ = 0x6D;
                break;
            case 2:
                PORTH = 2;
                PORTJ = 0x3F;
                break;
            case 3:
                PORTH = 1;
                PORTJ = 0x38;
                break;
        }    
    }
    
    else if(current == POSTGAME_VICTORY){    //!!!!NEED TO UPDATE WIN
        gameStarted=0;
        //PORTC=0;
        switch (victorySegmentSelector++ % 3) {
        case 0:
            PORTH = 4;
            PORTJ = 0x5E;
            break;
        case 1:
            PORTH = 2;
            PORTJ = 0x54 ;
            break;
        case 2:
            PORTH = 1;
            PORTJ = 0x79;
            break ;
        }
    }
    else {
        switch (displaySegmentSelector++ % 2) {
        case 0:
            PORTH = 8;
            PORTJ = getlevel();
            break;
        case 1:
            PORTH = 1;
            PORTJ = gethealth() ;
            break;
        }
    }    
}

void generateRandomNote(void) {
  uint8_t tmr1=TMR1L;

  uint8_t place = (tmr1 << 5) % 5;
  
 
  LATA=0x01;
  switch (place){
      case 0:
          LATA=0x01;
          break;
      case 1:
          LATA=0x02;
          break;
      case 2:
          LATA=0x04;
          break;
      case 3:
          LATA=0x08;
          break;
      case 4:
          LATA=0x10;
          break;
      default:
          break;
  }
  
  
  
  if(level==1){
      tmr1=tmr1 >> 1; 
      tmr1 += ((1<<7) * STATUSbits.C);
  }
  else if(level==2){
       for (int i = 0; i < 3; i++) {
        tmr1=tmr1 >> 1; 
        tmr1 +=  ((1<<7) * STATUSbits.C);
      }
  }
  else if(level==3){
      for (int i = 0; i < 5; i++) {
        tmr1 = tmr1 >> 1; 
        tmr1 += ((1<<7) * STATUSbits.C);  
      }
  }
  
  

}

void handleUserButtonPress(){
    switch(LATF){
//        case 0: //none
//            if(sumRG0 > 2) healthFlag += 1;
//            if(sumRG1 > 2) healthFlag += 1;
//            if(sumRG2 > 2) healthFlag += 1;
//            if(sumRG3 > 2) healthFlag += 1;
//            if(sumRG4 > 2) healthFlag += 1;
//            passedNotes++;
//            break;
        case 1: //RF0
            if(sumRG0 < 2) healthFlag += 1;
            if(sumRG1 > 2) healthFlag += 1;
            if(sumRG2 > 2) healthFlag += 1;
            if(sumRG3 > 2) healthFlag += 1;
            if(sumRG4 > 2) healthFlag += 1;
            passedNotes++;
            break;
        case 2:
            if(sumRG1 < 2) healthFlag += 1;
            if(sumRG0 > 2) healthFlag += 1;
            if(sumRG2 > 2) healthFlag += 1;
            if(sumRG3 > 2) healthFlag += 1;
            if(sumRG4 > 2) healthFlag += 1;
            passedNotes++;
            break;
        case 4:
            if(sumRG2 < 2) healthFlag += 1;
            if(sumRG1 > 2) healthFlag += 1;
            if(sumRG0 > 2) healthFlag += 1;
            if(sumRG3 > 2) healthFlag += 1;
            if(sumRG4 > 2) healthFlag += 1;
            passedNotes++;
            break;
        case 8:
            if(sumRG3 < 2) healthFlag += 1;
            if(sumRG1 > 2) healthFlag += 1;
            if(sumRG2 > 2) healthFlag += 1;
            if(sumRG0 > 2) healthFlag += 1;
            if(sumRG4 > 2) healthFlag += 1;
            passedNotes++;
            break;
        case 16:
            if(sumRG4 < 2) healthFlag += 1;
            if(sumRG1 > 2) healthFlag += 1;
            if(sumRG2 > 2) healthFlag += 1;
            if(sumRG3 > 2) healthFlag += 1;
            if(sumRG0 > 2) healthFlag += 1;
            passedNotes++;
            break;
    }
}

void levelprogress(){
     if (current == L1 && passedNotes == 5 && health != 0) {
         passedNotes = 0;
         current = L2;
         cycleCount = 0;
         maxNotes = 10;
         nNotes = 0;
     } else if (current == L2 && passedNotes == 10 && health != 0) {
         current = L3;
         passedNotes = 0;
         maxNotes = 15;
         cycleCount = 0;
         nNotes = 0;
     } else if (current == L3 && passedNotes == 15 && health != 0) {
         current = POSTGAME_VICTORY;
         passedNotes = 0;
     } else if (health == 0) {
         current = POSTGAME_LOSS;
         passedNotes = 0;
     }
     
     
}


void main(void) {
    
    //Control LED with button:
    TRISAbits.RA0 = 0; //output. TODO: Should I use TRISAbits.TRISA0?
    PORTAbits.RA0 = 1;
    TRISCbits.RC0 = 1; //input
    TRISG = 0x1F;
    PORTAbits.RA0 = PORTCbits.RC0; //If button RC0 is pressed, turn on LED A0
    
    //LEDs = output -> TRISX = 0
    
    TRISA = 0; TRISB = 0; TRISC = 1; TRISD = 0; TRISE = 0; TRISF = 0;
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTE = 0;
    PORTF = 0;
    
    
//        early0 = early1 = early2 = early3 = early4 = 0;

    //Clear interrupts
    INTCONbits.GIE = 0; //Global interrupt enable
    INTCONbits.TMR0IE = 0; //Timer0 interrupt enable.
    TMR0L = 0x00;
    TMR1L = 0x00;
    TMR1H = 0xAF;
   
    //TIMER 0 (PIC18F8722)
    INTCONbits.GIE = 1; //Global interrupt enable
    INTCONbits.TMR0IE = 1; //Timer0 interrupt enable.
    INTCONbits.TMR0IF = 0;
    T0CONbits.T0CS = 0; //internal clock selected
    T0CONbits.PSA = 0; //prescaler is NOT assigned
    T0CONbits.T0PS0 = 0;
    T0CONbits.T0PS1 = 0;
    T0CONbits.T0PS2 = 0;
    T0CONbits.TMR0ON = 1;

    //TIMER 1
    T1CONbits.RD16 = 1;
    T1CONbits.T1CKPS0 = 0;
    T1CONbits.T1CKPS1 = 0;
    T1CONbits.T1OSCEN = 0;
    T1CONbits.TMR1CS = 0;
    T1CONbits.TMR1ON = 1;
    T1CONbits.T1RUN = 1;
    
   

    while(1) {
        if (gameStarted) {
            switch(current) {
                case (L1): maxTimer0 = nTimer0_500ms; break;
                case (L2): maxTimer0 = nTimer0_400ms; break;
                case (L3): maxTimer0 = nTimer0_300ms; break;
            }
            TRISC = 0;
            
            if (cTimer1 > 0) {
                update7SegmentDisplay();
                cTimer1 = 0;
            }
             
//            handleEarlyPress();
//            
//            if (early0) {
//                health--;
//                early0 = 0;
//            }
//            if (early1) {
//                health--;
//                early1 = 0;
//            }
//            if (early2) {
//                health--;
//                early2 = 0;
//            }
//            if (early3) {
//                health--;
//                early3 = 0;
//            }
//            if (early4) {
//                health--;
//                early4 = 0;
//            }
//    early0 = early1 = early2 = early3 = early4 = 0;
//            
            levelprogress();
            
           if (cTimer0 > maxTimer0) {
                startButtonPressSumming = 1;
                cTimer0 = 0;
                updateMatrix();                
                if (nNotes++ < maxNotes) generateRandomNote();
                
                if (healthFlag) {
                    health-=healthFlag;
                    healthFlag = 0;
                    //health--;
                }
                
//                if (LATE > 0) {
////                    handleUserButtonPress();
//                    startButtonPressSumming = 1;
//                }
            }
            
            if (resetBool) {
                Reset();
            }
            if (current == POSTGAME_VICTORY || current == POSTGAME_LOSS) {
                LATA = LATB = LATC = LATD = LATE = LATF = 0;
                TRISC = 1;
                gameStarted = 1;
            }
            
            if (!resetBool && (current == POSTGAME_VICTORY || current == POSTGAME_LOSS)) {
                
                resetBool = PORTCbits.RC0;
            }
            
            if (startButtonPressSumming) {
                if(iSum++ < N_SUM) {
                    sumRG0 += PORTGbits.RG0;
                    sumRG1 += PORTGbits.RG1;
                    sumRG2 += PORTGbits.RG2;
                    sumRG3 += PORTGbits.RG3;
                    sumRG4 += PORTGbits.RG4;
                   // startButtonPressSumming = 0;
                }
               else {
                    handleUserButtonPress();
                    sumRC0 = 0;
                    sumRG0 = 0;
                    sumRG1 = 0;
                    sumRG2 = 0;
                    sumRG3 = 0;
                    sumRG4 = 0;
                    iSum = 0;
                    startButtonPressSumming = 0;
                }
            }
        }
        else {
            gameStarted = PORTCbits.RC0;
            if (gameStarted) current = L1;
            last3Bits = TMR1L;
        }
    }

    return;
}

void __interrupt() isr(void) {
    if(INTCONbits.TMR0IF) {
        TMR0L = 0;
        //TMR0IF = 0;
        INTCONbits.TMR0IF = 0;   // clear the timer0 interrupt flag
        cTimer0++;
    }
    if(TMR1IF) {
        TMR1L = 0x00;
        TMR1H = 0xAF; // initial timer value
        TMR1IF = 0; //interrupt flag
        cTimer1++;
        showHealth = ~showHealth;
    }
}