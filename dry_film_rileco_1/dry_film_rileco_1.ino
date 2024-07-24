/*
   Codice di Davide Di Stasio
   “All Rights Reserved"
   17/07/2024
*/

byte customChardwn[8] = {                                                 //carattere personalizzato freccia in basso
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};

byte customCharup[8] = {                                                    //carattere personalizzato freccia in alto
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
#include <LiquidCrystal_I2C.h>                                              //inclusione libreria LCD
#include <Adafruit_Sensor.h>                                                //librerie sensore umidità e temperatura
#include <DHT.h>
#include <DHT_U.h>

float temperatura = 0;                                                      //variabile immagazzinamento temperatura
float umidita = 0;                                                          //è scritto male di proposito, non cambiare
int stato;
int hmax = 6; //6 perchè array da 7 va da 0 a 6
float scala [7] {0.3, 6, 12, 24, 48, 60, 72};                               //scala temporale prove
int h = 0;                                                                  //indice per scala temporale
int k = 6;                                                                  //costante per tarare la scala temporale della durata delle prove in base alla lunghezza della pista
bool first = 1;
bool ciclo = 0;
bool forward;
bool reverse;
bool home_movement = 0;
unsigned long t0, tde = 300 , t;
unsigned long tiniziociclo , tempo_ciclo, ore , minuti , secondi;
LiquidCrystal_I2C lcd(0x27, 16, 2);                                         //inizializzazione LCD indirizzo e tipo 16X2

#define DHTPIN 2                                                            //pin sensore DHT11
#define DHTTYPE    DHT22                                                    //definisci tipo di sensore
DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(3, OUTPUT);         //dir pin
  pinMode(4, OUTPUT);         //step pin
  pinMode(5, OUTPUT);         //MS1
  pinMode(6, OUTPUT);         //MS2
  pinMode(7, OUTPUT);         //MS3
  pinMode(13, INPUT_PULLUP);  //finecorsa fine
  pinMode(12, INPUT_PULLUP);   //finecorsa home
  pinMode(8, INPUT_PULLUP);   //pulsante freccia in basso
  pinMode(9, INPUT_PULLUP);   //pulsante freccia in alto
  pinMode(10, INPUT_PULLUP);   //pulsante freccia dx
  pinMode(11, INPUT_PULLUP);   //pulsante freccia sx
  pinMode(12, INPUT_PULLUP);  //finecorsa home
  pinMode(13, INPUT_PULLUP);  //finecorsa end

  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);

  Serial.begin(9600);
  dht.begin();                                                              //inizializza sensore dht
  lcd.init();                                                              //inizializza lcd
  lcd.backlight();                                                          //accendi retroilluminazione
  lcd.createChar(0, customChardwn);                                         //crea il carattere custom 0 fraccia basso
  lcd.createChar(1, customCharup);                                          //crea il carattere custom 1 freccia alto  lcd.begin(16, 2);                                                         //inizializza LCD
  lcd.setCursor(0, 0);
  lcd.print("DRY FILM TIME");
  lcd.setCursor(0, 1);
  lcd.print("RECORDER 17/7/24");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DAVIDE DI STASIO");
  lcd.setCursor(3, 1);
  lcd.print("17/07/2024");
  delay(2000);
  lcd.clear();
}

