#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

MODULE_LICENSE("Dual BSD/GPL");

#define LEVEL_MIN 0
#define LEVEL_MAX 8
#define RAW_MIN 0
#define RAW_MAX 255

static int brightness_level;
static int brightness_raw;

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);

static int __init n220_backlight_init(void)
{
	// get the current brightness value from the bus here
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}

static void __exit n220_backlight_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
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
	int result, success;
	success = kstrtoint(buf, 10, &result);
	if (!success) {
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
