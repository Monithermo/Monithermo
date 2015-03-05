//-----------------------------------------------------------------------------------------
//  Programme permettant de prendre la température puis de l'envoyer sur un serveur.
//
//  Auteur : MoniThermo
//  Date : 04/02/2014
//
//  License: GNU GPL v2 (voir license.txt)
//
//  Liste des sous-programmes :
//  -> void retrieveStationIdFromInternalEEPROM(char stationID[STATION_ID_BUFFER_LENGTH])
//  -> void Acquisition_Temp(void)
//  -> void date(void)
//  -> void stock(void)
//  -> void PostTemp(void)
//
//-----------------------------------------------------------------------------------------

#include "OneWire.h"
#include "DallasTemperature.h"  // librairie du capteur DS18B20
#include <stdlib.h>
#include <Bridge.h>
#include <FileIO.h>
#include <EEPROM.h>            // librairie permettant de stocker/retrouver l'identifiant
#include <SoftwareSerial.h>

//-----------------------------------------------------------------------------------------
//  Definitions
//-----------------------------------------------------------------------------------------

// Definitions ID
#define INTERNAL_EEPROM_STATION_ID_ADDRESS_START 0
#define STATION_ID_BUFFER_LENGTH  8

// Definitions pins DS18B20
#define SENSORS_POWER_PIN 11
#define DS18B20_SENSOR_DATA_PIN 10

OneWire oneWire(DS18B20_SENSOR_DATA_PIN);
DallasTemperature DS18B20Sensors(&oneWire);

// Variable Globale ID
char ID_EEPROM[STATION_ID_BUFFER_LENGTH];  // chaîne contenant l'ID du Monithermo

// Variable Globale DS18B20
float externalTemperature;  // variable contenant la température

// Variables Globales fichier
char nom[32]; // nom du fichier où sera stockée la température

// Variables Globales du temps
unsigned long int ts; // timestamp
String date_heure;  // jj-mm-aa-hh:mm:ss

unsigned long temps;  // variable d'attente entre deux mesures

// Definitions soft serial
#define DEBUG_TX_PIN 3
#define DEBUG_RX_PIN 255

SoftwareSerial softSerialDebug(DEBUG_RX_PIN, DEBUG_TX_PIN); 

//-----------------------------------------------------------------------------------------
//    Definition des sous-programmes
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//    Programme permettant de retrouver l'ID stocké en EEPROM
//    Entrée : Buffer où sera stocké l'ID
//    Sortie : /
//-----------------------------------------------------------------------------------------
void retrieveStationIdFromInternalEEPROM(char stationID[STATION_ID_BUFFER_LENGTH])
{
  for (byte i = 0 ; i < (STATION_ID_BUFFER_LENGTH - 1) ; i++)
  {
    stationID[i] = EEPROM.read(INTERNAL_EEPROM_STATION_ID_ADDRESS_START + i);
  }
  stationID[(STATION_ID_BUFFER_LENGTH - 1)] = '\0';
}// fin retrieveStationIdFromInternalEEPROM


//-----------------------------------------------------------------------------------------
//    Programme permettant d'acquerir la temperarure
//    Entree : /
//    Sortie : /
//    Algorithme : -> Alimenter le capteur DS18B20
//                 -> Récuperer la valeur de la temperature
//                 -> Eteindre le capteur
//-----------------------------------------------------------------------------------------
void Acquisition_Temp(void)
{
//  softSerialDebug.println("Acquisition... ");
  digitalWrite(SENSORS_POWER_PIN, HIGH);
  DS18B20Sensors.requestTemperatures();
  externalTemperature = DS18B20Sensors.getTempCByIndex(0);
  digitalWrite(SENSORS_POWER_PIN, LOW);
  softSerialDebug.println(externalTemperature);
}// fin Acquisition_Temp


