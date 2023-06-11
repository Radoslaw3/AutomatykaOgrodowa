#include <OneWire.h>
#include <DallasTemperature.h> //Biblioteka odpowiedzialna za czujnik temperatury DS18B20
#include <Servo.h> //Biblioteka odpowiedzialna za serwa
#include "DHT.h" //Biblioteka odpowiedzialna za czujnik wilgoci i temperatury
#define PIN_temperatura A5
#define PIN_serwo 9
#define PIN_wilgotnosc 6
//#define PIN_wilgotnoscGlebyCyfrowy 10  //Awaria
#define PIN_wilgotnoscGlebyAnalog A0
#define PIN_przekaznik1 11
#define PIN_przekaznik2 12
#define PIN_przekaznik3 A4
#define czerwona 4
#define zielona 8
#define niebieska 7
#define niebieskaMroz 5
#define czerwona2_przekaznik1 2
#define czerwona3_przekaznik2 3

/////////
/*Nowe tematy do wprowadzenia:
-Gdy wilgotnosc powietrza powyzej 90% i to rosliny znacznie mniej wody pobieraja z korzenia.
-Stworzenie Repo na GitHub

*/
///////////////////////////////////////////////////0.)
///////////////////Termometr:
OneWire oneWire(PIN_temperatura); //Podłączenie do A5
DallasTemperature sensors(&oneWire); //Przekazania informacji do biblioteki
DeviceAddress termometrWewnetrzny1 = { 0x28, 0xFF, 0xD9, 0x3E, 0x52, 0x19, 0x1, 0x8C };
//DeviceAddress termometrWewnetrzny2 = { 0x28, 0xFF, 0xF9, 0x15, 0x77, 0x18, 0x1, 0xF5 };

Servo serwomechanizm;  //Tworzymy obiekt, dzięki któremu możemy odwołać się do serwa 
int pozycja = 0; //Aktualna pozycja serwa 0-180 (Moj zakres: 996R (0-120))
int staraPozycja = 0; //Poprzednia pozycja serwa
float temperatura = 0.00;
int obrot = 0;
boolean blad = false; 

DHT dht; //Tworzymy obiekt, dzięki któremu możemy odwołać się do czujnika wilgoci DHT11
int wilgotnosc = 0;
int temperatura2 = 0;
int wilgotnoscPowietrzaSrednia = 60; //Neutralna wartosc to 60
int wilgotnoscLicz = 0;

int wilgotnoscGleby = 0; //Sucha
int wilgotnoscGlebyChwilowa = 0;
int wilgotnoscGlebyLCD = 0;
int wilgotnoscGlebySuma = 0;
int iloscOdczytowGleby = 0;

boolean ziemiaWilgotna = true;
int ziemiaSuchaZliczamRazy = 0;
int ziemiaMokraZliczamRazy = 0;
int cyklPompa = 0;
int cyklElektrozawor = 0;

long spowalniaczPodlewania = 0;


///////////////////////////////////////////////////1.)
void setup(void) {
  //ziemiaSuchaZliczamRazy = 10;  //Test
  //cyklElektrozawor = 130;   //Test
  //cyklPompa = 7;  //Test - Wylaczenie pompy
  //ziemiaMokraZliczamRazy = 498; //Test na zalaczenie podlewania na chwile gdy od 500 petli jest mokro i nie podlewa.

  
  Serial.begin(9600);
  sensors.begin(); //Inicjalizacja czujnikow temperatury
  serwomechanizm.attach(PIN_serwo);  //Serwomechanizm podłączony do pinu 9
  serwomechanizm.write(0); //Wykonaj ruch do pozycji 0
  pinMode(czerwona, OUTPUT); // Piny, podłączone do diody jako wyjścia
  pinMode(zielona, OUTPUT);
  pinMode(niebieska, OUTPUT);
  pinMode(niebieskaMroz, OUTPUT);
  dht.setup(PIN_wilgotnosc);
  
  pinMode(PIN_przekaznik1, OUTPUT); //Pin do przekaznika1
  digitalWrite(PIN_przekaznik1, HIGH);
  pinMode(czerwona2_przekaznik1, OUTPUT);
  digitalWrite(czerwona2_przekaznik1, LOW);
  pinMode(PIN_przekaznik2, OUTPUT); //Pin do przekaznika2
  digitalWrite(PIN_przekaznik2, HIGH);  
  pinMode(czerwona3_przekaznik2, OUTPUT);
  digitalWrite(czerwona3_przekaznik2, LOW);
  
  //pinMode(PIN_wilgotnoscGlebyCyfrowy, INPUT); //Awaria
  
  analogWrite(PIN_przekaznik3, 255);
}
 
