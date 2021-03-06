//////////////////////////// AMPLIFIER_BD37534FV_TDA7293 ВЕРСИЯ 1.17 //////////////////////////////
///////////////////// СКЕТЧ АНАТОЛИЯ НЕВЗОРОВА // CODE BY ANATOLY NEVZOROFF ///////////////////////

//КОДЫ КНОПОК ДЛЯ МОЕГО КОНКРЕТНОГО ПУЛЬТА, ДЛЯ ВАШЕГО ПОЛУЧИТЕ ПРИ НАЖАТИИ В МОНИТОРЕ ПОРТА
#define IR_1 0x8F00F // Кнопка вниз по меню
#define IR_2 0x808F7 // Кнопка вверх по меню
#define IR_3 0x8F807 // Кнопка громкость +
#define IR_4 0x802FD // Кнопка громкость -
#define IR_5 0x8827D // Кнопка следующее меню
#define IR_6 0x8E817 // Кнопка предыдущее меню
#define IR_7 0x800FF // Кнопка POWER/STANDBY
#define IR_8 0x818E7 // Кнопка MUTE ON/OFF
#define IR_9 0x8D827 // Кнопка включения наушников
#define IR_10 0xFFFFFFFF//Константа для Повтора
#define IR_11 0x8C837 // Кнопка Главное меню 

#include <Wire.h>
#include <BD37534FV.h>// https://github.com/liman324/BD37534FV/archive/master.zip
#include <EEPROM.h>
#include <Encoder.h>  // https://rcl-radio.ru/wp-content/uploads/2019/05/Encoder.zip
#include <MsTimer2.h> // https://rcl-radio.ru/wp-content/uploads/2018/11/MsTimer2.zip
#include <boarddefs.h>// Built into the Irremote library or take the finished file.
#include <IRremote.h> // https://rcl-radio.ru/wp-content/uploads/2019/06/IRremote.zip
#include <LiquidCrystal_I2C.h>
#include <DS3231.h>   // RTC Library in the archive.

LiquidCrystal_I2C lcd(0x27, 20, 4); //Устанавливаем адрес, число символов и строк у дисплея
IRrecv irrecv(3); decode_results ir; //Указываем вывод к которому подключен IR приемник
Encoder myEnc(11, 10); //DT и CLK Выходы энкодера
BD37534FV bd;//Объявляем bd для бибиотеки аудиопроцессора
DS3231 rtc(SDA, SCL); Time wt; //Подключаем модуль часов

//unsigned long от 0 до 4294967295
uint32_t time, oldPosition = -999, newPosition, btl, irkey, oldtime;

//int от -32768 до 32767
int16_t setime, setyear, btn, oldbtn0, oldbtn1, oldbtn2, oldbtn3, oldbtn4, oldbtn5;

//byte от 0 до 255
uint8_t i, w, w1, w2 = 1, w3, z, z0, z1, d1, d2, d3, d4, d5, d6, d7, d8, d9, e1, e2, e3, graf_d, gain_d, vol_d;
uint8_t power, line, wait, setwatch, sethour, setmin, setdate, setmon, setdow, arrow, save, trig;

//char от -128 до 127
int8_t vol, treb, middle, bass, gain, gain1, gain2, gain3, gain4, lon, graf, ledwait, rf, lf, rt, lt, sub;
int8_t a[6], equal, smenu0, smenu1, smenu2, in, smenu3, smenu4, smenu5, phone, mute, light, faza, out;
int8_t menu = 10, sub_out, lon_f, bastre, lonmid, treb_c, mid_c, bas_c, sub_f, treb_q, mid_q, bas_q;