//-----------------------------------------------------------------------------------------
//  Programme permettant de récupérer la date et le timestamp
//  Entrée : /
//  Sortie : /
//  Algorithme : -> Demande timestamp au linino
//               -> Demande de la date et de l'heure au linino
//-----------------------------------------------------------------------------------------
void date (void)
{
  Process date;
  
  // timestamp par la commande "date"
  date.runShellCommand("date +%s");
  while (date.running());

  if (date.available())
  {
    ts = date.parseInt();
  }
  date.flush();
  softSerialDebug.print("timestamp : ");
  softSerialDebug.println(ts);
  
  // date et heure locale par la commande "date"
  date.runShellCommand("date +%d-%m-%Y-%H:%M:%S");
  while (date.running());
  
  if (date.available())
  {
    date_heure = date.readString();
  }
  date.flush();
  
//  softSerialDebug.print("date : ");
//  softSerialDebug.println(date_heure);
}// fin date


//-----------------------------------------------------------------------------------------
//  Programme permettant de stocker la temperature dans un fichier
//  Entrée : /
//  Sortie : /
//-----------------------------------------------------------------------------------------
void stock (void)
{
//  softSerialDebug.println("stock des valeurs... ");
  // Fichier Serveur
  File fichier_serveur = FileSystem.open("/fichier_serveur.txt", FILE_APPEND);
  fichier_serveur.print(ts);
  fichier_serveur.print(",");
  fichier_serveur.print(ID_EEPROM);
  fichier_serveur.print(",");
  fichier_serveur.print(",");
  fichier_serveur.print(externalTemperature);
  fichier_serveur.print('\n');
  fichier_serveur.close();

  // Fichier Stock - l'heure stockée correspond à l'heure locale
  File fichier_stock = FileSystem.open(nom, FILE_APPEND);
  fichier_stock.print("Mesuree le : ");
  fichier_stock.print(date_heure);
  fichier_stock.print(" - Station : ");
  fichier_stock.print(ID_EEPROM);
  fichier_stock.print(", Temperature : ");
  fichier_stock.println(externalTemperature);
  fichier_stock.close();
}// fin stock


//-----------------------------------------------------------------------------------------
//  Programme permettant d'envoyer la temperature au serveur
//  Entrée : /
//  Sortie : /
//-----------------------------------------------------------------------------------------
void PostTemp(void) 
{
  Process pTemp;		
  pTemp.runShellCommand("./post_temp_V01.py");
  while(pTemp.running());
}// fin PostTemp


//-----------------------------------------------------------------------------------------
//  Initialisation du microcontroleur
//-----------------------------------------------------------------------------------------
void setup(void)
{
  // Initialisation du port de debugage
  softSerialDebug.begin(9600); 
  softSerialDebug.println("GO!");
  softSerialDebug.println("* MoniThermo V01 *");
  
  // Initialisation du Bridge
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);

  delay(60000);  // attendre 1 minute que le linino finisse de démarrer
  
  // Initialisation de la librairie de traitement des fichiers
  FileSystem.begin();
  
  // Retrouver l'identifiant du MoniThermo
  retrieveStationIdFromInternalEEPROM(ID_EEPROM);

  // initialisation du capteur DS18B20
  pinMode(SENSORS_POWER_PIN, OUTPUT);
  digitalWrite(SENSORS_POWER_PIN, HIGH);
  while (DS18B20Sensors.getDeviceCount() == 0)
  {
    DS18B20Sensors.begin();
    softSerialDebug.println("begin");  
  }
  digitalWrite(SENSORS_POWER_PIN, LOW);
  
  // chemin du fichier contenant l'historique de la temperature
  strcpy(nom, "/");
  strcat(nom, ID_EEPROM);
  strcat(nom, ".txt");

  softSerialDebug.println(nom); 

  temps = 300000;  // 5 minutes
}// fin de l'initialisation

//-----------------------------------------------------------------------------------------
//  Programmme principal
//  Fonction "delay" exprimée en ms
//  Algorithme : -> Acquisitin de la date
//               -> Acquisition de la temperature
//               -> Stockage des données
//               -> Post les données sur le serveur
//               -> Attente
//-----------------------------------------------------------------------------------------
void loop(void)
{
  softSerialDebug.println("Acquisition Date... ");
  date();
  delay(1000);
  softSerialDebug.println("Acquisition Temperature... ");
  Acquisition_Temp();
  delay(1000);
  softSerialDebug.println("Stockage des donnees... ");
  stock();
  delay(1000);
  softSerialDebug.println("Post des donnees... ");
  PostTemp();
  delay(1000);
  softSerialDebug.println("Attente... ");

  delay(temps);
}// fin du programme principal
