/*
 * drivers/usb/hub/usb2514.c
 *
 * Copyright (c) 2015 Usolex Co., Ltd.
 *	Kwangwoo Lee <ischoi@usolex.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <asm/uaccess.h>




//#include <linux/module.h>
//#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/slab.h>
//#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
//#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

//#include <mach/platform.h>
//#include <mach/devices.h>




// modified by ddanggzi
#if 0 
#define	USB_HUB_RESET_PIN			(PAD_GPIO_E+7)
#define	USB_HUB_PORT1_PWR_EN		(PAD_GPIO_E+14)
#define	USB_HUB_PORT2_PWR_EN		(PAD_GPIO_E+15)
#define	USB_HUB_PORT3_PWR_EN		(PAD_GPIO_E+19)
#else 
#define	USB_HUB_RESET_PIN			(PAD_GPIO_D+29) // for Rosemary
#define	USB_HUB_PORT1_PWR_EN		(PAD_GPIO_E+14)
#define	USB_HUB_PORT2_PWR_EN		(PAD_GPIO_E+15)
#define	USB_HUB_PORT3_PWR_EN		(PAD_GPIO_E+19)

#endif
// end


struct usb2514 {
	struct i2c_client	*client;
	struct delayed_work resume_work; // added by ddanggzi
	u16			model;
	int	resume_delay_ms;     
};



static struct i2c_client	*this_client;

typedef struct 
{
	u8 reg;
	u8 value;
} i2cparam;

//finevu blackbox  mode
i2cparam usb2514_default_setting[] =
{
	{0x00, 0x24},	{0x01, 0x04},	{0x02, 0x12},	{0x03, 0x25},	{0x04, 0xB3},	{0x05, 0x0B},	{0x06, 0x9B},	{0x07, 0x20},
	{0x08, 0x02},	{0x09, 0x00},	{0x0A, 0x04},	{0x0B, 0x04},	{0x0C, 0x01},	{0x0D, 0x32},	{0x0E, 0x01},	{0x0F, 0x32},
	{0x10, 0x32},	{0xF6, 0x00},	{0xF8, 0x00},	{0xFB, 0x21},	{0xFC, 0x43},	{0xFF, 0x01},
};

//finevu usb mode
i2cparam usb2514_finevu_on_setting[] =
{
	{0x00, 0x24},	{0x01, 0x04},	{0x02, 0x12},	{0x03, 0x25},	{0x04, 0xB3},	{0x05, 0x0B},	{0x06, 0x9B},	{0x07, 0x20},
	{0x08, 0x02},	{0x09, 0x00},	{0x0A, 0x00},	{0x0B, 0x00},	{0x0C, 0x01},	{0x0D, 0x32},	{0x0E, 0x01},	{0x0F, 0x32},
	{0x10, 0x32},	{0xF6, 0x00},	{0xF8, 0x00},	{0xFB, 0x21},	{0xFC, 0x43},	{0xFF, 0x01},
};


u8 usb2514_setting[] =
{
	0x24,0x04,0x12,0x25,0xB3,0x0B,0x9B,0x20,
	0x02,0x00,0x00,0x00,0x01,0x32,0x01,0x32,
	0x32,0x00,0x00,0x21,0x43,0x01,
};

struct usb2514 *ts;


static void gs_ts_work_resume(struct work_struct *work)
{
   int i;
	//struct key_info *key = container_of(work, struct key_info, resume_work.work);
	// struct usb2514 *ts = container_of(work, struct usb2514,resume_work.work);
	// struct usb2514 *ts = container_of(work, struct usb2514,work.work);
   
   lldebugout("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\r\n");


	// i2c_smbus_write_byte_data(ts->client, usb2514_finevu_on_setting[i].reg, usb2514_finevu_on_setting[i].value);
/*
    nxp_soc_gpio_set_io_dir(USB_HUB_RESET_PIN, 1);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
    mdelay(10);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 0);
    mdelay(10);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
    mdelay(700);

    // i2c를 gslX680.c의 gsl_ts_resume()을 참조해서 스캐쥴 이용할   것 (숙제) ************************
    for (i = 0; i < (sizeof(usb2514_finevu_on_setting)/sizeof(i2cparam)); i++)
    {
	   i2c_smbus_write_block_data(ts->client, usb2514_finevu_on_setting[i].reg, 1, &(usb2514_finevu_on_setting[i].value));
      // i2c_smbus_write_byte_data(client, usb2514_finevu_on_setting[i].reg, usb2514_finevu_on_setting[i].value);
	}
	*/
}


// ddanggzi -- end


#ifdef CONFIG_PM
//static int usb2514_suspend(struct device *dev){
static int usb2514_suspend(struct i2c_client *client, pm_message_t mesg){

	lldebugout("usb2514_suspend -- enter\r\n");
	//mdelay(200);
		
	return 0;
}

/*
static int gsl_ts_resume(struct i2c_client *client)
{
    struct gsl_ts *ts = i2c_get_clientdata(client);
	schedule_work(&ts->resume_work);
	return 0;
}
*/

/*
static int gsl_ts_resume(struct i2c_client *client)
{
    struct gsl_ts *ts = i2c_get_clientdata(client);
	schedule_work(&ts->resume_work);
	return 0;
}
*/

//static int usb2514_resume(struct device *dev){

/*
static int ili210x_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (device_may_wakeup(&client->dev))
		disable_irq_wake(client->irq);

	return 0;
}

*/


