#include <Wire.h>
#include <Servo.h>

Servo servoAlerones;
Servo servoElevador;
Servo servoTimon;
Servo motor;

// ---- Filtro complementario ----
const float ALPHA = 0.75;
float roll = 0, pitch = 0;
unsigned long tiempoAnterior = 0;
long gx_offset = 0, gy_offset = 0;

// ---- Canales receptor ----
int ch1 = 1500; // alerones
int ch2 = 1500; // elevador
int ch3 = 1000; // throttle
int ch4 = 1500; // timón

// ---- Leer i-Bus ----
void leerReceptor() {
  static uint8_t buf[32];
  static uint8_t idx = 0;

  while (Serial1.available()) {
    uint8_t b = Serial1.read();
    if (idx == 0 && b != 0x20) return;
    if (idx == 1 && b != 0x40) { idx = 0; return; }
    buf[idx++] = b;
    if (idx == 32) {
      idx = 0;
      uint16_t checksum = 0xFFFF;
      for (int i = 0; i < 30; i++) checksum -= buf[i];
      uint16_t recv = buf[30] | (buf[31] << 8);
      if (checksum == recv) {
        ch1 = buf[2]  | (buf[3]  << 8);
        ch2 = buf[4]  | (buf[5]  << 8);
        ch3 = buf[6]  | (buf[7]  << 8);
        ch4 = buf[8]  | (buf[9]  << 8);
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200); // i-Bus pin 19
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  servoAlerones.attach(6);
  servoElevador.attach(7);
  servoTimon.attach(8);
  motor.attach(9);

  // Armar ESC
  motor.writeMicroseconds(1000);
  delay(2000);

  Serial.println("Calibrando, no muevas el sensor...");

  for (int i = 0; i < 200; i++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 6, true);
    gx_offset += Wire.read() << 8 | Wire.read();
    gy_offset += Wire.read() << 8 | Wire.read();
    Wire.read(); Wire.read();
    delay(10);
  }
  gx_offset /= 200;
  gy_offset /= 200;

  Serial.println("Listo!");
  tiempoAnterior = millis();
}

void loop() {
  // ---- Leer receptor ----
  leerReceptor();

  // ---- Leer IMU ----
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();
  int16_t gx = Wire.read() << 8 | Wire.read();
  int16_t gy = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;
  float gx_ds = (gx - gx_offset) / 131.0;
  float gy_ds = (gy - gy_offset) / 131.0;

  unsigned long ahora = millis();
  float dt = (ahora - tiempoAnterior) / 1000.0;
  tiempoAnterior = ahora;

  float roll_acc  = atan2(ax_g, az_g) * 180.0 / PI;
  float pitch_acc = atan2(-ay_g, az_g) * 180.0 / PI;

  roll  = ALPHA * (roll  + gx_ds * dt) + (1 - ALPHA) * roll_acc;
  pitch = ALPHA * (pitch + gy_ds * dt) + (1 - ALPHA) * pitch_acc;

  // ---- Corrección IMU (escala suave) ----
  int corr_roll  = (int)(roll  * 2.0);
  int corr_pitch = (int)(pitch * 2.0);

  // ---- Mezclar radio + corrección ----
  int out_alerones = constrain(ch1 - corr_roll,  1000, 2000);
  int out_elevador = constrain(ch2 - corr_pitch, 1000, 2000);
  int out_timon    = constrain(ch4, 1000, 2000);
  int out_motor    = constrain(ch3, 1000, 2000);

  servoAlerones.writeMicroseconds(out_alerones);
  servoElevador.writeMicroseconds(out_elevador);
  servoTimon.writeMicroseconds(out_timon);
  motor.writeMicroseconds(out_motor);

  Serial.print("Roll:"); Serial.print(roll);
  Serial.print(" Pitch:"); Serial.print(pitch);
  Serial.print(" Alerón:"); Serial.print(out_alerones);
  Serial.print(" Elevador:"); Serial.print(out_elevador);
  Serial.print(" Motor:"); Serial.println(out_motor);

  // Leer receptor durante el delay
  unsigned long t = millis();
  while (millis() - t < 20) leerReceptor();
}