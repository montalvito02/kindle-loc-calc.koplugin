# Kindle Locations Calculator

Plugin for KOReader that tells you the number of locations in an epub file using the formula from [this MobileRead Forums post](https://www.mobileread.com/forums/showthread.php?t=159357).

[kindle-loc-calc.koplugin](kindle-loc-calc.koplugin) needs to be placed inside the plugins folder in koreader. Inside of it, you need to include a folder called epub_html_sumsize, with a single binary file also called epub_html_sumsize. This binary can be compiled from the code provided inside the [epub_html_sumsize/src](epub_html_sumsize/src) folder.
I compiled it using this command on WSL from the [epub_html_sumsize](epub_html_sumsize) directory (this works on my Kindle PW3 and should work on every [kindle with a Freescale/NXP i.MX6 SoloLite CPU](https://en.wikipedia.org/wiki/Amazon_Kindle)):
```bash
arm-linux-gnueabihf-gcc -O3 -march=armv7-a -mfpu=neon-vfpv3 -mfloat-abi=hard -static src/main.c src/miniz.c -o build/arm/epub_html_sumsize
```
The output will be inside the epub_html_sumsize/src/build/arm folder. If compilation fails, try to create the folder before compiling. After you have the file, place it in the appropiate folder.

Once finished, you should have the following folder structure inside of koreader:

```text
koreader/
│
└── plugins/
    │
    └── kindle-loc-calc.koplugin/
        ├── main.lua
        ├── _meta.lua
        └── epub_html_sumsize/
            └── epub_html_sumsize    # ARM binary (no extension)
```
The plugin will add a new menu entry in the "Tools" menu, and also a new action that can be associated with a gesture.

When executed, the following window will appear:
