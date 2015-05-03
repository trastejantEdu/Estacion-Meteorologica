#include <U8glib.h>//Libreria grafica para el display
#include <VirtualWire.h>
#include "DHT.h" //cargamos la librería DHT
#include <Wire.h>  //libreria para comunicacion I2C
#include "RTClib.h" //libreria del reloj real

//CONFIGURACIONES
#define DHTPIN 6 //Seleccionamos el pin en el que se //conectará el sensor
#define DHTTYPE DHT11 //Se selecciona el DHT11 (hay //otros DHT)
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)//tamaño de letra mediano
#define setFont_L u8g.setFont(u8g_font_7x13)//tamaño de letra grande
#define setFont_S u8g.setFont(u8g_font_chikitar)//tamaño de letra pequeño

//DECLARACION DE OBJETOS
RTC_DS1307 RTC;//Objeto para acceder al reloj real  
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
U8GLIB_ST7920_128X64 u8g(13, 11, 10, U8G_PIN_NONE); //Configuramos el display

//CONSTANTES
const int led_pin = 13;//Led indicativo
const int transmit_pin = 12;//pin de transmision RF
const int receive_pin = 9;//pin para recibir datos por RF
const int transmit_en_pin = 3;

//Variables
float temp_ext_max = 0;
float temp_ext_min = 0;
float temp_int_max = 0;
float temp_int_min = 0;

float hum_ext_max = 0;
float hum_ext_min = 0;
float hum_int_max = 0;
float hum_int_min = 0;

/*IMAGENES PARA MOSTRAR EN FUNCION DE LOS DATOS
Aqui tenemos las imagenes que se mostraran dependiendo de lo datos que obtengamos (humedad, temperatura, hora... etc). Estan en formato hexadimal y forman un array
que es enviado al display utilizando la libreria U8glib, en funcion de los valores exadecimales de cada posicion del array la libreria sabe que puntos del display tiene
que mantener encendidos y cuales apagados para mostrar la imagen (recuerda que estamos utilizando un display monocromo).
*/

//Imagen de un sol, se mostrara cuando la temperatura exterior sea positiva y estemos en horario diurno
#define soleado_img_width 32
#define soleado_img_height 32
static unsigned char soleado_img[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00,
   0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0xe1, 0xc7, 0x03, 0xc0, 0xfc, 0x9f, 0x03, 0x40, 0xfe, 0x3f, 0x01,
   0x00, 0xff, 0x7f, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x01,
   0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01, 0xc8, 0xff, 0xff, 0x0b,
   0xcc, 0xff, 0xff, 0x3b, 0xce, 0xff, 0xff, 0x3b, 0xd8, 0xff, 0xff, 0x0b,
   0xc0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x01, 0x80, 0xff, 0xff, 0x01,
   0x80, 0xff, 0xff, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x40, 0xfe, 0x3f, 0x01,
   0xc0, 0xfc, 0x9f, 0x03, 0xe0, 0xe1, 0xc7, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xe0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//Imagen de una luna, se mostrara cuando la temperatura sea positiva y nos encontremos en horario nocturno.
#define luna_width 32
#define luna_height 32
static unsigned char luna_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x80, 0x07, 0x00, 0x00,
   0xc0, 0x03, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00,
   0xf8, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00,
   0xfc, 0x03, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00,
   0xfe, 0x07, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00,
   0xff, 0x0f, 0x00, 0x00, 0xff, 0x1f, 0x00, 0x00, 0xff, 0x3f, 0x00, 0x00,
   0xfe, 0x7f, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x40,
   0xfc, 0xff, 0x1f, 0x78, 0xfc, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f,
   0xf8, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x07,
   0xc0, 0xff, 0xff, 0x03, 0x80, 0xff, 0xff, 0x01, 0x00, 0xfe, 0x7f, 0x00,
   0x00, 0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00 };
   