///////////////////////////////////////////////////2.)
void loop(void) { 
  sprawdzTemperature(); //da: srednia temperature z 5 sekund
  delay(500);
  oknoDachowe();
  delay(500); 
  sprawdzWilgotnoscPowietrza();
  delay(500);
  //nawilzaniePowietrza();
  delay(500);
  nawilzaniePowietrzaNagrzewnica();
  delay(500);
  spowalniacz();
  delay(500);
  //sprawdzWilgotnoscGlebyCyfrowo(); //czujnik ulegl uszkodzeniu/zkorodowaniu
  sprawdzWilgotnoscGlebyAnalog();  //uwaga: zamien pin 10 na pin A0
  delay(500); 
  podlewanie();
  delay(500);

  
 
  //delay(10000);
  
  //delay(60000); //Spowolnienie cykli
}

void sprawdzTemperature(){
  float temp[20];
  float temperaturaSuma = 0.00;
  float temperaturaSrednia = 0.00;
  int iloscPomiarow = 18;
  int iloscNiePoprawnychPomiarow = 0;
  //temperatura = 0;
  
  Serial.println("-------------------------");
  Serial.println("Aktualna srednia temperatura z 10 sekund: ");
  
  for(int i = 0; i < 20; i++){
    sensors.requestTemperatures(); //Pobranie temperatury czujnika
    temp[i] = sensors.getTempC(termometrWewnetrzny1);
      if(temp[i] == -127.00){ //Blad pomiaru (brak pomiaru)
        temp[i] = 0;
        iloscNiePoprawnychPomiarow = iloscNiePoprawnychPomiarow + 1;
        //continue;
      }
    temperaturaSuma += temp[i];
    //Serial.print(temp[i]);
    //Serial.print(", ");
    //Serial.println(temperaturaSuma);
    delay(500);   //docelowo ustaw 1000 
  }

  float MAX = -100.00;  //szukamy MAX i MIN do odrzucenia
  float MIN = 200.00;
  for(float a : temp){
    if(a > MAX && a != 0){
      MAX = a;
    }
    if(a < MIN && a != 0){
      MIN = a;
    }
  }

  if(MAX == -100.00){ //gdy nie bylo pomiaru, to lepiej zeby sie wyzerowal
    MAX = 0;
  }
  if(MIN == 200.00){
    MIN = 0;
  }
  
  Serial.print("MAX: ");
  Serial.print(MAX);
  Serial.print(" / MIN: ");  
  Serial.print(MIN);  

  temperaturaSrednia = ((temperaturaSuma - MAX - MIN) / (iloscPomiarow - iloscNiePoprawnychPomiarow)); //Odrzucamy MIN i MAX podczas obliczenia sredniej temperatury z 10 probek
  //temperaturaSrednia = ((temperaturaSuma) / (iloscPomiarow - iloscNiePoprawnychPomiarow)); //Odrzucamy MIN i MAX podczas obliczenia sredniej temperatury z 10 probek
  Serial.print(" / temperaturaSrednia: ");
  Serial.println(temperaturaSrednia); 
  Serial.print("iloscNiePoprawnychPomiarow: ");
  Serial.println(iloscNiePoprawnychPomiarow);
  temperatura = temperaturaSrednia;
  //Serial.println(temperatura);  //Wyswietlenie temperatury
}

