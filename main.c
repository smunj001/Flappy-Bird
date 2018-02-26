/*
 * OFFICIAL_FINAL_PROJECT.c
 *
 * Created: 3/15/2017 11:42:38 PM
 * Author : Karthik Munjeti
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <time.h>
#include "bit.h"
#include "scheduler.h"
#include "timer.h"

//Colors To Be Displayed on Matrix
unsigned char red = 0x00;
unsigned char green = 0x00;
unsigned char blue = 0x00;
unsigned char ground = 0xFF;

//Represent the Matrix as an 8 x 8 array
unsigned char Matrix[8][8];
unsigned char newPosition;
//Use Rows and Columns to Navigate through LED Matrix [Max 7]
int row = 0;
int col = 0;
int x_val;
int y_val;
int r = 0; //random num generator value
int wall_flag = 0; //go through walls flag

int height = 0;
int width = 0;
int wall_height = 0;

int flag = 0; // This will keep track of wall and dot intersection (IF 0 = Clear, IF 1 = END GAME)

void move_Bits()
{	
	if( row == 7 || ground == 0x80 )
	{
		ground = 0x01;
		row = 0;
	}
	else 
	{
		ground = (ground << 1);
		row++;
	}
	
	blue = 0x00;
	red = 0x00;
	green = 0x00;
	

	for(int col = 0; col < 8; col++) 
	{
		if(Matrix[row][col] == 1) 
		{
			green |= 0x80;
		}
		
		if(Matrix[row][col] == 2)
		{
			blue |= 0x80;
		}
		
		if(Matrix[row][col] == 3)
		{
			red |= 0x80;
		}
		
		if(Matrix[row][col] == 4)
		{
			blue |= 0x80;  green |= 0x80;
		}
		
		//Shift RGB to the right if Column less than 7
		
		if(col < 7) 
		{
			red = red >> 1;
			green = green >> 1;
			blue = blue >> 1;
		}
	}
	
	//Need to invert values because LED matrix is anode
	//Shift Register Code from iLearn Supplemental Lab
	
	green = ~green;
	blue = ~blue;
	red = ~red;
	
	for(int x = 7; x >= 0; x--) {
		PORTC = 0x88;
		PORTD = 0x88;
		
		
		PORTC |= ((green >> x) & 0x01);
		PORTC |= (((ground >> x) << 4) & 0x10);
		PORTD |= ((red >> x) & 0x01);
		PORTD |= (((blue >> x) << 4) & 0x10);
		
		PORTC |= 0x44;
		PORTD |= 0x44;
	}
	
	PORTC |= 0x22;
	PORTD |= 0x22;
	
	PORTC = 0x00;
	PORTD = 0x00;
}

//ADC conversion
void digitalConversion() 
{
	ADCSRA |= ( 1<<ADSC );
	while ( !( ADCSRA & ( 1<<ADIF )));
}

void ADC_init() 
{
	
	//Modified Code
	//Source: http://maxembedded.com/2011/06/the-adc-of-the-avr/
	
	ADMUX = ( 1 << REFS0 );
	ADCSRA=( 1<<ADEN )|( 1<<ADPS2 )|( 1<<ADPS1 )|( 1<<ADPS0 );	
}

// STATE MACHINE to detect WALLS

enum change_Pos_SM {init, wait, LR, UD};
int change_Pos(int state) 
{
	
	switch(state) 
	{
		case init:
			state = wait;
			break;
			
		case wait:
			if(ADMUX == 0x40) 
			{
				state = LR; //Detect Movement for Left and Right
			}
			
			else if(ADMUX == 0x41) 
			{
				state = UD; //Detect Movement for Up and Down
			}
			
			else 
			{
				state = wait;
			}
			break;
		
		case LR:
			state = wait;
			break;
		
		case UD:
			state = wait;
			break;
			
		default:
			break;
	}
	
		//ACTIONS FOR SM
	
	switch(state) 
	{
		case init:
			break;
		
		case wait:
			newPosition = 0x00;
			break;
		
		case LR:
			digitalConversion();
			x_val = ADC;	
			
			if(x_val > 800) 
			{
				newPosition = 0x01; //Right
			}
			
			else if(x_val < 200) 
			{
				newPosition = 0x02; //Left
			}
			
			ADMUX = 0x41;
			break;
		
		case UD:
			digitalConversion();
			y_val = ADC;
			
			if(y_val > 800) 
			{
				newPosition = 0x04; //Up
			}
			
			else if(y_val < 200) 
			{
				newPosition = 0x08; //Down
			}
			
			ADMUX = 0x40;
			break;
			
		default:
			break;
	}
	
	PORTB = newPosition;
	return state;
} 

enum make_Movement_SM {make_init, make_wait, up, down, left, right};
int moveObject(int state) 
{
	switch(state) 
	{
		case make_init:
			state = make_wait;
			break;
			
		case make_wait:
			if(newPosition == 0x01) 
			{
				state = left;
			}
			
			else if(newPosition == 0x02) 
			{
				state = right;
			}
			
			else if(newPosition == 0x04)
			{
				state = up;
			}
			
			else if(newPosition == 0x08)
			{
				state = down;
			}
			
			else 
			{
				state = make_wait;
			}
			break;
			
		case up:
			state = make_wait;
			break;
		
		case down:
			state = make_wait;
			break;
			
		case left:
			state = make_wait;
			break;
			
		case right:
			state = make_wait;
			break;
			
		default: 
			break;
	}
	
	switch(state) 
	{
		case make_init:
			break;
		
		case make_wait:
			break;
		
		case left:
			Matrix[height][width] = 0;
			
			if(width == 0) 
			{
				width = 7;
			}
			
			else 
			{
				--width;
			}
			
			//Display
			Matrix[height][width] = 1;			
			break;
			
		case right:
			Matrix[height][width] = 0;
			
			if(width == 7) 
			{
				width = 0;
			}
			
			else 
			{
				++width;
			}
			
			Matrix[height][width] = 1;
			break;
			
		case down:
			
			Matrix[height][width] = 0;
			
			if(height == 7) 
			{
				height = 0;
			}
			
			else 
			{
				++height;
			}
			
			Matrix[height][width] = 1;
			break;
			
		case up:
			Matrix[height][width] = 0;
			
			if(height == 0) 
			{
				height = 7;
			}
			
			else 
			{
				--height;
			}
			
			Matrix[height][width] = 1;
			break;
			
		default:
			break;
	}
	
	return state;
}

// STATE MACHINE TO GENERATE WALLS

enum make_Walls_SM {walls_init, walls_wait, generate_walls, move_walls};
int make_Walls(int state)
{
	switch(state)
	{
		case walls_init:
			state = walls_wait;
			break;
		
		case walls_wait:
			state = generate_walls;
			break;
		
		case generate_walls:
			state = move_walls;
			break;
		
		case move_walls:
			if (wall_height == 7)
			{
				state = generate_walls;
			}
			else
			{
				state = move_walls;
			}
			break;
		
		default:
			break;
	}
	//ACTIONS
	switch(state)
	{
		case walls_init:
			break;
		
		case walls_wait:
			break;
		
		// Have a random number generator select walls randomly
		
		case generate_walls:
			wall_height = 0;
			r = random() % 5; 
			//generate a random number between 0 and 5
			//r = 0;
		
			for(int x = 0; x < 8; ++x)
			{
				if(Matrix[7][x] == 2 || Matrix[7][x] == 3 || Matrix[7][x] == 4 )
				{
					Matrix[7][x] = 0;
				}
// 				else
// 				{
// 					Matrix[7][x] = 1;
// 
// 				}
			}
		//Display the matrix
			if (r == 0)
			{
				Matrix[0][0] = 0;
				Matrix[0][1] = 2;
				Matrix[0][2] = 2;
				Matrix[0][3] = 2;
				Matrix[0][4] = 2;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 2;
			}
			else if (r == 1)
			{
				Matrix[0][0] = 2;
				Matrix[0][1] = 0;
				Matrix[0][2] = 2;
				Matrix[0][3] = 2;
				Matrix[0][4] = 2;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 2;
			}
			else if (r == 2)
			{
				Matrix[0][0] = 2;
				Matrix[0][1] = 2;
				Matrix[0][2] = 0;
				Matrix[0][3] = 2;
				Matrix[0][4] = 2;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 2;
			}
			else if (r == 3)
			{
				Matrix[0][0] = 2;
				Matrix[0][1] = 2;
				Matrix[0][2] = 2;
				Matrix[0][3] = 0;
				Matrix[0][4] = 2;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 2;
			}
			else if (r == 4)
			{
				Matrix[0][0] = 2;
				Matrix[0][1] = 2;
				Matrix[0][2] = 2;
				Matrix[0][3] = 2;
				Matrix[0][4] = 4;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 2;
			}
			else if (r == 5)
			{
				Matrix[0][0] = 2;
				Matrix[0][1] = 2;
				Matrix[0][2] = 2;
				Matrix[0][3] = 2;
				Matrix[0][4] = 2;
				Matrix[0][5] = 2;
				Matrix[0][6] = 2;
				Matrix[0][7] = 0;
			}
			break;
// 			else if (r == 6)
// 			{
// 				Matrix[0][0] = 2;
// 				Matrix[0][1] = 2;
// 				Matrix[0][2] = 2;
// 				Matrix[0][3] = 3;
// 				Matrix[0][4] = 2;
// 				Matrix[0][5] = 2;
// 				Matrix[0][6] = 2;
// 				Matrix[0][7] = 2;
// 			}
		
		case move_walls:
			if(r == 0)
			{
				Matrix[wall_height][1] = 0;
				Matrix[wall_height][2] = 0;
				Matrix[wall_height][3] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
				Matrix[wall_height][7] = 0;
				
				++wall_height;
				
				if(Matrix[wall_height][1] == 1 || Matrix[wall_height][2] == 1 || Matrix[wall_height][3] == 1 || Matrix[wall_height][4] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 || Matrix[wall_height][7] == 1)
				{
					//exit(0);
					flag = 1;
				}
				
				Matrix[wall_height][1] = 2;
				Matrix[wall_height][2] = 2;
				Matrix[wall_height][3] = 2;
				Matrix[wall_height][4] = 2;
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
				Matrix[wall_height][7] = 2;
			}
			
						
			else if(r == 1)
			{
				Matrix[wall_height][0] = 0;
				Matrix[wall_height][2] = 0;
				Matrix[wall_height][3] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
				Matrix[wall_height][7] = 0;
					
				++wall_height;
				
				if(Matrix[height][0] == 1 || Matrix[height][2] == 1 || Matrix[height][3] == 1 || Matrix[height][4] == 1 || Matrix[height][5] == 1 || Matrix[height][6] == 1 || Matrix[height][7] == 1)
				{
					//exit(0);
					flag = 1;
				}
					
				Matrix[wall_height][0] = 2;
				Matrix[wall_height][2] = 2;
				Matrix[wall_height][3] = 2;
				Matrix[wall_height][4] = 2;
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
				Matrix[wall_height][7] = 2;

			}
				
				
			else if(r == 2)
			{
				Matrix[wall_height][0] = 0;
				Matrix[wall_height][1] = 0;
				Matrix[wall_height][3] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
				Matrix[wall_height][7] = 0;
					
				++wall_height;
				
				if(Matrix[wall_height][0] == 1 || Matrix[wall_height][1] == 1 || Matrix[wall_height][3] == 1 || Matrix[wall_height][4] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 || Matrix[wall_height][7] == 1)
				{
					//exit(0);
					flag = 1;
				}
				
				Matrix[wall_height][0] = 2;
				Matrix[wall_height][1] = 2;
				Matrix[wall_height][3] = 2;
				Matrix[wall_height][4] = 2;
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
				Matrix[wall_height][7] = 2;
			}
			
			
			else if(r == 3)
			{
				Matrix[wall_height][0] = 0;
				Matrix[wall_height][1] = 0;
				Matrix[wall_height][2] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
				Matrix[wall_height][7] = 0;
			
				++wall_height;
							
				if(Matrix[wall_height][0] == 1 || Matrix[wall_height][1] == 1 || Matrix[wall_height][2] == 1 ||  Matrix[wall_height][4] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 || Matrix[wall_height][7] == 1)
				{
					//exit(0);
					flag = 1;
				}
				
				Matrix[wall_height][0] = 2;
				Matrix[wall_height][1] = 2;
				Matrix[wall_height][2] = 2;
				Matrix[wall_height][4] = 2;
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
				Matrix[wall_height][7] = 2;

			}
						
			else if(r == 4)
			{
				Matrix[wall_height][0] = 0;
				Matrix[wall_height][1] = 0;
				Matrix[wall_height][2] = 0;
				Matrix[wall_height][3] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
				Matrix[wall_height][7] = 0;
				
				++wall_height;

				if(Matrix[wall_height][0] == 1 || Matrix[wall_height][1] == 1 || Matrix[wall_height][2] == 1 || Matrix[wall_height][3] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 || Matrix[wall_height][7] == 1)
				{
					//exit(0);
					flag = 1;
				}
								
				Matrix[wall_height][0] = 2;
				Matrix[wall_height][1] = 2;
				Matrix[wall_height][2] = 2;
				Matrix[wall_height][3] = 2;
				
// 				if(Matrix[wall_height + 1][4] == 4 && Matrix[height][4] == 1)
// 				{
// 					Matrix[wall_height][4] = 1;
// 					wall_flag = 1;
// 					// set var
// 				}
// 				else
// 				{
// 					Matrix[wall_height][4] = 4;
// 				}
				
				
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
				Matrix[wall_height][7] = 2;

			}
			else if(r == 5) 
			{
				Matrix[wall_height][0] = 0;
				Matrix[wall_height][1] = 0;
				Matrix[wall_height][2] = 0;
				Matrix[wall_height][3] = 0;
				Matrix[wall_height][4] = 0;
				Matrix[wall_height][5] = 0;
				Matrix[wall_height][6] = 0;
			
				++wall_height;

				if(Matrix[wall_height][0] == 1 || Matrix[wall_height][1] == 1 || Matrix[wall_height][2] == 1 || Matrix[wall_height][3] == 1 || Matrix[wall_height][4] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 )
				{
					//exit(0);
					flag = 1;
				}
								
				Matrix[wall_height][0] = 2;
				Matrix[wall_height][1] = 2;
				Matrix[wall_height][2] = 2;
				Matrix[wall_height][3] = 2;
				Matrix[wall_height][4] = 2;
				Matrix[wall_height][5] = 2;
				Matrix[wall_height][6] = 2;
			}
			
// 			else if(r == 6)
// 			{
// 				Matrix[wall_height][0] = 0;
// 				Matrix[wall_height][1] = 0;
// 				Matrix[wall_height][2] = 0;
// 				Matrix[wall_height][3] = 0;
// 				Matrix[wall_height][4] = 0;
// 				Matrix[wall_height][5] = 0;
// 				Matrix[wall_height][6] = 0;
// 				Matrix[wall_height][7] = 0;
// 				
// 				++wall_height;
// 
// 				if(Matrix[wall_height][0] == 1 || Matrix[wall_height][1] == 1 || Matrix[wall_height][2] == 1 || Matrix[wall_height][4] == 1 || Matrix[wall_height][5] == 1 || Matrix[wall_height][6] == 1 || Matrix[wall_height][7] == 1)
// 				{
// 					//exit(0);
// 					flag = 1;
// 				}
// 				
// 				Matrix[wall_height][0] = 2;
// 				Matrix[wall_height][1] = 2;
// 				Matrix[wall_height][2] = 2;
// 				Matrix[wall_height][3] = 4;
// 				Matrix[wall_height][4] = 2;
// 				Matrix[wall_height][5] = 2;
// 				Matrix[wall_height][6] = 2;
// 			}

			break;
		
		default:
			break;
	}
	
	return state;
}

int main(void)
{
	srand(time(0));
	// (DDR) F is output; 0 is input
	// A is input rest are outputs

	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	unsigned long int change_Pos_period = 50;
	unsigned long int moveObject_period = 50;
	unsigned long int make_Walls_period = 500;
	
	//calculate GCD
	unsigned long int GCD = 1;
	
	//declare tasks and task array
	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = 3; 
	
	//Task 1 and Period
	task1.state = init; 
	task1.period = change_Pos_period;
	task1.elapsedTime = change_Pos_period;
	task1.TickFct = &change_Pos;

	//Task 2 and Period	
	task2.state = make_init;
	task2.period = moveObject_period;
	task2.elapsedTime = moveObject_period;
	task2.TickFct = &moveObject;

	//Task 3 and Period	
	task3.state = walls_init;
	task3.period = make_Walls_period;
	task3.elapsedTime = make_Walls_period;
	task3.TickFct = &make_Walls;

	
	TimerSet(1);
	TimerOn();
	ADC_init();
	
	// Assign Height and Width Values for Character and Walls
	
	height = 7;
	width = 7;
	wall_height = 0;
	
	//Assign Matrix Values
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 8; j++)
		{
			Matrix[i][j] = 0;
		}
	}
	
	//Insert the Dot Starting Position
	Matrix[height][width] = 1;
	
	while (1) 
	{
		move_Bits();
		if(flag == 0)
		{
			for(int i = 0; i < numTasks; ++i) 
			{
				if(tasks[i]->elapsedTime == tasks[i]->period) 
				{
					tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
					tasks[i]->elapsedTime = 0;
				}
				tasks[i]->elapsedTime += GCD;
			}
		}
		//Check to see Flappy didnt hit wall or display 'X' on Matrix
		else if(flag == 1)
		{
			for(int row = 0; row < 8; row++)
			{
				for(int col = 0; col < 8; col++)
				{
					Matrix[row][col] = 0;
				}
			}

			Matrix[0][0] = 4;
			Matrix[1][1] = 4;
			Matrix[2][2] = 4;
			Matrix[3][3] = 4;
			Matrix[4][4] = 4;
			Matrix[5][5] = 4;
			Matrix[6][6] = 4;
			Matrix[7][7] = 4;

			Matrix[0][7] = 4;
			Matrix[1][6] = 4;
			Matrix[2][5] = 4;
			Matrix[3][4] = 4;
			Matrix[4][3] = 4;
			Matrix[5][2] = 4;
			Matrix[6][1] = 4;
			Matrix[7][0] = 4;
		}
		
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
	
	return 0;
}