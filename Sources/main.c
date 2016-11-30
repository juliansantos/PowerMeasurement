#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include <stdio.h>
#include <math.h>

// Preprocessor directives for PORT definition of a GLCD

#define GLCD_Enable PTFD_PTFD5 // LCD Enable
#define TRIS_Enable PTFDD_PTFDD5 // LCD Enable
#define GLCD_RS PTFD_PTFD1// LCD command/data mode
#define TRIS_RS PTFDD_PTFDD1
#define GLCD_BUS PTED //GLCD bus
#define TRIS_BUS PTEDD
#define TRIS_LOAD PTGDD_PTGDD0
#define LOAD PTGD_PTGD0

//Preprocessor directives {commands used for the controller of GLCD} 

#define cmd_clear 0x01 // Command to clear the display
#define cmd_8bitmode 0x38 // Command to set parallel mode at 8 bits
#define cmd_line1 0x80 // Command to set the cursor in the line 1
#define cmd_line3 0x88 // Command to set the cursor in the line 3
#define cmd_line2 0x90 // Command to set the cursor in the line 2
#define cmd_line4 0x98  // Command to set the cursor in the line 4
#define cmd_displayON 0x0C // Command to turn on the display
#define cmd_displayOFF 0x80 // Command to turn off the display
#define cmd_home 0x2 //set the cursor in the initial position

//Signature of the functions used in the program
void delayAx5ms(unsigned char var_delay); //Signature process for delay creation
void glcd_instruction(unsigned char instruction) ; //Signature process for instruction sending
void glcd_data(unsigned char data); //Signature process for data sending
void glcd_init(void); //Signature process for GLCD initialization
void glcd_message(unsigned char message[]);
void init_IRQ(void);
void init_ADC(void);
void init_PWM(void);
void init_SCI(void);
void init_I2C(void);
void sampling_pf();
float sampling_sin(unsigned char chanel);
unsigned char contador = 0;

//const unsigned char t[]="************************************************************";
//const unsigned char t1[]="					POWER MEASUREMENT      					 ";
//const unsigned char t2[]="				DEVELOPED BY JULIAN SANTOS					 ";
unsigned char voltage_measurement;
unsigned char temp[10]="" ;
unsigned char temp2[10]="";
unsigned char flag = 0;
unsigned int Duty_cycle[12];
float Vrms,Irms,PF,Pact,Prea,cosT,sinT,Pcom;
unsigned int i, j;
float signal[100],a;


void main(void) {
	SOPT1=0; // Disenabling WDT
	asm("LDHX #0x4B0"); 
	asm("TXS");  // Setting new direction of stack pointer
	init_IRQ(); // initial configuration IRQ
	init_ADC(); // initial configuration ADC
	//init_PWM(); // initial configuration PWM
	//init_SCI(); // initial configuration SCI
	//init_I2C(); // initial configuration I2C
	glcd_init(); // initial configuration GLCD
	glcd_message("Power Analysis"); // Displaying initial message
	
	while(1){ 
		
		Vrms = sampling_sin(1); // Function to obtain the RMS value of the voltage
		Irms = sampling_sin(2); // Function to obtain the RMS value of the current
		sampling_pf(); // Function to measure the time on of the MCU
		//Vrms = Vrms;
		Irms =Irms*3.84;
		//PF=(30*3.141)/180;
		cosT=cosf((PF*3.1416/180));  // Power Factor
		sinT=sinf(PF); // Sin(theta[V]-theta[I])
		Pcom = Vrms * Irms; // Complex power
		Pact = (Pcom * cosT); // Active power
		Prea = (Pcom * sinT); // Reactive power		 
		 
		sprintf(temp,"%3.2f",Vrms);		
		temp[5]='\0'; // sure the end of the array
		glcd_instruction(cmd_line2); // Setting cursor at second line
		glcd_message("v=");
		glcd_message(temp);	
		glcd_message("V ");
		
		sprintf(temp,"%3.2f",Irms);		
		temp[5]='\0'; // sure the end of the array
		glcd_message("i=");
		glcd_message(temp);
		glcd_message("A ");
		
		glcd_instruction(cmd_line3); // Setting cursor at second line
		sprintf(temp,"%6.2f",Pact);		
		temp[5]='\0'; // sure the      end of the array
		glcd_message("P=");
		glcd_message(temp);
		glcd_message("W ");
		
		sprintf(temp,"%3.1f",Prea);		
		temp[5]='\0'; // sure the end of the array
		glcd_message("Q=");
		glcd_message(temp);
		glcd_message("Va");
		
		glcd_instruction(cmd_line4+2); // Setting cursor at second line
		sprintf(temp,"%6.4f",cosT);		
		temp[5]='\0'; // sure the end of the array
		glcd_message("PF=");
		glcd_message(temp);
	}
   
}


void init_IRQ(void){  /* Function to initialize the IRQ*/
   IRQSC=0b01110100 ; // ACK clearing flag 01110100B
       // Description:  Not enabling interrupt, pin enable, + Edge, Mode 0 
}

//***************************************************************************************
//          					POWER MEASUREMENT
//      				DEVELOPED BY JULIAN SANTOS BUSTOS
// V=		I=			PF=   		P=				Q=  		S=