//Imagen de lluvia, se mostrara cundo la humedad sea superior al 80% (En Madrid(España) indicativo de lluvia o niebla)
#define lluvia_width 32
#define lluvia_height 30
static unsigned char lluvia_bits[] U8G_PROGMEM = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x38, 0x00, 0x00, 0xff, 0xff, 0x01,
   0x80, 0xff, 0xff, 0x03, 0x80, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
   0xf8, 0xff, 0xff, 0x07, 0xfc, 0xff, 0xff, 0x1f, 0xfc, 0xff, 0xff, 0x3f,
   0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x7f,
   0xf8, 0xff, 0xff, 0x1f, 0xe0, 0xfe, 0xff, 0x03, 0x00, 0xfc, 0xff, 0x01,
   0x00, 0x70, 0xfc, 0x00, 0x00, 0x00, 0x30, 0x00, 0xc0, 0x00, 0x01, 0x02,
   0xc0, 0x80, 0x01, 0x06, 0xe0, 0x81, 0x03, 0x07, 0xe0, 0xc1, 0x03, 0x07,
   0xc0, 0x88, 0x13, 0x07, 0x00, 0x18, 0x30, 0x00, 0x00, 0x1c, 0x30, 0x00,
   0x00, 0x1c, 0x78, 0x00, 0x00, 0x1c, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00 };
   
//Copo de nieve, se mostrara cuando la temperatura sea negativa
#define hielo_width 32
#define hielo_height 32
static unsigned char hielo_bits[] U8G_PROGMEM = {
   0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff,
   0xff, 0x0f, 0xf0, 0xff, 0xff, 0x0d, 0xf0, 0xff, 0xff, 0x19, 0x98, 0xff,
   0xbf, 0x38, 0x1e, 0xfd, 0x1f, 0x78, 0x1e, 0xf8, 0x3f, 0x78, 0x1e, 0xfc,
   0x7f, 0x60, 0x06, 0xfe, 0x1f, 0x00, 0x00, 0xfc, 0x3f, 0x07, 0xe0, 0xf8,
   0xff, 0x0f, 0xf0, 0xff, 0xbf, 0x1f, 0xf8, 0xff, 0x3f, 0x07, 0xe0, 0xf8,
   0x1f, 0x00, 0x00, 0xfc, 0x7f, 0x60, 0x06, 0xfe, 0x3f, 0x78, 0x1e, 0xfc,
   0x1f, 0x78, 0x1e, 0xf8, 0xbf, 0x38, 0x1c, 0xfd, 0xff, 0x39, 0x9c, 0xff,
   0xff, 0x0f, 0xb0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x7f, 0xfe, 0xff,
   0xff, 0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f };

void draw(void) {

  u8g.setFont(u8g_font_unifont);
  u8g.drawStr( 0, 22, "Iniciando....");
}

