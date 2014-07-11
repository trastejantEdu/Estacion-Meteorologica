#include <VirtualWire.h>
#include "DHT.h" //cargamos la librería DHT

#define DHTPIN 6 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT11 (hay //otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor

const int led_pin = 13;
const int transmit_pin = 12;
const int receive_pin = 11;
const int transmit_en_pin = 3;

void setup(){
    delay(1000);
    Serial.begin(9600); // Debugging only
    Serial.println("setup");
    dht.begin(); //Se inicia el sensor
    
    // Initialise the IO and ISR
    vw_set_tx_pin(transmit_pin);
    vw_set_rx_pin(receive_pin);
    vw_set_ptt_pin(transmit_en_pin);
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000); // Bits per sec

    vw_rx_start(); // Start the receiver PLL running

    pinMode(led_pin, OUTPUT);
}

void loop(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)){
     int i;
     digitalWrite(led_pin, HIGH); // Flash a light to show received good message
     Serial.print("Temperatura: ");//Sensor del lado del emisor

     for (i = 0; i < buflen; i++){
        Serial.print(buf[i],DEC);
        Serial.print(' ');
      }

      Serial.println();
      
      Serial.print("Temperatura 2: ");
      Serial.println(temp());//Sensor del lado del receptor
      Serial.print("Humedad: ");
      Serial.println(humedad());//Sensor del lado del receptor

      digitalWrite(led_pin, LOW);
    }
}

float humedad(){return dht.readHumidity();} //Se lee la humedad
float temp(){return dht.readTemperature();} //Se lee la temperatura
