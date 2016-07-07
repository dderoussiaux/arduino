#include <SD.h>          /* Pour la carte SD */
#include <Ethernet2.h>    /* Pour la shield ethernet V2 */
#include <SPI.h>         /* Pour la communication bas niveau (carte SD et ethernet) */
#include <EthernetUdp2.h> /* Pour la communication UDP */
#include <Time.h>        /* Pour la gestion du temps */

/** Taille du buffer pour la lecture du fichier texte et l'envoi de la requete HTTP **/
static const byte BUFFER_SIZE = 50;

/** Taille d'un paquet NTP */
static const byte NTP_PACKET_SIZE = 48;

/** Temps d'attente entre deux écritures (en ms) **/
static const unsigned long PERIOD_WRITE_DATA  =  300000;

/** Broche CS de la carte SD **/
static const byte PIN_SD_CS = 4;

/** Numero du Arduino pour différenciation en base **/
static const byte numArduino = 1;

/** Adresse MAC de l'ethernet shield **/
static byte localMacAddressV1[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x63, 0x95 };
static byte localMacAddressV2[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xFD, 0x00 };

/** Port d'écoute local **/
static const unsigned int listenPort = 8888;

/** Adresse IP de l'ethernet shield **/
static IPAddress localIpAddress(172, 16, 128, 249); 

/** Adresse de ntp **/
static char timeServer[] = "time.nist.gov";

/** Adresse IP serveur perso PHP **/
static IPAddress serverIpAddress(172, 16, 128, 120);

/** Instance de EthernetUDP permettant l'échange de données via UDP **/
static EthernetUDP Udp;

/** Zone horaire pour NTP **/
static const byte timeZoneOffset = 1; // GMT zone

/** Objet File pointant sur le répertoire root de la carte SD **/
static File root;

/** PIN capteur temperature **/
static int sensorPin = A0;

/** Temperature **/
int sensorValue = 0;

// A, B et C sont des coefficients de Steinhart-Hart qui caractérisent chaque thermistance.
static double  A = 0.1129148;
static double  B = 0.0234125;
static double  C = 0.00000876741;


/** Fonction d'initialisation hardware **/
void setup() {
  /* Initialisation du port série */
  Serial.begin(115200);
  Serial.println(F("Start."));

  /* Initialisation de la shield ethernet et UDP */
  Serial.println(F("Initialisation de la carte reseau ... "));
  if (Ethernet.begin(localMacAddressV2) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP
    Ethernet.begin(localMacAddressV2, localIpAddress);
  }
  Serial.print("Mon IP : ");
  Serial.println(Ethernet.localIP());
  Udp.begin(listenPort);

  delay(2000);

  /* Mise à l'heure via NTP */
  Serial.print(F("Synchronisation NTP ... "));
  setSyncProvider(getNtpSync);
  while (timeStatus() == timeNotSet);
    Serial.println(F("Ok !"));

  delay(2000);

  /* Initialisation de la carte SD */
  Serial.print(F("Initialisation de la carte SD ... "));
  pinMode(53, OUTPUT); /* Obligatoire pour que la communication SPI fonctionne (10 UNO, 53 Mega)*/
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println(F("Echec !"));
    for (;;);
  }
  Serial.println(F("Ok !"));

  /* Ouverture du dossier "root" */
  /*root = SD.open("/");
  if (!root) {
    Serial.println(F("Erreur lors de l'ouverture de /"));
    for (;;);
  }*/

  Serial.println(F("Setup termine !"));
}


/** Routine de traitement **/
void loop() {
  static unsigned long last_write_time = 0;
  static byte currentDay = day();

  sensorValue = analogRead(sensorPin);

  /* Test s'il est temps de faire un rapport */
  if ((millis() - last_write_time) > PERIOD_WRITE_DATA) {
    Serial.println( "--- Debut boucle ---" );
    Serial.print( "Valeur : " );
    Serial.println( sensorValue );
    sendReport(sensorValue);

    delay(2000);

    /*root = SD.open("/");
    printDirectory(root, 0);
    root.close();*/
    
    Serial.println("--- Fin boucle ---");
    last_write_time = millis();
  }
}


