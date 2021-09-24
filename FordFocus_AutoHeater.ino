/*
Скрипт для восстановления режима нагревателя сидения на Форд Фокус3 2019гв
Управление режимом нагревателя осуществляется с помощью одной кнопки, 
которая последовательно переключает ключает режимы по кругу 0-1-2-3-0-1-...
Теущий режим отображается 3мя светодиодами.
  // 0 - не горит ни один светодиод
  // 1 - горит LED1
  // 2 - горят LED1 и LED2
  // 3 - горят LED1, LED2, LED3

Скрипт запоминает режим, который был в последний раз (когда двигатель работал)
и устанавливает его как только двигатель запущен
*/
#include <avr/eeprom.h>

enum EngineStates {ENGINE_OFF=0, ENGINE_RUNS, ENGINE_STARTS, ENGINE_UNKNOWN};

#define DEBUG

//********************************************************************
// Настройка портов
//********************************************************************

// Кнопка управления режимом нагревателя (нажатие переключает режим 0-1-2-3-0-1-...)
#define HEATER_PIN 1

// Светодиод 1
#define LED1_PIN 0

// Светодиод 2
#define LED2_PIN 2

// Светодиод 3
#define LED3_PIN 3

// Состояние двигателя (заведен?)
#define ENG_START_PIN 4


//********************************************************************
// Глобальные переменные
//********************************************************************

// продолжительность нажатия кнопки и паузы между нажатиями
#define BUTTON_PRESS_DURATION_MS 200
#define BUTTON_PAUSE_DURATION_MS 500

// адреса трех ячеек EEPROM для хранения последнего значения статуса нагревателя (должны быть меньше 256)
#define EERPOM_ADDR1 10
#define EERPOM_ADDR2 150
#define EERPOM_ADDR3 230

// неопределенное значение для статуса нагревателя
#define UNKNOWN 99

// Последнее записанное значение режима нагревателя (0-3) - оно и будет установлено при старте двигателя
byte last_saved_heater_state = UNKNOWN;

// Статус двигателя
byte last_engine_state = ENGINE_UNKNOWN;

byte debug_heater_state = 0;

//********************************************************************
// Функции
//********************************************************************

// чтение и запись значения типа byte в EEPROM
bool save_EEPROM(byte value);
byte read_EERPOM();

// Проверяет статус двигателя (0-заглушен, 1-запущен, 2-запущен только что)
EngineStates get_engine_state();

// Устанавливает режим нагревателя
void set_heater(byte heater_level);

// Возвращает текущий режим нагревателя
// (считывая светодиоды)
byte get_heater_level();

// нажатие кнопки
void press_button();

//********************************************************************
// Инициализация
//********************************************************************
void setup() 
{                
  // initialize the digital pin as an output.
  pinMode(LED1_PIN, INPUT);
  pinMode(LED2_PIN, INPUT);
  pinMode(LED3_PIN, INPUT);
  pinMode(ENG_START_PIN, INPUT);
  pinMode(HEATER_PIN, OUTPUT);
  delay(100);

  
  for(int i=0; i<3; i++)
  {
    digitalWrite(HEATER_PIN, 1);
    delay(100);
    digitalWrite(HEATER_PIN, 0);
    delay(100);
  }
  
}

//********************************************************************
// Главный цикл
//********************************************************************
void loop() 
{
  switch (get_engine_state())
  {  
    case ENGINE_STARTS:
      // двигатель только что запущен 
    
    case ENGINE_RUNS:
      // двигатель работает
      
      // если текущий уровень нагревателя отличается от записанного, то обновим записанное
      byte current_level = get_heater_level();
      if (current_level != last_saved_heater_state)
      {
        save_EEPROM(current_level);
        last_saved_heater_state = current_level;
      }
  }
}

