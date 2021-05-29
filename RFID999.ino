#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define RESTART 7
#define GREEN 6
#define RED 5
#define MASTERLIGHT 4

boolean programMode = false; // флаг мастер-режима
boolean successRead;

void addNewMaster();

byte storedCard[4];   // хранит ID считанной из EEPROM метки
byte readCard[4]; // хранит считанный датчиком ID
byte masterCard[4]; // хранит ID мастер-карты

byte resetStr[5];
byte beginStr[5];
byte endStr[3];

MFRC522 mfrc522(SS_PIN, RST_PIN);  // создаем обьект класса датчик

class UserInterface {
  public:
    static void greenDiode() {
      digitalWrite(GREEN, HIGH);
      delay(600);
      digitalWrite(GREEN, LOW);
    }
    
    static void redDiode() {
      Serial.println("Card is not registered");
      digitalWrite(RED, HIGH);
      delay(600);
      digitalWrite(RED, LOW);
    }

    static void printScannedCard() {
      for (int i = 0; i < 4; i++) {
      readCard[i] = mfrc522.uid.uidByte[i]; // чтение данных из считывателя
      Serial.print(readCard[i], HEX); //вывод идентификатора новой карты
      }
      Serial.println("");
    }

    static void printMasterCard() {
      Serial.println("Master Card's UID");
      for (int i = 0; i < 4; i++) {     // Считываем мастер-карту в массив, и выводим на экран ID
       masterCard[i] = EEPROM.read(2 + i);
       Serial.print(masterCard[i], HEX);
      }
      Serial.println("");
    }

    static void welcomeToMaster() {
      Serial.println("Welcome to Master Mode!");
      digitalWrite(MASTERLIGHT, HIGH);
    }
    
    static void exitFromMaster() {
      Serial.println("Exit from Master Mode!");
      digitalWrite(MASTERLIGHT, LOW);
    }

    static void yesMasterCard() {
      Serial.println("Master Card defined");
    }
    
    static void noMasterCard() {
      Serial.println("Master Card not defined");
      Serial.println("Scan a PICC to define as Master Card");
    }

    static void waitingForACard() {
       Serial.println("Waiting PICCs to be scanned:");
    }

    static void deletingResultPrint(boolean flag) {
      if(flag){
        Serial.println("Card succesfully removed!");
        digitalWrite(RED, HIGH);
        delay(200);
        digitalWrite(RED, LOW);}
      else {
        Serial.println("Card is not deleted.");}
    }

    static void addingResultPrint(boolean flag) {
      if(flag){
        Serial.println("Card succesfully added!");
        digitalWrite(GREEN, HIGH);
        delay(200);
        digitalWrite(GREEN, LOW);}
      else {
        Serial.println("Card is not added.");}
    }
    static void blinkThreeTimes(){

      for(int i = 0; i < 3; i++){
      digitalWrite(MASTERLIGHT, HIGH);
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      delay(300);
      digitalWrite(MASTERLIGHT, LOW);
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      delay(300);
      }
    }
};

class ReaderInterface {
  public:
    static int getID() {
    if ( ! mfrc522.PICC_IsNewCardPresent()) { //Если не приложена новая карта, возвращаем 0 (карту не удалось считать)
      return 0;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) { //Считывание ID карты
      return 0;
    }
    UserInterface::printScannedCard();
    mfrc522.PICC_HaltA(); // остановка чтения
    return 1; //если дошли до конца, то карта считана, возвращаем 1
  }
  
  static boolean resetCheck() {
    boolean flag = false;
    if(Serial.read() == 45){   //символ минус -
    byte checkByte;
    byte checkStr[5] = {114, 101, 115, 101, 116};  //reset в байтах
    for(int i = 0; i < 5; i++){
      checkByte = Serial.read();
      resetStr[i] = checkByte;
    }
    if ( resetStr[0] != NULL ){flag = true;}
    for ( int k = 0; k < 5; k++ ) { // Loop 4 times
    if ( resetStr[k] != checkStr[k] ) {flag = false;}
    }
  }
  return flag;
  }
  static void reset(){
    Serial.println("Master Card not defined");
    addNewMaster();
    digitalWrite(RESTART, LOW);
  }
};