/** Arborescence des fichiers sur la carte **/
void printDirectory(File dir, int numTabs) {
  Serial.println("Fichiers en attente : ");
  while(true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i=0; i<numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs+1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      Serial.println(" octet");
      
      Serial.print("Datas : ");
      while (entry.available()) {
        Serial.write(entry.read());
      }
    }
    entry.close();
  }
}


/** Fonction d'envoi d'un rapport **/
void sendReport(int sensorValue) {
  /* Temps actuel */
  time_t t = now();

  /* envoi du rapport */
  if (sendData(t, sensorValue)) {
    /* envoi des rapports en attente */
    Serial.println(F("Tentative d'envoi des rapports en attente ..."));
    sendWaitingReports();
  } else {
    /* Mise en attente du rapport */
    Serial.println(F("Echec envoi, mise en attente du rapport ..."));
    createWaitingReport(t, sensorValue);
  }
}


/** Synchronise le temps courant via NTP **/
unsigned long getNtpSync() {
  unsigned long currentTime = 0;

  Serial.println(F("Tentative de synchronisation NTP ..."));

  /* synchronisation avec le serveur NTP */
  if ((currentTime = getNtpTime(timeServer)) == 0) {
    Serial.println(F("Erreur serveur NTP !"));
  }

  return currentTime;
}


/** Retourne le temps courant via NTP **/
unsigned long getNtpTime(char* address) {
  /* Buffer contenant le paquet UDP NTP */
  byte buffer[NTP_PACKET_SIZE] = { 0 };

  /* Création de la requéte NTP */
  buffer[0] = 0b11100011; // LI, Version, Mode
  buffer[1] = 0;          // Stratum, or type of clock
  buffer[2] = 6;          // Polling Interval
  buffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  buffer[12] = 49; 
  buffer[13] = 0x4E;
  buffer[14] = 49;
  buffer[15] = 52;

  /* Envoi d'une requete NTP au serveur */   
  Udp.beginPacket(address, 123);
  Udp.write(buffer, NTP_PACKET_SIZE);
  Udp.endPacket();
  delay(1000);

  if (Udp.parsePacket()) {
    Serial.print("Reponse recue du serveur NTP ");

    /* Mise en buffer du paquet UDP */
    Udp.read(buffer, NTP_PACKET_SIZE);

    /* Lecture des données binaire */
    unsigned int highWord = word(buffer[40], buffer[41]);
    unsigned int lowWord = word(buffer[42], buffer[43]);

    /* Calcul du timestamp (nombre de secondes depuis 1 janvier 1900) */
    unsigned long timestamp = ((unsigned long) highWord << 16) | lowWord;

    /* Calcul du temps courant en fonction de la zone */
    unsigned long currentTime = timestamp - 2208988800UL + (timeZoneOffset * 3600L);

    return currentTime;
  }

  return 0;
}


/** Création d'un fichier de rapport en attente **/
void createWaitingReport(time_t t, int sensorValue) {

  /* Buffer temporaire pour le nom et le contenu du fichier */
  char buffer[BUFFER_SIZE];

Serial.println(t);
Serial.println(sensorValue);
   
  /* Calcul du nom du fichier (format 8.3) */
  sprintf(buffer, "%02d%02d%02d%02d.txt", day(t), hour(t), minute(t), second(t));
  //sprintf(buffer, "%lu.txt", t);
   
  /* Message de debug */
  Serial.print(F("Creation du fichier d'attente : "));
  Serial.println(buffer);
   
  /* Création d'un nouveau fichier en écriture */
  File outfile = SD.open(buffer, FILE_WRITE);
   
  /* Calcul du contenu du fichier */
  sprintf(buffer, "%lu %i", t, sensorValue);
   
  /* Ecriture du fichier */
  outfile.println(buffer);
   
  /* Fermeture du fichier */
  outfile.close();
  
}


