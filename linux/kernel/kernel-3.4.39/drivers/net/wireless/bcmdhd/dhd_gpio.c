
#include <osl.h>
#include <dngl_stats.h>
#include <dhd.h>

// Added by ddanggzi
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>


#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>
// end



extern void brcm4356_wlan_set_carddetect(int onoff);

struct wifi_platform_data dhd_wlan_control = {0};
//extern void sw_mci_rescan_card(int state);
//extern int get_host_wake_irq(void);
//extern int ap621x_wifi_on_ctrl(int state);

#ifdef CUSTOMER_OOB
uint bcm_wlan_get_oob_irq(void)
{
	uint host_oob_irq = 0;

	//host_oob_irq = get_host_wake_irq();
	printf("host_oob_irq: %d\n", host_oob_irq);
	host_oob_irq = gpio_to_irq(IRQ_GPIO_C_START+11);  // dummy gpio (E¢¬¡¤I¢¯¢® ¨ú¨¡©ö¡ì¡¤¡¾ ¢¯¡þ¡Æa X)

	// .start = CFG_IO_WIFI_HOST_WAKE_UP_IRQ_NUM,
   //     .end   = CFG_IO_WIFI_HOST_WAKE_UP_IRQ_NUM,

	return host_oob_irq;
}

uint bcm_wlan_get_oob_irq_flags(void)
{
	uint host_oob_irq_flags = 0;

#ifdef HW_OOB
#ifdef HW_OOB_LOW_LEVEL
	host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL | IORESOURCE_IRQ_SHAREABLE;
#else
	host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE;
#endif
#else
	host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_SHAREABLE;
#endif

	host_oob_irq_flags = (IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE) & IRQF_TRIGGER_MASK;
	printf("host_oob_irq_flags=0x%X\n", host_oob_irq_flags);

	return host_oob_irq_flags;
}
#endif

int bcm_wlan_set_power(bool on)
{
	int err = 0;

	if (on) {
		printf("======== PULL WL_REG_ON HIGH! ========\n");

        // Modified by ddanggzi
		//ap621x_wifi_on_ctrl(1);
		nxp_soc_gpio_set_out_value((PAD_GPIO_C+25), 1);
		// end
		
		/* Lets customer power to get stable */
		mdelay(100);
		
	} else {
		printf("======== PULL WL_REG_ON LOW! ========\n");
		// Modified by ddanggzi
		//ap621x_wifi_on_ctrl(0);
	//	nxp_soc_gpio_set_out_value((PAD_GPIO_C+25), 0);  
			nxp_soc_gpio_set_out_value((PAD_GPIO_C+25), 0);  
		// end
	}

	return err;
}

int bcm_wlan_set_carddetect(bool present)
{
	int err = 0;

// modified by ddanggzi
#if 0
	if (present) {
		printf("======== Card detection to detect SDIO card! ========\n");
		sw_mci_rescan_card(1);	
	} else {
		printf("======== Card detection to remove SDIO card! ========\n");
		sw_mci_rescan_card(0);
	
	}
#else
	brcm4356_wlan_set_carddetect(present);
	msleep(500); /* wait for carddetect */
	
	if (present) {
		printf("======== Card detection to detect SDIO card! ========\n");
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+22, NX_GPIO_PADFUNC_1); //SDMMC1_CLK
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+23, NX_GPIO_PADFUNC_1); //SDMMC1_CMD
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+24, NX_GPIO_PADFUNC_1); // DATA[0]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+25, NX_GPIO_PADFUNC_1); // DATA[1]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+26, NX_GPIO_PADFUNC_1); // DATA[2]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+27, NX_GPIO_PADFUNC_1); // DATA[3]
		msleep(100);
	} else {
		printf("======== Card detection to remove SDIO card! ========\n");
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+22, NX_GPIO_PADFUNC_1); //SDMMC1_CLK
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+23, NX_GPIO_PADFUNC_0); //SDMMC1_CMD
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+24, NX_GPIO_PADFUNC_0); // DATA[0]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+25, NX_GPIO_PADFUNC_0); // DATA[1]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+26, NX_GPIO_PADFUNC_0); // DATA[2]
		nxp_soc_gpio_set_io_func(PAD_GPIO_D+27, NX_GPIO_PADFUNC_0); // DATA[3]
	}
#endif

	return err;
}

int bcm_wlan_get_mac_address(unsigned char *buf)
{
	int err = 0;

	printf("======== %s ========\n", __FUNCTION__);
#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return err;
}

#ifdef CONFIG_DHD_USE_STATIC_BUF
extern void *bcmdhd_mem_prealloc(int section, unsigned long size);
void* bcm_wlan_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	alloc_ptr = bcmdhd_mem_prealloc(section, size);
	if (alloc_ptr) {
		printf("success alloc section %d, size %ld\n", section, size);
		if (size != 0L)
			bzero(alloc_ptr, size);
		return alloc_ptr;
	}
	printf("can't alloc section %d\n", section);
	return NULL;
}
#endif

struct cntry_locales_custom {
	char iso_abbrev[WLC_CNTRY_BUF_SZ];	/* ISO 3166-1 country abbreviation */
	char custom_locale[WLC_CNTRY_BUF_SZ];	/* Custom firmware locale */
	int32 custom_locale_rev;		/* Custom local revisin default -1 */
};

static struct cntry_locales_custom brcm_wlan_translate_custom_table[] = {
	/* Table should be filled out based on custom platform regulatory requirement */
	{"",   "XT", 49},  /* Universal if Country code is unknown or empty */
	{"US", "US", 0},
};

static void *bcm_wlan_get_country_code(char *ccode)
{
	struct cntry_locales_custom *locales;
	int size;
	int i;

	if (!ccode)
		return NULL;

	locales = brcm_wlan_translate_custom_table;
	size = ARRAY_SIZE(brcm_wlan_translate_custom_table);

	for (i = 0; i < size; i++)
		if (strcmp(ccode, locales[i].iso_abbrev) == 0)
			return &locales[i];
	return NULL;
}

int bcm_wlan_set_plat_data(void) {
	printf("======== %s ========\n", __FUNCTION__);
	dhd_wlan_control.set_power = bcm_wlan_set_power;
	dhd_wlan_control.set_carddetect = bcm_wlan_set_carddetect;
	dhd_wlan_control.get_mac_addr = bcm_wlan_get_mac_address;
#ifdef CONFIG_DHD_USE_STATIC_BUF
	dhd_wlan_control.mem_prealloc = bcm_wlan_prealloc;
#endif
	return 0;
}

