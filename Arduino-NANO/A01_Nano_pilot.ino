#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

#define CE 7
#define CSN 8

#define lY A0
#define rX A1

int manualY[2];
int manualX[2];

RF24 asmhs_base(CE, CSN); //inicjalizacja modułu nRF24L01
const byte address[][6] = {"00001", "00002"}; //ustawienie adresów dla kanałów transmisji

LiquidCrystal lcd(10, 9, 6, 5, 4, 3); //inicjalizacja wyświetlacza lcd

float recConditions[3] = {0, 0, 0}; //tablica, do której zapisywane są otrzymane od robota dane środowiskowe

void setup() {
        manualY[0] = 300;
        manualX[0] = 400;
        Serial.begin(115200);
        while(!Serial) {

        }
        if (!asmhs_base.begin()) {
                Serial.println("radio hardware is not responding!!"); //debug
        }
        else
        {
                Serial.println("Controller initialized"); //debug
        }
        asmhs_base.setPALevel(RF24_PA_HIGH);
        asmhs_base.openWritingPipe(address[0]); //otwarcie kanału wysyłającego na adresie 00001
        asmhs_base.openReadingPipe(1, address[1]); //otwarcie 1 kanału odbiorczego na adresie 00002 (nRF24L01 jest w stanie odbierać na 6 kanałach)
        asmhs_base.startListening(); //nRF24L01 jest half-duplexowy, może jednocześnie albo nasłuchiwać, albo transmitować, otwieramy nasłuch

        lcd.begin(16, 2);
        lcd.setCursor(0,0);
}

void loop() {
        manual();
}

void manual()
{
        delay(10);
        if(asmhs_base.available()) //sprawdzenie, czy w buforze nasłuchowym są jakieś dane
        {
                asmhs_base.read(&recConditions, sizeof(recConditions)); //odczytaj tablicę otrzymaną od ASMHS-1
                Serial.println(recConditions[0]); //debug
                Serial.println(recConditions[1]); //debug
                Serial.println(recConditions[2]);
                String conditions = String(recConditions[0]) + "*C | " + String(recConditions[1]) + "%"; //sformatuj otrzymane dane
                lcd.setCursor(0, 0);
                if(recConditions[1] != 0)
                {
                        lcd.print(conditions);
                }
                lcd.setCursor(9, 1);
                lcd.print(distance_normalise(recConditions[2]));
        }
        normalise();
        //Serial.println(manualY[1]);
        //Serial.println(manualX[1]);

        if(manualY[1] >= 10 or manualY[1] <= -10)
        {
                Serial.print(manualY[1]);
                lcd.setCursor(0,1);
                lcd.print("        ");
                lcd.setCursor(0,1);
                lcd.print("Y = ");
                lcd.print(manualY[1]);
                //Serial.println("---");
                //Serial.print("Y = ");
                //Serial.println(manualY[1]);
                asmhs_base.stopListening();
                asmhs_base.write(&manualY, sizeof(manualY));
                asmhs_base.startListening();
                delay(10);
        }
        else if(manualX[1] >= 10 or manualX[1] <= -10)
        {
                Serial.print(manualX[1]);
                lcd.setCursor(0,1);
                lcd.print("        ");
                lcd.setCursor(0,1);
                lcd.print("X = ");
                lcd.print(manualX[1]);
                //Serial.println("---");
                //Serial.print("X = ");
                //Serial.println(manualX[1]);
                asmhs_base.stopListening();
                //Serial.print(manualX[1]);
                asmhs_base.write(&manualX, sizeof(manualX));
                asmhs_base.startListening();
                delay(10);
        }
        else if(manualY[1] < 10 and manualY[1] > -10)
        {
                lcd.setCursor(0,1);
                lcd.print("        ");
                lcd.setCursor(0,1);
                lcd.print("STOP");
                manualY[1] = 0;
                asmhs_base.stopListening();
                asmhs_base.write(&manualY, sizeof(manualY));
                asmhs_base.startListening();
        }
        delay(10);
}

void clear()
{
        lcd.setCursor(0,0);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print("                ");
}

void normalise()
{
        manualY[1] = -1 * int((analogRead(lY) * 0.5) - 255);
        manualX[1] = -1 * int((analogRead(rX) * 0.5) - 255);
        //Serial.println(manualY[1]);
        //Serial.println(manualX[1]);
        if(manualY[1] > 255)
        {
                manualY[1] = 255;
        }
        else if(manualY[1] < -255)
        {
                manualY[1] = -255;
        }
        if(manualX[1] > 255)
        {
                manualX[1] = 255;
        }
        else if(manualX[1] < -255)
        {
                manualX[1] = -255;
        }
} //normalizacja wskazań gałek analogowych z zakresu 0:512:1024 do -255:0:255

String distance_normalise(float dist)
{
        String result = "";
        if(dist < 10)
        {
                result = "   " + String(int(dist)) + " cm";
        }
        else if(dist >= 10 && dist < 100)
        {
                result = "  " + String(int(dist)) + " cm";
        }
        else if(dist >= 00 && dist <= 200)
        {
                result = " " + String(int(dist)) + " cm";
        }
        else if(dist > 200)
        {
                result = ">200 cm";
        }
        return result;
}