void oknoDachowe(){
  //Zalozenia: 0 = 20 stopni = zamkniete / 78 = 30stopni = otwarte
  //temperatura = 5; //Test
  Serial.print("Okno dachowe - temperatura: ");
  Serial.println(temperatura);
  Serial.print("Test: oknoDachowe()");
  delay(1000);
  if(temperatura >= 30.00){
    Serial.print("-1-");
    ustawSerwo(78);
  digitalWrite(czerwona, HIGH);// Świecimy tylko na czerwono
  digitalWrite(zielona, LOW);// Świecimy tylko na zielono
  digitalWrite(niebieska, LOW); // Świecimy tylko na niebiesko
    delay(2000);
  }else if(temperatura >= 20.00){  
    Serial.print("-2-");
    ustawSerwo(40);
  digitalWrite(czerwona, LOW);// Świecimy tylko na czerwono
  digitalWrite(zielona, HIGH);// Świecimy tylko na zielono
  digitalWrite(niebieska, LOW); // Świecimy tylko na niebiesko
    delay(2000);
  }else{
    Serial.print("-3-");
    ustawSerwo(0);
  digitalWrite(czerwona, LOW);// Świecimy tylko na czerwono
  digitalWrite(zielona, LOW);// Świecimy tylko na zielono
  //digitalWrite(niebieska, HIGH); // Świecimy tylko na niebiesko
  delay(2000);
  }
  if(temperatura < 1.00){
    Serial.print("-4-");
    for(int i = 0; i < 5; i++){
       digitalWrite(niebieskaMroz, 20); // Świecimy tylko na niebiesko
      delay(100);
      digitalWrite(niebieskaMroz, LOW);
      delay(100); 
    } 
  }else{
    digitalWrite(niebieskaMroz, LOW); // Świecimy tylko na niebiesko    
  }
}

void ustawSerwo(int poziom){
  pozycja = 0;
  pozycja = (abs(poziom));
//  if(pozycja != staraPozycja){
  //serwomechanizm.write(pozycja); //Wykonaj ruch
  Serial.print(" / ustawSerwo()");
  Serial.print("_1_");
  if (pozycja <= 78) { //Jeśli pozycja mieści się w zakresie (0-180 ma zwykle serwo)
    delay(200);
    serwomechanizm.write(pozycja); //Wykonaj ruch
    Serial.print("_2_");
    Serial.print(" / pozycja ");
    Serial.println(pozycja);
  }else { //Jeśli nie, to powrót na początek
    delay(200);
    serwomechanizm.write(0); //Wykonaj ruch
    Serial.print("_3_");
    Serial.println(pozycja);
  }    
  staraPozycja = pozycja;
//  }else{
//    Serial.println("Taka sama pozycja serwa = skip.");
//  }
  delay(200); //Opóźnienie dla lepszego efektu     
}

void sprawdzWilgotnoscPowietrza(){
    //Pobranie informacji o wilgotnosci
  wilgotnosc = dht.getHumidity();
  //Pobranie informacji o temperaturze
  temperatura2 = dht.getTemperature();
  
  if (dht.getStatusString() == "OK") {
    Serial.print("Wilgotnosc powietrza: ");
    Serial.print(wilgotnosc);
    Serial.print("%RH | ");
    Serial.print(temperatura2);
    Serial.println("*C");
  wilgotnoscPowietrzaSrednia = wilgotnosc;
  }else{
    Serial.println("Wilgotnosc powietrza oraz temperatura2: Blad ");
  wilgotnoscPowietrzaSrednia = 100;  //kasowanie do 100% 
  }
  
  //Odczekanie wymaganego czasu
  delay(dht.getMinimumSamplingPeriod());  
}

void nawilzaniePowietrza(){  // do 30% = ON / od 30% do 50% = ON/OFF 50% / od 50% do 100% = OFF
  //wilgotnoscPowietrzaSrednia = 52; //Test
  
  if(wilgotnoscPowietrzaSrednia <= 30){
    zalaczPrzekaznik3(true);
    Serial.println("Nawilzacz ON");
  }else if(wilgotnoscPowietrzaSrednia <= 50){
    if(wilgotnoscLicz <= 20){
      zalaczPrzekaznik3(true);
      Serial.print("Nawilzacz ON, wilgotnoscLicz: ");
      Serial.println(wilgotnoscLicz);
    }else if(wilgotnoscLicz <= 40){
      zalaczPrzekaznik3(false);
      Serial.print("Nawilzacz OFF, wilgotnoscLicz: ");
      Serial.println(wilgotnoscLicz);
    }else{
      wilgotnoscLicz = 0;
    }
    wilgotnoscLicz = wilgotnoscLicz + 1;
  }else{
    zalaczPrzekaznik3(false);
    Serial.println("Nawilzacz OFF");
  }
}

