
// UNCOMMENT TO JOIN WIFI INSTEAD OF MAKING ACCESS POINT
// #define JOIN_WIFI 1
// #define JOIN_WIFI_SSID "myWifi"
// #define JOIN_WIFI_PASSWORD "myPassword"
// YOU WILL ALSO HAVE TO EDIT IP AND MASK IN config_wifi() inside networking.hpp

//REMEMBER TO CHECK/WRITE THE SERIAL NUMBER ON THE PCB
//IN THE DESEGNATED SPOT!
#define PCB_SERIAL_NUM 1

#if PCB_SERIAL_NUM == 1
#define WIFI_SSID "1 - Professor X"
#define WIFI_PASSWORD "Charles Xavier"

#elif PCB_SERIAL_NUM == 2
#define WIFI_SSID "2 - Cyclops"
#define WIFI_PASSWORD "Scott Summers"

#elif PCB_SERIAL_NUM == 3
#define WIFI_SSID "3 - Beast"
#define WIFI_PASSWORD "Hank McCoy"

#elif PCB_SERIAL_NUM == 4
#define WIFI_SSID "4 - Phoenix"
#define WIFI_PASSWORD "Jean Grey"

#elif PCB_SERIAL_NUM == 5
#define WIFI_SSID "5 - Havok"
#define WIFI_PASSWORD "Alex Summers"

#elif PCB_SERIAL_NUM == 6
#define WIFI_SSID "6 - Nightcrawler"
#define WIFI_PASSWORD "Kurt Wagner"

#elif PCB_SERIAL_NUM == 7
#define WIFI_SSID "7 - Wolverine"
#define WIFI_PASSWORD "Logan Howlett"

#elif PCB_SERIAL_NUM == 8
#define WIFI_SSID "8 - Storm"
#define WIFI_PASSWORD "Ororo Munroe"

#elif PCB_SERIAL_NUM == 9
#define WIFI_SSID "9 - Rogue"
#define WIFI_PASSWORD "Anna LeBeau"

#elif PCB_SERIAL_NUM == 10
#define WIFI_SSID "10 - Magneto"
#define WIFI_PASSWORD "Erik Lehnsherr"

#elif PCB_SERIAL_NUM == 11
#define WIFI_SSID "11 - Mystique"
#define WIFI_PASSWORD "Raven Darkholme"

#elif PCB_SERIAL_NUM == 12
#define WIFI_SSID "12 - Deadpool"
#define WIFI_PASSWORD "Wade Wilson"

#elif PCB_SERIAL_NUM == 13
#define WIFI_SSID "13 - Quicksilver"
#define WIFI_PASSWORD "Peter Maximoff"

#elif PCB_SERIAL_NUM == 14
#define WIFI_SSID "14 - Iceman"
#define WIFI_PASSWORD "Bobby Drake"

#elif PCB_SERIAL_NUM == 15
#define WIFI_SSID "15 - Scarlet Witch"
#define WIFI_PASSWORD "Wanda Maximoff"

#else
#error "Invalid PCB_SERIAL_NUM. Please choose a number between 1 and 15."
#endif

