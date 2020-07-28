/*
Estacion Helada V1.0
CLIENTE: Federico Franic
COMIENZO DEL PROYECTO : 26/07/2020

Programa que controla dos temperaturas para prevenir heladas.
Programa funcionando para controlar el encendido de un motor.
Cuenta con un reloj y una programacion por DIA, HORA INI, HORA FIN
FORMATO PARA ENVIAR LA PROGRAMACION (2,15,30,16,30)(dia,hora_ini,min_ini,hora_fin,min_fin)
Topico programacion: /placa_0200/programa1


28/07/2020 : Creada 8 temperaturas de control - en loop control por temp_control_1
            Publicacion de Temp_ext y Temp_hoja cada 60seg
            agregado al git

-------------BORRADO DE EEPROM---------------
for (int i = 0 ; i < EEPROM.length() ; i++) {
   EEPROM.write(i, 0);
 }
------------------------------------------------

*/


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
 #include <Wire.h>
 #include <cstdlib>
 //-------------------------------sensor 18b20-------------------------//
 #include <OneWire.h>
 #include <DallasTemperature.h>
 #define ONE_WIRE_BUS D3
 OneWire oneWire(ONE_WIRE_BUS);
 DallasTemperature sensor (&oneWire);
 float temperatura;
//--------------------------------------------------------------------//

 const int led = D2;
 const int sirena = D7;
 const int mot = D1;
 //#define OUT_1 D7 //LED o  Motor
 #define PULSADOR D5   //MANUAL

 int SENSOR;		// variable almacena valor leido de entrada analogica A0
int contconexion = 0;
long lastMsg = 0;

char msg[25];                              // es un char para el mensaje
String strtemp = "";
String strtemp_ext = "";
String strtemp_hoja = "";
char valueStrTEMP[15];
char valueStrleido_ext[15];
char valueStrleido_hoja[15];
float temp;
float temp_hoja;
String estado_motor;
String temp_ajuste;
String temperatura_ajuste = "";
String temp_leido;
String autom;
String automatico = "HIGH";
String motor;
int str_debug =0;
//Temperaturas control
String temp_control_1 = "4";
String temp_control_2 = "3.5";
String temp_control_3 = "3";
String temp_control_4 = "3.5";
String temp_control_5 = "2";
String temp_control_6 = "2.5";
String temp_control_7 = "2";
String temp_control_8 = "1";



unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;

char PLACA[50];
char NOMBRE[50];
char TEMP_EXT[50];
char TEMP_HOJA[50];
char MOTOR[50];
char TEMPERATURA_AJUSTE[50];
char TEMP_AJUSTE_LEIDA[50];
char DEBUG[50];
char ACTUALIZACION[50];
char RESET[50];

const char *mqtt_server = "serviciosiot.com.ar";
const int mqtt_port = 1883;

String USERNAME = "placa_0200";
char   PASSWORD[50] = "12345678";
String NAME = "helada";
String ARCHIVO = "helada_28_07_2020.ino";
String VER_ARCHIVO = "Ver.:1.28072020";

WiFiClient espClient;
PubSubClient client(espClient);


//----------------------PAGINA WEB---------------------///
String pral = "<html>"
"<meta http-equiv='Content-Type' content='text/html; charset=utf-8'/>"
"<title>WIFI CONFIG by Fer@Fra</title> <style type='text/css'> body,td,th { color: #036; } body { background-color: #bbbd13; } </style> </head>"
"<body> "
"<h1>WIFI CONF</h1><br>"
"<form action='config' method='get' target='pantalla'>"
"<fieldset align='left' style='border-style:solid; border-color:#336666; width:400px; height:180px; padding:10px; margin: 5px;'>"
"<legend><strong>Configurar WI-FI</strong></legend>"
"Programar (si / no):  <input name='prog' type='text' size='15'/> <br><br>"
"SSID:  <input name='ssid' type='text' size='15'/> <br><br>"
"PASSWORD:  <input name='pass' type='password' size='15'/> <br><br>"
"<input type='submit' value='Comprobar conexion' /><br><br>"
"</fieldset>"
"<fieldset align='left' style='border-style:solid; border-color:#336666; width:400px; height:120px; padding:10px; margin: 5px;'>"
"<legend><strong>Config PROGRAMA 1</strong></legend>"
"PROGRAMA1:  <input name='prog1' type='text' size='15'/> <br><br>"
"<input type='submit' value='Guardar configuracion' />"
"</fieldset><br><br>"
"<fieldset align='left' style='border-style:solid; border-color:#336666; width:400px; height:100px; padding:10px; margin: 5px;'>"
"<legend><strong>MANUAL / AUTOMATICO</strong></legend>"
"MANUAL (si) AUTOMATICO (no):  <input name='auto' type='text' size='15'/> <br><br>"
"<input type='submit' value='Activar' />"
"</fieldset>"
"</form>"
"<iframe id='pantalla' name='pantalla' src='' width=900px height=400px frameborder='0' scrolling='no'></iframe>"
"</body>"
"</html>";

