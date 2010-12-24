Heimdall (c) 2010 Benjamin Dobell, Glass Echidna
http://www.glassechidna.com.au/products/heimdall/

DISCLAIMER:

    This software attempts to flash your Galaxy S device. The very nature of
    flashing is dangerous. As with all flashing software, Heimdall has the
    potential to damage (brick) your phone if not used carefully. If you're
    concerned, don't use this software. Flashing ROMs onto your phone may also
    void your warranty. Benjamin Dobell and Glass Echidna are not responsible
    for the result of your actions.



These instructions are for OS X operating systems.

Installing Heimdall and Heimdall Frontend Binaries:

    1. Run "Heimdall Suite.pkg" and follow the instructions.

    2. Reboot your system.
		
    3. Done



Installing Heimdall from Source:

    1. First make sure you have installed XCode and pkgconfig.

       NOTE: There are several different ways you can install pkgconfig, one
             option is to use Macports (http://www.macports.org/).

    2. Open a terminal and navigate to the directory where you extracted
       Heimdall.

    3. Then enter the following commands to compile and install libusb-1.0:

            cd libusb-1.0
            ./configure
            make
            sudo make install
            cd ..

       If you have problems please consult http://www.libusb.org/        

    4. Enter the following commands to compile and install Heimdall:

            cd heimdall
            ./configure
            make
            sudo make install
            cd ..

    5. If you haven't installed the driver before, enter the following:

            cd OSX
            sudo ./install-kext.sh

    6. Done


Installing Heimdall Frontend from Source (Optional):

    1. First make sure you have installed XCode and Qt 4.6 or above,
       available from http://qt.nokia.com/.

    2. Open a terminal and navigate to the directory where you extracted
       Heimdall.

    3. Enter the following commands to compile and install Heimdall Frontend:

            cd heimdall-frontend
            qmake
	   
	   Note: If you are using Qt via homebrew you may need to do:

			qmake -spec macx-xcode heimdall-frontend.pro

	   Instead of just qmake. This forces qmake to create the XCode Project.

    4. This will produce an XCode project called heimdall-frontend.xcodeproj
       in the heimdall-frontend folder. Open this file in XCode.

    5. From the menu bar select Build -> Build. This outputs heimdall-frontend
       to /Applications

    6. Done



Flashing Firmware with Heimdall Frontend:    

    1. Fully charge your device (use the wall charger as it's faster).

    2. Download a decrypted Samsung Galaxy S ROM and extract it to a directory.

    3. If the ROM is provided as one of more archives (nested or otherwise),
       extract them all to the same location.

       NOTE: If you want to use the CSC then extract it last.

    3. Open Heimdall Frontend.

    4. Put your Galaxy S device into download mode.

    5. For each file you wish to flash use "Browse" to select the file.

    6. Press "Start".

    7. Heimdall Frontend will display the progress and inform you when the
       flash is complete.



Flashing Firmware from Command Line / Terminal:
	
    1. Fully charge your device (use the wall charger as it's faster).

    2. Download a decrypted Samsung Galaxy S ROM and extract it to a directory.

    3. If the ROM is provided as one of more archives (nested or otherwise),
       extract them all to the same location.

       NOTE: If you want to use the CSC then extract it last.

    4. Open a terminal and navigate to the directory where you extracted the
       ROM files.

    5. Type the following to list all the functionality Heimdall supports:

            heimdall help

    8. Use the instructions to manually enter a command with all the files you
       want to flash.

       Here is an example that does a full flash and repartition:
		
            heimdall flash --repartition --pit s1_odin_20100512.pit --factoryfs factoryfs.rfs --cache cache.rfs --dbdata dbdata.rfs --primary-boot boot.bin --secondary-boot Sbl.bin --param param.lfs --kernel zImage --modem modem.bin

    9. Done

