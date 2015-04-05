#include "TExaS.h"
#include <stdlib.h>



#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

#define GPIO_PORTA_DATA_R 			(*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R  			(*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_DEN_R  			(*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AFSEL_R 			(*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_AMSEL_R 			(*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R 			(*((volatile unsigned long *)0x4000452C))
	
#define GPIO_PORTB_DATA_R 			(*((volatile unsigned long *)0x40005000))
#define GPIO_PORTB_DIR_R  			(*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_DEN_R  			(*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AFSEL_R 			(*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_AMSEL_R 			(*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R 			(*((volatile unsigned long *)0x4000552C))

void PortF_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000020;
	delay = SYSCTL_RCGC2_R;

	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R |= 0x1F; // check later for all pins

	GPIO_PORTF_AMSEL_R &= 0x00;
	GPIO_PORTF_AFSEL_R &= 0x00;
	GPIO_PORTF_PCTL_R &= 0xFFF00000;
	GPIO_PORTF_DIR_R &= 0x0E;
	GPIO_PORTF_DIR_R |= 0x0E;

	GPIO_PORTF_PUR_R |= 0x11;
	GPIO_PORTF_DEN_R |= 0x1F;
}
void PortB_Init(void){
volatile unsigned long delay;
SYSCTL_RCGC2_R |= 0x00000002;
delay = SYSCTL_RCGC2_R;

GPIO_PORTB_DIR_R = 0xFF;
GPIO_PORTB_AMSEL_R = 0x00;
GPIO_PORTB_AFSEL_R = 0x00;
GPIO_PORTB_PCTL_R = 0x00000000;

GPIO_PORTB_DEN_R = 0xFF;
}
void PortA_Init(void){

volatile unsigned long delay;
SYSCTL_RCGC2_R |= 0x00000001;
delay = SYSCTL_RCGC2_R;

GPIO_PORTA_DIR_R = 0xFF;
GPIO_PORTA_AMSEL_R = 0x00;
GPIO_PORTA_AFSEL_R = 0x00;
GPIO_PORTA_PCTL_R = 0x00000000;

GPIO_PORTA_DEN_R = 0xFF;

}
void min_delay(unsigned long time){
  //50 msec per 1 input
	unsigned long i;
  while(time > 0){
    i = 666666;
    while(i > 0){
      i = i - 1;
    }
    time = time - 1;
  }
}
void delay(unsigned long time){
  //500 msec per 1 input
	unsigned long i;
  while(time > 0){
    i = 6666665;
    while(i > 0){
      i = i - 1;
    }
    time = time - 1;
  }
}
void EnableInterrupts(void);

volatile unsigned long* PORTF_ARRAY[5];
volatile unsigned long* PORTA_ARRAY[8];
volatile unsigned long* PORTB_ARRAY[8];
volatile unsigned long* MAP_ARRAY[4][4];
unsigned short SNAKE_PATH[16][2];




long find_next_dir(long cur_dir,long turn);
void move_point(long* cur_dir,long* turn,
										long* head_line,long* head_col);
void move_line(long* cur_dir,long* turn,
										long* head_line,long* head_col,
												long* tail_line,long* tail_col);
void map_pins_to_map(void);
void map_mem_address_to_pins(void);

void light_up_snake(void);
void light_down_snake(void);
void create_bait(void);	
short check_snake(void);										


volatile unsigned long SW1; // input from PF4
volatile unsigned long SW2; // input from PF0
unsigned long sw_both;
										
long head_pos = 2;
long tail_pos = 0;
										
long bait_line=0;
long bait_col =0;
long last_bait_eaten = 0;


