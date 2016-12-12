#ifndef Board_h
#define Board_h

#if (LUMINARDO_REV) && (LUMINARDO_REV >= 2)
    #define RTC_INT_PIN		1	//PB1
    #define VFD_SPRK_PIN	3	//PB3
#else
    #define VFD_SPRK_PIN    	13 	//PD5
    #define RTC_INT_PIN		3	//PB3
#endif

#define TEMP_SNSR_PIN 		24	//PA0
#define VFD_CS_PIN   		29 	//PA5
#define VFD_SCLK_PIN 		28 	//PA4
#define VFD_DATAI_PIN 		19 	//PC3
#define VFD_DATAO_PIN 		20 	//PC4
#define VFD_SHDN_PIN    	26 	//PA2
#define VFD_R_PIN       	15 	//PD7
#define VFD_G_PIN       	14 	//PD6
#define VFD_B_PIN       	12 	//PD4

#define VFD_PWR_EN_PIN      	27 	//PA3
#define VFD_LIGHT_SNR_PIN    	31	//PA7
#define VFD_IR_REC_PIN		25	//PA1
#define SPI_SS2_PIN		30	//PA6
#define SPI_SCK_PIN		7	//PB7
#define SPI_MISO_PIN		6	//PB6
#define SPI_MOSI_PIN		5	//PB5
#define SPI_SS_PIN		4	//PB4

#define PIR_PIN			2	//PB2
#define MAX_INT_PIN		18	//PC2

#define FAILED_TEMPERATURE -999

enum enum_DsState
{
    dsIdle,
    dsTempConversion,
    dsInitDelay
};

#endif
