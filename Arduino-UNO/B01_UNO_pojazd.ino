#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>

#define M1 5
#define DIR1 4
#define M2 6
#define DIR2 7 //definicje pinów odpowiedzialnych za ruch i kierunek silników

#define trigPinFront 3
#define echoPinFront 2 //definicje pinów odpowiedzialnych za czujnik hc-sr04

#define CE 9
#define CSN 8

#define buzz 10

DHT dht = DHT(A0, DHT11); //inicjalizacja czujnika dht11

RF24 asmhs(CE, CSN); //inicjalizacja modułu nRF24L01

const byte address[][6] = {"00001", "00002"}; //ustawienie adresów dla kanałów transmisji

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 500; //sekcja używana w funkcji obliczania czasu do transmisji danych środowiskowych

float conditions[3]; //tablica, w której zapisywane są gotowe do transmisji dane środowiskowe

void setup()
{
        tone(buzz, 700, 500);
        Serial.begin(115200);

        dht.begin();

        pinMode(DIR1, OUTPUT);
        pinMode(DIR2, OUTPUT);
        pinMode(M1, OUTPUT);
        pinMode(M2, OUTPUT);

        pinMode(trigPinFront, OUTPUT);
        pinMode(echoPinFront, INPUT);

        if (!asmhs.begin()) {
                Serial.println("radio hardware is not responding!!"); //debug
                while (1) {}
        }
        else
        {
                Serial.println("Drone initialized"); //debug
        }
        asmhs.setPALevel(RF24_PA_HIGH);
        asmhs.openReadingPipe(1, address[0]); //otwarcie 1 kanału odbiorczego na adresie 00001 (baza ma otwarty kanał nadawczy na adresie 00001)
        asmhs.openWritingPipe(address[1]); //otwarcie kanału nadawczego na adresie 00002 (baza ma otwarty kanał 1 odbiorczy na adresie 00002)
        asmhs.startListening(); //otwarcie nasłuchu

        delay(1000); //debug
        tone(buzz, 700, 500);
        delay(500);
        tone(buzz, 1000, 500);
        startMillis = millis(); //zapisanie wartości milisekund od uruchomienia jazdy
}

void loop()
{
        manual();
}

void manual()
{
        if(asmhs.available())
        {
                currentMillis = millis();
                if(currentMillis - startMillis >= period)
                {
                        check_conditions();
                        distance();
                        Serial.println(conditions[2]);
                        asmhs.stopListening();
                        asmhs.write(&conditions, sizeof(conditions)); //wyślij do bazy tablicę z danymi środowiskowymi
                        asmhs.startListening();
                        startMillis = millis(); //zastąp początkowy czas obecnym
                }

                int recievedManual[2];
                asmhs.read(&recievedManual, sizeof(recievedManual));
                //Serial.print("recievedManual[0] = ");
                //Serial.println(recievedManual[0]);
                if(recievedManual[0] == 300)
                {
                        //Serial.println("Moving Y");
                        if(recievedManual[1] > 0)
                        {
                                //Serial.print("###");
                                //Serial.println(recievedManual[1]);
                                digitalWrite(DIR1, HIGH);
                                digitalWrite(DIR2, HIGH);
                                analogWrite(M1, abs(recievedManual[1]));
                                analogWrite(M2, abs(recievedManual[1]));
                        }
                        else if (recievedManual[1] < 0)
                        {
                                //Serial.print("###");
                                //Serial.println(recievedManual[1]);
                                digitalWrite(DIR1, LOW);
                                digitalWrite(DIR2, LOW);
                                analogWrite(M1, abs(recievedManual[1]));
                                analogWrite(M2, abs(recievedManual[1]));
                        }
                        else if(recievedManual[1] == 0)
                        {
                                halt();
                        }
                }
                else if(recievedManual[0] == 400)
                {
                        //Serial.println("Moving X");
                        if(recievedManual[1] > 0)
                        {
                                //Serial.print("###");
                                //Serial.println(recievedManual[1]);
                                digitalWrite(DIR1, LOW);
                                digitalWrite(DIR2, HIGH);
                                analogWrite(M1, abs(recievedManual[1]));
                                analogWrite(M2, abs(recievedManual[1]));
                        }
                        else if (recievedManual[1] < 0)
                        {
                                //Serial.print("###");
                                //Serial.println(recievedManual[1]);
                                digitalWrite(DIR1, HIGH);
                                digitalWrite(DIR2, LOW);
                                analogWrite(M1, abs(recievedManual[1]));
                                analogWrite(M2, abs(recievedManual[1]));
                        }
                }
        }
}

void halt()
{
        analogWrite(M1, 0);
        analogWrite(M2, 0);
} //całkowite zatrzymanie

void distance()
{
        long timing, dist;

        digitalWrite(trigPinFront, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPinFront, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPinFront, LOW); //nadanie 10 milisekundowego pulsu dźwięku

        timing = pulseIn(echoPinFront, HIGH); //funkcja rozpoczynająca nasłuch i obliczająca (w mikrosekundach) czas, który minął od rozpoczęcia nasłuchu do otrzymania fali zwrotnej wysłanego pulsu; dzięki zastosowaniu zegara 16 MHz opóźnienie spowodowane czasem linearnego wykonania kodu jest pomijalnie małe
        dist = timing * 0.034 /2; //przeliczenie otrzymanego czasu podróży fali dźwiękowej; v = 0,034 cm/us, dzielone na 2, ponieważ mamy czas podróży i powrotu
        //Serial.println(dist);
        conditions[2] = dist;
} //sprawdzenie odległości do najbliższej przeszkody

void check_conditions()
{
        conditions[0] = dht.readTemperature();
        conditions[1] = dht.readHumidity(); //wypełnij tablicę danymi środowiskowymi
        /*Serial.println("----");
           Serial.println(conditions[0]);
           Serial.println(conditions[1]);
           Serial.println("----"); //debug*/
} //sprawdzenie i wysłanie warunków środowiskowych