ESP8266WebServer server(80);

char ssid[20];
char pass[20];
String ssid_leido;
String pass_leido;
int ssid_tamano = 0;
int pass_tamano = 0;

String arregla_simbolos(String a) {
 a.replace("%C3%A1", "á");
 a.replace("%C3%A9", "é");
 a.replace("%C3%A", "i");
 a.replace("%C3%B3", "ó");
 a.replace("%C3%BA", "ú");
 a.replace("%21", "!");
 a.replace("%23", "#");
 a.replace("%24", "$");
 a.replace("%25", "%");
 a.replace("%26", "&");
 a.replace("%27", "/");
 a.replace("%28", "(");
 a.replace("%29", ")");
 a.replace("%3D", "=");
 a.replace("%3F", "?");
 a.replace("%27", "'");
 a.replace("%C2%BF", "¿");
 a.replace("%C2%A1", "¡");
 a.replace("%C3%B1", "ñ");
 a.replace("%C3%91", "Ñ");
 a.replace("+", " ");
 a.replace("%2B", "+");
 a.replace("%22", "\"");
 return a;
}
//---------------FUNCIONES--------------------//
void lee_temperatura();
void(* resetFunc) (void) = 0; //declare reset function @ address 0

//*******  G R A B A R  EN LA  E E P R O M  ***********
void graba(int addr, String a) {
 int tamano = (a.length() + 1);
 Serial.print(tamano);
 char inchar[30];    //'30' Tamaño maximo del string
 a.toCharArray(inchar, tamano);
 EEPROM.write(addr, tamano);
 for (int i = 0; i < tamano; i++) {
  addr++;
  EEPROM.write(addr, inchar[i]);
 }
 EEPROM.commit();
}

//*******  L E E R   EN LA  E E P R O M    **************
String lee(int addr) {
 String nuevoString;
 int valor;
 int tamano = EEPROM.read(addr);
 for (int i = 0;i < tamano; i++) {
  addr++;
  valor = EEPROM.read(addr);
  nuevoString += (char)valor;
 }
 return nuevoString;
}


//----------------------CALLBACK------------------------------//
void callback(char* topic, byte* payload, unsigned int lenght){

  char PAYLOAD[5] = "    ";

  //String str_topic(topic);
                                            //deja un espacio en blanco
  //String temperatura_ajuste = "";
  temperatura_ajuste = "";
 if (String(topic) ==  TEMPERATURA_AJUSTE) {
   Serial.println("");
   Serial.print("Mensaje recibido bajo el topico->");
   Serial.println(topic);
   //Serial.print(topic);
   Serial.print("\n");
  for (int i = 0; i < lenght; i++) {
    temperatura_ajuste += (char)payload[i];
   }
   //temperatura_ajuste.trim();
   graba(100,temperatura_ajuste);
  }

  if (String(topic) == "actualizar") {
    Serial.println("");
    Serial.print("Mensaje recibido bajo el topico->");
    Serial.println(topic);
    //Serial.print(topic);
    Serial.print("\n");
    lee_temperatura();
   }

   if (String(topic) ==  DEBUG) {
     Serial.println("");
     Serial.print("Mensaje recibido bajo el topico->");
     Serial.println(topic);
     Serial.print("\n");
    for (int i = 0; i < lenght; i++) {
      PAYLOAD[i] = (char)payload[i];
      }
        if (PAYLOAD[0] == 'S'){   // debug si
          str_debug= 1;
         }
         if (PAYLOAD[0] == 'N'){   // debug si
           str_debug= 0;
          }
    }

    if (String(topic) == RESET) {
      Serial.println("");
      Serial.print("Mensaje recibido bajo el topico->");
      Serial.println(topic);
      Serial.print("\n");
     for (int i = 0; i < lenght; i++) {
       PAYLOAD[i] = (char)payload[i];
       }
       if (PAYLOAD[0] == 'S'){   // debug si
         str_debug= 1;
         resetFunc();  //call reset
          }
         }

 if (String(topic) ==  MOTOR) {
   Serial.println("");
   Serial.print("Mensaje recibido bajo el topico->");
   Serial.println(topic);
   //Serial.print(topic);
   Serial.print("\n");
  for (int i = 0; i < lenght; i++) {
    PAYLOAD[i] = (char)payload[i];
   }

      if (PAYLOAD[1] == 'M'){   // MANUAL
        Serial.println("Mensaje recibido : N");
        automatico = "LOW";
        estado_motor = "ON";
        graba(200,estado_motor);
        graba(400,automatico);


      }
      if (PAYLOAD[1] == 'A'){  // Automatico
        Serial.println("Mensaje recibido : F");
       automatico = "HIGH";
       estado_motor = "OFF";
       graba(200,estado_motor);
       graba(400,automatico);

      }


}
}
//------------------------------------------------------------//
//----LEE TEMPERATURA ------
void lee_temperatura(){
  temp = sensor.getTempCByIndex(0);
  //temp = ((SENSOR * 5000.0) / 1024) / 10;
  strtemp = String(temp,1); //1 decimal
  //to_send.toCharArray(valueStrTEMP, 15);
  strtemp.toCharArray(valueStrTEMP, 15);
  //Serial.println("Enviando: [" +  String(TEMP_EXT) + "] " + String(to_send));
  Serial.println("Enviando: [" +  String(TEMP_EXT) + "] " + String(strtemp));
  client.publish(TEMP_EXT, valueStrTEMP);
}

