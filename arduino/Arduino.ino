#include <LiquidCrystal.h>

// ---------------- LCD ----------------
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// ---------------- Relais ----------------
const int relayPin = 53;
const int relayDisplay = 18;

// ---------------- RFID / RPI ----------------
byte rfidBuffer[14];
byte responseBuffer[4];
char response[5]; // +1 pour \0

const byte LF = 0x0A;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Scanner Pret");

  Serial.begin(115200);
  Serial1.begin(9600);   // RFID
  Serial3.begin(9600);   // Raspberry Pi

  pinMode(relayPin, OUTPUT);
  pinMode(relayDisplay, OUTPUT);

  digitalWrite(relayPin, HIGH);

  Serial.println("System Ready");
}

// ---------------- LOOP ----------------
void loop() {

  // ================= RFID READ =================
  if (Serial1.available() >= 14) {

    int ret = Serial1.readBytes(rfidBuffer, 14);

    if (ret == 14) {

      // envoi vers Raspberry
      for (int i = 0; i < 14; i++) {
        Serial.print(rfidBuffer[i], HEX);
        Serial.print(" ");
        Serial3.print(rfidBuffer[i], HEX);
      }

      Serial3.write(LF);
      Serial.println("\nRFID sent to RPI");
    }
  }

  // ================= WAIT RESPONSE =================
  while (Serial3.available() == 0) {
    // attente active (possible amélioration future avec timeout)
  }

  int ret = Serial3.readBytes(responseBuffer, 4);

  // sécurisation string
  for (int i = 0; i < 4; i++) {
    response[i] = responseBuffer[i];
  }
  response[3] = '\0';

  Serial.print("Response: ");
  Serial.println(response);

  // ================= LOGIC =================
  if (response[0] == 'o' && response[1] == 'u' && response[2] == 'i') {

    Serial.println("ACCESS GRANTED");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Acces autorise");
    lcd.setCursor(0, 1);
    lcd.print("Tag valide");

    digitalWrite(relayPin, LOW);
    delay(3000);
    digitalWrite(relayPin, HIGH);

  }
  else if (response[0] == 'n' && response[1] == 'o' && response[2] == 'n') {

    Serial.println("ACCESS DENIED");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Acces refuse");
    lcd.setCursor(0, 1);
    lcd.print("Tag invalide");

  }
  else {

    Serial.println("INVALID RESPONSE");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erreur");
    lcd.setCursor(0, 1);
    lcd.print("Communication");

  }

  delay(1000);
  lcd.clear();
  lcd.print("Scanner Pret");
}