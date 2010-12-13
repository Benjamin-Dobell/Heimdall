Heimdall (c) 2010 Benjamin Dobell, Glass Echidna
http://www.glassechidna.com.au/products/heimdall/

DISCLAIMER:

    This software attempts to flash your Galaxy S device. The very nature of
    flashing is dangerous. As with all flashing software, Heimdall has the
    potential to damage (brick) your phone if not used carefully. If you're
    concerned, don't use this software. Flashing ROMs onto your phone may also
    void your warranty. I am not responsible for the result of your actions.



These instructions were written for Windows Vista / Server 2008 however the producedure
should be essentially the same for all supported versions of Windows (XP onwards). 

Driver Installation Instructions:

    1. Put your Samsung Galaxy S into download mode and plug it in.

    2. Run zadig.exe included in the Drivers subdirectory.

    3. From the menu chose Options -> List All Devices.

    4. From the USB Device list chose "Samsung USB Composite DEvice".

    5. Press "Install Driver", click "Yes" to the prompt and if you receive
       a message about being unable to verify the publisher of the driver
       click "Install this driver software anyway".

    6. Done


Driver Uninstallation Instructions:

    1. Put your Samsung Galaxy S into download mode by holding Volume Down + Home + Power.

    2. Plug your phone into your PC.

    3. Open up Device Manager on your PC (Control Panel -> System -> Device Manager).

    4. Under "Libusb (WinUSB) devices" right click on "Samsung USB Composite Device" and chose Properties.

    5. Go to the Driver tab and select Update Driver.

    6. Chose to browse your computer for the driver.

    7. Chose to pick from a list of devices drivers on your computer.

    8. Pick the original Samsung Composite driver, press next and then follow the prompts.

    9. Done



Flashing Firmware with Heimdall Frontend:    

    1. Fully charge your device (use the wall charger as it's faster).

    2. Download a decrypted Samsung Galaxy S ROM and extract it to a directory.

    3. If the ROM is provided as one of more archives (nested or otherwise),
       extract them all to the same location.

       NOTE: If you want to use the CSC then extract it last.

    3. Open Heimdall Frontend.

    4. Put your Galaxy S device into download mode and plug it in.

    5. For each file you wish to flash use "Browse" to select the file.

    6. Press "Start".

    7. Heimdall Frontend will display the progress and inform you when the
       flash is complete.



Flashing Firmware from Command Prompt:
	
    1. Fully charge your phone (use the wall charger as it's faster).

    2. Download a decrypted Samsung Galaxy S ROM and extract it to a directory.

    3. If the ROM is provided as one of more archives (nested or otherwise),
       extract them all to the same location.

       NOTE: If you want to use the CSC then extract it last.

    4. Put your Galaxy S device into download mode and plug it in..

    5. Open command prompt and navigate to the directory where you extracted the
       ROM files.

    6. Type the following to list all the functionality Heimdall supports:

            heimdall help

    7. Use the instructions to manually enter a command with all the files you
       want to flash.

       Here is an example that does a full flash and repartition:
		
            heimdall flash --repartition --pit s1_odin_20100512.pit --factoryfs factoryfs.rfs --cache cache.rfs --dbdata dbdata.rfs --primary-boot boot.bin --secondary-boot Sbl.bin --param param.lfs --kernel zImage --modem modem.bin

    8. Done