void init_ADC(void){ /* Function to initialise the ADC*/ 
	ADCCFG = 0b00010100; // 12 bit resolution
	APCTL1 = 0b00100001; // CH0 & CH5
	ADCSC1 = 0b00100000; // Continuous Conversion
}

void init_PWM(void){ // Sin realizar /* Function to initialise the PWM*/
	
}

void init_SCI(void){ //  /* Function to initialize the SCI*/
	
	
   // SCI2BDH = 0;
   // SCI2BDL = 60 ;// Setting baud rate to 9600 bps
   // SCI2C2_TE = 1; // Enabling transmision 
    
    //SCI1C2_RE=1; 
    // Enabling Transmision 
}

void init_I2C(void){ // Sin realizar /* Function to initialize the I2C*/
	
}

void glcd_init(void){ /* Function to initialize the GLCD*/  
	TRIS_BUS = 0xFF;  //Setting data direction of GLCD data bus
	TRIS_RS = 1; // Setting data direction of  RS GLCD control pin 
	TRIS_Enable=1; // Setting data direction of Enable GLCD control pin
	delayAx5ms(8); // For Power up	
	glcd_instruction(cmd_8bitmode); // 8 bit operation 
	glcd_instruction(cmd_displayON); // Turn on the display
	glcd_instruction(cmd_clear); // Clearing display
	glcd_instruction(cmd_line1); // Setting cursor at first line
}

void glcd_instruction(unsigned char instruction){
	GLCD_RS=0;	
	GLCD_BUS=instruction;
	GLCD_Enable=1;
	delayAx5ms(4);
	GLCD_Enable=0;
	delayAx5ms(4);
}

void glcd_data(unsigned char data){
	GLCD_RS=1;
	GLCD_BUS=data;
	GLCD_Enable=1;
	delayAx5ms(4);
	GLCD_Enable=0;
	delayAx5ms(4);
}

void glcd_message(unsigned char message[]){
	int size  = sizeof(message)/1;
	int counter=0;
	for(counter=0;message[counter]!='\0'; counter++){
		glcd_data(message[counter]);
	}
}

float sampling_sin(unsigned char channel){
	// develop an if script to select the channel (1 or x)
	if(channel==1){
		ADCSC1 = 0b00100000; // Input channel 0
	}
	else{
		ADCSC1 = 0b00100101; // Input channel 5
	}
			
	while(ADCSC1_COCO == 0); // wait until conversion is complet
	
	for(contador=0; contador<100; contador++){
		signal[contador]=ADCR;
		asm{
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
		}
	}
	// develop a code to calculate the average of the pick function
	for (i=0 ; i < 100; ++i) //code for AA
	    {
	        for (j = i + 1; j < 100; ++j)
	        {
	            if (signal[i] > signal[j])
	            {
	                a =  signal[i];
	                signal[i] = signal[j];
	                signal[j] = a;
	            }
	        }
	    }
	
	return (((((signal[99]+signal[98]+signal[97]+signal[96]+signal[95]+signal[94])/(4095*6))*5)-2.5));
}

void sampling_pf(){
	//asm{BIL *}
	TRIS_LOAD = 1;
	TPM1SC = 0b00000010; //Temporizador programado para medir y actualizar medidas cada 0.5s
	TPM1MOD = 31250;
	TPM1SC_CLKSA = 0; // Start Clock
	i=0;
	Duty_cycle[1]=0;
	Duty_cycle[2]=0;
	Duty_cycle[3]=0;
	Duty_cycle[4]=0;
	Duty_cycle[5]=0;
	Duty_cycle[6]=0;
	Duty_cycle[7]=0;
	Duty_cycle[8]=0;
	while(i<7){
	// while(TPM1SC_TOF == 0);
	 //TPM1SC_TOF = 0;
	 TPM1CNT=0;
	 TPM1SC_CLKSA = 1;
	 //asm("H: BIL H")
	 //asm("L: BIH L")
	 delayAx5ms(1);
	 Duty_cycle[i]=TPM1CNT;
	 TPM1SC_CLKSA = 0;
	 /*
		 glcd_instruction(cmd_line3); // Setting cursor at second line
		 sprintf(temp,"%d",Duty_cycle);		
		 temp[9]='\0'; // sure the      end of the array
		 glcd_message("P=");
		 glcd_message(temp);
		 
		 sprintf(temp,"%6.2f",PF);		
		 temp[5]='\0'; // sure the end of the array
		 glcd_message(" d=");
		 glcd_message(temp);
	 */	 
	 i++;
	}
	 PF=(Duty_cycle[1]+Duty_cycle[2]+Duty_cycle[3]+Duty_cycle[4]+Duty_cycle[5])/5;
	 PF*=0.0108;// Conversion to degrees	
}

void delayAx5ms(unsigned char var_delay){  // Process To delay creation *DEVELOPED BY JULIAN SANTOS*
	asm{
		PSHH ; //save context H
		PSHX ; //save context X
		PSHA ; //save context A
		LDA var_delay ; // 2 cycles
		delay_2:    LDHX #$1387 ; //3 cycles 
		delay_1:    AIX #-1  //cycles
		CPHX #0 ; //3 cycles  
		BNE delay_1 ; // 3 cycles
		DECA ; //1 cycle
		CMP #0 ; // 2 cycles
		BNE delay_2  ; //3 cycles
		PULA ; // restore context A
		PULX ; // restore context X
		PULH ; // restore context H
	}
}