void addNewMaster() {
  do {
      successRead = ReaderInterface::getID();
    }
   while (!successRead); //Цикл крутится, пока не считана новая карта
    
    for (int j = 0; j < 4; j++ ) {
      EEPROM.write( 2 + j, readCard[j] ); // Записываем мастер-карту в EEPROM по адресу 2...5
    }
    EEPROM.write(1, 1); //Записываем в 1-ый байт 1. Это значит, что мастер-карта определена
    UserInterface::yesMasterCard();
}



void setup() {

  digitalWrite(RESTART, HIGH);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(MASTERLIGHT, OUTPUT);
  pinMode(RESTART, OUTPUT);
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  
  UserInterface::blinkThreeTimes();
  
  if (EEPROM.read(1) != 1) {  //В 1-ом байте EEPROM хранится 1, если мастер-карта уже определена. Если нет, то заходим в if и считываем карту, которая станет мастер-картой
    UserInterface::noMasterCard();
    addNewMaster();
  }

  UserInterface::printMasterCard();
  UserInterface::waitingForACard();

  EEPROM.write(1000,0);
  
  delay(300);
  //UserInterface::blinkThreeTimes();
  endOfStopwatch();
}

void loop() {
   do {
    successRead = ReaderInterface::getID(); //Цикл крутится, пока не считана новая метка
    if(programMode){if(ReaderInterface::resetCheck()){ReaderInterface::reset();}}  //если находимся в мастер режиме и вводим -reset, происходит перезапуск
   }
   while (!successRead);
   
   if(programMode){
    usingMasterMode();
   }
   else {
      usingClientMode();
      stopwatch();
   }
}

void usingMasterMode() {
  if (isMaster(readCard)) { //Если в мастер-режиме приложена мастер-карта, то выходим из мастер-режима, порядок не имеет значения
     delay(500);
     programMode = false;
     UserInterface::exitFromMaster();
     return;
  }
  else {
   if (findID(readCard)) { //Если приложенная карта уже была в базе, то удаляем её
    deleteID(readCard);
   }
   else { //Если приложенной карты нет в базе, добавляем её
    writeID(readCard);
   }
  }
}

void usingClientMode() {
  if (isMaster(readCard)){  //Если в клиент-режиме приложена мастер-карта, то переходим в мастер-режим
      delay(500);
      programMode = true;
      UserInterface::welcomeToMaster();
      return;
  }
  else {
    if (findID(readCard)) { //если карта найдена в базе - проход открыт
      UserInterface::greenDiode();
    }
    else { //если карты нет в базе - проход закрыт
      UserInterface::redDiode();
    }
  }
}

boolean isMaster( byte test[] ) {
  if (checkTwo(test, masterCard))
    return true;
  else
    return false;
}


boolean checkTwo(byte a[], byte b[]) {
  boolean flag = false;
  if ( a[0] != NULL )
    flag = true;
  for ( int k = 0; k < 4; k++ ) { // Loop 4 times
    if ( a[k] != b[k] ) // IF a != b then set match = false, one fails, all fail
      flag = false;
  }
  return flag;
}

  
boolean findID( byte find[] ) {
  int count = EEPROM.read(0); // Читаем первый байт из EEPROM, в котором хранится кол-во зарегестрированных меток
  for ( int i = 1; i <= count; i++ ) {  // Проверяем всё хранящиеся метки на равенство с той, которую ищём
    readID(i); // поочередное чтение меток из EEPROM
    if (checkTwo( find, storedCard)) { // проверка на равенство
      return true;
      break; // если нашли, то можно выходить из цикла
    }
  }
  return false;
}