void lee_temperatura_hoja(){
 
  temp = sensor.getTempCByIndex(0);
  strtemp_hoja = String(temp,1); //1 decimal
  strtemp_hoja.toCharArray(valueStrleido_hoja, 15);
  Serial.println("Enviando: [" +  String(TEMP_EXT) + "] " + String(strtemp_hoja));
  client.publish(TEMP_HOJA, valueStrleido_hoja);
}

void lee_temp_guardadas(){
 
  temp_leido = lee(100);
  strtemp_ext = String(temp,1); //1 decimal
//  String to_send = String(NAME) + "," + String(strtemp);
  //to_send.toCharArray(valueStrTEMP, 15);
  strtemp_ext.toCharArray(valueStrleido_ext, 15);
  Serial.println("Enviando: [" +  String(TEMP_EXT) + "] " + String(strtemp_ext));
  client.publish(TEMP_EXT, valueStrleido_ext);
}






//**** CONFIGURACION WIFI  *******
void wifi_conf() {
 int cuenta = 0;

 String getssid = server.arg("ssid"); //Recibimos los valores que envia por GET el formulario web
 String getpass = server.arg("pass");

 getssid = arregla_simbolos(getssid); //Reemplazamos los simbolos que aparecen cun UTF8 por el simbolo correcto
 getpass = arregla_simbolos(getpass);

 ssid_tamano = getssid.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
 pass_tamano = getpass.length() + 1;

 getssid.toCharArray(ssid, ssid_tamano); //Transformamos el string en un char array ya que es lo que nos pide WIFI.begin()
 getpass.toCharArray(pass, pass_tamano);

 Serial.println(ssid);     //para depuracion
 Serial.println(pass);

 WiFi.begin(ssid, pass);     //Intentamos conectar
 while (WiFi.status() != WL_CONNECTED)
 {
  delay(500);
  Serial.print(".");
  cuenta++;
  if (cuenta > 20) {
   graba(70, "noconfigurado");
   server.send(200, "text/html", String("<h2>No se pudo realizar la conexion<br>no se guardaron los datos.</h2>"));
   return;
  }
 }
 Serial.println(WiFi.localIP());
 graba(70, "configurado");
 graba(1, getssid);
 graba(30, getpass);
 server.send(200, "text/html", String("<h2>Conexion exitosa a: "
  + getssid + "<br> El pass ingresado es: " + getpass + "<br>Datos correctamente guardados."));

}