void loop() {
  /*
     misura umidità e temperatura
  */
  /*  sensors_event_t event;
    dht.temperature().getEvent(&event);
    temperatura = event.temperature;
    dht.humidity().getEvent(&event);
    umidita = event.relative_humidity;                                        //umidità è scritto male di proposito NON CAMBIARE
  */
  /*
    ---------------------------------------MACCHINA A STATI PER LCD-------------------------------
  */
  switch (stato) {
    case 0:
      S0();
      break;
    case 1:
      S1();
      break;
    case 2:
      S2();
      break;
    case 3:
      S3();
      break;
    case 10:
      S10();
      break;
    case 11:
      S11();
      break;
    case 31:
      S31();
      break;
    case 32:
      S32();
      break;
    case 33:
      S33();
      break;
  }
  Serial.print("HOME");
  Serial.print('\t');
  Serial.print(digitalRead(12));
  Serial.print('\t');
  Serial.print("END");
  Serial.print('\t');
  Serial.println(digitalRead(13));

  if (home_movement) {
    if (digitalRead(12)) {  //finecorsa home
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      digitalWrite(3, HIGH); //direzione
      digitalWrite(4, HIGH);
      delayMicroseconds(5);
      digitalWrite(4, LOW);
      delayMicroseconds(5);
    }
    else {
      home_movement = 0;
      setStato(1);
    }
  }
  if ((ciclo) && ((millis() - tiniziociclo) < scala[h] * 3600000)) {
    if (digitalRead(13)) {   //finecorsa fine
      if (millis() - t > scala[h]*k) {
        t = millis();
        digitalWrite(5, HIGH);
        digitalWrite(6, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(3, LOW);   //direzione
        digitalWrite(4, HIGH);
        delayMicroseconds(1);
        digitalWrite(4, LOW);
        delayMicroseconds(1);

      }
    }
    else {
      ciclo = 0;
      setStato(33);
    }
  }
  else {
    ciclo = 0;
  }

}
/*
   funzione ausiliaria di padding
*/
String pad(int a) {                                                         //restituisci una stringa input numero a
  String ret = "";                                                          //forma una stringa nome ret
  if (a < 10) ret = "0" + String(a);                                        //se il numero ha 1 cifra cioè <10 aggiungi uno 0 davanti ad a
  else ret = String(a);                                                     //altrimenti lascia stare
  return ret;                                                               //restituisci stringa
}
/*
   funzione ausiliaria di padding
*/
String pad2(float a) {                                                         //restituisci una stringa input numero a
  String ret = "";                                                          //forma una stringa nome ret
  if (a < 10) {
    if (a < 1) {
      ret = String (int(a * 60)) + "m";
    }
    else {
      ret = "0" + String(int(a)) + "h";                                        //se il numero ha 1 cifra cioè <10 aggiungi uno 0 davanti ad a
    }
  }
  else ret = String(int(a)) + "h";                                                    //altrimenti lascia stare
  return ret; //restituisci stringa
}

/*
   STATI DELLO SWITCH E CAMBIO STATO
*/
void setStato(int s) {                                                  //funzione di comodo per passare da uno stato all'altro
  stato = s;                                                            //impone lo stato desiderato
  first = true;                                                         //resetta la flag first imponendola true
  lcd.clear();                                                          //ripulisci LCD
}

void S0() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("SCALA TEMPORALE          ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.write((byte)0);                                                 //freccia basso
    lcd.print("NEXT       >SET");
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone basso -> stato = 1
    t0 = millis();
    setStato(1);
  }
  if ((digitalRead(10) == 0) && (millis() - t0 > tde) ) {               //se spingi bottone destra -> stato = 10
    t0 = millis();
    setStato(10);
  }
};

void S1() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("POSIZIONE HOME          ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.write((byte)0);                                                 //freccia basso
    lcd.print("NEXT       >SET");
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i botton2i e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone basso -> stato = 2
    t0 = millis();
    setStato(2);
  }
  if ((digitalRead(10) == 0) && (millis() - t0 > tde) ) {               //se spingi bottone destra -> stato = 11
    t0 = millis();
    setStato(11);
    home_movement = 1;
  }
};

