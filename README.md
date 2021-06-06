ICLOG

    This project is about automating an airport bathroom. When a clogging has been detected by the IPEE sensor, 
    a door will be lockd and a message will be sent via the LoRaWAN network. The message will reach our database
    and Grafana using The Things Network, Pipedream that sends a http webhook to our Raspberry Pi and Flask that
    inserts the data into MySQL. Two readings will be sent via LoRaWAN: an analog reading from our water flow sensor
    and a 0 or 1 reading from our UART IPEE sensor. These readings will be displayed on our Grafana dashboard that will
    notify the cleaning staff in case of a clogging. 

    The circuit:
      Input:
        - Button on pins 7 and GND to unlock the door if a person is trapped
        - NFC reader using pins SDA, SCL, GND and 5V to unlock the door if the cleaning staff scans their badge
        - Water flow sensor using pins A0, GND and 5V to measure the use of water
        - IPEE sensor using pins RX, TX, 5V and GND to measure cloggings 
      Output:
        - Solenoid lock connected via a relay on pin 4 to lock the door in case of a clogging
        - LoRaWAN connectivity using the onboard antenna connector to send clogging and water flow readings via the 
          LoRaWAN network

     Created on 20/05/2021
     By Senne De Winter and Tibo Smet
     Modified on 6/06/2021
     By Senne De Winter