int main(){
	short i=0;
	long dir = 0;	
	//directions are represented as integers.
	//Up 		= 0
	//Right = 1
	//Down 	= 2
	//Left =3
	
	long turn = 0;
	//Turns are represented as integers
	//Right = 1
	//Left	=-1;
	long head_line = 1, head_col = 1;
	long tail_line = 3,	tail_col = 1;
	
	SNAKE_PATH[tail_pos][0] = tail_line;
	SNAKE_PATH[tail_pos][1] = tail_col;
	SNAKE_PATH[tail_pos+1][0] = 2;
	SNAKE_PATH[tail_pos+1][1] = 1;
	SNAKE_PATH[head_pos][0] = head_line;
	SNAKE_PATH[head_pos][1] = head_col;
		
		TExaS_Init(SW_PIN_PF40, LED_PIN_PF321);  // activate grader and set system clock to 80 MHz
    PortF_Init(); // Init port PF4 PF2 PF0
    PortA_Init(); //Init Porta
		PortB_Init();

	EnableInterrupts();           // enable interrupts for the grader


	map_mem_address_to_pins();
	map_pins_to_map();
	
	while(1){
	min_delay(1);
	SW1 = *PORTF_ARRAY[4] &= 0x10;
	SW2 = *PORTF_ARRAY[0] &= 0x01;
		i++;
		if((SW1 == 0x00)&&(SW2 == 0x00)){
			break;
		}
	}
	
	srand((i*i)/(7*29));
	create_bait();
	while(1){
		if(check_snake()==1){
		break;
	}
		if((head_pos-tail_pos)==-1 ||(head_pos-tail_pos)==1 || (head_pos-tail_pos)==15|| (head_pos-tail_pos)==-15){
		break;
		}
		min_delay(2);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		min_delay(3);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		*(MAP_ARRAY[SNAKE_PATH[head_pos][0]][SNAKE_PATH[head_pos][1]]) ^= 0xFF;
		min_delay(2);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		min_delay(3);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		*(MAP_ARRAY[SNAKE_PATH[head_pos][0]][SNAKE_PATH[head_pos][1]]) ^= 0xFF;
		min_delay(2);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		min_delay(3);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		*(MAP_ARRAY[SNAKE_PATH[head_pos][0]][SNAKE_PATH[head_pos][1]]) ^= 0xFF;
		min_delay(2);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		min_delay(3);
		*(MAP_ARRAY[bait_line][bait_col]) ^= 0xFF;
		*(MAP_ARRAY[SNAKE_PATH[head_pos][0]][SNAKE_PATH[head_pos][1]]) ^= 0xFF;
		//delay(2);
		SW1 = *PORTF_ARRAY[4] &= 0x10;
		SW2 = *PORTF_ARRAY[0] &= 0x01;
		
		if((SW1 == 0x00)&&(SW2 != 0x00)){
			turn = -1;
			move_line(&dir,&turn,&head_line,&head_col,&tail_line,&tail_col);
		}else if((SW1 != 0x00)&&(SW2 == 0x00)){
			turn = +1;
			move_line(&dir,&turn,&head_line,&head_col,&tail_line,&tail_col);
		}else{
			turn = 0;
			move_line(&dir,&turn,&head_line,&head_col,&tail_line,&tail_col);
		}
	}
	light_down_snake();
	min_delay(3);
	light_up_snake();
	min_delay(3);
	light_down_snake();
	min_delay(3);
	light_up_snake();
	min_delay(3);
	light_down_snake();
	min_delay(3);
	light_up_snake();
	min_delay(3);

}	
void map_mem_address_to_pins(void){
	
	PORTF_ARRAY[0] = ((volatile unsigned long *)0x40025004);    //SW2
	PORTF_ARRAY[1] = ((volatile unsigned long *)0x40025008);	//RED
	PORTF_ARRAY[2] = ((volatile unsigned long *)0x40025010);	//BLUE
	PORTF_ARRAY[3] = ((volatile unsigned long *)0x40025020);	//GREEN
	PORTF_ARRAY[4] = ((volatile unsigned long *)0x40025040);	//SW1

	PORTA_ARRAY[0] = ((volatile unsigned long *)0x40004004);    //PA0
	PORTA_ARRAY[1] = ((volatile unsigned long *)0x40004008);	//PA1
	PORTA_ARRAY[2] = ((volatile unsigned long *)0x40004010);	//PA2
	PORTA_ARRAY[3] = ((volatile unsigned long *)0x40004020);	//PA3
	PORTA_ARRAY[4] = ((volatile unsigned long *)0x40004040);	//PA4
	PORTA_ARRAY[5] = ((volatile unsigned long *)0x40004080);    //PA5
	PORTA_ARRAY[6] = ((volatile unsigned long *)0x40004100);	//PA6
	PORTA_ARRAY[7] = ((volatile unsigned long *)0x40004200);	//PA7

	PORTB_ARRAY[0] = ((volatile unsigned long *)0x40005004);    //PA0
	PORTB_ARRAY[1] = ((volatile unsigned long *)0x40005008);	//PA1
	PORTB_ARRAY[2] = ((volatile unsigned long *)0x40005010);	//PA2
	PORTB_ARRAY[3] = ((volatile unsigned long *)0x40005020);	//PA3
	PORTB_ARRAY[4] = ((volatile unsigned long *)0x40005040);	//PA4
	PORTB_ARRAY[5] = ((volatile unsigned long *)0x40005080);    //PA5
	PORTB_ARRAY[6] = ((volatile unsigned long *)0x40005100);	//PA6
	PORTB_ARRAY[7] = ((volatile unsigned long *)0x40005200);	//PA7
}
void map_pins_to_map(){
	MAP_ARRAY[0][0] = PORTB_ARRAY[5];
	MAP_ARRAY[0][1] = PORTF_ARRAY[1];
	MAP_ARRAY[0][2] = PORTF_ARRAY[2];
	MAP_ARRAY[0][3] = PORTB_ARRAY[2];
	
	MAP_ARRAY[1][0] = PORTB_ARRAY[0];
	MAP_ARRAY[1][1] = PORTB_ARRAY[1];
	MAP_ARRAY[1][2] = PORTB_ARRAY[6];
	MAP_ARRAY[1][3] = PORTB_ARRAY[7];
	
	MAP_ARRAY[2][0] = PORTB_ARRAY[4];
	MAP_ARRAY[2][1] = PORTA_ARRAY[5];
	MAP_ARRAY[2][2] = PORTB_ARRAY[3];
	MAP_ARRAY[2][3] = PORTA_ARRAY[4];
	
	MAP_ARRAY[3][0] = PORTA_ARRAY[6];
	MAP_ARRAY[3][1] = PORTA_ARRAY[7];
	MAP_ARRAY[3][2] = PORTA_ARRAY[2];
	MAP_ARRAY[3][3] = PORTA_ARRAY[3];
	
}
void move_line(long* cur_dir,long* turn,
										long* head_line,long* head_col,
												long* tail_line,long* tail_col){
	
		(*cur_dir) = find_next_dir(*cur_dir,*turn);	
		*turn = 0;
		
		light_down_snake();
		
		min_delay(2);
		
		if(*cur_dir == 0 ){
		(*head_line)--;
	}else if(*cur_dir == 2){
		(*head_line)++;
	}else if(*cur_dir == 1){
		(*head_col)++;
	}else if(*cur_dir == 3){
		(*head_col)--;
	}
		if(*head_line < 0) 			(*head_line) += 4;
		else if(*head_line > 3) (*head_line) -=	4;
		
		if(*head_col < 0)				(*head_col) += 4;
		else if(*head_col > 3)			(*head_col) -= 4;
		
	head_pos++;
	tail_pos++;
	
	if(head_pos > 15)	head_pos -= 16;
		
	if(tail_pos > 15) tail_pos -= 16;
	
	SNAKE_PATH[head_pos][0] = *head_line;
	SNAKE_PATH[head_pos][1] = *head_col;
	
	light_up_snake();
	
	if((SNAKE_PATH[head_pos][0] == bait_line)&&(SNAKE_PATH[head_pos][1]==bait_col)){
		create_bait();
		tail_pos--;
		if(tail_pos < 0) tail_pos += 16;
	}
	
	
	
} 
short check_snake(void){
	short i;
	if(head_pos > tail_pos){
		for(i = tail_pos;i <= head_pos;i++){
		if(i!=head_pos)	
			if((SNAKE_PATH[head_pos][0]==SNAKE_PATH[i][0])&&(SNAKE_PATH[head_pos][1]==SNAKE_PATH[i][1])) 
				return 1;
		}
	}else{
		
		for(i=0 ; i <= head_pos;i++){
			if(i!=head_pos)	
				if((SNAKE_PATH[head_pos][0]==SNAKE_PATH[i][0])&&(SNAKE_PATH[head_pos][1]==SNAKE_PATH[i][1])) 
					return 1;
		}
			
		for(i=tail_pos;i<=15;i++){
		if(i!=head_pos)	
			if((SNAKE_PATH[head_pos][0]==SNAKE_PATH[i][0])&&(SNAKE_PATH[head_pos][1]==SNAKE_PATH[i][1])) 
				return 1;
		}
	}
	return 0;
}
void create_bait(void){
while(1){
	bait_line = rand()%4;
	bait_col 	= rand()%4;
	if((*MAP_ARRAY[bait_line][bait_col])==0x00){
			*MAP_ARRAY[bait_line][bait_col] |= 0xFF;
		break;
		}
	}
}										
void light_up_snake(void){
	short i;
	if(head_pos > tail_pos){
		for(i = tail_pos;i <= head_pos;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) |= 0xFF;
		}
	}else{
		for(i=0 ; i <= head_pos;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) |= 0xFF;
		}
		for(i=tail_pos;i<=15;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) |= 0xFF;
		}
	}
}
void light_down_snake(void){
	short i;
	if(head_pos > tail_pos){
		for(i = tail_pos;i <= head_pos;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) &= 0x00;
		}
	}else{
		for(i=0 ; i <= head_pos;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) &= 0x00;
		}
		for(i=tail_pos;i<=15;i++){
			*(MAP_ARRAY[SNAKE_PATH[i][0]][SNAKE_PATH[i][1]]) &= 0x00;
		}
	}
	
}												
void move_point(long* cur_dir,long* turn,
										long* head_line,long* head_col){
	
		(*cur_dir) = find_next_dir(*cur_dir,*turn);	
		*turn = 0;
	
	*(MAP_ARRAY[*head_line][*head_col]) &= 0x00;			
	
	if(*cur_dir == 0 ){
		(*head_line)--;
	}else if(*cur_dir == 2){
		(*head_line)++;
	}else if(*cur_dir == 1){
		(*head_col)++;
	}else if(*cur_dir == 3){
		(*head_col)--;
	}
		if(*head_line < 0) 			(*head_line) += 4;
		else if(*head_line > 3) (*head_line) -=	4;
		
		if(*head_col < 0)				(*head_col) += 4;
		else if(*head_col > 3)			(*head_col) -= 4;
		
		*(MAP_ARRAY[*head_line][*head_col]) |= 0xFF;
	
}
void move_one_step(short* cur_dir,short* turn,
										short head_line,short head_col,
											short tail){
	
}

long find_next_dir(long cur_dir,long turn){
	//directions and turns are represented as
	// integers.adding them up gives new direction
	cur_dir += turn;
	
	if(cur_dir > 3) 
		cur_dir -= 4;
	else if(cur_dir < 0)
		cur_dir += 4;
	
	return cur_dir;
}









