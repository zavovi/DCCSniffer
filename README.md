# DCCArduinoSniffer
DCC sniffer with **Arduino Nano board** (It is NOT Arduino IDE code!). You need **AVR Studio 4** and **ISP** or ASP programmer.

This is a Atmel C code for the DCC Sniffer. More information about this device is on my web page.  
EN: http://www.zavavov.cz/en/model-railway/electronics/the-cheapest-dcc-sniffer-with-board-from-arduino-nano/  
CZ: http://www.zavavov.cz/cz/modelova-zeleznice/elektronika/nejlevnejsi-dcc-analyzator-z-arduina-nano/  
  
And video on YouTube: https://youtu.be/mTE3l7VWnxk  
  
## DCC Decoder
This code is prepared for use it for simple DCC decoder. You should add only Address comparation and handling. 
It is used in this decoder for example (Only in Czech language article).  
CZ: http://www.zavavov.cz/cz/modelova-zeleznice/elektronika/levny-jednoduchy-dcc-dekoder-prislusenstvi/  

## How to connect DCC to Arduino board
For connecting DCC signal to one pin of the Arduino board, you can use this schematic.  
Put the DCC signal to **DCC-IN**. Connector **OUT** is used for connection to Arduino:  
1 – Arduino VCC (3V3)  
2 – Signal to Arduino PB3  
3 – Arduino GND  

![IMAGE!](http://www.zavavov.cz/wp-content/uploads/2017/02/schematic_opto_dcc.png "Connection DCC to Arduino")

## Example usage
All DCC commands are printed to console like on the picture.  
![IMAGE!](http://www.zavavov.cz/wp-content/uploads/2017/02/16402663_1239509922803893_4670285958642025133_o.png "Example of usage DCC Sniffer")
  
## License
You can use this code for free, but you should put the web address (www.zavavov.cz) on places, where you publish this code or device, which is using this code. 
  
## Contact and links
WEB: http://www.zavavov.cz  
YouTube: https://www.youtube.com/user/zavovi  
Facebook: https://www.facebook.com/zavavov/  
Instagram: https://www.instagram.com/zavavov/  
Forum: http://forum.zavavov.cz/  
