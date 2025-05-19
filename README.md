# W83C42 driver for arduino
This is a simple arduino driver for the Winbond W83C42 keyboard controller for PS2/AT keyboards. 

### To use it:
- Download the source files.
- Create a new folder called "W83C42" in your local folder with arduino libraries.
- Add the source files to the folder.
- If youre not familiar with the W83C42, i added a schematic schowing the basic setup of how to connect it up to an arduino nano (or any arduino)
- After you have connected everything you can run a simple example i included in the "Code" folder.
- Enjoy :DDDD !

## Some important details
- This chips is very finicky, i have worked very hard to get it working and even after all this time now and again it acts weirdly after i change something.
- The invertee setup shown on the schematic is not neccesary if youre not going to be sending command *directly to the keyboard*, if that is the case, leave the KB_CLK_OUT and KB_DATA_OUT pins not connected.
