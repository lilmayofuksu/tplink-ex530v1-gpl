#include <asm/io.h>

/* Register base address */
#define IO_PHYS				(0x10000000)
#define EN7523_IOMUX_1		(IO_PHYS + 0xFA20210)

void LAN_LED0_enable(void)
{
	unsigned int val;

#ifndef CONFIG_TP_IMAGE
	/* enable HW switch LED0 */
	val = readl(EN7523_IOMUX_1);
	val |= (0x1 << 3) | (0x1 << 5) | (0x1 << 7) | (0x1 << 9);
	writel(val, EN7523_IOMUX_1);
#endif /* !CONFIG_TP_IMAGE */
}

