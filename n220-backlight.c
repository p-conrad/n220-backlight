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
static uint8_t brightness_raw;

// Setup for the kernel object attributes of the above two brightness values
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

// PCI device setup and driver registration
int probe(struct pci_dev *dev, const struct pci_device_id *id);
void remove(struct pci_dev *dev);

static const struct pci_device_id device_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, DEVICE_ID) },
	{ /* zeroes */ }
};
MODULE_DEVICE_TABLE(pci, device_ids);
static struct pci_driver driver = {
	.name = "n220_backlight",
	.id_table = device_ids,
	.probe = probe,
	.remove = remove
};
static struct pci_dev *my_device;

static int __init n220_backlight_init(void)
{
	int err;

	err = pci_register_driver(&driver);
	if (err) {
		printk(KERN_ERR "Failed to register the PCI driver.\n");
		return err;
	}

	// create the kernel objects and sysfs entries for the module
	my_kobj = kobject_create_and_add("n220-backlight", kernel_kobj);
	err = sysfs_create_group(my_kobj, &attr_group);
	if (err) {
		printk(KERN_ERR "Failed to create the attribute group.\n");
		kobject_put(my_kobj);
		pci_unregister_driver(&driver);
		return err;
	}

	printk(KERN_INFO "Backlight module initialized.\n");
	return 0;
}

static void __exit n220_backlight_exit(void)
{
	sysfs_remove_group(my_kobj, &attr_group);
	kobject_put(my_kobj);
	pci_unregister_driver(&driver);
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

int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int err;
	if (my_device != NULL) {
		printk(KERN_WARNING "PCI device already initialized.\n");
		return -EBUSY;
	}
	err = pci_enable_device(dev);
	if (err) {
		printk(KERN_ERR "Failed to enable the PCI device.\n");
		return err;
	}
	err = pci_read_config_byte(dev, BRIGHTNESS_REG, &brightness_raw);
	if (err) {
		printk(KERN_ERR "Failed to read device configuration.\n");
		pci_dev_put(dev);
		return err;
	}
	printk(KERN_INFO "Successfully enabled PCI device, current brightness is %u.\n", brightness_raw);
	brightness_level = -1; // this is still undetermined

	my_device = dev;
	return 0;
}

void remove(struct pci_dev *dev)
{
	printk(KERN_INFO "PCI device removed.\n");
	my_device = NULL;
}

module_init(n220_backlight_init);
module_exit(n220_backlight_exit);