void nawilzaniePowietrzaNagrzewnica(){  // do 30% = ON / od 30% do 50% = ON/OFF 50% / od 50% do 100% = OFF
  //wilgotnoscPowietrzaSrednia = 52; //Test
  
  if(temperatura <= -5){
    zalaczPrzekaznik3(true);
    Serial.println("Grzelka ON");
  }else{
    zalaczPrzekaznik3(false);
    Serial.println("Grzalka OFF");
  }
}

void spowalniacz(){  
    //1 cykl = 30sec *120 = 60min / 1,5 min *120 = 3h+1h / 60min. / 1,5 min *360 = 9h+3h
  if(wilgotnoscPowietrzaSrednia > 90){  //(120=1h podlewania,120= h przerwa)x2, 6h przerwa
  spowalniaczPodlewania = 90000;    //60 000ms = 1min
    Serial.println("spowalniaczPodlewania ON 2 cykle podlan/dobe (Przerwa 1,5 min) ");   //90000 = 2 podlania na dobe
  //Serial.println(spowalniaczPodlewania);
  }else if(wilgotnoscPowietrzaSrednia > 80){  //(2h podlewania, 4h przerwa)x2, 6h przerwa
  spowalniaczPodlewania = 30000;     //30 000ms = 30sec //1h / 1h+1h / 1h / 3h+3h // 1h / 1h+1h / 1h / 3h+3h/
    Serial.println("spowalniaczPodlewania ON 4 cykle podlan/dobe (Przerwa 0,5 min) ");   //30000 = 4 podlania na dobe 
  //Serial.println(spowalniaczPodlewania);  
  }else{      //1h 1h 1h 3h /x3, czyli bez ograniczen jest 6 cyklow podlewania na dobe
    spowalniaczPodlewania = 0;  //ponizej 80% wilgotnosci: 2 podlewania, 2 przerwa, 2 podlewania, 6h przerwa
    Serial.println("spowalniaczPodlewania OFF 6 cykli podlan/dobe ");
  }
  
}