//*********  INTENTO DE CONEXION   *********************
void intento_conexion() {
 if (lee(70).equals("configurado")) {
  ssid_leido = lee(1);      //leemos ssid y password
  pass_leido = lee(30);

  Serial.println(ssid_leido);  //Para depuracion
  Serial.println(pass_leido);

  ssid_tamano = ssid_leido.length() + 1;  //Calculamos la cantidad de caracteres que tiene el ssid y la clave
  pass_tamano = pass_leido.length() + 1;

  ssid_leido.toCharArray(ssid, ssid_tamano); //Transf. el String en un char array ya que es lo que nos pide WiFi.begin()
  pass_leido.toCharArray(pass, pass_tamano);

  int cuenta = 0;
  WiFi.begin(ssid, pass);      //Intentamos conectar
  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   cuenta++;
   if (cuenta > 20) {
    Serial.println("Fallo al conectar");
    return;
   }
  }
 }
 if (WiFi.status() == WL_CONNECTED) {
  Serial.print("Conexion exitosa a: ");
  Serial.println(ssid);
  Serial.println(WiFi.localIP());
 }
}

void reconnect(){                                              //FUNCION PARA RECONECTAR AL SERVIDOR
  if (!client.connected()) {
    Serial.println("Intentando Conexion MQTT");

    String clientId = "iot_2_";                                        //SOLUCION DE DAR DE BAJA LA ANTERIOR CONEXION
    clientId = clientId + String(random(0xffff), HEX);                //

    USERNAME.toCharArray(PLACA, 50);
    if (client.connect("", PLACA, PASSWORD)) {
    //if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {      // DATOS PARA LA CONEXION
      Serial.println("Conexion MQTT exitosa!!");
      String str_NAME = NAME + USERNAME +"online";
      str_NAME.toCharArray(NOMBRE, 50);
      client.publish("alertas",NOMBRE);
      client.subscribe(MOTOR);
      client.subscribe(TEMPERATURA_AJUSTE);
      client.subscribe(ACTUALIZACION);
      client.subscribe(DEBUG);
      client.subscribe(RESET);

      Serial.println("subscribe to /"+USERNAME+"/temp_ext");
      Serial.println("subscribe to /"+USERNAME+"/temp_hoja");
      Serial.println("subscribe to /"+USERNAME+"/temp_ajuste_leida");
      Serial.println("subscribe to /"+USERNAME+"/temp_ajuste");
      Serial.println("subscribe to /"+USERNAME+"/debug");
      Serial.println("subscribe to /"+USERNAME+"/actualizacion");
      Serial.println("subscribe to /"+USERNAME+"/motor");
    
      Serial.println("");
      ////////////////////PUBLICACION TEMP AL INICIO//////////////////////////////
      lee_temperatura();

        ////////////////////////////////////////////////////////////////////////////

    }
    else{
       Serial.println("Fallo la  Conexion ");
       Serial.print(client.state());                                      // QUIERO SABER EL ESTADO DEL CLIENTE CON CODIGO DE ERROR
       Serial.print("Se intentara de nuevo en 5 segundos");

       delay(5000);
    }
  }
}



