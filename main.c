#include "stm32f4xx.h"

static uint8_t counter1=0;
void can_init(void);
void EXTI0_IRQHandler(void);
void CAN1_RX0_IRQHandler(void);

int main(void)
{
	can_init();

  while (1){
  }
}

void can_init(void)
{
//   ****   PORT CONFIGURATION   Port A Pin11,12***
RCC->AHB1ENR |=0x01;

GPIOA->MODER &= 0xFEBFFFFF;
GPIOA->MODER |= 0x02800000;

GPIOA->OTYPER &= 0xFFFFEFFF;//12 Push Pull output

GPIOA->PUPDR |= ((1<<24)|(1<<22));//  12 and 11 PULL UP
GPIOA->PUPDR &=~0xFD7FFFFF;

GPIOA->AFR[1]|= ((1<<12)|(1<<15)|(1<<16)|(1<<19)) ;// Aternate-function Set Can Bus
GPIOA->AFR[1]&= ((1<<13)|(1<<14)|(1<<17)|(1<<18));

/*Configure GPIO pin : PA0 INPUT  */
GPIOA->MODER &= 0xFFFFFFFC;
GPIOA->PUPDR &= 0xFFFFFFFC;// PULL UP
GPIOA->PUPDR |= 0x01;
//   ***   GPIO PORT A Line 1 INTERRUPT CONFIGUARATION  ***
SYSCFG->EXTICR[0] &= 0xFFFFFFF0;
EXTI->IMR |= 0x01;//Interrupt request from line 1 is not masked
EXTI->FTSR |= 0x01;// Falling trigger enabled
NVIC->ISER[0] |=(1<<6);
NVIC_SetPriority(6,1);


    //   ***   CAN INTERRUPT CONFIGUARATION  ***
CAN1->IER |=0x02;
NVIC->ISER[0] |= (1<<20);//Set Interrupt RX0
//NVIC_EnableIRQ(CAN1_RX0_IRQn);//NVIC_EnableIRQ(CAN1_TX_IRQn); Method 2
NVIC_SetPriority(CAN1_RX0_IRQn,2);

//   ***   CAN CONFIGURATION   ***
RCC->APB1ENR &= (1<<25); //Enable Can bus CLock
CAN1->MCR |=(1<<6);//automatic bus off management
CAN1->MCR |= (1<<2);//Priority of transmit message driven by the request order
CAN1->MCR |=0x01; //initialization mode set
CAN1->MCR &=0xFFFFFFFD; //Reset sleep Mode
while ((0x02==(CAN1->MSR & 0x02)) & (0x01!=(CAN1->MSR & 0x01))){
}
CAN1->BTR = 0x00060004;

CAN1->sTxMailBox[0].TIR = (0x100<<21); //identifier 100
CAN1->sTxMailBox[0].TDTR &=(0xFFFFFFF8);
CAN1->sTxMailBox[0].TDTR |=(0x00000001);//data length 1bytes
CAN1->sTxMailBox[0].TDLR=0;
CAN1->sTxMailBox[0].TDHR=0;

//   ***   FILTER CONFIGURATION   ***

CAN1->FMR |=0x01;//filter in initialization mode
CAN1->FM1R |= 0x03;// filter 0,1 in list mode
CAN1->FS1R |=0x03;//Filter Scale 16 bit for 0,1 filter
CAN1->sFilterRegister[0].FR1=(0x100<<21);//list mode with ID100
CAN1->sFilterRegister[1].FR2=(0x100<<21);//list mode with ID100
CAN1->FA1R |=0x03; //Activate filter 0,1
CAN1->FFA1R &= 0xFFFFFFFC; //Assign FILTER 0,1 to FIFO 0
CAN1->FMR &=0xFFFFFFFE ;//activate all filter
CAN1->MCR &=0xFFFFFFFC  ;//switch to normal mode
}

void EXTI0_IRQHandler(void)
{
	if( CAN1->TSR & (0x04000000)){
			CAN1->sTxMailBox[0].TDLR=1;
			CAN1->sTxMailBox[0].TIR |=0x01;
			EXTI->PR |=0x01; //Clear Pending Bit
		}
}

void CAN1_RX0_IRQHandler (void){
	if(CAN1->RF0R & 0x03 ){
		if (CAN1->sFIFOMailBox[0].RIR>>21 == 0x100){
			if(CAN1->sFIFOMailBox[0].RDLR == 1){
				counter1++;
			}
       CAN1->RF0R |=0x20; //release mailbox
       }
	}
}