void sprawdzWilgotnoscGlebyAnalog(){  
  // https://youtu.be/swEHgFdjdiY
  wilgotnoscGlebyChwilowa = analogRead(A0);
  wilgotnoscGlebyLCD = map(wilgotnoscGlebyChwilowa, 0, 1023, 1, 99);//Przeskalowanie wartości
                //wilgotnoscGlebyLCD: 57 = mokro / 49 = sucho / 1 = sucho
                //521 LCD 51 = ozn. 2,5 cm wody
  //wilgotnoscGlebySuma += wilgotnoscGlebyChwilowa; //Odczytanie wartości z ADC
  wilgotnoscGlebySuma += wilgotnoscGlebyLCD; //Odczytanie wartości z ADC
  iloscOdczytowGleby += 1;
    Serial.print("Wilgotnosc Gleby Analog: Odczyt: ");
  Serial.print(iloscOdczytowGleby);
    Serial.print(".) ");
  Serial.print(wilgotnoscGlebyChwilowa);
  Serial.print(" LCD: ");
  Serial.println(wilgotnoscGlebyLCD);
  
  if (iloscOdczytowGleby == 10){
    wilgotnoscGleby = wilgotnoscGlebySuma / iloscOdczytowGleby;
    Serial.print("Wilgotnosc Gleby Analog z 10 pomiarow: ");
  Serial.println(wilgotnoscGleby);
  iloscOdczytowGleby = 0;
  wilgotnoscGlebySuma = 0;
  
  }

//wilgotnoscGleby = 501;  //Test na wymuszenie Mokrej gleby 500 petli - usun

  if(wilgotnoscGleby > 43){ //Ziema Mokra. STOP podlewania.
  //wartosc 43 (wilgotnoscGlebyLCD) uwzglednia 1 bledny pomiar na 10. 
  //Ta czesc niestety moze calkowicie odciac podlewanie. 
  //Mysle ze kazdego dnia powinno byc wlaczone jakies minimalne podlewanie
  //musze cos takiego tu dodac.
  //------------------
    analogWrite(niebieskaMroz, 40);
    //digitalWrite(niebieskaMroz, HIGH);
    ziemiaWilgotna = true;
    ziemiaSuchaZliczamRazy = 0;   //Kasuje licznik do START-u podlewania
    Serial.print("Wilgotnosc Gleby: Mokro(dioda na czujniku sie pali od) ");
  Serial.println(ziemiaMokraZliczamRazy);
  ziemiaMokraZliczamRazy = ziemiaMokraZliczamRazy + 1;
    if(ziemiaMokraZliczamRazy == 500){ //Wymuszenie na chwile podlewania gdy od 500 petli jest mokro
      ziemiaWilgotna = false;
      Serial.println("Wilgotnosc Gleby: Mokro od 500 petli (Awaryjne wlaczenie podlewania na chwile)");
      ziemiaMokraZliczamRazy = 0;
    }
  }else{  //Ziemia Sucha. START podlewania.
      analogWrite(niebieskaMroz, 30); //Mrugniecie niebieska dioda
      delay(300);
      digitalWrite(niebieskaMroz, LOW);
      ziemiaSuchaZliczamRazy = ziemiaSuchaZliczamRazy + 1;
    ziemiaMokraZliczamRazy = 0;
    if(ziemiaSuchaZliczamRazy >= 10){
      ziemiaWilgotna = false;
      Serial.println("Wilgotnosc Gleby: Sucho(dioda na czujniku wylaczona)"); 
      
    }else{
      Serial.print("Wilgotnosc Gleby: Sucho(licze do 10 zanim START podlewania) "); 
      Serial.println(ziemiaSuchaZliczamRazy);  
      ziemiaWilgotna = true;
    }

  }
  

  
  wilgotnoscGlebyChwilowa = 0;
  delay(100);
 
}

void sprawdzWilgotnoscGlebyCyfrowo(){
  //Gdy czujnik wilgoci gleby dziala:  
  //if(digitalRead(PIN_wilgotnoscGlebyCyfrowy) == HIGH){ //Odwracam //Mokra -> Sucha ziemia
  //------------------
  //Tymczasowo gdy czujnik wilgoci gleby zepsuty:
  if(LOW == HIGH){ //Odwracam //Mokra -> Sucha ziemia
  //------------------
    analogWrite(niebieskaMroz, 40);
    //digitalWrite(niebieskaMroz, HIGH);
    ziemiaWilgotna = true;
    ziemiaSuchaZliczamRazy = 0;   //Kasuje licznik do START-u podlewania
    Serial.println("Wilgotnosc Gleby: Mokro(dioda na czujniku sie pali)");
  }else{
      analogWrite(niebieskaMroz, 30); //Mrugniecie niebieska dioda
      delay(300);
      digitalWrite(niebieskaMroz, LOW);
      ziemiaSuchaZliczamRazy = ziemiaSuchaZliczamRazy + 1;
    if(ziemiaSuchaZliczamRazy >= 10){
        ziemiaWilgotna = false;
        Serial.println("Wilgotnosc Gleby: Sucho(dioda na czujniku wylaczona)");     
    }else{
        Serial.print("Wilgotnosc Gleby: Sucho(licze do 10 zanim START podlewania) "); 
        Serial.println(ziemiaSuchaZliczamRazy);  
    ziemiaWilgotna = true;
    }

  }

}

