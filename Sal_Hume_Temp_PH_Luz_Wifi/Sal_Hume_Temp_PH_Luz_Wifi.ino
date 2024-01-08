#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>

//Tiempo de sueño
const int sleepTimeS = 5;


// Humedad
int channelValueHumedad = 2;
int sensorValue = 0;
int humidityValue = 0;

// Temperatura
int channelValueTemperatura = 1;

// Salinidad
#define power_pin 5  // Pin para alimentar el sensor de salinidad
int x[] = { 300, 650, 740 }; //Valores que van a cambiar con la calibración 
//int y[] = { 0g, 10g, 20g }; //Los gramos de sal utilizados

//Sensor de PH
const int channelValuePH =0;
const double Offset =2.00;
const int samplingInterval =20;
const int printInterval =800;
const int ArrayLength =40;
int pHArray[ArrayLength];
int pHArrayIndex = 0;

// Sensor de luz
int channelValueLuz = 3;

// Construimos el ADS1115
Adafruit_ADS1115 ads1115;

// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/360979)

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "TP-LINK_6CAE";
  const char WiFiPSK[] = "41422915";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "MySSID";
  const char WiFiPSK[] = "MyPassWord";
#endif

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="ARADFY6T4YD3W3VO"; // Escribe la clave de tu canal ThingSpeak
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="PruebaGTI"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 5 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

const int LED_PIN = 5; // Thing's onboard, green LED

void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}

void setup() {
  // Inicializamos el monitor serie
  Serial.begin(9600);
  Serial.println("Inicializando el medidor de humedad");
  Serial.println("Inicializando el medidor de Temperatura");
  Serial.println("Inicializando el medidor de salinidad");
  Serial.println("Inicializando el medidor de PH");
  Serial.println("Inicializando el sensor de luz");
  Serial.println("Encendiendo ESP8266 ");
  Serial.println("ESP8266: Buenos dias mundo ");

  // Inicializamos el ADS1115
  ads1115.begin(0x48);

  // Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);

  // Configuramos el pin de alimentación para el sensor de salinidad como salida
  pinMode(power_pin, OUTPUT);

  // Realizamos la calibración
  //calibracion();

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(9600);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif

}

//Función de la temperatura
float temperatura(int channelValue){
  // Captura una muestra del ADS1115

  int16_t adc0 = ads1115.readADC_SingleEnded(channelValue);

  // Aplicamos la formula de la temperatura en funcion de
  // la lectura digital

  float voltaje = (adc0 * 4.096/ 32767);
  float temperatura = ((voltaje-0.786)/0.034)-4;

  //Devolvemos el valor de la temperatura
  return temperatura;
                        
}

//Función de la humedad
float humedad(int channelValue) {
  sensorValue=ads1115.readADC_SingleEnded(channelValue);
  humidityValue = map(sensorValue,2150,15940,100,0);

  Serial.print(humidityValue,DEC);

  //Devolvemos el valor de la humedad
  return humidityValue;

}

//Calibración salinidad
void calibracion() {
  int adc0;
  Serial.println("Meter 0 gramos y pulsa A");
  while (Serial.available() == 0) {
    delay(1000);
  }
  if (Serial.available() > 0) {
    int n_bytes = Serial.available();
    char a = Serial.read();
    if (a == 'A') {
      
      digitalWrite(power_pin, HIGH);
      delay(100);
      
      adc0 = analogRead(A0);
      digitalWrite(power_pin, LOW);
      delay(100);

      x[0] = adc0;
      Serial.print("Valor =");
      Serial.println(x[0]);
    }
    for (int i = 1; i < n_bytes; i++) {
      Serial.read();
    }
  }
  delay(1000);

  Serial.println("Meter 10 gramos y pulsa B");
  while (Serial.available() == 0) {
    delay(1000);
  }
  if (Serial.available() > 0) {
    int n_bytes = Serial.available();
    char b = Serial.read();
    if (b == 'B') {
      digitalWrite(power_pin, HIGH);
      delay(100);
      
      adc0 = analogRead(A0);
      digitalWrite(power_pin, LOW);
      delay(100);
  
      x[1] = adc0;
      Serial.print("Valor =");
      Serial.println(x[1]);
    }
    for (int i = 1; i < n_bytes; i++) {
      Serial.read();
    }
  }
  delay(1000);

  Serial.println("Meter 20 gramos y pulsa C");
  while (Serial.available() == 0) {
    delay(1000);
  }
  if (Serial.available() > 0) {
    int n_bytes = Serial.available();
    char c = Serial.read();
    if (c == 'C') {
      digitalWrite(power_pin, HIGH);
      delay(100);
      
      adc0 = analogRead(A0);
      digitalWrite(power_pin, LOW);
      delay(100);

      x[2] = adc0;
      Serial.print("Valor =");
      Serial.println(x[2]);
    }
    for (int i = 1; i < n_bytes; i++) {
      Serial.read();
    }
  }
  delay(1000);
}

