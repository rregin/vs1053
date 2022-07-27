#include <SPI.h>
#define vs1053_XCS    6 // Control Chip Select Pin (for accessing SPI Control/Status registers)
#define vs1053_XDCS   7 // Data Chip Select / BSYNC Pin
#define vs1053_DREQ   2 // Data Request Pin: Player asks for more data
#define vs1053_RESET  8 // Reset is active low
#define sd_DATA  9
#define sd_CLK 52
const unsigned short sVS1053b_Realtime_MIDI_Plugin[28] = { /* Compressed plugin */
  0x0007, 0x0001, 0x8050, 0x0006, 0x0014, 0x0030, 0x0715, 0xb080, /*    0 */
  0x3400, 0x0007, 0x9255, 0x3d00, 0x0024, 0x0030, 0x0295, 0x6890, /*    8 */
  0x3400, 0x0030, 0x0495, 0x3d00, 0x0024, 0x2908, 0x4d40, 0x0030, /*   10 */
  0x0200, 0x000a, 0x0001, 0x0050,
};
void VSWriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte){
  while(!digitalRead(vs1053_DREQ)) ; 
  digitalWrite(vs1053_XCS, LOW); 
  SPI.transfer(0x02);             
  SPI.transfer(addressbyte);
  SPI.transfer(highbyte);
  SPI.transfer(lowbyte);
  while(!digitalRead(vs1053_DREQ)); 
  digitalWrite(vs1053_XCS, HIGH); 
}
void VSLoadUserCode(void) {
  int i = 0;
  while (i<sizeof(sVS1053b_Realtime_MIDI_Plugin)/sizeof(sVS1053b_Realtime_MIDI_Plugin[0])) {
    unsigned short addr, n, val;
    addr = sVS1053b_Realtime_MIDI_Plugin[i++];
    n = sVS1053b_Realtime_MIDI_Plugin[i++];
    while (n--) {
      val = sVS1053b_Realtime_MIDI_Plugin[i++];
      VSWriteRegister(addr, val >> 8, val & 0xFF);
    }
  }
}
void setup(){
  // When using Arduino Mega set 11,12,13 to high
  pinMode(11, INPUT_PULLUP);       // Connect to Pin 50
  pinMode(12, INPUT_PULLUP);       // Connect to Pin 51
  pinMode(13, INPUT_PULLUP);       // Connect to Pin 52

  pinMode(vs1053_DREQ,     INPUT);
  pinMode(vs1053_XCS,     OUTPUT);
  pinMode(vs1053_XDCS,    OUTPUT);
  pinMode(vs1053_RESET,   OUTPUT);
  digitalWrite(vs1053_XCS,  HIGH);  
  digitalWrite(vs1053_XDCS, HIGH); 
  digitalWrite(vs1053_RESET, LOW); 
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV16); 
  delayMicroseconds(1);
  digitalWrite(vs1053_RESET, HIGH); 
  VSLoadUserCode();
  Serial.begin(9600);
  Serial.println("vs1053 MP3 Shield Real Time MIDI test");
}
void sendSPIMIDI(byte data){
  SPI.transfer(0);
  SPI.transfer(data);
}
void sendMIDI(byte cmd, byte data1, byte data2) {
  while (!digitalRead(vs1053_DREQ));
  digitalWrite(vs1053_XDCS, LOW);
  sendSPIMIDI(cmd);
  if( (cmd & 0xF0) <= 0xB0 || (cmd & 0xF0) >= 0xE0) {
    sendSPIMIDI(data1);
    sendSPIMIDI(data2);
  } 
  else {
    sendSPIMIDI(data1);
  }
  digitalWrite(vs1053_XDCS, HIGH);
}
void noteOn(byte channel, byte note, byte attack_velocity) {
  sendMIDI( (0x90 | channel), note, attack_velocity);
}
void noteOff(byte channel, byte note, byte release_velocity) {
  sendMIDI( (0x80 | channel), note, release_velocity);
}
void loop() {
  for(int m=0; m<16;m++){
    for(int n=60;n<90;n++){
    noteOn(m,n,90);
    delay(100);
    noteOff(m,n,0);
    delay(100);
    }
  }
}
