# N220 Backlight Driver for Linux

This is a simple PCI driver to control the backlight brightness of older netbook models. It has
developed specifically for the Samsung N220, but might work on other devices with Intel graphics
cards as well. Brightness can be set using two kernel objects located in
`/sys/kernel/n220-backlight`: `brightness_raw` for setting the brightness to any value between 0 and
255, and `brightness_level` for setting it to a level between 0 (2<sup>0</sup>-1 = 0) and
8 (2<sup>8</sup>-1 = 255), resulting in a subjectively even distribution.

This project has been developed as the final assignment for device drivers class in university, and
there is a story to it: I've been using Linux on an N220 netbook for most of my time as an
undergraduate student. At the time there was no proper support for setting the brightness in the
Intel video drivers, but it was still possible by calling `setpci` to manually write a brightness
value into a certain PCI register of the graphics card (located at address 0xF4),
so I wrote a simple Bash script to automate that process and bind it to the proper Fn keys.
Fortunately, support for setting the
brightness has since been added to the i915 module, making my script (and this driver) obsolete,
but I still wanted to try and implement my hacky solution the "proper" way once I got the the
required knowledge, which is why I chose this as my programming assignment. Unfortunately, using
this driver will now require to disable the i915 module, turning this project into just a simple
proof of concept as well as an example on how to write a simple PCI driver.


## Building and Usage

Building this module should be relatively straightforward on a Linux system if you have the
necessary build tools (make, gcc) and the kernel sources installed. Simply run `make` to build the
module, and `make clean` to clean all generated files. The resulting module can be inserted into the
kernel using `insmod n220-backlight.ko`, and removed using `rmmod n220_backlight` (note the naming
difference). `lsmod` and/or `dmesg | tail` can be used to verify that the module has been
successfully loaded and is working properly.

Once loaded, you can interact with the module over the /sys interface in the usual way:
```
$ echo 5 >/sys/kernel/n220-backlight/brightness_value  # setting a level
$ echo 200 >/sys/kernel/n220-backlight/brightness_raw  # setting a raw value
$ cat /sys/kernel/n220-backlight/brightness_level      # getting the level (-1 if raw value is set)
$ cat /sys/kernel/n220-backlight/brightness_raw        # getting the raw value
```

Notice, however, that you need to boot your system with the i915 module disabled if you want to try
this on an actual system, as this module will simply have no effect otherwise: Simply add
`module_blacklist=i915` to the kernel parameters of your bootloader, then boot the usual way.

If you would like to develop this project, you may like to have proper code inspection, so you can
have code completion features and inspect the related kernel sources easily. One way to achieve this
is using [clangd](https://clangd.llvm.org/): Follow the instructions on the website to install the
extension for your editor of choice, and edit the `compile_commands.json`, changing `REPLACE_THIS`
to the path where your project resides. Alternatively, you may use
[Bear](https://github.com/rizsotto/Bear) to create your own compilation database (simply calling
`bear -- make`). This approach might be more robust, as compilation flags may change between
compiler and/or kernel version.


## License

This project is licensed under the conditions of the [BSD 2-Clause "Simplified" License](LICENSE).
