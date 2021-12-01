#include <pgmspace.h>

const char* WIFI_SSID = ""; // Your SSID
const char* WIFI_PASS = ""; // Your PASSWORD

const char* MQTT_BROKER = ""; // ENDPOINT FROM AWS
const int MQTT_PORT = 8883; // VALID PORT FROM MQTT

const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

const char CERTIFICATE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)KEY";

const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)KEY";