void setup(){
  u8g.drawStr( 0, 22, "Iniciando....");
  delay(1000);
  Serial.begin(9600); // Debugging only
  Serial.println("setup");
  dht.begin(); //Se inicia el sensor de temperatura y humedad

  //Iniciando la comunicacion con el modulo exterior
  vw_set_tx_pin(transmit_pin);
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_pin(transmit_en_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000); // Bits per sec
  vw_rx_start(); // Start the receiver PLL running
  pinMode(led_pin, OUTPUT);
  
  //inicializar reloj  
  Wire.begin();//Inicializamos la librería wire  
  RTC.begin();//Inicializamos el módulo reloj 
  RTC.adjust(DateTime(__DATE__, __TIME__));//poniendo el reloj en hora
  
  //Conexion con el display
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ){u8g.setColorIndex(255);}
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ){u8g.setColorIndex(3);}
  else if ( u8g.getMode() == U8G_MODE_BW ){u8g.setColorIndex(1);}
  
  
}
void loop(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  
  DateTime now = RTC.now();//Creamos un objeto que contiene la hora y fecha actual
 
 //DEBUGG 
  if (vw_get_message(buf, &buflen)){

    digitalWrite(led_pin, HIGH); // Flash a light to show received good message
    Serial.print("Humedad: ");//humedad del lado del emisor
    Serial.println(buf[0],DEC);
    Serial.print("Temperatura: ");//Sensor del lado del emisor
    Serial.println(buf[1]);
    Serial.print("Temperatura Barometro: ");//Sensor del lado del emisor
    Serial.println(buf[2],DEC);
    Serial.print("Presion: ");//Sensor del lado del emisor
    Serial.println(buf[3],DEC);
    Serial.print("Altitud: ");//Sensor del lado del emisor
    Serial.println(buf[4],DEC);
    Serial.print("Presion sobre el nivel del mar: ");//Sensor del lado del emisor
    Serial.println(buf[5],DEC);
    Serial.print("Altitud Real: ");//Sensor del lado del emisor
    Serial.println(buf[6],DEC);
    
    Serial.println("--------------------");
   
    Serial.println();
    Serial.print("Temperatura Interior: ");
    Serial.println(temp());//Sensor del lado del receptor
    Serial.print("Humedad: ");
    Serial.println(humedad());//Sensor del lado del receptor
    digitalWrite(led_pin, LOW);
    Serial.println("___________________________________________________________");
  }
  
  //¿?
   int temperaturaInterior = temp();
   int humedadInterior = humedad();
   String tInterior = String(temperaturaInterior);
   String hInterior = String(humedadInterior);
   String tExterior = String(buf[1]);
    
   //ACTUALIZAR MAXIMOS Y MINIMOS
   //maximos y minimos exteriores
   if(buf[0]< hum_ext_min){hum_ext_min = buf[0];}
   if(buf[0]> hum_ext_max){hum_ext_max = buf[0];}
   if(buf[1]< temp_ext_min){temp_ext_min = buf[1];}
   if(buf[1]> temp_ext_max){temp_ext_max = buf[1];}
   //maximos y minimos interiores
   if(humedad()< hum_int_min){hum_int_min = humedad();}
   if(humedad()> hum_int_max){hum_int_max = humedad();}
   if(temp()< temp_int_min){temp_int_min = temp();}
   if(temp()> temp_int_max){temp_int_max = temp();}
   
   
   //Bucle de pintado del display
   u8g.firstPage();      
  do {    
      
      u8g.setFont(u8g_font_unifont);    
      u8g.setFontPosTop();    

      //RELOJ
      u8g.setPrintPos(0,1);    
      u8g.print(now.hour(), DEC);//Mostramos la hora
      u8g.print(":");//Dos puntos para separar horas de minutos
      u8g.print(now.minute(), DEC);//Mostramos los minutos
      
      u8g.drawHLine(0, 1+14, 40);//Linea de separacion

      setFont_M;//Tipo de letra pequeño
      
      //INTERIOR--------------------------------------------------------------------------->
      
      //mostramos la temperatura interior
      u8g.setPrintPos(0,30);//Nos colocamos en la posicion del displat donde queremos escribir      
      u8g.print(temperaturaInterior);//temperatura interior  
      u8g.print(" ºC");//Mostramos la unidades

      //mostramos la humedad interior
      u8g.setPrintPos(0,45);      
      u8g.print(hInterior);//humedad interior
      u8g.print(" %");
      
      u8g.drawVLine(10,70,30);//Linea vertical de separacion interior/exterior
      
      //Mostramos imagenes en funcion de los datos
      
      if(buf[1]>0 && buf[0]<80){//Si la temperatura es positiva y la humedad inferior al 80, mostramos el sol. 
        u8g.drawXBMP( 90, 0, soleado_img_width, soleado_img_height, soleado_img);
      }
      else if(buf[1]<=0 && buf[0]<80){//si la temperatura es negativa, mostramos hielo
                u8g.drawXBMP( 90, 0, hielo_width, hielo_height, hielo_bits);
      }
      else if(buf[1]>0 && buf[0]>80){//Si la humedad es mayor de 80%, mostramos lluvia
                u8g.drawXBMP( 90, 0, lluvia_width, lluvia_height, lluvia_bits);
      }
      
      else if(6>now.hour() or now.hour()>20){//Si es de noche, moestramos la luna
                u8g.drawXBMP( 90, 0, luna_width, luna_height, luna_bits);
      }
      //EXTERIOR---------------------------------------------------------------------------->
      
      //Mostramos la temperatura exterior
      setFont_S;//Tipo de letra pequeño
      u8g.setPrintPos(100,40);      
      u8g.print(buf[1],DEC);//temperatura exterior
      u8g.print(" ºC");

      //Mostramos la humedad exterior
      u8g.setPrintPos(100,50);      
      u8g.print(buf[0],DEC);//humedad exterior
      u8g.print(" %");
      
      //Mostramos la presion atmosferica
      setFont_S;//Tipo de letra pequeño
      u8g.setPrintPos(100,60);      
      u8g.print(buf[3],DEC);//presion atmosferica
      u8g.print(" mB");
        
  
  } while( u8g.nextPage() );  
}

/*Devuelve la humedad del sensor DHT11*/
float humedad(){
  return dht.readHumidity();
} //Se lee la humedad

/*Devuelve la temperatura del sensor DHT11*/
float temp(){
  return dht.readTemperature();
} //Se lee la temperatura

