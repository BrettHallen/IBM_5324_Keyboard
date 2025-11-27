# IBM System/23 Model 5324 Keyboard
Recreation of the keyboard for this computer to bring those without keyboards back to life.

## [Design References](/Design_Reference)
![Actual keyboard layout](/Design_Reference/IBM_5324_Keyboard_Keys.jpg)

![Keyboard matrix](/Design_Reference/IBM_5324_Keyboard_Matrix.jpg)

## Keyboard Data Flow 
Refer p3-19 of the [SY34-0241-1 service manual.](/Datasheets/SY34-0241-1_IBM_5324_Computer_Service_Manual_May82_Keyboard.pdf)

The original keyboard uses a capacitive matrix where there is a capacitor for each key. When a key is pressed the capacitance increases.<br>

The keyboard MCU has 11 drive lines to repeatedly scan the matrix - in my schematic these are the columns.<br>  

The increased capacitance of the active key's capacitor permits the drive-line (column) scan pulse to appear at one of the eight input lines to the sense amplifier - in my schematic these are the rows.<br>

The MCU uses the combination of the active drive line (column) and sense amplifier input line (row) to determine a scan code character (88 possible, 83 actually used).<br>

An interrupt request is sent to the Model 5324 processing unit which will then read the scan code character.<br>

## Key Caps
[Key cap layout](https://gist.github.com/0ddjob/6a0642893fc0fdca74740a1e6c51e858)

|Size |Count|
|-----|-----|
|1u   |68   |
|1.25u|2    |
|1.5u |2    |
|1.75u|2    |
|2u   |6    |
|2.25u|1    |
|3u   |1    |
|9.5u |1    |
|TOTAL|83   |

|Size|Key|
|-----|---|
|1.25u|Backslash|
|1.25u|Shift LH|
|1.5u|Shift RH|
|1.5u|Field Advance|
|1.75u|Backspace|
|1.75u|Shift Lock|
|2u|Field Exit|
|2u|NP +|
|2u|NP -|
|2u|NP 0|
|2u|NP .|
|2u|Alt|
|2u|Enter (vertical)|
|3u|Field + (vertical)|
|9.5u|Space|

![Keyboard layout](/Design_Reference/ibm-5324-datamaster.jpg)

## [KiCad Design](/KiCad)
[Schematic](/IBM_5324_Keyboard_Schematic.pdf)

![Keyboard matrix layout](/Design_Reference/IBM_5324_Keyboard_Matrix_Table.png)

![Keyboard 3D](/IBM_5324_Keyboard_3D.png)

## [N-Key Rollover (NKRO) Design](/KiCad_NKRO)
Adding diodes to each key switch on their row connection.  The original keyboard doesn't have these so the modified behaviour might not be expected, thus it's "experimental".

![KNRO schematic](/KiCad_NKRO/IBM_5324_Keyboard_NKRO.png)


