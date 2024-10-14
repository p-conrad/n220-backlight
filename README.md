# N220 Backlight Driver for Linux

This is a simple PCI driver to control the backlight brightness of
older netbook models. It was developed specifically for the Samsung
N220, but it may work on other devices with Intel graphics
cards. Brightness can be adjusted using two kernel objects located in
`/sys/kernel/n220-backlight`:
- `brightness_raw` allows setting the brightness to any value between
  0 and 255.
- `brightness_level` allows setting it to a level between 0
  (2<sup>0</sup> - 1 = 0) and 8 (2<sup>8</sup> - 1 = 255), resulting
  in a subjectively even distribution.

This project was created as the final assignment for my device drivers
class at university. There is a bit of a backstory: I used Linux on an
N220 netbook throughout most of my undergraduate studies. At that
time, proper brightness control support was missing in the Intel video
drivers, but it was possible to adjust the brightness by using
`setpci` to manually write a value into a specific PCI register of the
graphics card (located at address 0xF4). I wrote a simple Bash script
to automate this process and bind it to the function keys.

Since then, brightness control support has been added to the i915
module, making both my old script and this driver obsolete. However, I
still wanted to implement my hacky solution the "proper" way once I
acquired the necessary knowledge, which led me to choose this project
as my programming assignment. Note that using this driver now requires
disabling the i915 module, making this project a simple proof of
concept and an example of writing a basic PCI driver.

For more details, you can read my final report (in German)
[here](report_de.pdf).


## Building and Usage

To build this module on a Linux system, ensure you have the necessary
build tools (`make`, `gcc`) and the kernel sources installed. Then,
follow these steps:
1. Run `make` to build the module.
2. Insert the compiled module into the kernel with `insmod
   n220-backlight.ko`, and remove it with `rmmod n220_backlight` (note
   the naming difference). Use `lsmod` and/or `dmesg | tail` to verify
   that the module has been successfully loaded and is working
   properly.
The generated files can be removed with `make clean`.

Once the module is loaded, you can interact with it through the `/sys`
interface as follows:

```bash
$ echo 5 > /sys/kernel/n220-backlight/brightness_level  # Set a brightness level
$ echo 200 > /sys/kernel/n220-backlight/brightness_raw  # Set a raw brightness value
$ cat /sys/kernel/n220-backlight/brightness_level       # Get the current brightness level (-1 if raw value is set)
$ cat /sys/kernel/n220-backlight/brightness_raw         # Get the current raw brightness value
```


### Important Note

To use this driver on a real system, you must boot with the i915
module disabled, as this module will otherwise have no
effect. Disabling the i915 module means forgoing other graphics
features. To disable it, add `module_blacklist=i915` to the kernel
parameters in your bootloader, then boot as usual.


## Development

If you wish to develop this project further, consider setting up
proper code inspection for better code completion and easier access to
the related kernel sources. One way to achieve this is by using
[clangd](https://clangd.llvm.org/). Follow the installation
instructions on the clangd website for your preferred editor, and edit
the `compile_commands.json` file, replacing `REPLACE_THIS` with the
path to your project.

Alternatively, you can use [Bear](https://github.com/rizsotto/Bear) to
create a compilation database by running `bear -- make`. This approach
may be more robust, as compilation flags can vary between compiler or
kernel versions.


## License

This project is licensed under the conditions of the [BSD 2-Clause
"Simplified" License](LICENSE.md).
