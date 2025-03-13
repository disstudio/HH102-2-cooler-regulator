#include <HH102-2.h>

#define PROBE_PIN_1 0x10 //PIN_A4
#define PROBE_PIN_2 0x20 //PIN_A5
#include <ds18b20.c>

#define TOUCH_CMD_SKIP_ROM 0xCC
#define TOUCH_CMD_CONVERT_T 0x44
#define TOUCH_CMD_WRITE_SCRATCHPAD 0x4E
#define TOUCH_CMD_READ_SCRATCHPAD 0xBE

#rom int8 getenv("EEPROM_ADDRESS") = {0x45, 0x65, 0x03}
// min temperature, max temperature, hysteresis (subtract from min temperature)

#define EEPROM_MIN_TEMP 0x00
#define EEPROM_MAX_TEMP 0x01
#define EEPROM_HYSTERESIS 0x02

void setup_db18b20(int8 mask)
{
    // Reset DS18B20
    ds18b20_reset_pulse(mask);
    
    // Config 0x1F - set 9 bit resolution
    ds18b20_write_byte(mask, TOUCH_CMD_SKIP_ROM);
    ds18b20_write_byte(mask, TOUCH_CMD_WRITE_SCRATCHPAD);
    ds18b20_write_byte(mask, 0x03); // byte 2 - Th register
    ds18b20_write_byte(mask, 0x04); // byte 3 - Tl register
    ds18b20_write_byte(mask, 0x1F); // byte 4 - config register
    
    ds18b20_present(mask);
    ds18b20_write_byte(mask, TOUCH_CMD_SKIP_ROM);
    ds18b20_write_byte(mask, TOUCH_CMD_READ_SCRATCHPAD);
    ds18b20_read_byte(mask);
    ds18b20_read_byte(mask);
    ds18b20_read_byte(mask);
}

int16 read_temp(int8 mask)
{
    union {
        int8 buffer[2];
        int16 value;
    } temp;

    if(!ds18b20_present(mask)) {
        return 0;
    }
    
    // Read temperature
    ds18b20_write_byte(mask, TOUCH_CMD_SKIP_ROM);
    ds18b20_write_byte(mask, TOUCH_CMD_CONVERT_T);
    
    delay_ms(100);
    
    ds18b20_present(mask);
    ds18b20_write_byte(mask, TOUCH_CMD_SKIP_ROM);
    ds18b20_write_byte(mask, TOUCH_CMD_READ_SCRATCHPAD);
    temp.buffer[0] = ds18b20_read_byte(mask);
    temp.buffer[1] = ds18b20_read_byte(mask);
    ds18b20_read_byte(mask);
    
    return temp.value;
}

void display_error(int8 code)
{
    while(TRUE) {
        for (int8 i = 0; i < code; i++) {
            output_high(LED);
            delay_ms(800);
            output_low(LED);
            delay_ms(800);
        }
        
        delay_ms(5000);
    }
}

void blink(int8 code)
{
    for (int8 i = 0; i < code; i++) {
        output_high(LED);
        delay_ms(75);
        output_low(LED);
        delay_ms(75);
    }
}

void main()
{
    int16 temp_value_1;
    int16 temp_value_2;
    
    int16 temp_min = 45 << 4;
    int16 temp_max = 65 << 4;
    int16 hysteresis = 3 << 4;
    unsigned int16 output = 0;
    int1 fan_on = 0;
    
    //      DS18B20_DATA   **  *   - button
    set_tris_a(0x0);  // 0000 0000
    
    //      DCDC_DISABLE    * **** - DAC output
    //               LED   *  3210 - bits
    set_tris_c(0xC0); // 1100 0000
    output_c(0xDF);   // 1101 1111
                                              
    port_a_pullups(0x04);
    
    int1 sensor_present = 0;
    if(ds18b20_present(PROBE_PIN_1)) {
        setup_db18b20(PROBE_PIN_1);
        sensor_present = 1;
        blink(1);
    }
    if(ds18b20_present(PROBE_PIN_2)) {
        setup_db18b20(PROBE_PIN_2);
        sensor_present = 1;
        blink(2);
    }
    
    if (0 == sensor_present) {
        // No temp sensor present at all - fall in error
        display_error(3);
    }
    
    while(TRUE)
    {
        temp_value_1 = read_temp(PROBE_PIN_1);
        temp_value_2 = read_temp(PROBE_PIN_2);
        if (temp_value_2 > temp_value_1) {
            temp_value_1 = temp_value_2;
        }
        
        unsigned int8 portvalue = 0xC0;
        if(
            (bit_test(temp_value_1, 15) == 1) ||
            (temp_value_1 < (temp_min - (fan_on ? hysteresis : 0)))
        ) {
            // turn off the fan
            output = 0;
            fan_on = 0;
            // turn off LM2596
            portvalue |= 0x10;
        } else {
            if(temp_value_1 < temp_min) {
                output = 0;
            } else {
                // output = 0 .. 15 (PWM) | 16 (Full power; not supported yet)
                // output = (cur - min) / (max - min) * 15
                // output16 = (cur16 - min16) * 255 / (max16 - min16)
                output = ((temp_value_1 - temp_min) << 4) / (temp_max - temp_min);
            }
            
            if(output > 0x0F) {
                output = 0x0F;
            }
            
            // turn on LM2596
            portvalue |= (~output & 0x0F);
            fan_on = 1;
        } 
        output_c(portvalue);
        
        // starting blink
        output_high(LED);
        delay_ms(DELAY);
        output_low(LED);
        delay_ms(DELAY * 2);
        
        // display current output value
        for (int8 i = 0; i < 16; i++) {
            if (output > i) output_high(LED); else output_low(LED);
            delay_ms(65);
            output_low(LED);
            delay_ms(65);
        }
    }

}