static int usb2514_resume(struct i2c_client *client)
//static int usb2514_resume(struct platform_device *pdev)
//static int usb2514_resume(struct device *dev)
{
	//struct usb2514 *ts = platform_get_drvdata(pdev);
	//struct i2c_client *client = to_i2c_client(dev);
	//struct usb2514 *ts = i2c_get_clientdata(client);

	int i = 0;

    nxp_soc_gpio_set_io_dir(USB_HUB_RESET_PIN, 1);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
    mdelay(10);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 0);
    mdelay(10);
    nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
    mdelay(700);

    // i2c를 gslX680.c의 gsl_ts_resume()을 참조해서 스캐쥴 이용할   것 (숙제) ************************
    for (i = 0; i < (sizeof(usb2514_finevu_on_setting)/sizeof(i2cparam)); i++)
    {
	   i2c_smbus_write_block_data(client, usb2514_finevu_on_setting[i].reg, 1, &(usb2514_finevu_on_setting[i].value));
      // i2c_smbus_write_byte_data(client, usb2514_finevu_on_setting[i].reg, usb2514_finevu_on_setting[i].value);
	}
    
//	printk("usb2514_resume -- complate\r\n");
    lldebugout("usb2514_resume -- complate\r\n");

    //schedule_delayed_work(&ts->resume_work, msecs_to_jiffies(5000));

	return 0;
}

//static SIMPLE_DEV_PM_OPS(usb2514_pm_ops, usb2514_suspend, usb2514_resume);
//static UNIVERSAL_DEV_PM_OPS(usb2514_pm_ops, usb2514_suspend, usb2514_resume, NULL);

#endif
// ddanggzi --- end


static int __devinit usb2514_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
    struct usb2514 *ts1;
	int err = 0;
	int i, j, ret;
	u16 value;;
  // u8 block_buffer[I2C_SMBUS_BLOCK_MAX + 1];
   //s32 nread;
    

	printk("%s: Enter\n",__FUNCTION__);


   if (!i2c_check_functionality(client->adapter,
					I2C_FUNC_SMBUS_READ_WORD_DATA))
	   return -EIO;

   // Enable Power of Hub Port
  // disabled by ddanggzi
  /*
   nxp_soc_gpio_set_out_value(USB_HUB_PORT1_PWR_EN, 1);
   nxp_soc_gpio_set_out_value(USB_HUB_PORT2_PWR_EN, 1);
   nxp_soc_gpio_set_out_value(USB_HUB_PORT3_PWR_EN, 1);
   */
  // end

   // reset the hub chip
   nxp_soc_gpio_set_io_dir(USB_HUB_RESET_PIN, 1);
   nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
   mdelay(10);
   nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 0);
   mdelay(10);
   nxp_soc_gpio_set_out_value(USB_HUB_RESET_PIN, 1);
   mdelay(10);
/*
   for (i = 0; i < (sizeof(usb2514_default_setting)/sizeof(i2cparam)); i++)
   {
	   i2c_smbus_write_block_data(client, usb2514_default_setting[i].reg, 1, &(usb2514_default_setting[i].value));
   }
*/
   for (i = 0; i < (sizeof(usb2514_finevu_on_setting)/sizeof(i2cparam)); i++)
   {
	   i2c_smbus_write_block_data(client, usb2514_finevu_on_setting[i].reg, 1, &(usb2514_finevu_on_setting[i].value));
   }
   
   //ts = kzalloc(sizeof(struct usb2514), GFP_KERNEL);

   ts1 = kzalloc(sizeof(struct usb2514), GFP_KERNEL);

   ts1->client = client;
   INIT_DELAYED_WORK(&ts1->resume_work, gs_ts_work_resume);
   
   //INIT_DELAYED_WORK(&key->resume_work, nxp_key_resume_work);

   i2c_set_clientdata(client, ts); 
   
   this_client = client;   
   
   kfree(ts);
   return 0;

err_free_mem:
   kfree(ts);
   return err;
	
}

static int __devexit usb2514_remove(struct i2c_client *client)
{
	kfree(ts);

	return 0;
}


static const struct i2c_device_id usb2514_idtable[] = {
	{ "usb2514", 0 },
	{ }
};


MODULE_DEVICE_TABLE(i2c, usb2514_idtable);

// Modified by ddanggzi 
#if 0
static struct i2c_driver usb2514_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "usb2514"
	},
	.id_table	= usb2514_idtable,
	.probe		= usb2514_probe,
	.remove		= __devexit_p(usb2514_remove),
};
#else

/*
static struct i2c_driver usb2514_driver = {
	.driver = {
		.name	= "usb2514",
		.owner	= THIS_MODULE,
#ifdef CONFIG_PM
		.pm     = &usb2514_pm_ops,
#endif
	},
	.probe		= usb2514_probe,
	.remove		= __devexit_p(usb2514_remove),
	.id_table	= usb2514_idtable,
};
*/


static struct i2c_driver usb2514_driver = {
	.driver = {
		.name = "usb2514",
		.owner = THIS_MODULE,
	},
	.suspend	= usb2514_suspend,
	.resume = usb2514_resume,
	.probe		= usb2514_probe,
	.remove 	= __devexit_p(usb2514_remove),
	.id_table	= usb2514_idtable,
};


#endif
// ddanggzi -- end


static int __init usb2514_init(void)
{
    int ret;
	printk("==usb2514_init==\n");
	ret = i2c_add_driver(&usb2514_driver);
	printk("ret=%d\n",ret);
	return ret;
}
static void __exit usb2514_exit(void)
{
	printk("==usb2514_exit==\n");
	i2c_del_driver(&usb2514_driver);
	return;
}

module_init(usb2514_init);
module_exit(usb2514_exit);


//module_i2c_driver(usb2514_driver);


MODULE_AUTHOR("Joseph Choi <ischoi@usolex.com>");
MODULE_DESCRIPTION("USB Hub Driver");
MODULE_LICENSE("GPL");
