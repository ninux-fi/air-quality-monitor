# README #

Lo sviluppo si basa sul progetto.

[https://github.com/kadamski/arduino_sds011](https://github.com/kadamski/arduino_sds011)

che permette di gestire l'acquisizione dei dati dal sensore SD011 e lo spengimento via software della ventola e del laser.
Inoltre e' stata sfruttata la libreria PubSubClient per la comunicazione via MQTT e la libreria 

[WiFiManager](https://github.com/tzapu/WiFiManager)

che semplifica il processo di configurazione remota. Infatti nessuna credenziale viene esplicitamente scritta nello sketch, ma viene salvata su EEPROM, rimanendo disponibile anche al successivo riavvio e proponendo anche una soluzione d’acquisizione più logica ed efficace sotto ogni punto di vista:

  - All’avvio dell’ESP si entra in Station Mode, cercando di connettersi all’Access Point precedentemente salvato.
  - Se non ci sono network salvati o il collegamento fallisce si entra in Access Point Mode, avviando DNS e WebServer (su un ip di default 192.168.4.1).
  - Usando qualsiasi dispositivo dotato di wifi e browser, ci si può connettere all’access point generato dall’ESP.
  - Grazie al Captive Portal e al server DNS qualunque sito si tenterà di visitare dall’access point si verrà rimandati alla pagina di configurazione.
  - Dalla pagina di configurazione verranno elencati gli access point disponibili alla connessione, basterà dunque selezionarne uno e inserire la password ed eventualmente inserire gli altri parametri opzionali richiesti.
  - L’ESP si riavvierà tentando la connessione. Se la connessione ha successo il controllo passa alla logica implementata nella nostra app, altrimenti si avvia nuovamente un access point in attesa di una nuova configurazione.

Dalle ultime release è stata introdotto la possibilità di salvare anche parametri addizionali, oltre al semplice SSID e password, anche l’host MQTT e la porta, nonchè le stesse credenziali d’accesso al network MQTT.

Fonte 
[Tesi-Rocco-Musolino](https://hackerstribe.com/wp-content/uploads/2016/04/Tesi-Rocco-Musolino.pdf)
