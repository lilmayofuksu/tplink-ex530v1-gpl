#include <asm/tc3162.h>
#include <asm/io.h>

#ifdef CONFIG_ECNT_MULTIUPGRADE
#define MAXMULTILEDNUM 16
#define SYS_BLK_TIME				0x10

extern volatile unsigned long Jiffies;
extern volatile char finishMultiUpgrade;
uint8 multi_upgrade_gpio[MAXMULTILEDNUM];
uint8 internet_gpio;
uint8 dsl_gpio;
#endif

void LED_OEN(uint8 x)
{
	if(x>31){
		if(x>47){
			regWrite32(CR_GPIO_CTRL3,regRead32(CR_GPIO_CTRL3)|(1<<((x-48)*2)));
		}
		else{
			regWrite32(CR_GPIO_CTRL2,regRead32(CR_GPIO_CTRL2)|(1<<((x-32)*2)));
		}
		regWrite32(CR_GPIO_ODRAIN1,regRead32(CR_GPIO_ODRAIN1)|(1<<(x-32)));
	}
	else{
		if(x > 15){
			regWrite32(CR_GPIO_CTRL1,regRead32(CR_GPIO_CTRL1)|(1<<((x-16)*2)));
		}
		else{
			regWrite32(CR_GPIO_CTRL,regRead32(CR_GPIO_CTRL)|(1<<(x*2)));
		}
		regWrite32(CR_GPIO_ODRAIN,regRead32(CR_GPIO_ODRAIN)|(1<<(x)));
		
	}
}

void LED_OFF(uint8 x)
{
	if(x>31){
		regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)|(1<<(x-32)));
	}
	else{
		regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)|(1<<x));
	}
}

void LED_ON(uint8 x)
{
	if(x > 31){
		regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)& ~(1<<(x-32)));
	}
	else{
		regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)& ~(1<<x));
	}
}

void multiupgrade_blink() 
{
	int i = 0;
	static int MultiUpgradeTag = 0;
	uint8 multi_led;


	/*
		after multi-boot, system led will blink slowly
	*/
	if (finishMultiUpgrade){
		if (Jiffies & SYS_BLK_TIME){
			if(MultiUpgradeTag == 0){
				for(i=0; i<MAXMULTILEDNUM; i++){
					multi_led = multi_upgrade_gpio[i];
					if(multi_led != 0)
#if defined(TCSUPPORT_CT_WAN_PTM)
					{
						if(multi_led > 31)
							regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)& ~(1<<(multi_led-32)));
						else
							regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)& ~(1<<multi_led));
					}
#else
					LED_ON(multi_led);
#endif
				}
				MultiUpgradeTag = 1;
			}
		}
		else{
			if(MultiUpgradeTag == 1){
				for(i=0; i<MAXMULTILEDNUM; i++){
					multi_led = multi_upgrade_gpio[i];
					if(multi_led != 0)
#if defined(TCSUPPORT_CT_WAN_PTM)
					{
						if(multi_led > 31)
							regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)|(1<<(multi_led-32)));	
						else
							regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)|(1<<multi_led));
					}
#else
					LED_OFF(multi_led);

#endif
				}
				MultiUpgradeTag = 0;
			}
		}
	}

}

#ifdef CONFIG_ECNT_MULTIUPGRADE
/* parse gpio information from env*/
void multiupgrade_led_init(void)
{	
	int i = 0;
	char *multi_gpio;
	char temp[2] = {0};
	multi_gpio = getenv("multi_upgrade_gpio");
	for (i = 0; i < MAXMULTILEDNUM;i++)
	{
		temp[0] = multi_gpio[i<<2];
		temp[1] = multi_gpio[(i<<2)+1];
		multi_upgrade_gpio[i] = simple_strtoul(temp, NULL, 16);
	}	
	internet_gpio =  simple_strtoul(getenv("internet_gpio"), NULL, 16);
	dsl_gpio = simple_strtoul(getenv("dsl_gpio"), NULL, 16);

}
#endif

void lan_led_init(void)
{
	LAN_LED0_enable();
}

void led_init(void)
{	
	lan_led_init();
	
#ifdef CONFIG_ECNT_MULTIUPGRADE
	multiupgrade_led_init();
#endif
}