//Función salinidad
float interpolate(int* x) {
  float salinidad = 0;

  // Alimentamos la sonda con un tren de pulsos
  digitalWrite(power_pin, HIGH);
  delay(100);

  // Leemos cuando hay un nivel alto
  int adc0 = analogRead(A0);
  digitalWrite(power_pin, LOW);
  delay(100);

  Serial.println(x[2]);
 
  float l1 = 10*(adc0-x[0])/(x[1]-x[0]) * (adc0-x[2])/(x[1]-x[2]);
  float l2 = (20*(adc0-x[0]))/(x[2]-x[0]) * (adc0-x[1])/(x[2]-x[1]);

  salinidad = l1 + l2;

  return salinidad;
}

//Función del PH
void PH(int channelValuePH){

  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static double pHValue, PHvoltage;
  if ((millis() - samplingTime) > samplingInterval){
    double PHsensorValue=ads1115.readADC_SingleEnded(channelValuePH);
    pHArray[pHArrayIndex++] = PHsensorValue;
    
    if (pHArrayIndex == ArrayLength){
    pHArrayIndex = 0; 
    }
    PHvoltage = (PHsensorValue*4.096)/32767;
    pHValue = (3.5*PHvoltage)+Offset;
    samplingTime - millis();
  }
  if (millis() - printTime > printInterval){
    Serial.print("Voltage:");
    Serial.print(PHvoltage, 2);
    Serial.print("    pH value: ");
    Serial.println(pHValue, 2);
    printTime = millis();
  }
  
  if(pHValue <=8 && pHValue >=6){
     Serial.println("Ph típico del agua:");

  }
  if(pHValue <=9 && pHValue >=8){
     Serial.println("Ph típico de la sangre:");

  }
  if(pHValue <=6 && pHValue >=4){
     Serial.println("Ph típico de un cafe:");

  }
  
  String data[ NUM_FIELDS_TO_SEND + 1];

  data[ 4 ] = pHValue; 
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "PH = " );
        Serial.println( data[ 4 ] );
    #endif

  //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

}

//Función sensor de luz
void luz(int channelValueLuz){
  
  int16_t adc0 = ads1115.readADC_SingleEnded(channelValueLuz);

  // Muestra la señal analógica obtenida
  Serial.print("Señal analógica: "); 
  Serial.println(adc0);

  // Informa por pantalla del nivel de luz
  if (adc0 < 45) {
    Serial.println("Oscuridad");
  } else if (adc0 < 65) {
    Serial.println("Sombra");
  } else if (adc0 < 85) {
    Serial.println("Luz ambiente");
  } else {
    Serial.println("Nivel alto de iluminación");
  }

  delay(1000);

  String data[ NUM_FIELDS_TO_SEND + 1]; 

    
    data[ 5 ] = adc0; //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Luz adc = " );
        Serial.println( data[ 5 ] );
    #endif

    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

}


void loop() {
  // Llamada a la función de temperatura
  float resultadoTemp = temperatura(channelValueTemperatura);

  // Imprime la temperatura en el monitor serie
  Serial.print(resultadoTemp);
  Serial.println("Cº");

  // Llamada a la función de humedad
  float resultadoHume = humedad(channelValueHumedad);

  // Impresión en pantalla de la humedad 
  Serial.print("Humedad: ");
  Serial.print(resultadoHume);
  Serial.println("%");

  

  // Realizamos el cálculo de la salinidad con Langrange
 
  float ressalinidad = interpolate(x);

  // Presentamos lectura de sal
  Serial.print("Salinidad = ");
  Serial.print(ressalinidad, 2);+
  Serial.println("g");
  delay(1000);  

  //Llamada a la función del PH
  PH(channelValuePH);

  //Llamada a la función del sensor de luz
  luz(channelValueLuz);

  //Parte del wifi

  String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = resultadoHume; //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = resultadoTemp; //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Temperatura = " );
        Serial.println( data[ 2 ] );
    #endif

    data[ 3 ] = ressalinidad; //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.println( data[ 3 ] );
    #endif


    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
    delay( 1000 );   
    


  //A dormir
  Serial.println("ESP8266 dulces sueños");
  //ESP.deepSleep(sleepTimeS * 1000);

}