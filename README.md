# hcSource
Game library I've been developing as I learn x86 asm and C. Folders with descriptions as follows:

BMP2GFX: Converts 24bit bmp files into simplified bitmaps

USAGE:

1.) In DOSbox or a dos based operating system , open a command line
2.) type: BMP2GFX source.bmp dest.gfx
3.) If all ends well, you'll have your new *.GFX file.
4.) Go to OUTTILE and load it by pressing F2, happy level editing!

DOSPAL: Converts photoshop pal files to a simple rgb binary data file to read in dos programs

USAGE:

1.) In DOSbox or a dos based operating system , open a command line
2.) type: DOSPAL source.pal dest.pal
3.) If all ends well, you'll have your new *.PAL file.

TOOLS: Header files for sound, keyboard and timer drivers

OUTTILE: Currently in beta testing, opens *.GFX files slices the binary image data into 20x20 tiles, and saves
them in *.TLE files. BELOW IS THE KEYMAP

A - Save as
S - Save current file
F2 - open *.GFX file
C - place coord change at mouse position
X - delete coord change at mouse position
P - Load custom *.PAL file. Need to be a *.PAL file coverted from JASC format using the DOSPAL converter

MB1 - place tile at mouse position
MB2 - make mouse tile position solid

Yes, I know it's a hot buggy mess. I uploaded this mostly to test my git abilities for Coding Boot Camp.
