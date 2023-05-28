# Heimdall

Heimdall is a cross-platform open-source tool suite used to flash firmware (aka ROMs) onto Samsung mobile devices.

## Supported Platforms

Officially, AMD64/x86-64 (64-bit) or x86 (32-bit) computers running:

* Linux
* macOS
* Windows (XP, Vista, 7 etc.)

However, several third-parties have reported success running Heimdall on ARM chipsets
(in particular Raspberry Pi), as well as additional operating systems such as FreeBSD.

## How does Heimdall work?

Heimdall connects to a mobile device over USB and interacts with low-level software
running on the device, known as Loke. Loke and Heimdall communicate via the custom
Samsung-developed protocol typically referred to as the 'Odin 3 protocol'.

USB communication in Heimdall is handled by the popular open-source USB library, [libusb](http://libusb.info).

## Free & Open Source

Heimdall is both free and open source. It is licensed under the MIT license (see LICENSE).

Heimdall is maintained and predominantly developed by [Glass Echidna](http://glassechidna.com.au/),
a _tiny_ independent software development company. If you appreciate our work and would like
to support future development please consider making a [donation](http://glassechidna.com.au/donate/).

## Documentation

For more details about how to compile and install Heimdall please refer to the
appropriate platform specific README:

<details>
<summary><b>Linux</b></summary>

:warning: _These instructions are for Linux operating systems._

[Documentation Source](https://raw.githubusercontent.com/Benjamin-Dobell/Heimdall/master/Linux/README)

<details>
 <summary><b>DISCLAIMER:</b></summary>

> This software attempts to flash your Galaxy S device. The very nature of
flashing is dangerous. As with all flashing software, Heimdall has the
potential to damage (brick) your device if not used carefully. If you're
concerned, don't use this software. Flashing ROMs onto your phone may also
void your warranty. Benjamin Dobell and Glass Echidna are not responsible
for the result of your actions.

</details>

<br>

<details>
 <summary><b>Flashing Heimdall Firmware Package with Heimdall Frontend</b></summary>

<br>

As of Heimdall Frontend 1.3, there are now two main ways to flash a ROM using Heimdall Frontend. The simpler and preferred option is to download a Heimdall Firmware Package and follow the steps below:

1. Fully charge your device (use a wall charger as it's faster).

2. Open a terminal and run Heimdall Frontend by typing: `heimdall-frontend`.

3. In the "Load Package" tab, under the "Heimdall Firmware Package" section, click the "Browse" button.

4. Use the dialog that appears to navigate to and select the Heimdall firmware package that you wish to flash.

5. Progress bars will appear as the package is decompressed and extracted. When the package has finished being decompressed, you should see information about the particular firmware package that has been selected.

6. Verify that your device is listed under "Supported Devices". If it's not, then **STOP** immediately! **DO NOT** flash this firmware to your device! Instead, search for an appropriate firmware package for your device. If you believe there is a mistake and your device is actually supported, please get in contact with the firmware developer (not Glass Echidna!) and ask them to rectify the issue. If the developer provided a URL, you may be able to contact them by pressing the "Homepage" button.

7. If you've verified that your device is supported, you may continue by pressing the "Load / Customize" button.

8. You should now be on the "Flash" tab. If not, verify that you did indeed press the "Load / Customize" button.

   Generally, you won't _NEED_ or _WANT_ to customize a firmware package! In that case, you can safely move on to step 9.

   Nonetheless, the "Flash" tab provides you with a means to customize the firmware package before flashing it to your device. See "Performing a Custom Flash with Heimdall Frontend" for more details.

9. Put your Galaxy S device into download mode and plug it into your PC. Download mode can be accessed in several different ways depending on your particular device model. If you're unsure how to do this, please search online for the appropriate method.

10. Press the "Start" button.

11. Heimdall Frontend will display the progress and inform you when the flash is complete.

   If something went wrong, for example, if your device wasn't detected because it wasn't in download mode, the status section will let you know the cause of the problem.

</details>

<details>
<summary><b>Performing a Custom Flash with Heimdall Frontend</b></summary>

<br>

This is the advanced means of flashing firmware to your device.

If you're not an advanced user or a developer, and if a Heimdall Firmware Package doesn't exist for the particular firmware or files that you wish to flash, then I strongly recommend you get in touch with the developer of the firmware or files and politely ask them to create a Heimdall Firmware Package for you. This way, you can avoid the risk of making mistakes due to inexperience.

If you're looking to customize an existing Heimdall Firmware Package, follow steps 1-8 of "Flashing Heimdall Firmware Package with Heimdall Frontend" and start from below with step 5.

1. Fully charge your device (use the wall charger as it's faster).

2. Download a decrypted device ROM or a Heimdall Firmware Package and extract everything to the same directory.

3. If the ROM is not a Heimdall Firmware Package and is instead provided as multiple archives (nested or otherwise), extract them all to the same location.

   **NOTE:** If you want to use the CSC, extract it last. If you're asked to overwrite files, do so.

4. Open a terminal and run Heimdall Frontend by typing: `heimdall-frontend`

5. Select the "Flash" tab. From the "Flash" tab, you're able to completely customize a flash.

6. Before you can choose which partitions you want to flash with particular files, you MUST first select a PIT file. To do this, click the "Browse" button in the "PIT" section. This will open a dialogue allowing you to navigate to and select a valid PIT (.pit) file.  
  If you don't have a valid PIT file stored on your computer, you can download your device's PIT file from the "Utilities" tab.

7. If a valid PIT file has been selected, the "Add" button below the "Partitions (Files)" list-box will be enabled. Press this button to add a partition to your flash.

8. When you first add a partition, you will see the "Partition Name" and "Partition ID" populated with information. Use the "Partition Name" dropdown to select the partition you wish to flash. "Partition ID" will automatically update and cannot be directly edited.

9. Then, select a file to flash to the specified partition using the "Browse" button under the "File / Partition". You must select a file before you can flash, create a firmware package, or add another partition. However, you can still press the "Remove" button if you've decided not to flash the specified partition.

10. Once you've specified a file name, you'll see the updated information in the partition list to the right. You can select any partition from this list and customize it as desired
   To remove a partition from the list, select it and click the "Remove" button. Removing a partition from the list doesn't remove it from your device; it simply means it will not be flashed.

11. Repeat steps 7-9 as necessary to specify all the partitions/files you wish to flash.

12. Now, you can choose whether to repartition your device and whether to prevent the device from rebooting once the flash is complete. These options can be enabled or disabled by toggling the "Repartition" and "No Reboot" checkboxes.
 Generally, you'll only need to enable repartition if you want to change the PIT file on your device. Keep in mind that repartitioning will wipe your device!  
 The "No Reboot" option is rarely required. It's mostly in place so you can manually boot straight into recovery mode after a flash instead of booting up normally.

13. If you've added at least one partition to your flash (and selected a file for that partition), the "Start" button will be enabled. Press the "Start" button to begin the flashing process.  
 You may notice that the "Create Package" tab becomes available whenever the "Start" button is available. From this tab, you can create a reusable, redistributable Heimdall Firmware Package with the files and partitions you just selected. See "How to Create a Heimdall Firmware Package" for details.

14. Heimdall Frontend will display the progress and inform you when the flash is complete.

 If something went wrong (e.g., your device wasn't detected because it wasn't in download mode), the status section will indicate the cause of the problem.

</details>

<details>
<summary><b>Flashing Firmware from Command Line</b></summary>

<br>

1. Fully charge your phone (use the wall charger as it's faster).

2. Download a decrypted device ROM or a Heimdall Firmware Package and extract everything to the same directory.

3. If the ROM is not a Heimdall Firmware Package and is instead provided as multiple archives (nested or otherwise), extract them all to the same location.

   **NOTE:** If you want to use the CSC, extract it last.

4. Put your Galaxy S device into download mode and plug it in.

5. Open a terminal and navigate to the directory where you extracted the ROM/firmware files.

6. Type the following command to list all the functionality Heimdall supports: `heimdall help`

7. Before flashing, you must first know the names of the partitions you wish to flash. You can obtain this information by executing the following command: `heimdall print-pit --no-reboot`  
 The inclusion of `--no-reboot` ensures that the phone will not reboot after the PIT file has been downloaded and displayed. After executing a command with the `--no-reboot` argument, the next command should include the `--resume` argument.
 **NOTE:** You can still safely reboot your phone manually (with the power button) after executing `--no-reboot` commands.

8. Use the help and print-pit output to construct a command with all the files you want to flash. Here is an example that does a full flash and repartition on a GT-I9000: `heimdall flash --repartition --resume --pit s1_odin_20100512.pit --FACTORYFS factoryfs.rfs --CACHE cache.rfs --DBDATA dbdata.rfs --IBL+PBL boot.bin --SBL Sbl.bin --PARAM param.lfs --KERNEL zImage --MODEM modem.bin`

9. Heimdall will display the progress as it flashes so that you know things are working as they should.

</details>

<details>
<summary><b>How to Create a Heimdall Firmware Package</b></summary>

<br>

Firstly, Heimdall's firmware package format is just a regular TAR archive compressed with gzip. The only two real requirements are that a valid firmware.xml must be included (refer to Appendix A), and you can only include files (no directories, links, etc.). As such, if you'd like, there is nothing preventing you from creating Heimdall packages manually. Of course, Heimdall Frontend provides a simple user interface that takes care of all the hard work for you.

There are two ways in which you can create a firmware package. You can create a package from scratch, or you can load an existing package, apply modifications, and then save the package. Creating a package from scratch is the preferred approach; by taking this approach, you're far less likely to run into file name length limitations.

Before you can access Heimdall Frontend's firmware creation functionality (available from the "Create Package" tab), you must first specify which files will be included in your package, as well as a few flashing options, i.e., whether or not users should repartition when flashing. This information must be filled out from the "Flash" tab in exactly the same fashion you would provide information to flash your device (see "Performing a Custom Flash with Heimdall Frontend"). As mentioned above, it's not the preferred means, but you're able to load an existing package as a starting point for this information.

Once you've specified the files/partitions you wish to include in your firmware package, the "Create Package" tab will become available. Clicking this tab will display additional information that you can include in your package. In order to continue, you must fill out all sections except for the URLs section, which is optional. The following is a breakdown of what all these options mean.

* General Firmware Information:

  * Firmware Name - This is the name of your particular firmware. An example would be "Cyanogenmod".

  * Firmware Version - This is the version identifier for your package. Any valid string will be accepted, although the inclusion of decimal point version number is preferred i.e. "7.1". If it makes sense then feel free to append a text string like "RC1" or "Beta 1" to the decimal point version.

  * Platform Name - This is the name of platform (or operating system) that your firmware is based on. In most cases this will simply be "Android".

  * Platform Version - This is the operating system version that your firmware is based on. Again decimal point version numbers are preferred over text, i.e. "2.3.4" is preferred over "Gingerbread".

* Developers
  * URLs (Optional):

  * Homepage - Here you can enter your personal URL or a URL particularly pertaining to the firmware being packaged. The URL must be well formed for it to work. An example of a well formed URL is "<http://www.glassechidna.com.au/products/heimdall/>". It is important to include "http://" in order to specify the protocol as other protocols such as "ftp://" are equally valid although unlikely to be used.

  * Donate - Here you can enter a URL that will link users to a page to make donations for the effort you've put into developing your firmware. Once again the URL must be well formed but there is no requirement on how your donation page should work. For instance both "<http://www.glassechidna.com.au/donate/>" and "<http://forum.xda-developers.com/donatetome.php?u=2710388>" are equally valid.

    Developer Info:

  * Name - Here you can enter in the name of individual team members or a team name. Click "Add" and the developer will be added to the list on the right. If you make a mistake you can select a developer from the list and click "Remove". You can list as many developers as you like, however visual constraints of the "Load Package" tab means only a few names will be visible. Where possible you may want to opt for team names over listing individual team members.

* Supported Devices:  

  * This section allows you to create a list of devices that are supported by your particular firmware. Although Heimdall isn't capable of enforcing this we strongly recommend you take this section seriously. If filled out correctly you could help save a number of accidental bricks!

  * Device Info:

  * Manufacturer - This is where you can enter the name of the manufacturer for a particular device. For now this will most likely be "Samsung".

  * Name - This is the human readable name for a particular device. "Galaxy S", "Galaxy S II", "Droid Charge", "Vibrant" and "Galaxy S (Telstra)" are all valid names. There are a lot of possible variations here so be as specific as you think is necessary.

  * Product Code - This is by far the most important bit of device information. Device names tend to be region specific and further subject to the whims of telecommunication companies and resellers. Product Codes (or product IDs) are designated by manufacturers and are generally the definitive means of referring to a particular device. Examples are "GT-I9000", "GT-I9100" and "SCH-I897". Ifyou're unsure of a particular product code then both Google and GSMArena are your friends!

After filling out all the necessary information the "Build" button will be enabled. If it's still disabled then you know you're missing some required information. In particular you must specify at least one developer and at least one supported device. Pressing the "Build" button will bring up a save dialogue where you must chose a file name for your particular package. Don't worry about specifying the ".tar.gz" extension Heimdall Frontend will take care of this automatically.

Once you've chosen a file name Heimdall Frontend will begin the process of building the firmware package. In doing so a valid firmware.xml file will be generated from the information entered. All files will be archived in a single TAR file then the TAR archive will be compressed via gzip compression. Compression will take a little while but you will see progress bars so you know the application hasn't hung. When the progress bars disappear you're finished making your package.

Congratulations! You're now ready to redistribute your firmware package online or by any means you see fit.

</details>

<details>
<summary><b>Appendix A - firmware.xml</b></summary>

The following details a part of the Heimdall Firmware Package format. This
is only relevant to developers or advanced users who wish to create Heimdall
Firmware Packages outside of Heimdall Frontend or in some way integrate support
for the format in their own software.

All Heimdall Firmware Packages must contain a file called firmware.xml. This
file stores flash information and meta-data for the package as well as
information about other files contained within the package.

The format is fairly straight-forward so it won't be explained in great detail.
Nonetheless the following is an example of a valid firmware.xml file.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<firmware version="1">
 <name>Test Firmware</name>
 <version>1.1</version>
 <platform>
  <name>Android</name>
  <version>2.3.4</version>
 </platform>
 <developers>
  <name>Benjamin Dobell</name>
  <name>Hedonism Bot</name>
 </developers>
 <url>http://www.glassechidna.com.au/</url>
 <donateurl>http://www.glassechidna.com.au/donate/</donateurl>
 <devices>
  <device>
   <manufacturer>Samsung</manufacturer>
   <product>GT-I9000</product>
   <name>Galaxy S</name>
  </device>
  <device>
   <manufacturer>Samsung</manufacturer>
   <product>GT-I9000T</product>
   <name>Galaxy S (Telstra)</name>
  </device>
  <device>
   <manufacturer>Samsung</manufacturer>
   <product>GT-I9000M</product>
   <name>Vibrant</name>
  </device>
 </devices>
 <pit>Nl3276-I9000 s1_odin_20100512.pit</pit>
 <repartition>0</repartition>
 <noreboot>0</noreboot>
 <files>
  <file>
   <id>0</id>
   <filename>gq3276-boot.bin</filename>
  </file>
  <file>
   <id>24</id>
   <filename>Uh3276-cache.rfs</filename>
  </file>
  <file>
   <id>22</id>
   <filename>em3276-factoryfs.rfs</filename>
  </file>
  <file>
   <id>11</id>
   <filename>fl3276-modem.bin</filename>
  </file>
  <file>
   <id>21</id>
   <filename>Xd3276-param.lfs</filename>
  </file>
  <file>
   <id>3</id>
   <filename>if3276-Sbl.bin</filename>
  </file>
  <file>
   <id>6</id>
   <filename>cr3276-zImage</filename>
  </file>
 </files>
</firmware>
```

New lines need not be included and the order in which elements are specified
does not need to match that of the above example.

One and only one `<firmware>`` element must be included. The`<firmware>` element
must also have a version attribute specified. The version must be parsable as
an integer and indicates what version of the Heimdall Firmware Package
specification the package adheres to.

All data is stored as strings, however a `<file>'s` `<id>`` element must be parsable
as an integer. The`<id>` value represents the partition ID (according to the
specified PIT file) that the file should be flashed to.

A `<firmware>'s` `<repartition>` and `<noreboot>` elements must also be parsable as
an integer. However, as they represent boolean values, a value of zero ("0")
means false (or disabled) where as a non-zero value (typically "1") means true
(or enabled).

File names are specified relative to the TAR archive in which firmware.xml and
all other files are to be stored. Heimdall Firmware Packages do not support
directories or links, as such file names should only be a name and not a path.

`<url>` and `<donateurl>` are the only optional elements, all other elements must
be included.

</details>

<details>
<summary><b>Appendix - B Installing Heimdall Suite from Source</b></summary>

1. First make sure you have installed `uild-essential, cmake, zlib1g-dev,
       qt5-default, libusb-1.0-0-dev and OpenGL (e.g libgl1-mesa-glx and
       libgl1-mesa-dev).`

       *NOTE:* Package names may not be absolutely identical to those above.

2. Open a terminal and navigate to the directory you downloaded,
       or extracted, Heimdall to.

3. Enter the following commands to compile Heimdall Suite:

    ```sh
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    ```

</details>

<br>

[Heimdall (c) 2010-2017 Benjamin Dobell, Glass Echidna](http://www.glassechidna.com.au/products/heimdall/)
</details>

#### OS X

* OSX/README.txt ([online](https://raw.githubusercontent.com/Benjamin-Dobell/Heimdall/master/OSX/README.txt))

#### Windows

* Win32/README.txt ([online](https://raw.githubusercontent.com/Benjamin-Dobell/Heimdall/master/Win32/README.txt))
