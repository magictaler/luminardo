#include <util/delay.h>
#include <Board.h>
#include <MVFDPanel_16S8D.h>

#define MAIN_LOOP_DELAY 200

volatile uint8_t sysState;      //current system state (mode)
volatile uint8_t redValue;
volatile uint8_t greenValue;
volatile uint8_t blueValue;

enum enum_SysState
{
    sysSelfTest,
    sysStandby,
    sysDispParams,
    sysFailure
};


MVFD_16S8D vfd(VFD_CS_PIN, VFD_SCLK_PIN, VFD_DATAI_PIN, VFD_SHDN_PIN, VFD_PWR_EN_PIN);    //VFD display

SIGNAL(PCINT0_vect)
{
  
  
}

void setup()
{
  vfd.initLED(VFD_R_PIN, VFD_G_PIN, VFD_B_PIN);
  vfd.initLightSensor(VFD_LIGHT_SNR_PIN);
  vfd.initTone(VFD_SPRK_PIN);
  vfd.initIR_RC(VFD_IR_REC_PIN);
  
  vfd.standby(false);
  vfd.powerdown(false);

  redValue = 0;
  greenValue = 0;
  blueValue = 0;

  vfd.setLED(redValue, greenValue, blueValue);
  
  pinMode(VFD_DATAO_PIN, INPUT);
  vfd.reset();
  
  vfd.setB_LED(255);  
  vfd.playTone(VFD_DEF_TONE);
  _delay_ms(100);
  vfd.setB_LED(0);    
  vfd.playNoTone();  
  _delay_ms(100);
  
  vfd.setB_LED(255);  
  vfd.playTone(VFD_DEF_TONE);
  _delay_ms(100);
  vfd.setB_LED(0);    
  vfd.playNoTone();  
  _delay_ms(500);

  vfd.setR_LED(255);    
  vfd.playTone(VFD_DEF_TONE);
  _delay_ms(100);
  vfd.setR_LED(0);      
  vfd.playNoTone();  
  _delay_ms(100);
  
  vfd.setG_LED(255);      
  vfd.playTone(VFD_DEF_TONE);
  _delay_ms(100);
  vfd.setG_LED(0);          
  vfd.playNoTone();  
  
  sysState = sysSelfTest;
  
//  uint8_t mode = 1;
//  EICRA = (EICRA & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
//  EIMSK |= (1 << INT1);
  
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT1);
}

void updateLEDs()
{
  blueValue++;
  if (blueValue > 60)
  {
    blueValue = 0;  
  }
  if (blueValue < 30)
  {
    vfd.setLED(blueValue/1.17, blueValue/8.6, blueValue);
  }else
  {
    uint8_t newVal = (30 - (blueValue - 30));
    vfd.setLED(newVal/1.17, newVal/8.6, newVal);
  }
}

void loop()
{  
    uint16_t lightSen = 300;
    uint8_t lightConditions = true;
    uint8_t loopCntr = MAIN_LOOP_DELAY / 10;
    
    while (1)
    {
        loopCntr++;
        updateLEDs();
        
        if (loopCntr >= 10)
        {
            loopCntr = 0;
            
            switch (sysState)
            {
                case sysSelfTest:
                    {
                        if (vfd.testStep() == vfd.COMPLETED)
                        {
                            sysState = sysDispParams;
                            redValue = 0;
                            greenValue = 1;
                        }

                    }
                    break;
                case sysDispParams:
                    {
                        vfd.setCur(0);
                        lightSen = vfd.getLightSensorVal();
                        vfd.print((int)lightSen, DEC);

                        for (uint8_t i = 0; i < VFD_DIGITS-2; i++) 
                          vfd.write(0x20);
                    }
                default:
                    ;       
            }
        //Handling RC commands
        decode_results* rc_code = vfd.getIR_RC_Code();
        if (rc_code != NULL)
        {
            vfd.setLED(redValue, greenValue, 255);

            vfd.setCur(0);
            if (rc_code->decode_type == NEC)
            {
                vfd.print("NEC  ");
            }else if  (rc_code->decode_type == SONY)
            {
                vfd.print("SONY ");
            }else if (rc_code->decode_type == RC5)
            {
                vfd.print("RC5  ");
            }else if (rc_code->decode_type == RC6)
            {
                vfd.print("RC6  ");
            }else 
                vfd.print("UNKN ");

            vfd.print((int)rc_code->value, DEC);

            vfd.write(0x20);
            vfd.write(0x20);

            vfd.resumeIR_RC();
        }
        if (lightSen < 200) 
        {
            //Dark conditions
            if (lightConditions == true)
            {
                vfd.displayOnCmd(3);
                lightConditions = false;
            }

        }else if (lightSen > 300)
        {
            //Light conditions
            if (lightConditions == false)
            {
                vfd.displayOnCmd(7);
                lightConditions = true;
            }
        }
        
            
        }

//        longBeepAsync();

        _delay_ms(MAIN_LOOP_DELAY / 10);
        
        
        //greenValue = 2;
    }
}

