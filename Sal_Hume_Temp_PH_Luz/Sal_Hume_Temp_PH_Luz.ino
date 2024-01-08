#include <Adafruit_ADS1X15.h>

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

void setup() {
  // Inicializamos el monitor serie
  Serial.begin(9600);
  Serial.println("Inicializando el medidor de humedad");
  Serial.println("Inicializando el medidor de Temperatura");
  Serial.println("Inicializando el medidor de salinidad");
  Serial.println("Inicializando el medidor de PH");
  Serial.println("Inicializando el sensor de luz");

  // Inicializamos el ADS1115
  ads1115.begin(0x48);

  // Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);

  // Configuramos el pin de alimentación para el sensor de salinidad como salida
  pinMode(power_pin, OUTPUT);

  // Realizamos la calibración
  //calibracion();
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
float interpolate(int* x, int adc0) {
  float salinidad = 0;

  Serial.println(x[2]);
 
  float l1 = 10*(adc0-x[0])/(x[1]-x[0]) * (adc0-x[2])/(x[1]-x[2]);
  float l2 = (20*(adc0-x[0]))/(x[2]-x[0]) * (adc0-x[1])/(x[2]-x[1]);

  salinidad = l1 + l2;

  return salinidad;
}

//Función del PH
void PH(){

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

}

//Función sensor de luz
void luz(){
  
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

  //Ahora todo lo de salinidad
  // Alimentamos la sonda con un tren de pulsos
  digitalWrite(power_pin, HIGH);
  delay(100);

  // Leemos cuando hay un nivel alto
  int16_t adc0 = analogRead(A0);
  digitalWrite(power_pin, LOW);
  delay(100);

  // Realizamos el cálculo de la salinidad con Langrange
  float ressalinidad = interpolate(x, adc0);

  // Presentamos lectura de sal
  Serial.print("Lectura digital = ");
  Serial.println(adc0, DEC);
  Serial.print("Salinidad = ");
  
  if(ressalinidad >= 100){
    ressalinidad = 0;

  }  
  Serial.print(ressalinidad, 2);+
  Serial.println("g");
  delay(1000);  

  //Llamada a la función del PH
  PH();

  //Llamada a la función del sensor de luz
  luz();

  deepSleep(15000);

}
