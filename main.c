/*so do ket noi
                      |-------------|
            BUTTON -->|P1.0     P1.4|--> LOA
                      |             |
            BUTTON -->|P1.1     P1.5|--> LED
                      |             |
          |--> SCL -->|P1.6     P2.0|--> SDA --|
    BH1750|           |             |          | I2C_LCD
          |--> SDA -->|P1.7     P2.1|--> SCL --|
                      |-------------|
                        
 */

//Pham Tan Hoai1
#include "msp430g2553.h"
#include "delay.h"
#include "HW_I2C.h"
#include "SW_I2C.h"
#include "PCF8574.h"
#include "lcd.h"
#include "BH1750.h"

unsigned long cuongdoset= 30;

void lcd_3n(unsigned int x);
void intro(void);
void inset(void);
void GPIO_graceInit(void);
void BCSplus_graceInit(void);
void USCI_B0_graceInit(void);
void System_graceInit(void);
void WDTplus_graceInit(void);
void lcd_print(unsigned char x_pos, unsigned char y_pos, unsigned int value);


void main(void)
{
    unsigned int LX = 0x0000;
    unsigned int tmp = 0x0000;
    
    /* Stop watchdog timer from timing out during initial start-up. */
    WDTCTL = WDTPW | WDTHOLD; 
    GPIO_graceInit();
    BCSplus_graceInit();
    USCI_B0_graceInit();
    System_graceInit();
    WDTplus_graceInit();

    BH1750_init();                              // khoi dong cam bien
    LCD_init();                                 // khoi dong lcd
    LCD_clear_home();                           // xoa man hinh 
    intro();
    _BIS_SR(GIE);                                // cho phep ngat toan cuc
    while(1)
    {
  
      tmp = get_lux_value(cont_H_res_mode2, 20); // gan gia tri doc duoc cho bien tmp

        if(tmp > 10)
        {
            LX = tmp;
        }
        else
        {
            LX = get_lux_value(cont_H_res_mode2, 140); // gan gia tri doc duoc cho bien LX
        }
        if(LX < (cuongdoset-5))
        {
            P1OUT |= 0x20;                               //bat den o P1.5
            P1OUT |= 0x10;                               //bat loa o P1.4
            __delay_cycles(10000);
        }
        else if(LX > (cuongdoset+5))
        {
            P1OUT &= ~0x20;                                 // tat den o bit P1.5
            P1OUT &= ~0x10;                                 //tat loa o P1.4
        }
      LCD_goto(0, 1);
      LCD_putstr("Set Lux:");
      LCD_goto(11, 1);
      lcd_3n(cuongdoset);                       // xuat ra gia tri nguoi dung cai dat
      LCD_goto(0, 0);
      LCD_putstr("Lux:");
      lcd_print(11, 0, LX);                     // xuat gia tri ra lcd o vi tri (11,1)
      delay_ms(100);  
    }
}

//Chuong trinh phuc vu ngat

#pragma vector=PORT1_VECTOR
__interrupt void Port(void)
{
  if((P1IN& BIT0)==0)
  {
    while((P1IN & BIT0) != BIT0)        // chong doi nut
    __delay_cycles(5000); 
    cuongdoset += 5;
    P1IFG &= ~BIT0;
  }
  if((P1IN& BIT1)==0)
  {
    while((P1IN & BIT1) != BIT1)        // chong doi nut
    __delay_cycles(5000); 
    cuongdoset -= 5;
    P1IFG &= ~BIT1;
  }
}

void lcd_3n(unsigned int x)
{
  LCD_putchar((x/10000)+48);
  LCD_putchar(((x/1000) %10) +48);
  LCD_putchar(((x/100) %10) +48);
  LCD_putchar(((x/10) % 10) +48);
  LCD_putchar((x%10)+48);

}

void intro (void)
{
    LCD_goto(0, 0);
    LCD_putstr("BAO CAO CUOI KY");
    LCD_goto(1, 1);
    LCD_putstr("DO AN VI XU LY");
    delay_ms(5000);
    LCD_clear_home();
    
    LCD_goto(0, 0);
    LCD_putstr("DO CUONG DO");
    LCD_goto(8, 1);
    LCD_putstr("ANH SANG");
    delay_ms(5000);
    LCD_clear_home();
    
    LCD_goto(0, 0);
    LCD_putstr("GVHD:");
    LCD_goto(3, 1);
    LCD_putstr("TRAN HUU DANH");
    delay_ms(5000);
    LCD_clear_home();
    
    LCD_goto(0, 0);
    LCD_putstr("SVTH:");
    LCD_goto(2, 1);
    LCD_putstr("PHAM TAN HOAI");
    delay_ms(5000);
    LCD_clear_home();
    
}
void GPIO_graceInit(void)
{
    /* Port 1 Port Select 2 Register */
    P1SEL2 = BIT6 | BIT7;

    /* Port 1 Output Register */
    P1OUT &= ~BIT6 + BIT7; 

    /* Port 1 Port Select Register */
    P1SEL = BIT6 | BIT7;

    /* Port 1 Direction Register */
    P1DIR =0x40,0x80; 

    /* Port 2 Output Register */
    P2OUT&= ~BIT1 + BIT2;

    /* Port 2 Direction Register */
    P2DIR = 0;
    P1IE  |=   BIT0+BIT1;// Cho phep ngat tai chan
    P1IES |=   BIT0+BIT1;// Cau hinh ngat theo suon xuong
    P1IFG &=~ (BIT0+BIT1);// Xoa co ngat
    P1DIR |= BIT4 + BIT5; // khai bao ngo ra P1.4 va P1.5
    P1OUT&= ~(BIT4 + BIT5);
    
}


void BCSplus_graceInit(void)
{
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;

    if (CALBC1_1MHZ != 0xFF)
    {
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ;      //Set DCO to 1MHz 
        DCOCTL = CALDCO_1MHZ;
    }
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_0 | XCAP_1;
}


void USCI_B0_graceInit(void)
{
    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;

    UCB0CTL1 = UCSSEL_2 | UCSWRST;

    /* I2C Slave Address Register */
    UCB0I2CSA = BH1750_addr;

    /* Bit Rate Control Register 0 */
    UCB0BR0 = 20;

    /* Enable USCI */
    UCB0CTL1 &= ~UCSWRST;

}


void System_graceInit(void)
{
    __bis_SR_register(GIE);

}

void WDTplus_graceInit(void)
{
    WDTCTL = WDTPW | WDTHOLD;

}
void lcd_print(unsigned char x_pos, unsigned char y_pos, unsigned int value)
{
    char tmp[10] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20,'\0'};

    tmp[0] = ((value / 10000) + 48);
    tmp[1] = (((value / 1000) % 10) + 48);
    tmp[2] = (((value / 100) % 10) + 48);
    tmp[3] = (((value / 10) % 10) + 48);
    tmp[4] = ((value % 10) + 48);
    LCD_goto(x_pos, y_pos);
    LCD_putstr(tmp);
}