void setup() {
  Wire.begin(); lcd.init(); irrecv.enableIRIn(); rtc.begin();
  bd.mix(); bd.setSetup_1(1, 2, 0); //default (1,2,0)
  MsTimer2::set(5, to_Timer); MsTimer2::start();

  pinMode(13, OUTPUT); // POWER ON/OFF (ENABLES/DISABLES RELAY)
  pinMode(12, INPUT);// КНОПКА ПОДМЕНЮ (ВЫВОД SW ЭНКОДЕРА)
  pinMode(11, INPUT);// КНОПКА ПОВОРОТА (ВЫВОД DT ЭНКОДЕРА)
  pinMode(10, INPUT);// КНОПКА ПОВОРОТА (ВЫВОД CLK ЭНКОДЕРА)
  pinMode(9, OUTPUT);// ПОДСВЕТКА КНОПКИ НАУШНИКИ (HEADPHONES)
  pinMode(8, OUTPUT);// ПОДСВЕТКА КНОПКИ ОТКЛЮЧИТЬ ЗВУК (MUTE)
  pinMode(7, OUTPUT);// ПОДСВЕТКА КНОПКИ ПИТАНИЕ (POWER/STANDBY)
  pinMode(6, OUTPUT);// ПОДСВЕТКА КНОПКИ МЕНЮ (MENU)
  pinMode(3,  INPUT);// INFRARED RECEIVER
  pinMode(2, OUTPUT);// POWER ON/OFF BLUETOOTH MODUL
  pinMode(A0, INPUT);// КНОПКА НАУШНИКИ (HEADPHONES) ON/OFF
  pinMode(A1, INPUT);// КНОПКА ОТКЛЮЧИТЬ ЗВУК (MUTE) ON/OFF
  pinMode(A2, INPUT);// КНОПКА ПИТАНИЕ (POWER/STANDBY)
  pinMode(A3, INPUT);// КНОПКА МЕНЮ (MENU)
  digitalWrite(13, LOW);
  ////////////////////////////////////////// EEPROM READ //////////////////////////////////////////
  vol = EEPROM.read(0) - 79; treb = EEPROM.read(1) - 20; middle = EEPROM.read(2) - 20; bass = EEPROM.read(3) - 20;
  in = EEPROM.read(4); gain1 = EEPROM.read(5); gain2 = EEPROM.read(6); gain3 = EEPROM.read(7); gain4 = EEPROM.read(8);
  lon = EEPROM.read(9); lon_f = EEPROM.read(10); rf = EEPROM.read(11) - 79; lf = EEPROM.read(12) - 79;
  rt = EEPROM.read(13) - 79; lt = EEPROM.read(14) - 79; sub = EEPROM.read(15) - 79; treb_c = EEPROM.read(16);
  mid_c = EEPROM.read(17); bas_c = EEPROM.read(18); sub_f = EEPROM.read(19); treb_q = EEPROM.read(20);
  mid_q = EEPROM.read(21); bas_q = EEPROM.read(22); faza = EEPROM.read(23); light = EEPROM.read(24);
  wait = EEPROM.read(25); phone = EEPROM.read(27); gain = EEPROM.read(28);
  //РАСКОМЕНТИРОВАТЬ ПРИ САМОМ ПЕРВОМ ВКЛЮЧЕНИИ
  //if(EEPROM.read(50)!=0){for(int i=0;i<51;i++){EEPROM.update(i,0);}}//Oчистка памяти
  //if(wait<5)wait=10; //Установка wait=10 сек.
  //Serial.begin(9600);//ЗАКОМЕНТИРОВАТЬ ПЕРЕД ФИНАЛЬНОЙ ЗАГРУЗКОЙ
}
/////////////////////////////////////////// END SETUP ////////////////////////////////////////////
void loop() {
  /////////////////////////////////////// IR REMOTE CONTROL ////////////////////////////////////////
  if (irrecv.decode(&ir)) { //В мониторе порта отображаются коды кнопок для использования любого пульта
    //Serial.print("0x");Serial.println(ir.value,HEX);//ЗАКОМЕНТИРОВАТЬ ПЕРЕД ФИНАЛЬНОЙ ЗАГРУЗКОЙ
    if (ir.value != IR_10) {
      irkey = ir.value;  //Предотвращаем повторы от случайных кнопок
      delay(150);
    }
    irrecv.resume();
  }

  /////////////////////////////////////// POWER ON/OFF KEYS ////////////////////////////////////////
  btn = analogRead(A2); if (btn != oldbtn2) {
    delay(40);
    btn = analogRead(A2);
  }
  if (btn > 900 && oldbtn2 < 900 || ir.value == IR_7) {
    power++;
    if (power > 1) {
      power = 0;
    } cl2();
    w2 = 1;
  }
  oldbtn2 = btn;

  //////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////// POWER ON ////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////
  if (power == 1) {
    if (w2 == 1) {
      mute = 0; menu = 0; smenu0 = 0; save = 1; w = 0; indicator(); phon(); lcd.backlight(); lcd.clear(); cl4();
      digitalWrite(7, HIGH); //POWER ON LED "POWER"
      digitalWrite(6, HIGH); //POWER ON LED "MENU"
      if (in == 0) {
        digitalWrite(2, HIGH); //POWER ON/OFF BLUETOOTH MODUL
      } else {
        digitalWrite(2, LOW);
      }
      w2 = 0;
    }

    /////////////////////////////////////////// HEADPHONES //////////////////////////////////////////
    btn = analogRead(A0); if (btn != oldbtn0) {
      delay(40);
      btn = analogRead(A0);
    }
    if (btn > 900 && oldbtn0 < 900 && mute == 0 || ir.value == IR_9 && mute == 0) {
      phone++; if (phone > 1) {
        phone = 0;
      } cl2();
      phon();
    } oldbtn0 = btn;

    ////////////////////////////////////////////// MUTE /////////////////////////////////////////////
    btn = analogRead(A1); if (btn != oldbtn1) {
      delay(40);
      btn = analogRead(A1);
    }
    if (btn > 900 && oldbtn1 < 900 || ir.value == IR_8) {
      mute++; if (mute > 1) {
        mute = 0;
      } audio(); cl2(); lcd.clear();
      switch (mute) {
        case 0: menu = 0; smenu0 = 0; cl4(); digitalWrite(8, LOW); digitalWrite(6, HIGH); break; //MUTE OFF
        case 1: menu = 9; w = 0; digitalWrite(8, HIGH); digitalWrite(6, LOW); break;
      }
    }//MUTE ON
    oldbtn1 = btn;

    ////////////////////////////////////////////// MENU /////////////////////////////////////////////
    btn = analogRead(A3); if (btn != oldbtn3) {
      delay(40);
      btn = analogRead(A3);
    }
    if (btn > 900 && oldbtn3 < 900 && mute == 0 || ir.value == IR_5 && mute == 0) {
      menu++; if (menu > 5) {
        menu = 0;
      } cl1(); lcd.clear();
    } oldbtn3 = btn; //МЕНЮ ВНИЗ
    if (ir.value == IR_6 && mute == 0) {
      menu--;  //МЕНЮ ВВЕРХ
      if (menu < 0) {
        menu = 5;
      } cl1();
      lcd.clear();
    }

    //////////////////////////////////////// RETURN MAIN MENU //////////////////////////////////////
    if (ir.value == IR_11 && mute == 0) {
      menu = 0;  //Возврат в главное меню из любого
      smenu0 = 0;
      cl2();
      lcd.clear();
    }

    //////////////////////////////////// VOLUME -79 ... +15 dB //////////////////////////////////////
    ///////////////////////////////////// BASS -20 ... +20 dB ///////////////////////////////////////
    //////////////////////////////////// MIDDLE -20 ... +20 dB //////////////////////////////////////
    //////////////////////////////////// TREBLE -20 ... +20 dB //////////////////////////////////////
    if (menu == 0) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        smenu0++;
        if (smenu0 > 3) {
          smenu0 = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        smenu0--;
        if (smenu0 < 0) {
          smenu0 = 3;
        } cl1();
      }
      switch (smenu0) {
        case 0: equal = vol; break;
        case 1: equal = bass; break;
        case 2: equal = middle; break;
        case 3: equal = treb; break;
      }
      if (newPosition != oldPosition) {
        equal = equal + newPosition;
        cl4();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        equal++;
        cl2();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        equal--;
        cl2();
      }
      if (w1 == 1) {
        //VOLUME
        lcd.setCursor(0, 0);
        if (smenu0 == 0) {
          vol = equal; vol = constrain(vol, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("VOL "); vol_g();
        //BASS
        lcd.setCursor(0, 1);
        if (smenu0 == 1) {
          bass = equal; bass = constrain(bass, -20, 20);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("BAS "); graf = bass; line = 1; graf_g();
      }
      //MIDDLE
      lcd.setCursor(0, 2);
      if (smenu0 == 2) {
        middle = equal; middle = constrain(middle, -20, 20);
        lcd.write(uint8_t(3));
      } else {
        lcd.print(' ');
      }
      lcd.print("MID "); graf = middle; line = 2; graf_g();
      //TREBLE
      lcd.setCursor(0, 3);
      if (smenu0 == 3) {
        treb = equal; treb = constrain(treb, -20, 20);
        lcd.write(uint8_t(3));
      } else {
        lcd.print(' ');
      }
      lcd.print("TRE "); graf = treb; line = 3; graf_g();
      audio(); w1 = 0;
    }
    ///////////////////////////// BASS FREQUENCY + BASS QUALITY FACTOR /////////////////////////////
    /////////////////////////// TREBLE FREQUENCY + TREBLE QUALITY FACTOR ///////////////////////////
    if (menu == 1) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        smenu1++;
        if (smenu1 > 3) {
          smenu1 = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        smenu1--;
        if (smenu1 < 0) {
          smenu1 = 3;
        } cl1();
      }
      switch (smenu1) {
        case 0: bastre = bas_c; break;
        case 1: bastre = bas_q; break;
        case 2: bastre = treb_c; break;
        case 3: bastre = treb_q; break;
      }
      if (newPosition != oldPosition) {
        bastre = bastre + newPosition;
        cl3();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        bastre++;
        cl1();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        bastre--;
        cl1();
      }
      if (w1 == 1) {
        lcd.setCursor(0, 0); //"Bass Center"
        if (smenu1 == 0) {
          bas_c = bastre; if (bas_c > 3) {
            bas_c = 0;
          } if (bas_c < 0) {
            bas_c = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("BASS FREQ. ");
        switch (bas_c) {
          case 0: lcd.print(" 6"); break;
          case 1: lcd.print(" 8"); break;
          case 2: lcd.print("10"); break;
          case 3: lcd.print("12"); break;
        } lcd.print("0 Hz");
        lcd.setCursor(0, 1); //"Bass Q"
        if (smenu1 == 1) {
          bas_q = bastre; if (bas_q > 3) {
            bas_q = 0;
          } if (bas_q < 0) {
            bas_q = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("BASS Q.FACTOR ");
        switch (bas_q) {
          case 0: lcd.print("0.5"); break;
          case 1: lcd.print("1.0"); break;
          case 2: lcd.print("1.5"); break;
          case 3: lcd.print("2.0"); break;
        }
        lcd.setCursor(0, 2); //"Treble Center"
        if (smenu1 == 2) {
          treb_c = bastre; if (treb_c > 3) {
            treb_c = 0;
          } if (treb_c < 0) {
            treb_c = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("TREB FREQ. ");
        switch (treb_c) {
          case 0: lcd.print(" 7.5"); break;
          case 1: lcd.print("10.0"); break;
          case 2: lcd.print("12.5"); break;
          case 3: lcd.print("15.0"); break;
        } lcd.print(" kHz");
        lcd.setCursor(0, 3); //"Treble Q"
        if (smenu1 == 3) {
          treb_q = bastre; if (treb_q > 1) {
            treb_q = 0;
          } if (treb_q < 0) {
            treb_q = 1;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("TREB Q.FACTOR ");
        switch (treb_q) {
          case 0: lcd.print("0.75"); break;
          case 1: lcd.print("1.25"); break;
        }
        audio(); w1 = 0;
      }
    }
    ///////////////////////////// LOUDNESS GAIN + LOUDNESS CENTER /////////////////////////////
    ///////////////////////// MIDDLE FREQUENCY + MIDDLE QUALITY FACTOR ////////////////////////
    if (menu == 2) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        smenu2++;
        if (smenu2 > 3) {
          smenu2 = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        smenu2--;
        if (smenu2 < 0) {
          smenu2 = 3;
        } cl1();
      }
      switch (smenu2) {
        case 0: lonmid = lon; break;
        case 1: lonmid = lon_f; break;
        case 2: lonmid = mid_c; break;
        case 3: lonmid = mid_q; break;
      }
      if (newPosition != oldPosition) {
        lonmid = lonmid + newPosition;
        cl3();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        lonmid++;
        cl1();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        lonmid--;
        cl1();
      }
      if (w1 == 1) {
        lcd.setCursor(0, 0); //"Loudness gain"
        if (smenu2 == 0) {
          lon = lonmid; lon = constrain(lon, 0, 20);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("LOUDNESS "); if (lon > 0) {
          lcd.print('+');
        }
        else {
          lcd.print(' ');
        } lcd.print(lon); lcd.print(' ');
        lcd.setCursor(0, 1); //"Loudness Frequency"
        if (smenu2 == 1) {
          lon_f = lonmid; if (lon_f > 3) {
            lon_f = 0;
          } if (lon_f < 0) {
            lon_f = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("LOUD CENTER ");
        switch (lon_f) {
          case 0: lcd.print("250Hz"); break;
          case 1: lcd.print("400Hz"); break;
          case 2: lcd.print("800Hz"); break;
          case 3: lcd.print("CLOSE"); break;
        }
        lcd.setCursor(0, 2); //"Middle Center"
        if (smenu2 == 2) {
          mid_c = lonmid; if (mid_c > 3) {
            mid_c = 0;
          } if (mid_c < 0) {
            mid_c = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("MIDD FREQ. ");
        switch (mid_c) {
          case 0: lcd.print("0.5"); break;
          case 1: lcd.print("1.0"); break;
          case 2: lcd.print("1.5"); break;
          case 3: lcd.print("2.5"); break;
        } lcd.print(" kHz");
        lcd.setCursor(0, 3); //"Middle Q"
        if (smenu2 == 3) {
          mid_q = lonmid; if (mid_q > 3) {
            mid_q = 0;
          } if (mid_q < 0) {
            mid_q = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("MIDD Q.FACTOR ");
        switch (mid_q) {
          case 0: lcd.print("0.75"); break;
          case 1: lcd.print("1.00"); break;
          case 2: lcd.print("1.25"); break;
          case 3: lcd.print("1.50"); break;
        }
        audio(); w1 = 0;
      }
    }
    //////////////////////////////////////// INPUT SELECTION //////////////////////////////////////////
    /////////////////////////////////////// GAIN 0 ... +20 dB /////////////////////////////////////////
    if (menu == 3) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        in++;
        if (in > 3) {
          in = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        in--;
        if (in < 0) {
          in = 3;
        } cl1();
      }
      switch (in) {
        case 0: gain = gain1; break;
        case 1: gain = gain2; break;
        case 2: gain = gain3; break;
        case 3: gain = gain4; break;
      }
      if (newPosition != oldPosition) {
        gain = gain + newPosition;
        cl3();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        gain++;
        cl1();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        gain--;
        cl1();
      }
      if (w1 == 1) {
        gain = constrain(gain, 0, 20);
        //INPUT BT GAIN
        lcd.setCursor(0, 0);
        if (in == 0) {
          gain1 = gain;
          lcd.write(uint8_t(3));
          digitalWrite(2, HIGH);
        } else {
          lcd.print(' ');
        }
        lcd.print("BT IN G "); if (gain1 > 0) {
          lcd.print('+');
        } else {
          lcd.print(' ');
        }
        lcd.print(gain1); lcd.print(' '); gain_d = gain1; line = 0; gain_g();
        //INPUT RCA GAIN
        lcd.setCursor(0, 1);
        if (in == 1) {
          gain2 = gain;
          lcd.write(uint8_t(3));
          digitalWrite(2, LOW);
        } else {
          lcd.print(' ');
        }
        lcd.print("RCA   A "); if (gain2 > 0) {
          lcd.print('+');
        } else {
          lcd.print(' ');
        }
        lcd.print(gain2); lcd.print(' '); gain_d = gain2; line = 1; gain_g();
        //INPUT IN 1 GAIN
        lcd.setCursor(0, 2);
        if (in == 2) {
          gain3 = gain;
          lcd.write(uint8_t(3));
          digitalWrite(2, LOW);
        } else {
          lcd.print(' ');
        }
        lcd.print("IN 1  I "); if (gain3 > 0) {
          lcd.print('+');
        } else {
          lcd.print(' ');
        }
        lcd.print(gain3); lcd.print(' '); gain_d = gain3; line = 2; gain_g();
        //INPUT IN 2 GAIN
        lcd.setCursor(0, 3);
        if (in == 3) {
          gain4 = gain;
          lcd.write(uint8_t(3));
          digitalWrite(2, LOW);
        } else {
          lcd.print(' ');
        }
        lcd.print("IN 2  N "); if (gain4 > 0) {
          lcd.print('+');
        } else {
          lcd.print(' ');
        }
        lcd.print(gain4); lcd.print(' '); gain_d = gain4; line = 3; gain_g();
      }
      audio(); w1 = 0;
    }
    ////////////////////////////////// OUTPUT SELECTION LEVEL ///////////////////////////////////
    /////////////////////////////////// Subwoofer Frequency /////////////////////////////////////
    //////////////////////////////////////// LPF Phase //////////////////////////////////////////
    if (menu == 4) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        smenu3++;
        if (smenu3 > 7) {
          smenu3 = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        smenu3--;
        if (smenu3 < 0) {
          smenu3 = 7;
        } cl1();
      }
      switch (smenu3) {
        case 0: out = lf;   break;
        case 1: out = rf;   break;
        case 2: out = lt;   break;
        case 3: out = rt;   break;
        case 4: out = sub;  break;
        case 5: out = sub_out; break;
        case 6: out = sub_f; break;
        case 7: out = faza; break;
      }
      if (newPosition != oldPosition) {
        out = out + newPosition;
        cl3();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        out++;
        cl1();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        out--;
        cl1();
      }
      if (w1 == 1) {
        lcd.setCursor(0, 0);
        if (smenu3 == 0) {
          lf = out;
          lf = constrain(lf, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("L-SP "); if (lf > 0) {
          lcd.print('+');
        } if (lf == 0) {
          lcd.print(' ');
        } lcd.print(lf); lcd.print(' ');
        lcd.setCursor(10, 0);
        if (smenu3 == 1) {
          rf = out;
          rf = constrain(rf, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("R-SP "); if (rf > 0) {
          lcd.print('+');
        } if (rf == 0) {
          lcd.print(' ');
        } lcd.print(rf); lcd.print(' ');
        lcd.setCursor(0, 1);
        if (smenu3 == 2) {
          lt = out;
          lt = constrain(lt, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("L-PH "); if (lt > 0) {
          lcd.print('+');
        } if (lt == 0) {
          lcd.print(' ');
        } lcd.print(lt); lcd.print(' ');
        lcd.setCursor(10, 1);
        if (smenu3 == 3) {
          rt = out;
          rt = constrain(rt, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("R-PH "); if (rt > 0) {
          lcd.print('+');
        } if (rt == 0) {
          lcd.print(' ');
        } lcd.print(rt); lcd.print(' ');
        lcd.setCursor(0, 2); //"Subwoofer OUT"
        if (smenu3 == 4) {
          sub = out;
          sub = constrain(sub, -79, 15);
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        }
        lcd.print("SUBW "); if (sub > 0) {
          lcd.print('+');
        } if (sub == 0) {
          lcd.print(' ');
        } lcd.print(sub); lcd.print(' ');
        lcd.setCursor(10, 2); //Subwoofer Output Select
        if (smenu3 == 5) {
          sub_out = out; if (sub_out > 3) {
            sub_out = 0;
          } if (sub_out < 0) {
            sub_out = 3;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("OUT  ");
        switch (sub_out) {
          case 0: lcd.print("LPF"); break;
          case 1: lcd.print("SPK"); break;
          case 2: lcd.print("HED"); break;
          case 3: lcd.print("BAN"); break;
        }
        lcd.setCursor(0, 3); //Subwoofer LPF fc
        if (smenu3 == 6) {
          sub_f = out; if (sub_f > 4) {
            sub_f = 0;
          } if (sub_f < 0) {
            sub_f = 4;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("FREQ ");
        switch (sub_f) {
          case 0: lcd.print("CLOSE"); break;
          case 1: lcd.print("55 Hz"); break;
          case 2: lcd.print("85 Hz"); break;
          case 3: lcd.print("120Hz"); break;
          case 4: lcd.print("160Hz"); break;
        }
        lcd.setCursor(12, 3); //"LPF Phase"
        if (smenu3 == 7) {
          faza = out; if (faza > 1) {
            faza = 0;
          } if (faza < 0) {
            faza = 1;
          }
          lcd.write(uint8_t(3));
        } else {
          lcd.print(' ');
        } lcd.print("PHA ");
        switch (faza) {
          case 0: lcd.print("0  "); break;
          case 1: lcd.print("180"); break;
        }
      }
      audio(); w1 = 0;
    }
    ///////////////////////////////////////// NIGHT BACKLIGHT /////////////////////////////////////////
    if (menu == 5) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH) {
        smenu4++;
        if (smenu4 > 2) {
          smenu4 = 0;
        } cl1();
      } oldbtn4 = btn;
      if (ir.value == IR_1) {
        smenu4++;
        if (smenu4 > 2) {
          smenu4 = 0;
        } cl1();
      }
      if (ir.value == IR_2) {
        smenu4--;
        if (smenu4 < 0) {
          smenu4 = 2;
        } cl1();
      }
      switch (smenu4) {
        case 0: ledwait = light; break;
        case 1: ledwait = wait; break;
      }
      if (newPosition != oldPosition) {
        ledwait = ledwait + newPosition;
        cl3();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        ledwait++;
        cl1();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        ledwait--;
        cl1();
      }
      if (w1 == 1) {
        lcd.setCursor(0, 0);
        if (smenu4 == 0) {
          light = ledwait;
          if (light > 1) {
            light = 0;
          } if (light < 0) {
            light = 1;
          } lcd.write(uint8_t(3));
        }
        else {
          lcd.print(' ');
        }
        lcd.print("NIGHT BACKLIGHT "); if (light == 0) {
          lcd.print("OFF");
        } else {
          lcd.print("ON ");
        }
        lcd.setCursor(0, 1);
        if (smenu4 == 1) {
          wait = ledwait;
          wait = constrain(wait, 5, 30);
          lcd.write(uint8_t(3));
        }
        else {
          lcd.print(' ');
        }
        lcd.print("WAIT MENU (SEC) "); lcd.print(wait); lcd.print(' ');
        lcd.setCursor(0, 2);
        if (smenu4 == 2) { //lcd.write(uint8_t(3));
          lcd.print(F("FOR CHANGE POWER OFF"));
          lcd.setCursor(0, 3);
          lcd.print(F("AND PRESS KEY 'MENU'"));
        }
        else {
          time_func();
        }
      }
    }
    ///////////////////////////////////////////// MUTE /////////////////////////////////////////////
    if (menu == 9) {
      if (oldtime + 1000 < millis()) {
        trig++;
        if (trig > 1) {
          trig = 0;
        } oldtime = millis();
      }
      lcd.setCursor(8, 0);
      switch (trig) {
        case 0: lcd.print("MUTE"); break;
        case 1: lcd.print("    "); break;
      }
      time_func();
    }
    //////////////////////////////////// RETURN TO THE MAIN MENU ////////////////////////////////////
    if (millis() - time > wait * 1000 && w == 1) {
      menu = 0;
      smenu0 = 0;
      w = 0;
      w1 = 1;
      lcd.clear();
    }
  }

  ////////////////////////////////////// POWER OFF (STANDBY) //////////////////////////////////////
  if (power == 0) {
    if (w2 == 1) { //SYMBOLS OF THE BIG CLOCK//byte v7[8]={ 0, 0, 0, 0, 0, 0, 0, 0};
      byte v1[8] = { 7, 7, 7, 7, 7, 7, 7, 7};
      byte v2[8] = {28, 28, 28, 28, 28, 28, 28, 28};
      byte v3[8] = {31, 31, 31, 0, 0, 0, 0, 0};
      byte v4[8] = {28, 28, 28, 0, 0, 0, 0, 0};
      byte v5[8] = { 7, 7, 7, 0, 0, 0, 0, 0};
      byte v6[8] = { 0, 0, 0, 0, 0, 12, 12, 0}; //Точка для двоеточия
      byte v7[8] = {24, 28, 30, 31, 30, 28, 24, 0}; //Треугольник-указатель для меню установки времени
      lcd.createChar(1, v1); lcd.createChar(2, v2); lcd.createChar(3, v3); lcd.createChar(4, v4);
      lcd.createChar(5, v5); lcd.createChar(6, v6); lcd.createChar(7, v7);
      menu = 10; mute = 1; save = 1; w = 0; w1 = 1; w2 = 0; w3 = light; audio(); lcd.clear();
      digitalWrite(2, LOW); digitalWrite(6, LOW); digitalWrite(7, LOW); digitalWrite(8, LOW); digitalWrite(9, LOW);
      digitalWrite(13, LOW);
      if (light == 0) {
        lcd.noBacklight(); //ПОДСВЕТКА ЭКРАНА В РЕЖИМЕ STANDBY ON/OFF
      } else {
        lcd.backlight();
      }
    }
    ///////////////////КНОПКА MENU ПЕРЕКЛЮЧАЕТ ЧАСЫ И МЕНЮ УСТАНОВКИ ВРЕМЕНИ ////////////////////////
    btn = analogRead(A3); if (btn != oldbtn3) {
      delay(40);
      btn = analogRead(A3);
    }
    if (btn > 900 && oldbtn3 < 900 || ir.value == IR_5 || ir.value == IR_6) {
      menu++; if (menu == 11) {
        set_time();
        setwatch = 0;
      } else {
        menu = 10;
      } cl2(); lcd.clear();
    } oldbtn3 = btn;

    if (menu == 10) {
      big_time();
    }

    ////////////////////////////////////// SET TIME & DATE //////////////////////////////////////////
    if (menu == 11) {
      sw();
      if (btn == LOW && oldbtn4 == HIGH || ir.value == IR_1) {
        smenu5++;
        if (smenu5 > 6) {
          smenu5 = 0;
        } cl2();
      } oldbtn4 = btn;
      if (ir.value == IR_2) {
        smenu5--;
        if (smenu5 < 0) {
          smenu5 = 6;
        } cl2();
      }
      switch (smenu5) {
        case 0: setime = sethour; break;
        case 1: setime = setmin; break;
        case 2: setime = setdate; break;
        case 3: setime = setmon; break;
        case 4: setime = setyear; break;
        case 5: setime = setdow; break;
        case 6: setime = setwatch; break;
      }
      if (newPosition != oldPosition) {
        setime = setime + newPosition;
        cl4();
      }
      if (ir.value == IR_3 || ir.value == IR_10 && irkey == IR_3) {
        setime++;
        cl2();
      }
      if (ir.value == IR_4 || ir.value == IR_10 && irkey == IR_4) {
        setime--;
        cl2();
      }
      if (w1 == 1) {
        lcd.setCursor(0, 0);
        if (smenu5 == 0) {
          sethour = setime; if (sethour > 23) {
            sethour = 0;
          } if (sethour < 0) {
            sethour = 23;
          }
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("HOUR "); lcd.print(sethour); lcd.print(' ');
        lcd.setCursor(10, 0);
        if (smenu5 == 1) {
          setmin = setime; if (setmin > 59) {
            setmin = 0;
          } if (setmin < 0) {
            setmin = 59;
          }
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("MINUT "); lcd.print(setmin); lcd.print(' ');
        lcd.setCursor(0, 1);
        if (smenu5 == 2) {
          setdate = setime; if (setdate > 31) {
            setdate = 1;
          } if (setdate < 1) {
            setdate = 31;
          }
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("DATE "); lcd.print(setdate); lcd.print(' ');
        lcd.setCursor(10, 1);
        if (smenu5 == 3) {
          setmon = setime; if (setmon > 12) {
            setmon = 1;
          } if (setmon < 1) {
            setmon = 12;
          }
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("MONTH "); lcd.print(setmon); lcd.print(' ');
        lcd.setCursor(0, 2);
        if (smenu5 == 4) {
          setyear = setime; setyear = constrain(setyear, 2020, 2050);
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("YEAR "); lcd.print(setyear);
        lcd.setCursor(10, 2);
        if (smenu5 == 5) {
          setdow = setime; if (setdow > 7) {
            setdow = 1;
          } if (setdow < 1) {
            setdow = 7;
          }
          lcd.write(uint8_t(7));
        } else {
          lcd.print(' ');
        } lcd.print("WEEKDAY "); lcd.print(setdow);
        lcd.setCursor(0, 3);
        if (smenu5 == 6) {
          setwatch = setime;
          lcd.write(uint8_t(7));
          lcd.print(F("ENTER '+/-' TO SAVE"));
        }
        else {
          lcd.print(rtc.getTimeStr());
          lcd.print("  ");
          lcd.print(rtc.getDateStr());
        }
        if (setwatch != 0) {
          time_set(); lcd.clear(); lcd.print(F("TIME AND DATE SAVE!")); setwatch = 0; menu = 10;
          delay(2000); lcd.clear();
        }
      }
    }
    ///////////////////// ПОДСВЕТКА ЭКРАНА ON/OFF LED STANDBY (КНОПКА MUTE) ///////////////////////
    btn = analogRead(A1); if (btn != oldbtn1) {
      delay(40);
      btn = analogRead(A1);
    }
    if (btn > 900 && oldbtn1 < 900 || ir.value == IR_8) {
      w3++; if (w3 > 1) {
        w3 = 0;
      } cl2();
      if (w1 == 1) {
        switch (w3) {
          case 0: lcd.noBacklight(); break;
          case 1: lcd.backlight(); break;
        }
      } w1 = 0;
    } oldbtn1 = btn;
  }
  //////////////////////////////////////// END POWER OFF ////////////////////////////////////////

  //////////////////////////////////////// EEPROM UPDATE ////////////////////////////////////////
  if (save == 1) {
    EEPROM.update(0, vol + 79); EEPROM.update(1, treb + 20); EEPROM.update(2, middle + 20); EEPROM.update(3, bass + 20);
    EEPROM.update(4, in); EEPROM.update(5, gain1); EEPROM.update(6, gain2); EEPROM.update(7, gain3);
    EEPROM.update(8, gain4); EEPROM.update(9, lon); EEPROM.update(10, lon_f); EEPROM.update(11, rf + 79);
    EEPROM.update(12, lf + 79); EEPROM.update(13, rt + 79); EEPROM.update(14, lt + 79); EEPROM.update(15, sub + 79);
    EEPROM.update(16, treb_c); EEPROM.update(17, mid_c); EEPROM.update(18, bas_c); EEPROM.update(19, sub_f);
    EEPROM.update(20, treb_q); EEPROM.update(21, mid_q); EEPROM.update(22, bas_q); EEPROM.update(23, faza);
    EEPROM.update(24, light); EEPROM.update(25, wait); EEPROM.update(27, phone); EEPROM.update(28, gain);
    save = 0;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// END LOOP ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void cl1() {
  time = millis(); w = 1; ir.value = 0; w1 = 1;
}
void cl2() {
  ir.value = 0; w1 = 1;
}
void cl3() {
  time = millis(); w = 1; myEnc.write(0); oldPosition = 0; newPosition = 0; w1 = 1;
}
void cl4() {
  myEnc.write(0); oldPosition = 0; newPosition = 0; w1 = 1;
}
void sw() {
  btn = digitalRead(12); if (btn != oldbtn4) {
    delay(15);
    btn = digitalRead(12);
  }
}
void indicator() { //SYMBOLS FOR INDICATORS
  byte v1[8] = {21, 21, 21, 21, 21, 21, 21, 0};
  byte v2[8] = {20, 20, 20, 20, 20, 20, 20, 0};
  byte v3[8] = {16, 16, 16, 16, 16, 16, 16, 0};
  byte v4[8] = {24, 28, 30, 31, 30, 28, 24, 0}; //Треугольник-указатель (обычный треугольник)
  lcd.createChar(0, v1); lcd.createChar(1, v2); lcd.createChar(2, v3); lcd.createChar(3, v4);
}
void phon() {
  switch (phone) {
    case 0: digitalWrite(13, HIGH); digitalWrite(9, LOW); break; //HEADPHONES OFF
    case 1: digitalWrite(13, LOW); digitalWrite(9, HIGH); break;
  }//HEADPHONES ON
}
void big_time() { //БОЛЬШИЕ ЧАСЫ
  wt = rtc.getTime(); //Считываем текущее время
  //Присваиваем элементам из массива а[6] значения каждого сегмента из 3 двухзначных цифр времени
  a[0] = wt.hour / 10;
  a[1] = wt.hour % 10;
  a[2] = wt.min / 10;
  a[3] = wt.min % 10;
  a[4] = wt.sec / 10;
  a[5] = wt.sec % 10;
  //Циклично устанавливаем значение трёх позиций в строке (из которых состоит одна цифра) всего цифр 6
  //Первая цифра (часы) занимает позиции 0,1,2 для цифры №1, затем 3,4,5 для цифры №2 затем пропуск,
  //Втора цифра (минуты) занимает позиции 7,8,9 для цифры №3, затем 10,11,12 для цифры №4 затем пропуск,
  //Третья цифра (секунды) занимает позиции 14,15,16 для цифры №5, затем 17,18,19 для цифры №6
  for (i = 0; i < 6; i++) {
    switch (i) {
      case 0: e1 = 0, e2 = 1, e3 = 2; break;
      case 1: e1 = 3, e2 = 4, e3 = 5; break;
      case 2: e1 = 7, e2 = 8, e3 = 9; break;
      case 3: e1 = 10, e2 = 11, e3 = 12; break;
      case 4: e1 = 14, e2 = 15, e3 = 16; break;
      case 5: e1 = 17, e2 = 18, e3 = 19; break;
    }
    //Массив описывающий из каких элементов состоят 10 цифр, "а" (по 3 элемента на строку)
    //всего по 9 элементов на каждую цифру.
    switch (a[i]) {
      case 0: d1 = 1, d2 = 3, d3 = 2, d4 = 1, d5 = 32, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "0"
      case 1: d1 = 5, d2 = 2, d3 = 32, d4 = 32, d5 = 2, d6 = 32, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "1"
      case 2: d1 = 5, d2 = 3, d3 = 2, d4 = 1, d5 = 3, d6 = 4, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "2"
      case 3: d1 = 5, d2 = 3, d3 = 2, d4 = 32, d5 = 3, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "3"
      case 4: d1 = 1, d2 = 32, d3 = 2, d4 = 5, d5 = 3, d6 = 2, d7 = 32, d8 = 32, d9 = 4; break; //Цифра "4"
      case 5: d1 = 1, d2 = 3, d3 = 4, d4 = 5, d5 = 3, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "5"
      case 6: d1 = 1, d2 = 3, d3 = 4, d4 = 1, d5 = 3, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "6"
      case 7: d1 = 5, d2 = 3, d3 = 2, d4 = 32, d5 = 32, d6 = 2, d7 = 32, d8 = 32, d9 = 4; break; //Цифра "7"
      case 8: d1 = 1, d2 = 3, d3 = 2, d4 = 1, d5 = 3, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break; //Цифра "8"
      case 9: d1 = 1, d2 = 3, d3 = 2, d4 = 5, d5 = 3, d6 = 2, d7 = 5, d8 = 3, d9 = 4; break;
    }//Цифра "9"
    //Устанавливаем курсор в позиции где будем печатать символы из которых состоит цифра "а"
    //с порядковым номером "i" За один проход мы можем отпечатать только 1 цифру!
    //Меняя номера строк, печатаем цифру a[i]
    lcd.setCursor(e1, 0); lcd.write(d1);
    lcd.setCursor(e2, 0); lcd.write(d2);
    lcd.setCursor(e3, 0); lcd.write(d3);
    lcd.setCursor(e1, 1); lcd.write(d4);
    lcd.setCursor(e2, 1); lcd.write(d5);
    lcd.setCursor(e3, 1); lcd.write(d6);
    lcd.setCursor(e1, 2); lcd.write(d7);
    lcd.setCursor(e2, 2); lcd.write(d8);
    lcd.setCursor(e3, 2); lcd.write(d9);
  }
  //Печатаем символы точек между цифрами времени
  lcd.setCursor(6, 0); lcd.write(uint8_t(6));
  lcd.setCursor(6, 1); lcd.write(uint8_t(6));
  lcd.setCursor(13, 0); lcd.write(uint8_t(6));
  lcd.setCursor(13, 1); lcd.write(uint8_t(6));
  //В ЧЕТВЕРТОЙ СТРОКЕ ПЕЧАТАЕМ ДАТУ, ТЕМПЕРАТУРУ И ДЕНЬ НЕДЕЛИ
  lcd.setCursor(0, 3); lcd.print(rtc.getDateStr());
  lcd.print(' '); lcd.print(rtc.getTemp(), 0); lcd.write(B11011111);
  switch (wt.dow) {
    case 1: lcd.print(" MON 1"); break;
    case 2: lcd.print(" TUE 2"); break;
    case 3: lcd.print(" WED 3"); break;
    case 4: lcd.print(" THU 4"); break;
    case 5: lcd.print(" FRI 5"); break;
    case 6: lcd.print(" SAT 6"); break;
    case 7: lcd.print(" SUN 7"); break;
  }
  //Возвращаемяся в цикл чтоб определить местоположение следующей цифры на 3-х следующих позициях.
}
void set_time() {
  wt = rtc.getTime();
  sethour = wt.hour;
  setmin = wt.min;
  setdate = wt.date;
  setmon = wt.mon;
  setyear = wt.year;
  setdow = wt.dow;
}
void time_set() {
  rtc.setTime(sethour, setmin, 0);
  rtc.setDate(setdate, setmon, setyear);
  rtc.setDOW(setdow);
}
void vol_g() {
  if (vol > 0) {
    lcd.print('+');
  }
  if (vol == 0) {
    lcd.print(' ');
  }
  lcd.print(vol); lcd.print(' ');
  vol_d = map(vol, -79, 15, 0, 33);
  for (z = 0, z0 = 0, z1 = 0; z <= vol_d; z++, z1++) {
    if (z1 > 2) {
      z1 = 0;
      z0++;
    }
    if (z1 == 1) {
      lcd.setCursor(z0 + 9, 0);
      lcd.write(uint8_t(0));
      lcd.setCursor(z0 + 10, 0);
      lcd.print(' ');
    }
  }
  if (z1 == 3) {
    lcd.setCursor(z0 + 9, 0);
    lcd.write(uint8_t(1));
  }
  if (z1 == 2) {
    lcd.setCursor(z0 + 9, 0);
    lcd.write(uint8_t(2));
  }
  if (vol < -77) {
    lcd.setCursor(9, 0);
    lcd.print(' ');
  }
}
void graf_g() {
  if (graf > 0) {
    lcd.print('+');
  }
  if (graf == 0) {
    lcd.print(' ');
  }
  lcd.print(graf); lcd.print(' ');
  graf_d = map(graf, -20, 20, 0, 33);
  for (z = 0, z0 = 0, z1 = 0; z <= graf_d; z++, z1++) {
    if (z1 > 2) {
      z1 = 0;
      z0++;
    }
    if (z1 == 1) {
      lcd.setCursor(z0 + 9, (line));
      lcd.write(uint8_t(0));
      lcd.setCursor(z0 + 10, (line));
      lcd.print(' ');
    }
  }
  if (z1 == 3) {
    lcd.setCursor(z0 + 9, (line));
    lcd.write(uint8_t(1));
  }
  if (z1 == 2) {
    lcd.setCursor(z0 + 9, (line));
    lcd.write(uint8_t(2));
  }
  if (graf < -19) {
    lcd.setCursor(9, (line));
    lcd.print(' ');
  }
}
void gain_g() {
  for (z = 0, z0 = 0, z1 = 0; z <= gain_d; z++, z1++) {
    if (z1 > 2) {
      z1 = 0;
      z0++;
    }
    if (z1 == 1) {
      lcd.setCursor(z0 + 13, (line));
      lcd.write(uint8_t(0));
      lcd.setCursor(z0 + 14, (line));
      lcd.print(' ');
    }
  }
  if (z1 == 3) {
    lcd.setCursor(z0 + 13, (line));
    lcd.write(uint8_t(1));
  }
  if (z1 == 2) {
    lcd.setCursor(z0 + 13, (line));
    lcd.write(uint8_t(2));
  }
  if (gain_d == 0) {
    lcd.setCursor(13, (line));
    lcd.print(' ');
  }
}
void time_func() {
  lcd.setCursor(0, 2); lcd.print(' '); lcd.print(rtc.getTimeStr()); lcd.print("  "); lcd.print(rtc.getDOWStr());
  lcd.setCursor(0, 3); lcd.print(' '); lcd.print(rtc.getDateStr()); lcd.print("     "); lcd.print(rtc.getTemp(), 0);
  lcd.print((char)223);
  lcd.print('C');
}
void to_Timer() {
  newPosition = myEnc.read() / 4;
}
void audio() {
  bd.setLoudness_f(lon_f);          // Loudness fo (250Hz 400Hz 800Hz Prohibition) = lon_f (0...3)
  bd.setIn(in);                     // Input Selector (A, B, C, D single, E1 single, E2 single) = in (0...6)
  bd.setIn_gain(gain, mute);        // Input Gain (0...20dB) = gain (0...20), Mute ON/OFF (1-on 0-off) = mute (0...1)
  bd.setVol(vol);                   // -79...+15 dB = int -79...15
  bd.setFront_1(lf);                // -79...+15 dB = int -79...15
  bd.setFront_2(rf);                // -79...+15 dB = int -79...15
  bd.setRear_1(lt);                 // -79...+15 dB = int -79...15
  bd.setRear_2(rt);                 // -79...+15 dB = int -79...15
  bd.setSub(sub);                   // -79...+15 dB = int -79...15
  bd.setBass_setup(bas_q, bas_c);   // 0.5 1.0 1.5 2.0 bas_q 0...3, 60Hz 80Hz 100Hz 120Hz bas_c 0...3
  bd.setMiddle_setup(mid_q, mid_c); // 0.75 1.0 1.25 1.5 = int 0...3, 500Hz 1kHz 1.5kHz 2.5kHz = int 0...3
  bd.setTreble_setup(treb_q, treb_c); // 0.75 1.25         = int 0...1, 7.5kHz 10kHz 12.5kHz 15kHz = int 0...3
  bd.setMiddle_gain(middle);        // -20 ... +20 dB = int -20 ... 20
  bd.setTreble_gain(treb);          // -20 ... +20 dB = int -20 ... 20
  bd.setBass_gain(bass);            // -20 ... +20 dB = int -20 ... 20
  bd.setLoudness_gain(lon);         //   0 ... +20 dB = int   0 ... 20
  bd.setSetup_2(sub_f, sub_out, 0, faza);
}
// Subwoofer LPF fc (OFF 55Hz 85Hz 120Hz 160Hz) = sub_f (0...4)
// Subwoofer Output Select (LPF Front Rear Prohibition) = sub_out (0...3)
// Level Meter RESET = level_metr (0...1)
// LPF Phase (0 180) = faza (0...1)

///////////////////////////////////////// END ///////////////////////////////////////////////