/** Envoi des fichiers en attente **/
void sendWaitingReports() {
  /* Buffer temporaire pour la lecture du fichier */
  char buffer[BUFFER_SIZE];

  /* Variables contenant les résultats de la lecture du fichier texte */
  time_t t;
  int sensorValue;

  /* Ouverture du premier fichier disponible */
  root = SD.open("/");
  root.rewindDirectory();
  File entry =  root.openNextFile();

  /* Tant qu'un fichier est disponible */
  while (entry) {
    /* Test pour savoir s'il s'agit d'un répertoire */
    if (!entry.isDirectory()) {
      /* Affichage du nom du fichier en cours (pour debug) */
      Serial.print(F("Traitement du fichier : "));
      Serial.println(entry.name());

      /* Lecture des données du fichier */
      byte i = 0;
      while (entry.available() > 0) { /* Jusqu'a la fin du fichier */
        if ((buffer[i] = entry.read()) == '\n') /* Lecture d'une ligne compléte */
          break;

        /* Test anti buffer-overflow */
        if (++i == BUFFER_SIZE) {
          Serial.println(F("Fichier trop gros !"));
          --i;
          break;
        }
      }
      buffer[i] = '\0';

      Serial.print(F("Datas : "));
      Serial.println( buffer );

      /* Extraction des champs */
      if(sscanf(buffer, "%lu %i", &t, &sensorValue) == 2) {
        /* Envoi de la requéte */
        if(sendData(t, sensorValue)) {
          /* Suppression du fichier d'attente */
          Serial.println(F("Rapport envoye !"));
          SD.remove(entry.name());
        } else {
          Serial.println(F("Rapport toujours en attente ..."));
        }
      }
    }

    /* Fermeture du fichier */
    entry.close();

    /* Ouverture du prochain fichier disponible sur la carte SD */
    entry =  root.openNextFile();
  }
  root.close();
}


/** Fonction d'envoi au serveur PHP **/
boolean sendData(time_t t, int sensorValue) {
  /* Buffer contenant une partie de la requete HTTP */
  char buffer[BUFFER_SIZE];

  Serial.println(F("Debut envoi ..."));

  /* Ouverture de la connexion TCP */
  EthernetClient client;
  Serial.print(F("Connexion au serveur ... "));
  if (!client.connect(serverIpAddress, 80)) {
    /* Erreur */
    Serial.println(F("Echec !"));
    return false;
  }
  Serial.println(F("Ok !"));
  float temp = convertTemp(sensorValue);
  Serial.print(F(" -> Envoie : "));
  Serial.println( temp );
  
  /* Envoi de la requete HTTP */
  client.print(F("GET /arduino/temperature/add/"));
  client.print( numArduino );
  client.print( "/" );
  client.print( temp );
  client.print( "/" );
  client.print( t );
  client.println(F(" HTTP/1.0"));
  client.println();

  /* Fin de la requete HTTP */
  Serial.println(F("Fin envoi ..."));
  delay(10); /* Permet au serveur distant de recevoir les données avant de fermer la connexion */
  client.stop();

  /* Pas d'erreur */
  return true;
}


/** Partie Calcul de la Temperature en Celsius **/
float convertTemp(int sensorValue){
  float Temp = 10000.0*((1024.0/sensorValue) - 1);

  /*
  En supposant une thermistance 10k.  Le calcul est: Resistance = (1024 /ADC -1) * BalanceResistor (10k ohm)
  Pour une "GND-thermistance-Pullup - circuit "Varef" il serait Rtherm=Rpullup/(1024.0/ADC-1)
  -----
  Enregistrement du Log(résistance) afin de ne pas calculer 4 fois plus tard.
  Sur la ligne suivant "Temp" signifie "Temporaire".
  Ici la fonction log() est la fonction mathematique ln().
  Vu que les fonctions mathematique sont, par exemple,  ln(10) = 2.3025 et que log(10) = 1 on multiplie le log(10) par 2.3025. 
  */
  Temp = log(Temp);

  // Maintenant, il signifie à la fois "temporaire" et "Température"
  Temp = 100.0 / (A + (B * Temp) + (C * Temp * Temp * Temp));

  // Convertir Kelvin en Celsius     // Maintenant, "Temp" signifie seulement "Température"
  Temp = Temp - 273.15;

  // On fait une division de 2.3025 a cause du premier calcul qu'on a fait avec la fonction log
  Temp = Temp / 2.3025;

  return Temp;
}

