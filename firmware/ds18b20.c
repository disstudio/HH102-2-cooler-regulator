
#ifndef  DB18B20_C
#define  DB18B20_C

#define TRIS_C_INITIAL 0x00

#define TOUCH_PIN_LOW(mask)    set_tris_a(TRIS_C_INITIAL); output_a(~mask)
#define TOUCH_PIN_HIGH(mask)   set_tris_a(TRIS_C_INITIAL); output_a(0xFF)
#define TOUCH_PIN_FLOAT(mask)  set_tris_a(TRIS_C_INITIAL | mask)
#define TOUCH_PIN_READ(mask)   ((input_a() & mask) == 0) ? 0 : 1


/*
int1 touch_read_bit()
This will read back a bit from the DS18B20
PARAMS: none
RETURNS: A bit from the DS18B20
*/
int1 ds18b20_read_bit(int8 mask)
{
   int1 data;

   TOUCH_PIN_LOW(mask);
   delay_us(1);
   TOUCH_PIN_FLOAT(mask);
   delay_us(5);
   data = TOUCH_PIN_READ(mask);
   delay_us(60);

   return data;
}

/*
BYTE touch_read_byte()
This will read back a byte from the DS18B20
PARAMS: none
RETURNS: A byte from the DS18B20
*/
BYTE ds18b20_read_byte(int8 mask)
{
   BYTE i,data;

   for(i=1; i <= 8; ++i)
      shift_right(&data, 1, ds18b20_read_bit(mask));

   return data;
}


/*
BOOLEAN touch_write_bit(int1 data)
This will write a bit to the DS18B20
PARAMS: The bit to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN ds18b20_write_bit(int8 mask, int1 data)
{
   TOUCH_PIN_LOW(mask);
   delay_us(10);
   if(data)
   {
      TOUCH_PIN_HIGH(mask);
      delay_us(10);
      if(!TOUCH_PIN_READ(mask))
         return FALSE;
   }
   else
   {
      TOUCH_PIN_LOW(mask);
      delay_us(10);
      if(TOUCH_PIN_READ(mask))
         return FALSE;
   }
   delay_us(50);
   TOUCH_PIN_HIGH(mask);
   delay_us(50);
   return TRUE;
}

/*
BOOLEAN touch_write_byte(BYTE data)
This will write a byte to the DS18B20
PARAMS: The byte to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN ds18b20_write_byte(int8 mask, BYTE data)
{
   BYTE i;

   for(i=1; i<=8; ++i)
      if(!ds18b20_write_bit(mask, shift_right(&data, 1, 0)))
         return FALSE;

   return TRUE;
}

/*
BOOLEAN touch_present()
This will evaluate whether or not there is a touch present on the DS18B20
PARAMS: none
RETURNS: True if a touch is present, false if otherwise
*/
BOOLEAN ds18b20_present(int8 mask)
{
   BOOLEAN present;
   TOUCH_PIN_LOW(mask);
   delay_us(500);
   TOUCH_PIN_FLOAT(mask);
   delay_us(5);

   if(!TOUCH_PIN_READ(mask))
      return FALSE;

   delay_us(65);
   present = !TOUCH_PIN_READ(mask);
   delay_us(240);
   return present;
}

/*
void reset_pulse()
This will send the DS18B20 a reset pulse
PARAMS: none
RETURNS: none
*/
void ds18b20_reset_pulse(int8 mask)
{
   TOUCH_PIN_LOW(mask);
   delay_us(500);
   TOUCH_PIN_FLOAT(mask);
   delay_us(5);
   while(!ds18b20_present(mask));
}

#endif