void podlewanie(){
  if(ziemiaWilgotna == false){  
    

    cyklElektrozawor = cyklElektrozawor + 1;
 
    if(cyklPompa < 7){
      cyklPompa = cyklPompa + 1;
      zalaczPrzekaznik2(true);  //Pompa
    }else if(cyklPompa >= 7){
      zalaczPrzekaznik2(false);
      //cyklPompa = 7;
    }
    
    if(cyklElektrozawor <= 120){  //Podlewanie 2h
      zalaczPrzekaznik1(true);  //Elektrozawor
    }else if(cyklElektrozawor <= 240){  //Przerwa 2h
      zalaczPrzekaznik1(false);
    Serial.print("Spowalniacz podlewania: ");
    Serial.println(spowalniaczPodlewania);
    analogWrite(niebieskaMroz, 30);  //Spowalniacz zalaczony
    delay(spowalniaczPodlewania);    //Spowalniacz
    analogWrite(niebieskaMroz, LOW); //Spowalniacz wylaczony
    }else if(cyklElektrozawor <= 360){  //Podlewanie 2h
      zalaczPrzekaznik1(true);
    if(cyklElektrozawor == 241){
    cyklPompa = 0;         //Byl Problem. Bez if, Pompa non stop chodzila.
    }
    }else if(cyklElektrozawor <= 720){  //Przerwa 6h
      zalaczPrzekaznik1(false);
      zalaczPrzekaznik2(false);
    Serial.print("Spowalniacz podlewania: ");
    Serial.println(spowalniaczPodlewania);
    analogWrite(niebieskaMroz, 30);  //Spowalniacz zalaczony
    delay(spowalniaczPodlewania);    //Spowalniacz
    analogWrite(niebieskaMroz, LOW); //Spowalniacz wylaczony
    }else{                //Po 12h zerowanie
      cyklPompa = 0;
      cyklElektrozawor = 0;
    }
  
  }else{
    zalaczPrzekaznik1(false);
    zalaczPrzekaznik2(false);
    cyklPompa = 0;
    cyklElektrozawor = 0;
  }
  
    Serial.print("Cykl Podlewania Pompa: ");
    Serial.print(cyklPompa);
    Serial.print(" / Elektrozawor: ");
    Serial.println(cyklElektrozawor);  
    delay(500);
}

void zalaczPrzekaznik1(boolean stanPrzekaznik1){  //Elektrozawor
  if(stanPrzekaznik1 == true){
  digitalWrite(czerwona2_przekaznik1, HIGH); //Zalacz zolta diode2(przez opornik)
  delay(300);
  digitalWrite(czerwona2_przekaznik1, LOW);
  digitalWrite(PIN_przekaznik1, LOW); //Stan niski zalacza przekaznik 1(bez opornika)    
  }else{
  digitalWrite(PIN_przekaznik1, HIGH); //Wylacz przekaznik
  digitalWrite(czerwona2_przekaznik1,LOW); //Stan niski wylacza diode2(przez opornik)    
  }
}

void zalaczPrzekaznik2(boolean stanPrzekaznik2){  //Pompa
  if(stanPrzekaznik2 == true){
  digitalWrite(czerwona3_przekaznik2, HIGH); //Zalacz czerwona diode3(przez opornik)
  digitalWrite(PIN_przekaznik2, LOW); //Stan niski zalacza przekaznik 2(bez opornika)    
  }else{
  digitalWrite(PIN_przekaznik2, HIGH); //Wylacz przekaznik
  digitalWrite(czerwona3_przekaznik2,LOW); //Stan niski wylacza diode3(przez opornik)    
  }
}

void zalaczPrzekaznik3(boolean stanPrzekaznik3){  //Nawilzacz
  if(stanPrzekaznik3 == true){
  digitalWrite(czerwona2_przekaznik1, HIGH); //Zalacz zolta diode2(przez opornik)
  analogWrite(PIN_przekaznik3, 0); //Stan niski zalacza przekaznik 3(bez opornika) 
  delay(300);
  digitalWrite(czerwona2_przekaznik1, LOW);
  delay(300);  
  digitalWrite(czerwona2_przekaznik1, HIGH); 
  delay(300); 
  digitalWrite(czerwona2_przekaznik1, LOW);
  }else{
  analogWrite(PIN_przekaznik3, 255); //Wylacz przekaznik
  digitalWrite(czerwona2_przekaznik1,LOW); //Stan niski wylacza diode2(przez opornik)    
  }
}
