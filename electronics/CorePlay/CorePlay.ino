#include <SD.h>
#include <DmxSimple.h>  // http://www.pjrc.com/teensy/td_libs_DmxSimple.html
#include <LiquidCrystal.h>
#include <Entropy.h>    // http://code.google.com/p/avr-hardware-random-number-generation/

#define MAX_DMX_CHANNELS 250

const int chipSelect = 0;
char buffer[MAX_DMX_CHANNELS*2+4];

LiquidCrystal lcd(12, 13, 16, 17, 18, 19);
unsigned int numfiles=0;

bool sdok=false;

void setup()
{
  //Serial1.begin(115200);
  //Serial1.println("******************");

  // LCD display to show status
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("CORE Lighting");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  Serial.begin(115200);

  // initialize the random number generator
  Entropy.Initialize();

  // five PWM pins to show first 5 channels
  analogWrite(4, 0);
  analogWrite(5, 0);
  analogWrite(9, 0);
  analogWrite(15, 0);
  analogWrite(14, 0);

  // DMX library transmits on pin 8
  DmxSimple.usePin(8);
  DmxSimple.maxChannel(MAX_DMX_CHANNELS);
  for (int i=1; i<=MAX_DMX_CHANNELS; i++) {
    DmxSimple.write(i, 0);
  }

  // initialize the SD card
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  if (SD.begin(chipSelect)) {
    sdok = true;
    // and count the number of .TXT files
    File dir = SD.open("/");
    while (dir) {
      File f = dir.openNextFile();
      if (!f) break;
      //Serial.print("File: ");
      //Serial.println(f.name());
      if (stringEndsWith(f.name(), ".txt") && !f.isDirectory()) {
        numfiles = numfiles + 1;
      }
      f.close();
    }
    dir.close();
  }
  //Serial.print("numfiles = ");
  //Serial.println(numfiles);
  digitalWrite(LED_BUILTIN, LOW);

  // unused pins should have pullups
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(24, INPUT_PULLUP);
  Serial1.println("setup end");
}


void loop()
{
  char filename[20];

  if (sdok) {
    // randomly choose the next file to play
    if (numfiles < 1) return;
    filename[0] = 0;
    //int num = random(numfiles);
    int num = Entropy.random(numfiles);
    //Serial.print("random number = ");
    //Serial.println(num);
    File dir = SD.open("/");
    dir.rewindDirectory();
    while (1) {
      File f = dir.openNextFile();
      if (!f) break;
      if (stringEndsWith(f.name(), ".txt") && !f.isDirectory()) {
        if (num == 0) {
          strcpy(filename, f.name());
          f.close();
          break;
        } else {
          num = num - 1;
          f.close();
        }
      }
    }
    dir.close();

    // play the file
    if (filename[0]) {
      //Serial.print("Play file: ");
      //Serial.println(filename);
      play(filename);
    }
  }

  // if somebody is sending data, better use it...
  if (Serial.available()) {
    emulate_enttec_dmx();
  }

  // TODO: would be nice to detect if the SD card is removed
  // and automatically recover, rather than requiring power cycle
}

void play(const char *filename)
{
  lcd.clear();
  lcd.print(filename);
  File f = SD.open(filename);
  if (f) {
    unsigned long size = f.size();
    // read the period so we know how fast to play
    long period = f.parseInt();
    f.readBytesUntil('\n', buffer, sizeof(buffer));
    //Serial.print("Period is ");
    //Serial.println(period);
    if (period < 10 || period > 2500) return;
    lcd.setCursor(0, 1);
    lcd.print("Playing: ");
    // then read every line and play it
    elapsedMillis msec=0;
    while (f.available()) {
      f.readBytesUntil('\n', buffer, sizeof(buffer));
      //Serial.print("Data: ");
      //Serial.print(buffer);
      int channels = hex2bin(buffer);
      //Serial.print(", ");
      //Serial.print(channels);
      //Serial.println(" channels");
      if (channels > 0) {
        //transmit all the channels with DMX
        DmxSimple.maxChannel(channels);
        for (int i=0; i < channels; i++) {
           DmxSimple.write(i+1, buffer[i]);
        }

        // display the first 5 channels on LEDs
        analogWrite(4, buffer[0]);
        analogWrite(5, buffer[1]);
        analogWrite(9, buffer[2]);
        analogWrite(15, buffer[3]);
        analogWrite(14, buffer[4]);

	// update the LCD
	lcd.setCursor(9, 1);
        lcd.print((float)f.position() * 100.0 / (float)size);
        lcd.print("%");

        // wait for the required period
        while (msec < period) {
		// wait
		if (Serial.available()) {
			// ANY incoming data immediately stops
			// the currently playing file.
			f.close();
			return;
		}
	}
        msec = msec - period;
      }
    }
    f.close();
  }
}