//********************************************************************
// Функции
//********************************************************************
// Проверяет статус двигателя (0-заглушен, 1-запущен, 2-запущен только что)
EngineStates get_engine_state()
{
  int cur_state = digitalRead(ENG_START_PIN);
  
  EngineStates ret_value;
  if (cur_state == LOW)
  {
    // двигатель не запущен
    ret_value = ENGINE_OFF;
  }
  else
  {
    if ((last_engine_state == ENGINE_OFF) or (last_engine_state == ENGINE_UNKNOWN))
    {
      // двигатель только что запущен
      ret_value = ENGINE_STARTS;
    }
    else
    {
      // двигатель работает
      ret_value = ENGINE_RUNS;
    }
  }

  last_engine_state = ret_value;
  
  return ret_value;
}

// Устанавливает режим нагревателя
void set_heater(byte heater_level)
{
  byte current_level = get_heater_level();

  // нажимаем кнопку до тех пор, пока режим не станет нужным
  while (heater_level != current_level)
  {
    // жмем кнопку, меняя режим
    press_button();
    
    // проверяем режим
    current_level = get_heater_level();
  }
}

// нажатие кнопки
void press_button()
{
  #ifdef DEBUG
    // в режиме отладки нажатие на кнопку изменяет режим в переменной debug_heater_state
    if ( (debug_heater_state < 0) or (debug_heater_state == 3)) debug_heater_state = 0;
    else debug_heater_state++;
    
    delay(BUTTON_PRESS_DURATION_MS);
    delay(BUTTON_PAUSE_DURATION_MS);
    
  #else  
    digitalWrite(HEATER_PIN, HIGH);
    delay(BUTTON_PRESS_DURATION_MS);
      
    digitalWrite(HEATER_PIN, LOW);
    delay(BUTTON_PAUSE_DURATION_MS);
  #endif
}

// Возвращает текущий уровень нагревателя, считывая светодиоды
byte get_heater_level()
{
  // Соответствие уровня нагревателя и состояния светодиодов:
  // 0 - не горит ни один светодиод
  // 1 - горит LED1
  // 2 - горят LED1 и LED2
  // 3 - горят LED1, LED2, LED3
  
  bool led1 = LOW;
  bool led2 = LOW;
  bool led3 = LOW;
  
  #ifdef DEBUG
    if (debug_heater_state = 0)
    {
      led1 = LOW;
      led2 = LOW;
      led3 = LOW;
    }
    if (debug_heater_state = 1)
    {
      led1 = HIGH;
      led2 = LOW;
      led3 = LOW;
    }
    if (debug_heater_state = 2)
    {
      led1 = HIGH;
      led2 = HIGH;
      led3 = LOW;
    }
    if (debug_heater_state = 3)
    {
      led1 = HIGH;
      led2 = HIGH;
      led3 = HIGH;
    }

  #else  

    led1 = digitalRead(LED1_PIN);
    led2 = digitalRead(LED2_PIN);
    led3 = digitalRead(LED3_PIN);

  #endif

  byte ret = 0;
  bitWrite(ret, 1, led1);
  bitWrite(ret, 2, led2);
  bitWrite(ret, 3, led3);
  
  return ret;
}

// запись значения типа byte в EEPROM
bool save_EEPROM(byte value)
{
  // работа с EEPROM https://alexgyver.ru/lessons/eeprom/
  
  // запись в 3х местах памяти - для надежности
  eeprom_write_byte((uint8_t*)EERPOM_ADDR1, value);
  eeprom_write_byte((uint8_t*)EERPOM_ADDR2, value);
  eeprom_write_byte((uint8_t*)EERPOM_ADDR3, value);
  
  return true;
}
// чтение значения типа byte из EEPROM
byte read_EERPOM()
{
  // работа с EEPROM https://alexgyver.ru/lessons/eeprom/

  // считывание из памяти
  byte val1 = eeprom_read_byte((uint8_t*)EERPOM_ADDR1);
  byte val2 = eeprom_read_byte((uint8_t*)EERPOM_ADDR2);
  byte val3 = eeprom_read_byte((uint8_t*)EERPOM_ADDR3);

  byte ret = 0;
  // из трех переменных найдем два одинаковых значения - это наш клиент
  if ((val1 == val2) or (val1 == val3))
    ret = val1;
  else
    ret = val2;

  return ret;
}
