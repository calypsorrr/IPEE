void setup() {


}
/**
 * @brief Calculate the crc of the given data
 *
 * @param data pointer to data
 * @param len length of the data object
 * @return the calculated crc
 */
  #define CRC_POLY_CRC8 0x8C
    uint8_t calculateCRC(uint8_t * crcData, uint8_t len)
  {
    uint8_t cnt;
    uint8_t arrayCnt;
    uint8_t crc_compute = 0x00;
    uint8_t crc_byte = 0x00;
  
    for (arrayCnt=0;arrayCnt<len;arrayCnt++)
    {
        crc_byte = crcData[arrayCnt];
        for (cnt = 0;cnt < 8;cnt++)
        {
            if ((crc_compute ^ crc_byte) & 0x01)
            {
                crc_compute = (crc_compute >> 1) ^ CRC_POLY_CRC8;
            }
            else
            {
                crc_compute = crc_compute >> 1;
            }
            crc_byte = crc_byte >> 1;
        }
    }
    Serial.println(crc_compute);
    return crc_compute;
    }

void loop(){
  byte message[] = {0xFF, 0x04, 0x00};                              //Message without crc byte.
  calculateCRC(message, 0x03);                                      //Calculating crc with list and amount of bytes, in this case 3. Don't count crc byte.
}