byte hexdigit(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 255;
}


int hex2bin(char *buf)
{
  byte b1, b2;
  int i=0, count=0;
  
  while (1) {
    b1 = hexdigit(buf[i++]);
    if (b1 > 15) break;
    b2 = hexdigit(buf[i++]);
    if (b2 > 15) break;
    buf[count++] = b1 * 16 + b2;
  }
  return count;
}

byte stringEndsWith(const char *str, const char *end)
{
  int i, len, elen;

  len = strlen(str);
  elen = strlen(end);
  if (len < elen) return 0;
  for (i=0; i < elen; i++) {
    char c1, c2;
    c1 = str[len - elen + i];
    c2 = end[i];
    if (c1 >= 'a' && c1 <= 'z') c1 = toupper(c1);
    if (c2 >= 'a' && c2 <= 'z') c2 = toupper(c2);
    if (c1 != c2) return 0;	
  }
  return 1;
}


// emulate a subset of the Entec DMX USB Pro
//
#define STATE_START    0
#define STATE_LABEL    1
#define STATE_LEN_LSB  2
#define STATE_LEN_MSB  3
#define STATE_DATA     4
#define STATE_END      5

elapsedMillis timeout = 0;

void emulate_enttec_dmx(void)
{
  byte state = STATE_START;
  byte label;
  unsigned int index, count;
  byte b;

  lcd.setCursor(0, 1);
  lcd.print("Control from USB");

  while (1) {
    while (!Serial.available()) {
      // 0.5 seconds without data, reset state
      if (timeout > 500) state = STATE_START;
      // 20 seconds without data, revert to playing files
      if (timeout > 20000) return;
    }
    timeout = 0;
    b = Serial.read();

    switch (state) {
      case STATE_START:
        if (b == 0x7E) state = STATE_LABEL;
        break;

      case STATE_LABEL:
        label = b;
        state = STATE_LEN_LSB;
        break;

      case STATE_LEN_LSB:
        count = b;
        state = STATE_LEN_MSB;
        break;

      case STATE_LEN_MSB:
        count |= (b << 8);
        index = 0;
        if (count > 0) {
          state = STATE_DATA;
        } else {
          state = STATE_END;
        }
        break;

      case STATE_DATA:
        if (index < sizeof(buffer)) {
          buffer[index++] = b;
        }
        count = count - 1;
        if (count == 0) state = STATE_END;
        break;

      case STATE_END:
        if (b == 0xE7 && label == 6 && index > 1) {
          count = index;
          if (count > MAX_DMX_CHANNELS) count = MAX_DMX_CHANNELS;
          // display the first 5 channels on LEDs
          if (count >= 1) analogWrite(4, buffer[1]);
          if (count >= 2) analogWrite(5, buffer[2]);
          if (count >= 3) analogWrite(9, buffer[3]);
          if (count >= 4) analogWrite(15, buffer[4]);
          if (count >= 5) analogWrite(14, buffer[5]);
//Serial1.printf("%02X%02X%02X\r\n", buffer[3]&255, buffer[4]&255, buffer[5]&255);
          for (index=1; index <= count; index++) {
            DmxSimple.write(index, buffer[index]);
          }
        }
      default:
        state = STATE_START;
        break;
    }
  }
}















