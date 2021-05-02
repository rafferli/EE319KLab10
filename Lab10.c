// Lab10.c
// Runs on TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 1/16/2021 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
	
struct PlayerSprite {
	const uint8_t *image;
	uint32_t X;
	int32_t Y;
	int32_t life;
};

struct EnemySprite {
	const uint8_t *image;
	uint32_t X;
	int32_t Y;
	int32_t VY;
	int32_t VYReload;
	int32_t VYCounter;
	int32_t points;
	int32_t life;
};

struct Missile {
	const uint8_t *image;
	uint32_t X;
	int32_t Y;
	int32_t VY;
	int32_t life;
};



typedef struct PlayerSprite Player_t;

typedef struct EnemySprite Enemy_t;

typedef struct Missile Missile_t;

Player_t Player1 = {PlayerShip0, 0, 63, 1};

Enemy_t Enemies[10];

Missile_t Missiles[10];

void Draw(void);
void Move(void);

void Level1Handler(void);
void Level2Handler(void);
void Level3Handler(void);

char score = 0;
int gameOver = 0;
// **************SysTick_Init*********************
// Initialize Systick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(unsigned long period){
  NVIC_ST_CTRL_R = 0;         									// disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;									// reload value
  NVIC_ST_CURRENT_R = 0;     									  // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority 1
  NVIC_ST_CTRL_R = 7; 
}

void SysTick_Handler(void){ // every 100 ms
  //PF1 ^= 0x02;     // Heartbeat
	//PF2 ^= 0x04;
	//PF3 ^= 0x08;
	Move();
	
	static uint32_t lastdown = 1;
	uint32_t down = GPIO_PORTF_DATA_R & 0x10;
	if(down == 0 && lastdown != 0){
	PF1 ^= 0x02;
	
	int notAssigned = 1;
	int assignIndex = 0;
		while(notAssigned){
			
			if(Missiles[assignIndex].life == 0)
			{
				Missiles[assignIndex].X = Player1.X+7	;
				Missiles[assignIndex].Y = Player1.Y;
				Missiles[assignIndex].VY = 2;
				Missiles[assignIndex].image = Laser0;
				Missiles[assignIndex].life = 1;
				notAssigned = 0;
			}else{
				assignIndex++;
				assignIndex = assignIndex%10;
			}
			
		}
	}	
	lastdown = down;
}

// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
	GPIO_PORTF_PUR_R |= 0x11;
  GPIO_PORTF_DEN_R |=  0x1F;   // enable digital I/O on PF3,2,1
	
		
		
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}
//********************************************************************************
 
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
int main(void){
	
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
  SSD1306_ClearBuffer();
	ADC_Init(SAC_32);
	SysTick_Init(2666666);
	EnableInterrupts();
	
	//void SSD1306_DrawBMP(uint8_t xpos, uint8_t ypos, const uint8_t *ptr, uint8_t threshold, uint16_t color);
	
	while(1){
	
	if(score < 3) {Level1Handler();}
	if(score == 3) {Level2Handler();}
	if(score == 6) {Level3Handler();}
	if(score == 11) {gameOver = 1;}
	Draw();
		
		
		
	if(gameOver){
		SSD1306_OutClear(); // set screen to black 	
		SSD1306_SetCursor(0,0);
    SSD1306_OutString("game over");
		SSD1306_SetCursor(0,1);
    SSD1306_OutString("Score:");
		SSD1306_SetCursor(0,2);
    LCD_OutDec(score);
		while(1){
		     

		}
	}
	
	}

	
}

void Draw(){
	
		SSD1306_ClearBuffer();
	
		for(int i = 0; i < 10; i++){
			if(Missiles[i].life == 1){
			SSD1306_DrawBMP(Missiles[i].X, Missiles[i].Y, Missiles[i].image, 0, SSD1306_INVERSE);
			}
		}
		
		for(int i = 0; i < 10; i++){
			if(Enemies[i].life == 1){
			SSD1306_DrawBMP(Enemies[i].X, Enemies[i].Y, Enemies[i].image, 0, SSD1306_INVERSE);
			}
		}
		
		SSD1306_DrawBMP(Player1.X, Player1.Y, Player1.image, 0, SSD1306_INVERSE);
		SSD1306_OutBuffer();
}