void readID(int number) {
  int start = (number * 6 ) + 2; // расчёт стартового адреса. Смещение на 2 байта, т.к. в 0-ом хранится кол-во зарегистрированных меток, а в 1-ом - зарегана ли мастер-карта(1,если зарегана). 
  for ( int i = 0; i < 4; i++ ) {
    storedCard[i] = EEPROM.read(start + i); //читаем карту из EEPROM в массив
  }
}

void deleteID( byte a[] ) {
  boolean isDeleted = false;
  if (!findID(a)){} //Если карты нет в базе, удалять нечего
  else {
    int num = EEPROM.read(0);
    int slot;
    int start;
    int looping;
    int j = 0;
    int count = EEPROM.read(0);
    slot = findIDSLOT(a); //индекс карты, которую нужно удалить
    start = (slot * 6) + 2;
    looping = ((num - slot) * 6); //Кол-во раз, которое необходимо сдвинуть влево
    num--; //Уменьшаем кол-во хранящихся меток на 1
    EEPROM.write( 0, num ); //Обновляем кол-во меток
    for (j = 0; j < looping; j++ ) { // Сдвигаем все метки,следующие после удаляемой
      EEPROM.write( start + j, EEPROM.read(start + 6 + j));
    }
    for ( int k = 0; k < 6; k++ ) { 
      EEPROM.write( start + j + k, 0);//заполняем освободившееся место нулями
    }
    isDeleted = true;
  }
  digitalWrite(RED, HIGH);
  delay(200);
  digitalWrite(RED, LOW);
  UserInterface::deletingResultPrint(isDeleted);
}

int findIDSLOT( byte find[] ) {
  int count = EEPROM.read(0);
  for ( int i = 1; i <= count; i++ ) {
    readID(i);
    if (checkTwo( find, storedCard )) {
      return i; // возвращаем номер найденной карты
      break;
    }
  }
}

void writeID( byte a[] ) { //Добавление(запись) новой 
  boolean isAdded = false;
  if (!findID(a)) { //Если карта не найдена в базе, то добавляем её
    int num = EEPROM.read(0);
    int start = ( num * 6 ) + 8; // В начальный момент num = 0, в байтах с 2 по 5 хранится мастер-карта, поэтому обычные метки записываются, начиная с адреса 6
    num++; //Увеличиваем кол-во хранящихся меток на 1
    EEPROM.write( 0, num ); //Обновляем кол-во меток
    int j = 0;
    for (; j < 4; j++ ) {
      EEPROM.write( start + j, a[j] ); //Побайтово записываем новую метку в EEPROM
    }
    word count = 0;
    EEPROM.put(start + j, count);
    isAdded = true;
  }
  UserInterface::addingResultPrint(isAdded);
}

void stopwatch() {
  if(isMaster(readCard)) {
    return;
  }
  else {
    if(findID(readCard)) {
      int number = findIDSLOT(readCard);
      int start = (number * 6 ) + 6;
      int start2 = 5000 + (number*4);
      word cur_count = 0;
      unsigned long prev_time = 0;
      unsigned long result = 0;
      unsigned long cur_time = 0;
      EEPROM.get(start, cur_count);
      if(cur_count == 0) {
        result = millis();
        EEPROM.put(start2, result);
        Serial.println("Run!");
      }
      else{
        EEPROM.get(start2, prev_time);
        cur_time = millis();
        result = cur_time - prev_time;
        EEPROM.put(start2, cur_time);
        Serial.println("Time: " + String(result) + " ms (Lap - " + String(cur_count) + ")" );
      }
      cur_count++;
      EEPROM.put(start, cur_count);
    }
  }
}

void endOfStopwatch() {
  int countOfPICCS = EEPROM.read(0); // Читаем первый байт из EEPROM, в котором хранится кол-во зарегестрированных меток
  int address = 0;
  word count = 0;
  for ( int i = 1; i <= countOfPICCS; i++ ) { // Проверяем всё хранящиеся метки на равенство с той, которую ищём
    address = (i * 6) + 6;
    count = 0;
    EEPROM.put(address, count);
  }

  for(int i = 5004; i < 5004 + (countOfPICCS * 4); i++) {
    EEPROM.write(i, 0);
  }
}