//*****  S E T U P  **************
void setup() {
 Serial.begin(115200);
 EEPROM.begin(4096);


 pinMode(led, OUTPUT);
 pinMode(sirena, OUTPUT);
 pinMode(mot, OUTPUT);

 WiFi.softAP("ESP_" + String(USERNAME) );      //Nombre que se mostrara en las redes wifi

 server.on("/", []() {
  server.send(200, "text/html", pral);
 });
 server.on("/config", wifi_conf);
 server.begin();
 Serial.println("Webserver iniciado...");

 Serial.println(lee(70));
 Serial.println(lee(1));
 Serial.println(lee(30));
 intento_conexion();
 client.setServer(mqtt_server, mqtt_port);
 client.setCallback(callback);
          //*****  TOPIC  **************
  String temp_ext = "/" + USERNAME + "/" + "temp_ext";
  temp_ext.toCharArray(TEMP_EXT, 50);
  String temp_hoja = "/" + USERNAME + "/" + "temp_hoja";
  temp_hoja.toCharArray(TEMP_HOJA, 50);
  String motor = "/" + USERNAME + "/" + "motor";
  motor.toCharArray(MOTOR, 50);
  String temperatura_ajuste = "/" + USERNAME + "/" + "temperatura_ajuste";
  temperatura_ajuste.toCharArray(TEMPERATURA_AJUSTE, 50);
  String temp_ajuste_leida = "/" + USERNAME + "/" + "temp_ajuste_leida";
  temp_ajuste_leida.toCharArray(TEMP_AJUSTE_LEIDA, 50);
  String actualizacion = "/" + USERNAME + "/" + "actualizacion";
  actualizacion.toCharArray(ACTUALIZACION, 50);
  String debug = "/" + USERNAME + "/" + "debug";
  debug.toCharArray(DEBUG, 50);
  String reset = "reset";
  reset.toCharArray(RESET, 50);


//temp_leido = lee(100); // Ajuste de Temperatura de Control
automatico = lee(400);  // Memoria de estado Automatico  o Manual
Serial.println("****ESTADO PLACA****");
Serial.println("estado -> " + String (automatico));
Serial.println("");
Serial.println("****TEMPERATURA AJUSTE****");
Serial.println("temperatura_ajuste-> " + String (temp_leido));
Serial.println("");
Serial.println("****TOPIC CREADOS****");
Serial.println("/"+USERNAME + "/salida_digital");
Serial.println("/"+USERNAME + "/temperatura");
Serial.println("/"+USERNAME + "/temp_ajuste_leida");
Serial.println("/"+USERNAME + "/temp_ajuste");
Serial.println("/"+USERNAME + "/actualizacion");
Serial.println("/"+USERNAME + "/debug");
Serial.println("reset");
Serial.println("");
Serial.println("****SERVIDOR****");
Serial.println("serviciosiot.com.ar");
Serial.println("");
Serial.println("****Ver.-***");
Serial.println(String (VER_ARCHIVO));
Serial.println("");
Serial.println("Conexion Conesa a: NuestraCasa");
Serial.println("El pass : Mateo2018");

////////////////////PUBLICACION TEMP AL INICIO//////////////////////////////
lee_temperatura();
lee_temperatura_hoja();

  ////////////////////////////////////////////////////////////////////////////


}




//*****   L O O P   **************
void loop() {
sensor.requestTemperatures();
  server.handleClient();
  delay(2000);

client.loop();
if (client.connected()==true){
  //Serial.println("cliente conectado ");
}
else{
  Serial.println("cliente NO conectado ");
}
if (client.connected()==false){
  reconnect();
}
//------------------------SENSOR DE TEMPERATURA------------------------//
//*************************LM35 & 18b20 *****************************************
//SENSOR = analogRead(A0);				// lectura de entrada analogica A0
temp = sensor.getTempCByIndex(0);
strtemp = String(temp,0); //1 decimal
//temp_hoja = sensor.getTempCByIndex(1);
//strtemp_hoja= String(temp_hoja,0); //1 decimal

//***************Control 1**********************//
temp_control_1 = "25";
int str_control_1= atoi(temp_control_1.c_str());
  if (temp <= str_control_1) {
  digitalWrite(sirena, HIGH);
  Serial.println("Temperatura control 1 : " + String(temp_control_1));
  Serial.println("Temperatura ext : " + String(strtemp));
  }else{ digitalWrite(sirena, LOW);}
//***************Control 2**********************//


//------------------------termostato------------------------//

//int str_temp_leido = atoi(temp_leido.c_str());

  ////////////////// termostato/////////////////////////////////////////////////
  /*
  if (temp >= str_control_1) {   //Compara la temperatura contra el temp_ajuste
    digitalWrite(led, HIGH);
    if (str_debug==1) {
    Serial.println("termostato funcionando  on");
    }
  }

  if (temp > -10) {
  digitalWrite(sirena, LOW);
   if (str_debug==1) {
   Serial.println("sirena sonando!!!!! ");
    }
  }
  if (millis() - lastMsg >600000) {
    lastMsg= millis();
      if (str_debug==1) {
        Serial.println("temperatura= " + String(temp));
        Serial.println("TEMPERATURA LEIDA= " + String(str_control_1));
        }
}*/



///////////////////////PUBLICACIONES////////////////////////////////////////


unsigned long currentMillis3 = millis();
if (currentMillis3 - previousMillis3 >= 60000) { //envia la temperatura cada 60 segundos
        previousMillis3 = currentMillis3;
        //int analog = analogRead(17);
        //float temp = analog*0.322265625;
        //temp = ((SENSOR * 5000.0) / 1023) / 10;
        //strtemp = String(temp, 1); //1 decimal
        lee_temperatura();
        lee_temperatura_hoja();
        //client.publish(TEMP_AJUSTE_LEIDA, valueStrleido);
        

      }

unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= 600000) { //envia la temperatura cada 25 segundos
    previousMillis = currentMillis;
   }

}