void Move(){
	
		Player1.X =	ADC_In()/35;
		if(Player1.X > 110){
		Player1.X = 111;
		}
		
		//missile movement
		for(int i = 0; i < 10; i++){
			Missiles[i].Y -= Missiles[i].VY;
			if(Missiles[i].Y <= 0){Missiles[i].life = 0;}
			
			//check to see if we hit enemy
			for(int j = 0; j < 10; j++){
				int Ydifference = Missiles[i].Y - Enemies[j].Y;
				if(Ydifference < 0){Ydifference*=-1;}
				
				if(Ydifference < 10 && Missiles[i].life == 1 && Enemies[j].life == 1){
					
					int Xdifference = Missiles[i].X - Enemies[j].X;
						if(Xdifference < 0){Xdifference*=-1;}
						if( Xdifference < 10){
						
								Enemies[j].life = 0;
								Missiles[i].life = 0;
								score++;
						}
				}
			}	
		}
		
		//enemy movement
		for(int i = 0; i < 10; i++){
			if(Enemies[i].VYCounter == 0){
				Enemies[i].Y += Enemies[i].VY;
				Enemies[i].VYCounter = Enemies[i].VYReload;
			}else{
				Enemies[i].VYCounter--;
			}
			
			if(Enemies[i].Y >= 63 && Enemies[i].life != 0){Enemies[i].life = 0; gameOver = 1;}
		}
		
}

void Level1Handler(){
	
	if(Enemies[0].life == 0){
				Enemies[0].X = 50;
				Enemies[0].Y = 8;
				Enemies[0].VY = 1;
				Enemies[0].VYReload = 5;
				Enemies[0].VYCounter = 5;
				Enemies[0].image = Alien10pointA;
				Enemies[0].life = 1;
	}

}

void Level2Handler(){
	
	if(Enemies[0].life == 0){
				Enemies[0].X = 10;
				Enemies[0].Y = 8;
				Enemies[0].VY = 1;
				Enemies[0].VYReload = 5;
				Enemies[0].VYCounter = 5;
				Enemies[0].image = Alien10pointA;
				Enemies[0].life = 1;
	}
	
	if(Enemies[1].life == 0){
				Enemies[1].X = 50;
				Enemies[1].Y = 8;
				Enemies[1].VY = 1;
				Enemies[1].VYReload = 5;
				Enemies[1].VYCounter = 5;
				Enemies[1].image = Alien10pointA;
				Enemies[1].life = 1;
	}
	
	if(Enemies[2].life == 0){
				Enemies[2].X = 100;
				Enemies[2].Y = 8;
				Enemies[2].VY = 1;
				Enemies[2].VYReload = 5;
				Enemies[2].VYCounter = 5;		
				Enemies[2].image = Alien10pointA;
				Enemies[2].life = 1;
	}
	
}

void Level3Handler(){
	
	if(Enemies[0].life == 0){
				Enemies[0].X = 10;
				Enemies[0].Y = 8;
				Enemies[0].VY = 1;
				Enemies[0].VYReload = 5;
				Enemies[0].VYCounter = 5;
				Enemies[0].image = Alien10pointA;
				Enemies[0].life = 1;
	}
	
	if(Enemies[1].life == 0){
				Enemies[1].X = 30;
				Enemies[1].Y = 8;
				Enemies[1].VY = 1;
				Enemies[1].VYReload = 5;
				Enemies[1].VYCounter = 5;
				Enemies[1].image = Alien10pointA;
				Enemies[1].life = 1;
	}
	
	if(Enemies[2].life == 0){
				Enemies[2].X = 45;
				Enemies[2].Y = 8;
				Enemies[2].VY = 1;
				Enemies[2].VYReload = 5;
				Enemies[2].VYCounter = 5;		
				Enemies[2].image = Alien10pointA;
				Enemies[2].life = 1;
	}
	if(Enemies[3].life == 0){
				Enemies[3].X = 60;
				Enemies[3].Y = 8;
				Enemies[3].VY = 1;
				Enemies[3].VYReload = 5;
				Enemies[3].VYCounter = 5;		
				Enemies[3].image = Alien10pointA;
				Enemies[3].life = 1;
	}
	if(Enemies[4].life == 0){
				Enemies[4].X = 100;
				Enemies[4].Y = 8;
				Enemies[4].VY = 1;
				Enemies[4].VYReload = 5;
				Enemies[4].VYCounter = 5;		
				Enemies[4].image = Alien10pointA;
				Enemies[4].life = 1;
	}
			
	
}