void S2() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("T:     C  H:   %          ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.write((byte)0);                                                 //freccia basso
    lcd.print("NEXT           ");
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone basso -> stato = 3
    t0 = millis();
    setStato(3);
  }
  lcd.setCursor(3, 0);
  lcd.print(temperatura);
  lcd.setCursor(12, 0);
  lcd.print(umidita);
};
void S3() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("INIZIO CICLO          ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.write((byte)0);                                                 //freccia basso
    lcd.print("NEXT     >START");
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone basso -> stato = 0
    t0 = millis();
    setStato(0
            );
  }
  if ((digitalRead(10) == 0) && (millis() - t0 > tde)) {                //se spingi bottone dx -> stato 31 e inizia ciclo
    t0 = millis();
    setStato(31);
    ciclo = 1;
    tiniziociclo = millis();
  }
};
void S10() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("h:");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.print("BACK< ");                                                 //freccia ALTO
    lcd.write((byte)1);
    lcd.print("+ ");
    lcd.write((byte)0);                                                 //freccia in alto
    lcd.print("-       ");
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(11) == 0) && (millis() - t0 > tde)) {                //se spingi bottone sx -> stato = 0
    t0 = millis();
    setStato(0);
  }
  if ((digitalRead(9) == 0) && (millis() - t0 > tde)) {                //se spingi bottone freccia alto -> h++
    t0 = millis();
    h++;
    if (h > hmax) {
      h = hmax;
    }
  }
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone freccia basso -> h--
    t0 = millis();
    h--;
    if (h < 0) {
      h = 0;
    }
  }
  lcd.setCursor(2, 0);
  lcd.print(pad2(scala[h]));
  lcd.print("    ");
};
void S11() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("RETURN HOME");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.print("STOP<                                   ");                                                 //freccia ALTO
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(11) == 0) && (millis() - t0 > tde)) {                //se spingi bottone sx -> stato = 1
    t0 = millis();
    setStato(1);
    home_movement = 0;
  }
};
void S31() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("TIME                     ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.print("STOP< ");                                                 //freccia ALTO
    lcd.print("     ");
    lcd.print("NEXT");
    lcd.write((byte)0);                                                 //freccia in alto
    first = false;                                                      //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(11) == 0) && (millis() - t0 > tde)) {                //se spingi bottone sx -> stato = 3
    t0 = millis();
    setStato(3);
    ciclo = 0;
  }
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone freccia basso -> stato = 32
    t0 = millis();
    setStato(32);
  }
  lcd.setCursor(5, 0);
  //scrivi il tempo in hh:mm:ss
  tempo_ciclo = (millis() - tiniziociclo);
  secondi = tempo_ciclo / 1000;
  minuti = secondi / 60;
  ore = minuti / 60;
  secondi %= 60;
  minuti %= 60;
  ore %= 24;
  lcd.print(pad(ore));
  lcd.print(":");
  lcd.print(pad(minuti));
  lcd.print(":");
  lcd.print(pad(secondi));
  lcd.print("  ");
  lcd.setCursor(6, 1);
  lcd.print(pad2(scala[h]));
};
void S32() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("T:     C  H:   %         ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.print("STOP< ");                                                 //freccia ALTO
    lcd.print("     ");
    lcd.print("NEXT");
    lcd.write((byte)0);                                                 //freccia in alto
    first = false;
    //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(11) == 0) && (millis() - t0 > tde)) {                //se spingi bottone sx -> stato = 3
    t0 = millis();
    setStato(3);
    ciclo = 0;
    tiniziociclo = millis();
  }
  if ((digitalRead(8) == 0) && (millis() - t0 > tde)) {                //se spingi bottone freccia basso -> stato = 32
    t0 = millis();
    setStato(31);
  }
  lcd.setCursor(3, 0);
  lcd.print(temperatura);
  lcd.setCursor(12, 0);
  lcd.print(umidita);
  lcd.setCursor(6, 1);
  lcd.print(pad2(scala[h]));
};

void S33() {
  if (first) {                                                          //se è la prima volta che entri in questo menù scrivi le etichette
    lcd.setCursor(0, 0);
    lcd.print("T:     C  H:   %         ");                             //gli spazzi servono per ripulire la riga da eventuali caratteri rimanenti
    lcd.setCursor(0, 1);
    lcd.print("STOP< ");                                                 //freccia ALTO
    lcd.print("                         ");
    first = false;
    //cambia il flag per far capire che hai già scritto le etichete
  }
  /*
     leggi i bottoni e muoviti di conseguenza tra gli stati
  */
  if ((digitalRead(11) == 0) && (millis() - t0 > tde)) {                //se spingi bottone sx -> stato = 3
    t0 = millis();
    setStato(3);
    ciclo = 0;
    tiniziociclo = millis();
  }
  lcd.setCursor(3, 0);
  lcd.print(temperatura);
  lcd.setCursor(12, 0);
  lcd.print(umidita);
  lcd.setCursor(5, 1);
  lcd.print(pad2(scala[h]));
  lcd.print(" ");
  lcd.print(pad(ore));
  lcd.print(":");
  lcd.print(pad(minuti));
  lcd.print(":");
  lcd.print(pad(secondi));
  lcd.print("  ");
};
