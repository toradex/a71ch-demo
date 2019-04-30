Overview
========

See "A71CH examples -> Hostlib API based examples"  in A71CH Host Lib Doxygen Guide.
"A71CH Host Lib Doxygen Guide" is installed to user PC when "A71CH Host Software Package 
(Windows Installer)" is installed from https://www.nxp.com/products/:A71CH 

Toolchain supported
===================
- MCUXpresso


Hardware requirements
=====================
- Micro USB cable
- Kinetis FRDM-K64F/FRDM-K82F/FRDM-KW41Z board
- Personal Computer
- A71CH Mini PCB
- A71CH Arduino shield


Board settings
==============
No special settings are required. A71CH must be with Debug mode enabled.


Prepare the Demo
================

1. Build the demo
2. Connect a USB cable between the PC host and the OpenSDA USB port on the
   target board.
3. Download the program to the target board.
4. Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
    - change Setup->Terminal->New-line->Receive->AUTO
5. Either press the reset button on your board or launch the debugger in your
   IDE to begin running the demo.

Running the demo
================

On successful run of the example, output similar to below can be seen.

a71ch HostLibrary light application (Rev 1.00:1.01)
**********************************************
Connect to A71CH-SM. Chunksize at link layer = 256.
ATR=0xB8.04.11.01.05.04.B9.02.01.01.BA.01.01.BB.0C.41.37.31.30.78.43.48.32.34.32.52.31.BC.00.
SCI2C_HostLib Version  : 0x0140
Applet Version   : 0x0131
SecureBox Version: 0x0000

==========SELECT-DONE=========
Platform: FRDM-K64F

-----------
Start exLight()
------------

-----------
Start exAesRfc3394Precooked(Reset)
------------

a71chInitModule(Reset)
Reset A71CH.


End of project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
