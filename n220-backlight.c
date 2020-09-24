#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/pci.h>

MODULE_LICENSE("Dual BSD/GPL");

#define LEVEL_MIN 0
#define LEVEL_MAX 8
#define RAW_MIN 0
#define RAW_MAX 255
#define DEVICE_ID 0xa011
#define BRIGHTNESS_REG 0xf4

static int brightness_level;
static int brightness_raw;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);

static struct kobject *my_kobj;
static struct kobj_attribute level_attr = __ATTR(brightness_level, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute raw_attr = __ATTR(brightness_raw, 0664, sysfs_show, sysfs_store);

// Turn the above attributes into an attribute_group so we can handle them easier
static struct attribute *attr_list[] = {
	&level_attr.attr,
	&raw_attr.attr,
	NULL
};
static struct attribute_group attr_group = {
	.attrs = attr_list
};

static struct pci_dev *graphics_card;

static int __init n220_backlight_init(void)
{
	int err;
	uint8_t brightness;

	// get the graphics card pci_dev struct and read the current brightness value
	// from the configuration register
	graphics_card = pci_get_device(PCI_VENDOR_ID_INTEL, DEVICE_ID, NULL);
	if (!graphics_card) {
		printk(KERN_ERR "Failed to obtain the graphics card PCI device.\n");
		return -ENODEV;
	}
	err = pci_read_config_byte(graphics_card, BRIGHTNESS_REG, &brightness);
	if (err) {
		printk(KERN_ERR "Failed to read the current brightness value.\n");
		pci_dev_put(graphics_card);
		return err;
	}
	printk(KERN_INFO "PCI device initialized, current brightness is %u\n", brightness);

	// create the kernel objects and sysfs entries for the module
	my_kobj = kobject_create_and_add("n220-backlight", kernel_kobj);
	err = sysfs_create_group(my_kobj, &attr_group);
	if (err) {
		printk(KERN_ERR "Failed to create the attribute group.\n");
		kobject_put(my_kobj);
		return err;
	}

	printk(KERN_INFO "Backlight module initialized.\n");
	return 0;
}

static void __exit n220_backlight_exit(void)
{
	sysfs_remove_group(my_kobj, &attr_group);
	kobject_put(my_kobj);
	pci_dev_put(graphics_card);
	printk(KERN_INFO "Backlight module removed.\n");
}

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if(strcmp(attr->attr.name, "brightness_level") == 0) {
		return sprintf(buf, "%d", brightness_level);
	}
	else if (strcmp(attr->attr.name, "brightness_raw") == 0) {
		return sprintf(buf, "%d", brightness_raw);
	}
	return sprintf(buf, "%d", 0); // should never happen?
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	int result, err;
	err = kstrtoint(buf, 10, &result);
	if (err) {
		return -1;
	}
	if(strcmp(attr->attr.name, "brightness_level") == 0) {
		if (result >= LEVEL_MIN && result <= LEVEL_MAX) {
			brightness_level = result;
			brightness_raw = (1 << result) - 1;
			// TODO: set the brightness here
			return count;
		}
		else {
			printk(KERN_WARNING "Refusing to set brightness level to invalid value %d\n", result);
		}
	}
	else if (strcmp(attr->attr.name, "brightness_raw") == 0) {
		if (result >= RAW_MIN && result <= RAW_MAX) {
			brightness_level = -1; // no longer valid
			brightness_raw = result;
			// TODO: set the brightness here
			return count;
		}
		else {
			printk(KERN_WARNING "Refusing to set brightness value to invalid value %d\n", result);
		}
	}
	return -1;
}

module_init(n220_backlight_init);
module_exit(n220_backlight_exit);
