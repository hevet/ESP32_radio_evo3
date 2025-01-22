// ESP32 Radio Evo3 Just Radio
// Source ->
// Changes:
// - stations "Banks" directly loaded from SD card
// - station text scrolling add
// Based on project https://github.com/sarunia/ESP32_radio_player_v2


#include "Arduino.h"      // Standardowy nagłówek Arduino, który dostarcza podstawowe funkcje i definicje
#include "Audio.h"        // Biblioteka do obsługi funkcji związanych z dźwiękiem i audio
#include "SPI.h"          // Biblioteka do obsługi komunikacji SPI
#include "SD.h"           // Biblioteka do obsługi kart SD
#include "FS.h"           // Biblioteka do obsługi systemu plików
#include <U8g2lib.h>      // Biblioteka do obsługi wyświetlaczy
#include <ezButton.h>     // Biblioteka do obsługi enkodera z przyciskiem
#include <HTTPClient.h>   // Biblioteka do wykonywania żądań HTTP, umożliwia komunikację z serwerami przez protokół HTTP
//#include <EEPROM.h>       // Biblioteka do obsługi pamięci EEPROM, przechowywanie danych w pamięci nieulotnej
#include <Ticker.h>       // Mechanizm tickera do odświeżania timera 1s, pomocny do cyklicznych akcji w pętli głównej
#include <WiFiManager.h>  // Biblioteka do zarządzania konfiguracją sieci WiFi, opis jak ustawić połączenie WiFi przy pierwszym uruchomieniu jest opisany tu: https://github.com/tzapu/WiFiManager
#include <WiFi.h>
//#include <ArduinoJson.h>          // Biblioteka do parsowania i tworzenia danych w formacie JSON, użyteczna do pracy z API
#include <Time.h>  // Biblioteka do obsługi funkcji związanych z czasem, np. odczytu daty i godziny


#define softwareRev "v3.14"  // wersja oprogramowania

// definicja pinow czytnika karty SD
#define SD_CS 47    // Pin CS (Chip Select) dla karty SD wybierany jako interfejs SPI
#define SD_SCLK 45  // Pin SCK (Serial Clock) dla karty SD
#define SD_MISO 21  // Pin MISO (Master In Slave Out) dla karty SD
#define SD_MOSI 48  // pin MOSI (Master Out Slave In) dla karty SD

// Definicja pinow dla wyswietlacza OLED
#define SPI_MOSI_OLED 39  // Pin MOSI (Master Out Slave In) dla interfejsu SPI OLED
#define SPI_MISO_OLED 0   // Pin MISO (Master In Slave Out) brak dla wyswietlacza OLED
#define SPI_SCK_OLED 38   // Pin SCK (Serial Clock) dla interfejsu SPI OLED
#define CS_OLED 42        // Pin CS (Chip Select) dla interfejsu OLED
#define DC_OLED 40        // Pin DC (Data/Command) dla interfejsu OLED
#define RESET_OLED 41     // Pin Reset dla interfejsu OLED

#define I2S_DOUT 13             // Podłączenie do pinu DIN na DAC
#define I2S_BCLK 12             // Podłączenie po pinu BCK na DAC
#define I2S_LRC 14              // Podłączenie do pinu LCK na DAC
#define SCREEN_WIDTH 256        // Szerokość ekranu w pikselach
#define SCREEN_HEIGHT 64        // Wysokość ekranu w pikselach
#define CLK_PIN1 6              // Podłączenie z pinu 6 do CLK na enkoderze prawym
#define DT_PIN1 5               // Podłączenie z pinu 5 do DT na enkoderze prawym
#define SW_PIN1 4               // Podłączenie z pinu 4 do SW na enkoderze prawym (przycisk)
#define CLK_PIN2 10             // Podłączenie z pinu 10 do CLK na enkoderze
#define DT_PIN2 11              // Podłączenie z pinu 11 do DT na enkoderze lewym
#define SW_PIN2 1               // Podłączenie z pinu 1 do SW na enkoderze lewym (przycisk)
#define MAX_STATIONS 99        // Maksymalna liczba stacji radiowych, które mogą być przechowywane w jednym banku
#define STATION_NAME_LENGTH 42  // Nazwa stacji wraz z bankiem i numerem stacji do wyświetlenia w pierwszej linii na ekranie
#define MAX_FILES 100           // Maksymalna liczba plików lub katalogów w tablicy directoriesz

#define STATIONS_URL  "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank01.txt"    // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL1 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank02.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL2 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank03.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL3 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank04.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL4 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank05.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL5 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank06.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL6 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank07.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL7 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank08.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL8 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank09.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL9 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank10.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL10 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank11.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL11 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank12.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL12 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank13.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL13 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank14.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL14 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank15.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL15 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank16.txt"  // Adres URL do pliku z listą stacji radiowych

// Przypisanie przycisków i adresu pilota w standardzie NEC 
// pierwszy bajt adres, drugi komenda (B914 - adres B9 komenda 14)
#define rcAdress 0xB9
#define rcCmdVolumeUp 0xB914      // Głosnosc +
#define rcCmdVolumeDown 0xB915    // Głośnosc -
#define rcCmdArrowRight 0xB90B          // strzałka w prawo - nastepna stacja
#define rcCmdArrowLeft 0xB90A      // strzałka w lewo - poprzednia stacja  
#define rcCmdArrowUp 0xB987    // strzałka w góre - lista stacji krok do gory
#define rcCmdArrowDown 0xB986  // strzałka w dół - lista stacj krok na dół
#define rcCmdOk 0xB90E          // Przycisk Ent - zatwierdzenie stacji
#define rcCmdSrc 0xB913
#define rcCmdMute 0xB916
#define rcCmdDirect 0xB90F
#define rcCmdBankMinus 0xB90C
#define rcCmdBankPlus 0xB90D
#define rcCmdKey0 0xB900
#define rcCmdKey1 0xB901
#define rcCmdKey2 0xB902
#define rcCmdKey3 0xB903
#define rcCmdKey4 0xB904
#define rcCmdKey5 0xB905
#define rcCmdKey6 0xB906
#define rcCmdKey7 0xB907
#define rcCmdKey8 0xB908
#define rcCmdKey9 0xB909


int currentSelection = 0;      // Numer aktualnego wyboru na ekranie OLED
int firstVisibleLine = 0;      // Numer pierwszej widocznej linii na ekranie OLED
uint8_t station_nr = 0;        // Numer aktualnie wybranej stacji radiowej z listy
int stationFromBuffer = 0;     // Numer stacji radiowej przechowywanej w buforze do przywrocenia na ekran po bezczynności
uint8_t bank_nr;               // Numer aktualnie wybranego banku stacji z listy
uint8_t previous_bank_nr = 0;  // Numer banku przed wejsciem do menu zmiany banku
int bankFromBuffer = 0;        // Numer aktualnie wybranego banku stacji z listy do przywrócenia na ekran po bezczynności
uint8_t screenRefresh = 0;     // odświezeanie ekranu dla stajci bez statoin string

int button_S1 = 17;  // Przycisk S1 podłączony do pinu 17
int button_S2 = 18;  // Przycisk S2 podłączony do pinu 18
//int button_S3 = 15;  // Przycisk S3 podłączony do pinu 15
int button_S4 = 16;  // Przycisk S4 podłączony do pinu 16


int CLK_state1;                        // Aktualny stan CLK enkodera prawego
int prev_CLK_state1;                   // Poprzedni stan CLK enkodera prawego
int CLK_state2;                        // Aktualny stan CLK enkodera lewego
int prev_CLK_state2;                   // Poprzedni stan CLK enkodera lewego
int counter = 0;                       // Licznik dla przycisków
int stationsCount = 0;                 // Aktualna liczba przechowywanych stacji w tablicy
int directoryCount = 0;                // Licznik katalogów
int fileIndex = 0;                     // Numer aktualnie wybranego pliku audio ze wskazanego folderu
int fileFromBuffer = 0;                // Numer aktualnie wybranego pliku do przywrócenia na ekran po bezczynności
int folderIndex = 0;                   // Numer aktualnie wybranego folderu podczas przełączenia do odtwarzania z karty SD
int folderFromBuffer = 0;              // Numer aktualnie wybranego folderu do przywrócenia na ekran po bezczynności
int totalFilesInFolder = 0;            // Zmienna przechowująca łączną liczbę plików w folderze
int volumeValue = 10;                  // Wartość głośności, domyślnie ustawiona na 10
int cycle = 0;                         // Numer cyklu do danych pogodowych wyświetlanych w trzech rzutach co 10 sekund
int maxVisibleLines = 4;               // Maksymalna liczba widocznych linii na ekranie OLED
int bitrateStringInt = 0;              // Deklaracja zmiennej do konwersji Bitrate string na wartosc Int aby podzelic bitrate przez 1000
int buttonLongPressTime1 = 2000;       // Czas reakcji na długie nacisniecie enkoder 1
int buttonLongPressTime2 = 2000;       // Czas reakcji na długie nacisniecie enkoder 2
int buttonShortPressTime2 = 500;       // Czas rekacjinna krótkie nacisniecie enkodera 2
int buttonSuperLongPressTime2 = 4000;  // Czas reakcji na super długie nacisniecie enkoder 2

int toneLowValue = 0;               // Wartosc filtra dla tonow niskich
int toneMidValue = 0;               //  Wartosc flitra dla tonow srednich
int toneHiValue = 0;                //Wartość filtra tonow wysokich

uint8_t rcInputDigit1 = 0xFF;
uint8_t rcInputDigit2 = 0xFF;
//uint32_t dataIr;
//uint8_t command;
//uint8_t address;

//const int maxVisibleLines = 5;    // Maksymalna liczba widocznych linii na ekranie OLED
bool encoderButton1 = false;      // Flaga określająca, czy przycisk enkodera 1 został wciśnięty
bool encoderButton2 = false;      // Flaga określająca, czy przycisk enkodera 2 został wciśnięty
bool fileEnd = false;             // Flaga sygnalizująca koniec odtwarzania pliku audio
bool displayActive = false;       // Flaga określająca, czy wyświetlacz jest aktywny
bool isPlaying = false;           // Flaga określająca, czy obecnie trwa odtwarzanie
bool mp3 = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie MP3
bool flac = false;                // Flaga określająca, czy aktualny plik audio jest w formacie FLAC
bool aac = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie AAC
bool id3tag = false;              // Flaga określająca, czy plik audio posiada dane ID3
bool timeDisplay = true;          // Flaga określająca kiedy pokazać czas na wyświetlaczu, domyślnie od razu po starcie
bool listedStations = false;      // Flaga określająca czy na ekranie jest pokazana lista stacji do wyboru
bool menuEnable = false;          // Flaga określająca czy na ekranie można wyświetlić menu
bool bankMenuEnable = false;      // Flaga określająca czy na ekranie jest wyświetlone menu wyboru banku
bool bitratePresent = false;      // Flaga określająca, czy na serial terminalu pojawiła się informacja o bitrate - jako ostatnia dana spływajaca z info
bool bankNetworkUpdate = false;   // Flaga wyboru aktualizacji banku z sieci lub karty SD - True aktulizacja z NETu
bool bank1NetworkUpdate = false;  // Flaga wyboru aktualizacji banku z sieci lub karty SD - True aktulizacja z NETu
bool bank2NetworkUpdate = false;  // Flaga wyboru aktualizacji banku z sieci lub karty SD
bool button_1 = false;            // Flaga określająca stan przycisku 1
bool button_2 = false;            // Flaga określająca stan przycisku 2
bool button_3 = false;            // Flaga określająca stan przycisku 3
bool button_4 = false;            // Flaga określająca stan przycisku 4
bool volumeSet = false;           // Flaga wejscia menu regulacji głosnosci
bool vuMeterOn = true;            // Flaga właczajaca wskazniki VU
bool vuMeterMode = false;         // tryb rysowania vuMeter
bool action3Taken = false;        // Flaga Akcji 3 - załaczenia VU
bool action4Taken = false;        // Flaga Akcji 4 - uzycie diplsayRadio
bool volumeUpFlag = false;
bool volumeDownFlag = false;
bool stationChangeFlag  = false;
bool ActionNeedUpdateTime = false;
bool bankChange = false;
bool debugAudioBuffor = false;

//unsigned long lastDebounceTime = 0;       // Czas ostatniego debouncingu
unsigned long debounceDelay = 300;    // Czas trwania debouncingu w milisekundach
unsigned long displayTimeout = 6000;  // Czas wyświetlania komunikatu na ekranie w milisekundach
unsigned long displayStartTime = 0;   // Czas rozpoczęcia wyświetlania komunikatu
unsigned long seconds = 0;            // Licznik sekund timera
unsigned int PSRAM_lenght = MAX_STATIONS * (STATION_NAME_LENGTH) + MAX_STATIONS; // deklaracjia długości pamięci PSRAM
uint8_t StationNameStreamWidth = 0;  // Test pełnej nazwy stacji
uint8_t x = 0;                             // Globalna zmienna pomocnicza 
unsigned long vuMeterTime;                 // Czas opznienia odswiezania wskaznikow VU w milisekundach
uint8_t vuMeterL;                          // Wartosc VU dla L kanału zakres 0-255
uint8_t vuMeterR;                          // Wartosc VU dla R kanału zakres 0-255
unsigned long debugTime;                   // Timer wywolujacy print funkcji przenzaczony do wyswietlenia debugu
unsigned long scrollingStationStringTime;  // Czas do odswiezania scorllingu
unsigned long scrollingRefresh = 65;       // Czas przewijania tekstu
unsigned long scrollingRefreshFlac = 500;  // Czas przewijania i refreshu VU, czasu przy streame FLAC
uint16_t stationStringWidth;               //szerokosc Stringu nazwy stacji
uint16_t xPositionStationString = 0;       // Pozycja początkowa dla przewijania tekstu StationString
uint16_t offset;                          // Zminnna offsetu dla funkcji Scrollera - przewijania streamtitle na ekranie OLED
unsigned char * psramData;                  // zmienna do trzymania danych stacji w pamieci PSRAM

// Serwer Web
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

// Sprawdzenie funkcji pilota
unsigned long czas1 = 0;
unsigned long czas2 = 0;

String stationStringScroll = "";
String directories[MAX_FILES];       // Tablica z indeksami i ścieżkami katalogów
String currentDirectory = "/music";  // Ścieżka bieżącego katalogu
String stationName;                  // Nazwa aktualnie wybranej stacji radiowej
String stationString;                // Dodatkowe dane stacji radiowej (jeśli istnieją)
String bitrateString;                // Zmienna przechowująca informację o bitrate
String sampleRateString;             // Zmienna przechowująca informację o sample rate
String bitsPerSampleString;          // Zmienna przechowująca informację o liczbie bitów na próbkę
String artistString;                 // Zmienna przechowująca informację o wykonawcy
String titleString;                  // Zmienna przechowująca informację o tytule utworu
String fileNameString;               // Zmienna przechowująca informację o nazwie pliku
String folderNameString;             // Zmienna przechowująca informację o nazwie folderu
String PlayedFolderName;             // Nazwa aktualnie odtwarzanego folderu
String currentIP;
String StationNameStream;  // Nazwa stacji wyciągnieta z danych wysylanych przez stream

String header;







// Przygotowanie danych pogody do wyświetlenia
/*
String tempStr;           // Zmienna do przechowywania temperatury
String feels_likeStr;     // Zmienna do przechowywania temperatury odczuwalnej
String humidityStr;       // Zmienna do przechowywania wilgotności
String pressureStr;       // Zmienna do przechowywania ciśnienia atmosferycznego
String windStr;           // Zmienna do przechowywania prędkości wiatru
String windGustStr;       // Zmienna do przechowywania prędkości porywów wiatru
*/

File myFile;  // Uchwyt pliku

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/CS_OLED, /* dc=*/DC_OLED, /* reset=*/RESET_OLED);  // Hardware SPI 3.12inch OLED
//U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ CS_OLED, /* dc=*/ DC_OLED, /* reset=*/ RESET_OLED);		// Hardware SPI  2.08inch OLED

WiFiServer server(80);

// Inicjalizacja WiFiManagera
WiFiManager wifiManager;


// Konfiguracja nowego SPI z wybranymi pinami dla czytnika kart SD
SPIClass customSPI = SPIClass(HSPI);  // Używamy HSPI, ale z własnymi pinami


ezButton button1(SW_PIN1);  // Utworzenie obiektu przycisku z enkodera 1 ezButton, podłączonego do pinu 4
ezButton button2(SW_PIN2);  // Utworzenie obiektu przycisku z enkodera 1 ezButton, podłączonego do pinu 1
Audio audio;                // Obiekt do obsługi funkcji związanych z dźwiękiem i audio
Ticker timer1;              // Timer do updateTimer co 1s
Ticker timer2;                            // Timer do getWeatherData co 60s
//Ticker timer3;                            // Timer do przełączania wyświetlania danych pogodoych w ostatniej linii co 10s
WiFiClient client;  // Obiekt do obsługi połączenia WiFi dla klienta HTTP

char stations[MAX_STATIONS][STATION_NAME_LENGTH + 1];  // Tablica przechowująca linki do stacji radiowych (jedna na stację) +1 dla terminatora null

const char *ntpServer = "pool.ntp.org";  // Adres serwera NTP używany do synchronizacji czasu
const long gmtOffset_sec = 3600;         // Przesunięcie czasu UTC w sekundach
const int daylightOffset_sec = 3600;     // Przesunięcie czasu letniego w sekundach, dla Polski to 1 godzina
//sntp_sync_status_t syncStatus;

// Czcionka Dot Matrix dla nazwy stacji radiowej. Rozmiar 0-17 (duze litery 13 male litery 17)
const uint8_t DotMatrix13pl[4602] U8G2_FONT_SECTION("DotMatrix13pl") =
  "\340\1\5\2\4\5\1\3\5\12\21\0\374\15\374\15\0\1\345\4\3\21\335 \7\34\203\177>\11!\11"
  "\23c\202\224n\305\1\42\13\27\343\202\214\244\214|v\0#\25\33c\213\214\304\214\274\204\356e\344%"
  "t/#\61#\37\21$\23\33c\223\314\204\326\62\62\23jf\244%\264\231\217\11%\21\33c\203\204"
  "\314\204\224\314\256$d&\344\343\1&\23\33c\213\314\214\304\214\314\32\325R\22\23\62\362\361\0'\15"
  "\25\243\202\204\234\204\244\234|\134\0(\12\25\243\212\234\244n\305\2)\12\25\243\202\254\244\356\4\3*"
  "\17\33c\343\214\314\304\204.ff\344\23\2+\15\33c\353\324\304\204.\246\346\223\2,\15\25\243~"
  "\244\204\234\204\244\234\70\0-\12\33c\177\42\11\335g\23.\13\25\243~\314\204\234\204X\0/\11\33"
  "c\373\314\356\223\3\60\27\33c\213\204z\71i)\11i\31\325\22R\322r\362\22\352#\2\61\14\27"
  "\343\212\254\204\264n%\224\7\62\22\33c\213\204z\71\251\211\11\211\231\251\11\335\307\3\63\23\33c\213"
  "\204z\71\251y\11u\323r\362\22\352#\2\64\22\33c\233\314\204\304\214\274\224\274\204n\246\346#\2"
  "\65\22\33c\203\204\256\245&\264\233\232\226\223\227P\37\21\66\23\33c\223\204\304\314\324\204\366r\322r"
  "\362\22\352#\2\67\14\33c\203\204\256f\266Z\37\25\70\25\33c\213\204z\71i\71y\11\365r\322"
  "r\362\22\352#\2\71\23\33c\213\204z\71i\71y\11\255f&&\344c\2:\16\25\243\322\204\234"
  "\204\304\204\234\204X\0;\20\25\243\252\204\234\204\304\204\234\204\244\234\70\0<\12\31#\233\274\66\333\307"
  "\1=\16\33c\177\264\204\356\203$t\237\70\0>\12\31#\203\314\366\332G\3\77\17\33c\213\204z"
  "\71\251\231\365\301\362\61\1@\24\33c\213\204z\31\325\62\252e$TK\315M\250\217\10A\25\33c"
  "\213\204z\71i\71i\11]\313I\313I\313\311\307\3B\25\33c\203\204\366r\322r\322\22\332\313I"
  "\313IKh\37\21C\20\33c\213\204z\71i\251\355\344%\324G\4D\25\33c\203\204\366r\322r"
  "\322r\322r\322r\322\22\332G\4E\20\33c\203\204\256\245Vh/\265B\367\361\0F\17\33c\203"
  "\204\256\245Vh/\265>.\0G\23\33c\213\204z\71i\251\65\22\252\345\344%\264\217\7H\25\33"
  "c\203\234\264\234\264\234\264\204\256\345\244\345\244\345\344\343\1I\14\27\343\202\204Zi\335J(\17J\15"
  "\33c\243\324\256\345\344%\324G\4K\25\33c\203\234\264\224\274\214\304\204\314\214\304\224\274\234|<\0"
  "L\13\33c\203\324\276\320}<\0M\27\33c\203\234\264\204\214\204\264\214j\31\325r\322r\322r\362"
  "\361\0N\27\33c\203\234\264\234\264\204\224\264\214j)\11i\71i\71\371x\0O\25\33c\213\204z"
  "\71i\71i\71i\71i\71y\11\365\21\1P\21\33c\203\204\366r\322r\322\22\332K\255\217\13Q"
  "\26\33c\213\204z\71i\71i\71i\31\325R\22\23\62\362\361\0R\25\33c\203\204\366r\322r\322"
  "\22\332\313HL\311\313\311\307\3S\23\33c\213\204z\71i\271\11u\323r\362\22\352#\2T\14\33"
  "c\203\204.\246\366>&\0U\25\33c\203\234\264\234\264\234\264\234\264\234\264\234\274\204\372\210\0V\23"
  "\33c\203\234\264\234\264\234\274\214\304\214\314\324|L\0W\27\33c\203\234\264\234\264\234\264\214j\31\325"
  "\22\62\22\322r\362\361\0X\23\33c\203\234\264\234\274\214\314\32y\71i\71\371x\0Y\17\33c\203"
  "\234\264\234\274\214\314\324\366\61\1Z\16\33c\203\204\256fv\65\241\373x\0[\13\27\343\202\204Ji"
  "\275P\36\134\11\33c\333\334\356\223\1]\13\27\343\202\204j\275\224P\36^\14\33c\223\314\214\274\234"
  "|N\1_\12\33c\177^M\350\4\0`\15\25\243\202\204\234\204\234\254|T\0a\21\33c\177\274"
  "\204\334\304\204z)\211\11\355\343\1b\24\33c\203\324\32\11y\11)i\71i\71i\11\355#\2c"
  "\17\33c\177\274\204z\251u\362\22\352#\2d\25\33c\243\324\274\204\214\264\224\204\264\234\264\234\274\204"
  "\366\361\0e\21\33c\177\274\204z\71i\11\355\345&\324G\4f\17\31#\223\274\214\264\274\204z\211"
  "\365\261\0g\24\33c\177\274\204z\71i\71i\71y\11\255\346%\24\1h\24\33c\203\324\32\11y"
  "\11)i\71i\71i\71\371x\0i\14\27\343\212\344\204\264\266\22\312\3j\21\31#\233|\210\204\304"
  "\266R\262R\322\22B\0k\22\31#\203\304*Y\31i\11y\31i)\371\70\0l\13\27\343\202\204"
  "\264\336J(\17m\22\33c\177\274\214\274\214j\31\325r\322r\362\361\0n\24\33c\177\264\214\204\274"
  "\204\224\264\234\264\234\264\234|<\0o\22\33c\177\274\204z\71i\71i\71y\11\365\21\1p\23\33"
  "c\177\264\204\366r\322\22R\322\62\22\362RK\1q\22\33c\177\274\204z)y\31\11\211\31\251-"
  "D\0r\17\31#\177\224\214\204\254\204\274\304\372h\0s\20\33c\177\274\204\326r\23\352\246%\264\217"
  "\10t\16\31#\323\274\204z\211\65\362\362\221\0u\24\33c\177\264\234\264\234\264\234\264\224\204\274\204\214"
  "|<\0v\21\33c\177\264\234\264\234\264\234\274\214\314|L\0w\22\33c\177\264\234\264\234\264\214j"
  "\31\365\62\362\21\1x\20\33c\177\264\234\274\214\314\32y\71\371x\0y\16\33c\177\264\234\274\214\314"
  "\324\372\230\0z\15\33c\177\264\204nv\241\373x\0{\15\27\343\222\254\264\254\274\264\274x\0|\12"
  "\23c\202\224Z)\345\0}\16\27\343\202\274\264\274\254\264\254|\14\0~\14\33c\177\274\314\214\232\371"
  "\344\1\177\15\26\303\312\201\210\376\377\23\3\241\0\200\5\20\3\2\201\5\20\3\2\202\5\20\3\2\203\5"
  "\20\3\2\204\5\20\3\2\205\5\20\3\2\206\5\20\3\2\207\5\20\3\2\210\5\20\3\2\211\5\20\3"
  "\2\212\5\20\3\2\213\5\20\3\2\214\24\33c\227\234\204\42\71i\261\11\315\246\345\344%\324G\4\215"
  "\5\20\3\2\216\5\20\3\2\217\17\33c\227\244\230\204nv\65\241\373x\0\220\5\20\3\2\221\7\33"
  "c\177\276\0\222\5\20\3\2\223\5\20\3\2\224\5\20\3\2\225\5\20\3\2\226\5\20\3\2\227\5\20"
  "\3\2\230\5\20\3\2\231\5\20\3\2\232\5\20\3\2\233\5\20\3\2\234\21\33c\363\244\234\204\326r"
  "\23\352\246%\264\217\10\235\5\20\3\2\236\5\20\3\2\237\16\33c\363\244\224\204nv\241\373x\0\240"
  "\7\34\203\177>\11\241\26\33c\213\204z\71i\71i\11]\313I\313I\313IM\212\3\242!\34\203"
  "\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\243!\34"
  "\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\244\17"
  "\33c\357\310\204z\251u\362\22\352#\2\245\26\33c\213\204z\71i\71i\11]\313I\313I\313I"
  "M\212\3\246\24\33c\227\234\204\42\71i\261\11\315\246\345\344%\324G\4\247!\34\203\263\201\3\21A"
  "\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\250!\34\203\263\201\3\21"
  "A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\251!\34\203\263\201\3"
  "\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\252!\34\203\263\201"
  "\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\253!\34\203\263"
  "\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\254\17\33c"
  "\227\244\230\204nv\65\241\373x\0\255!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21\3\7\362\61\1\256!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21\3\7\362\61\1\257\16\33c\223\304\204nv\65\241\373x\0\260!\34\203\263"
  "\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\261\22\33c"
  "\177\274\204\334\304\204z)\211\11-e\5\3\262!\34\203\263\201\3\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21\3\7\362\61\1\263\17\27\343\202\204\264\12Y\25\322\262\22\312\3\264!"
  "\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\265"
  "!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1"
  "\266\21\33c\363\244\234\204\326r\23\352\246%\264\217\10\267!\34\203\263\201\3\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\270!\34\203\263\201\3\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\271\22\33c\177\274\204\334\304\204z)\211"
  "\11-e\5\3\272!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21\3\7\362\61\1\273!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21\3\7\362\61\1\274\16\33c\363\244\224\204nv\241\373x\0\275!\34\203\263\201\3\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\276!\34\203\263\201\3\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\277\15\33c\353\304\204nv"
  "\241\373x\0\300!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "\3\7\362\61\1\301!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21\3\7\362\61\1\302!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21\3\7\362\61\1\303!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21\3\7\362\61\1\304!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21\3\7\362\61\1\305!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21\3\7\362\61\1\306\21\33c\227\310\204z\71i\251u\362\22\352#\2\307!\34\203\263"
  "\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\310!\34\203"
  "\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\311!\34"
  "\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\312\21"
  "\33c\203\204\256\245Vh/\265BW\223\342\0\313!\34\203\263\201\3\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21\3\7\362\61\1\314!\34\203\263\201\3\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21\3\7\362\61\1\315!\34\203\263\201\3\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\316!\34\203\263\201\3\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\317!\34\203\263\201\3\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\320!\34\203\263\201\3\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\321\27\33c\233\240\230\234\264\204\224"
  "\264\214j)\11i\71i\71\371x\0\322!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21\3\7\362\61\1\323\24\33c\227\310\204z\71i\71i\71i\71y\11\365\21"
  "\1\324!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362"
  "\61\1\325!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7"
  "\362\61\1\326!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3"
  "\7\362\61\1\327!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "\3\7\362\61\1\330!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21\3\7\362\61\1\331!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21\3\7\362\61\1\332!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21\3\7\362\61\1\333!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21\3\7\362\61\1\334!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21\3\7\362\61\1\335!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21\3\7\362\61\1\336!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21\3\7\362\61\1\337!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21\3\7\362\61\1\340!\34\203\263\201\3\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21\3\7\362\61\1\341!\34\203\263\201\3\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21\3\7\362\61\1\342!\34\203\263\201\3\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21\3\7\362\61\1\343!\34\203\263\201\3\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21\3\7\362\61\1\344!\34\203\263\201\3\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\345!\34\203\263\201\3\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\346\17\33c\357\310\204z\251u\362\22\352"
  "#\2\347!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7"
  "\362\61\1\350!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3"
  "\7\362\61\1\351!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "\3\7\362\61\1\352\22\33c\177\274\204z\71i\11\355\345&T\15J\4\353!\34\203\263\201\3\21A"
  "\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\354!\34\203\263\201\3\21"
  "A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\355!\34\203\263\201\3"
  "\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\356!\34\203\263\201"
  "\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\357!\34\203\263"
  "\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\360!\34\203"
  "\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\361\25\33"
  "c\307\244\300\214\204\274\204\224\264\234\264\234\264\234|<\0\362!\34\203\263\201\3\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\363\23\33c\307\244\310\204z\71i\71i"
  "\71y\11\365\21\1\364!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21\3\7\362\61\1\365!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21\3\7\362\61\1\366!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21\3\7\362\61\1\367!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21\3\7\362\61\1\370!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21\3\7\362\61\1\371!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21\3\7\362\61\1\372!\34\203\263\201\3\21A\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21\3\7\362\61\1\373!\34\203\263\201\3\21A\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21\3\7\362\61\1\374!\34\203\263\201\3\21A\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21\3\7\362\61\1\375!\34\203\263\201\3\21A\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21\3\7\362\61\1\376!\34\203\263\201\3\21A\21A\21A\21A\21"
  "A\21A\21A\21A\21A\21A\21\3\7\362\61\1\377!\34\203\263\201\3\21A\21A\21A\21A"
  "\21A\21A\21A\21A\21A\21A\21\3\7\362\61\1\0\0\0\4\377\377\0";

const uint8_t spleen6x12PL[2954] U8G2_FONT_SECTION("spleen6x12PL") =
  "\340\1\3\2\3\4\1\3\4\6\14\0\375\10\376\11\377\1\225\3]\13m \7\346\361\363\237\0!\12"
  "\346\361#i\357`\316\0\42\14\346\361\3I\226dI\316/\0#\21\346\361\303I\64HI\226dI"
  "\64HIN\6$\22\346q\205CRK\302\61\311\222,I\206\60\247\0%\15\346\361cQK\32\246"
  "I\324\316\2&\17\346\361#Z\324f\213\22-Zr\42\0'\11\346\361#i\235\237\0(\13\346\361"
  "ia\332s\254\303\0)\12\346\361\310\325\36\63\235\2*\15\346\361S\243L\32&-\312\31\1+\13"
  "\346\361\223\323l\320\322\234\31,\12\346\361\363)\15s\22\0-\11\346\361s\32t\236\0.\10\346\361"
  "\363K\316\0/\15\346q\246a\32\246a\32\246\71\15\60\21\346\361\3S\226DJ\213\224dI\26\355"
  "d\0\61\12\346\361#\241\332\343N\6\62\16\346\361\3S\226\226\246\64\35t*\0\63\16\346\361\3S"
  "\226fr\232d\321N\6\64\14\346q\247\245\236\6\61\315\311\0\65\16\346q\17J\232\16qZ\31r"
  "\62\0\66\20\346\361\3S\232\16Q\226dI\26\355d\0\67\13\346q\17J\226\206\325v\6\70\20\346"
  "\361\3S\226d\321\224%Y\222E;\31\71\17\346\361\3S\226dI\26\15ii'\3:\11\346\361"
  "\263\346L\71\3;\13\346\361\263\346\264\64\314I\0<\12\346\361cak\334N\5=\13\346\361\263\15"
  ":\60\350\334\0>\12\346\361\3qk\330\316\2\77\14\346\361\3S\226\206\325\34\314\31@\21\346\361\3"
  "S\226dI\262$K\262\304CN\5A\22\346\361\3S\226dI\226\14J\226dI\226S\1B\22"
  "\346q\17Q\226d\311\20eI\226d\311\220\223\1C\14\346\361\3C\222\366<\344T\0D\22\346q"
  "\17Q\226dI\226dI\226d\311\220\223\1E\16\346\361\3C\222\246C\224\226\207\234\12F\15\346\361"
  "\3C\222\246C\224\266\63\1G\21\346\361\3C\222V\226,\311\222,\32r*\0H\22\346qgI"
  "\226d\311\240dI\226dI\226S\1I\12\346\361\3c\332\343N\6J\12\346\361\3c\332\233\316\2"
  "K\21\346qgI\226D\321\26\325\222,\311r*\0L\12\346q\247}\36r*\0M\20\346qg"
  "\211eP\272%Y\222%YN\5N\20\346qg\211\224HI\77)\221\222\345T\0O\21\346\361\3"
  "S\226dI\226dI\226d\321N\6P\17\346q\17Q\226dI\226\14QZg\2Q\22\346\361\3"
  "S\226dI\226dI\226d\321\252\303\0R\22\346q\17Q\226dI\226\14Q\226dI\226S\1S"
  "\16\346\361\3C\222\306sZ\31r\62\0T\11\346q\17Z\332w\6U\22\346qgI\226dI\226"
  "dI\226d\321\220S\1V\20\346qgI\226dI\226dI\26m;\31W\21\346qgI\226d"
  "I\226\264\14\212%\313\251\0X\21\346qgI\26%a%\312\222,\311r*\0Y\20\346qgI"
  "\226dI\26\15ie\310\311\0Z\14\346q\17j\330\65\35t*\0[\13\346\361\14Q\332\257C\16"
  "\3\134\15\346q\244q\32\247q\32\247\71\14]\12\346\361\14i\177\32r\30^\12\346\361#a\22e"
  "\71\77_\11\346\361\363\353\240\303\0`\11\346\361\3q\235_\0a\16\346\361S\347hH\262$\213\206"
  "\234\12b\20\346q\247\351\20eI\226dI\226\14\71\31c\14\346\361S\207$m\36r*\0d\21"
  "\346\361ci\64$Y\222%Y\222ECN\5e\17\346\361S\207$K\262dP\342!\247\2f\14"
  "\346\361#S\32\16Y\332\316\2g\21\346\361S\207$K\262$K\262hN\206\34\1h\20\346q\247"
  "\351\20eI\226dI\226d\71\25i\13\346\361#\71\246v\325\311\0j\13\346\361C\71\230\366\246S"
  "\0k\16\346q\247\245J&&YT\313\251\0l\12\346\361\3i\237u\62\0m\15\346\361\23\207("
  "\351\337\222,\247\2n\20\346\361\23\207(K\262$K\262$\313\251\0o\16\346\361S\247,\311\222,"
  "\311\242\235\14p\21\346\361\23\207(K\262$K\262d\210\322*\0q\20\346\361S\207$K\262$K"
  "\262hH[\0r\14\346\361S\207$K\322v&\0s\15\346\361S\207$\236\323d\310\311\0t\13"
  "\346\361\3i\70\246\315:\31u\20\346\361\23\263$K\262$K\262h\310\251\0v\16\346\361\23\263$"
  "K\262$\213\222\60gw\17\346\361\23\263$KZ\6\305\222\345T\0x\16\346\361\23\263$\213\266)"
  "K\262\234\12y\21\346\361\23\263$K\262$K\262hH+C\4z\14\346\361\23\7\65l\34t*"
  "\0{\14\346\361iiM\224\323\262\16\3|\10\346q\245\375;\5}\14\346\361\310iY\324\322\232N"
  "\1~\12\346\361s\213\222D\347\10\177\7\346\361\363\237\0\200\6\341\311\243\0\201\6\341\311\243\0\202\6"
  "\341\311\243\0\203\6\341\311\243\0\204\6\341\311\243\0\205\6\341\311\243\0\206\6\341\311\243\0\207\6\341\311"
  "\243\0\210\6\341\311\243\0\211\6\341\311\243\0\212\6\341\311\243\0\213\6\341\311\243\0\214\16\346\361eC"
  "\222\306sZ\31r\62\0\215\6\341\311\243\0\216\6\341\311\243\0\217\14\346qe\203T\354\232\16:\25"
  "\220\6\341\311\243\0\221\6\341\311\243\0\222\6\341\311\243\0\223\6\341\311\243\0\224\6\341\311\243\0\225\6"
  "\341\311\243\0\226\6\341\311\243\0\227\16\346\361eC\222\306sZ\31r\62\0\230\6\341\311\243\0\231\6"
  "\341\311\243\0\232\6\341\311\243\0\233\6\341\311\243\0\234\16\346\361\205\71\66$\361\234&CN\6\235\6"
  "\341\311\243\0\236\6\341\311\243\0\237\15\346\361\205\71\64\250a\343\240S\1\240\7\346\361\363\237\0\241\23"
  "\346\361\3S\226dI\226\14J\226dI\26\306\71\0\242\21\346\361\23\302!\251%Y\222%\341\220\345"
  "\24\0\243\14\346q\247-\231\230\306CN\5\244\22\346\361\3S\226dI\226\14J\226dI\26\346\4"
  "\245\22\346\361\3S\226dI\226\14J\226dI\26\346\4\246\16\346\361eC\222\306sZ\31r\62\0"
  "\247\17\346\361#Z\224\245Z\324\233\232E\231\4\250\11\346\361\3I\316\237\1\251\21\346\361\3C\22J"
  "\211\22)\221bL\206\234\12\252\15\346\361#r\66\325vd\310\31\1\253\17\346\361\223\243$J\242\266"
  "(\213r\42\0\254\14\346qe\203T\354\232\16:\25\255\10\346\361s\333y\3\256\21\346\361\3C\22"
  "*\226d\261$c\62\344T\0\257\14\346qe\203\32vM\7\235\12\260\12\346\361#Z\324\246\363\11"
  "\261\20\346\361S\347hH\262$\213\206\64\314\21\0\262\14\346\361#Z\224\206\305!\347\6\263\13\346\361"
  "\3i\252\251\315:\31\264\11\346\361Ca\235\337\0\265\14\346\361\23\243\376i\251\346 \0\266\16\346\361"
  "\205\71\66$\361\234&CN\6\267\10\346\361s\314y\4\270\11\346\361\363\207\64\14\1\271\20\346\361S"
  "\347hH\262$\213\206\64\314\21\0\272\15\346\361#Z\324\233\16\15\71#\0\273\17\346\361\23\243,\312"
  "\242\226(\211r\62\0\274\15\346\361\205\71\64\250a\343\240S\1\275\17\346\361\204j-\211\302\26\245\24"
  "\26\207\0\276\21\346\361hQ\30'\222\64\206ZR\33\302\64\1\277\15\346\361#\71\64\250a\343\240S"
  "\1\300\21\346\361\304\341\224%Y\62(Y\222%YN\5\301\21\346\361\205\341\224%Y\62(Y\222%"
  "YN\5\302\22\346q\205I\66eI\226\14J\226dI\226S\1\303\23\346\361DI\242MY\222%"
  "\203\222%Y\222\345T\0\304\21\346\361\324\241)K\262dP\262$K\262\234\12\305\16\346\361eC\222"
  "\306sZ\31r\62\0\306\14\346\361eC\222\366<\344T\0\307\15\346\361\3C\222\366<di\30\2"
  "\310\17\346\361\304\341\220\244\351\20\245\361\220S\1\311\17\346\361\205\341\220\244\351\20\245\361\220S\1\312\20"
  "\346\361\3C\222\246C\224\226\207\64\314\21\0\313\17\346\361\324\241!I\323!J\343!\247\2\314\13\346"
  "\361\304\341\230v\334\311\0\315\13\346\361\205\341\230v\334\311\0\316\14\346q\205I\66\246\35w\62\0\317"
  "\13\346\361\324\241\61\355\270\223\1\320\15\346\361\3[\324\262D}\332\311\0\321\20\346\361EIV\221\22"
  ")\351'%\322\251\0\322\20\346\361\304\341\224%Y\222%Y\222E;\31\323\20\346\361\205\341\224%Y"
  "\222%Y\222E;\31\324\21\346q\205I\66eI\226dI\226d\321N\6\325\22\346\361DI\242M"
  "Y\222%Y\222%Y\264\223\1\326\21\346\361\324\241)K\262$K\262$\213v\62\0\327\14\346\361S"
  "\243L\324\242\234\33\0\330\20\346qFS\226DJ_\244$\213\246\234\6\331\21\346\361\304Y%K\262"
  "$K\262$\213\206\234\12\332\21\346\361\205Y%K\262$K\262$\213\206\234\12\333\23\346q\205I\224"
  "%Y\222%Y\222%Y\64\344T\0\334\22\346\361\324\221,\311\222,\311\222,\311\242!\247\2\335\17"
  "\346\361\205Y%K\262hH+CN\6\336\21\346\361\243\351\20eI\226dI\226\14QN\3\337\17"
  "\346\361\3Z\324%\213j\211\224$:\31\340\20\346q\305\71\64GC\222%Y\64\344T\0\341\20\346"
  "\361\205\71\66GC\222%Y\64\344T\0\342\20\346q\205I\16\315\321\220dI\26\15\71\25\343\21\346"
  "\361DI\242Cs\64$Y\222ECN\5\344\20\346\361\3I\16\315\321\220dI\26\15\71\25\345\20"
  "\346q\205I\30\316\321\220dI\26\15\71\25\346\15\346\361Ca\70$i\363\220S\1\347\15\346\361S"
  "\207$m\36\262\64\14\1\350\20\346q\305\71\64$Y\222%\203\22\17\71\25\351\20\346\361\205\71\66$"
  "Y\222%\203\22\17\71\25\352\20\346\361S\207$K\262dP\342!\254C\0\353\21\346\361\3I\16\15"
  "I\226d\311\240\304CN\5\354\13\346q\305\71\244v\325\311\0\355\13\346\361\205\71\246v\325\311\0\356"
  "\14\346q\205I\16\251]u\62\0\357\14\346\361\3I\16\251]u\62\0\360\21\346q$a%\234\262"
  "$K\262$\213v\62\0\361\21\346\361\205\71\64DY\222%Y\222%YN\5\362\20\346q\305\71\64"
  "eI\226dI\26\355d\0\363\20\346\361\205\71\66eI\226dI\26\355d\0\364\20\346q\205I\16"
  "MY\222%Y\222E;\31\365\21\346\361DI\242CS\226dI\226d\321N\6\366\20\346\361\3I"
  "\16MY\222%Y\222E;\31\367\13\346\361\223sh\320\241\234\31\370\17\346\361\223\242)RZ\244$"
  "\213\246\234\6\371\21\346q\305\71\222%Y\222%Y\222ECN\5\372\21\346\361\205\71\224%Y\222%"
  "Y\222ECN\5\373\22\346q\205I\216dI\226dI\226d\321\220S\1\374\22\346\361\3I\216d"
  "I\226dI\226d\321\220S\1\375\23\346\361\205\71\224%Y\222%Y\222ECZ\31\42\0\376\22\346"
  "q\247\351\20eI\226dI\226\14Q\232\203\0\377\23\346\361\3I\216dI\226dI\226d\321\220V"
  "\206\10\0\0\0\4\377\377\0";


static unsigned char sdcard[] PROGMEM = {
  0xf0, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xcf, 0xf3, 0x3f,
  0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c,
  0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c,
  0x38, 0x49, 0x92, 0x3c, 0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f,
  0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f,
  0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xfc, 0xff, 0xff, 0x3f, 0xfc, 0xc0, 0x80, 0x3f, 0x7c, 0xc0, 0x00, 0x3f,
  0x3c, 0xc0, 0x00, 0x3e, 0x1c, 0xfc, 0x38, 0x3e, 0x1e, 0xfc, 0x78, 0x3c,
  0x1f, 0xe0, 0x78, 0x3c, 0x3f, 0xc0, 0x78, 0x3c, 0x7f, 0x80, 0x78, 0x3c,
  0xff, 0x87, 0x78, 0x3c, 0xff, 0x87, 0x38, 0x3e, 0x3f, 0x80, 0x00, 0x3e,
  0x1f, 0xc0, 0x00, 0x3f, 0x3f, 0xe0, 0x80, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x1f,
  0xfc, 0xff, 0xff, 0x0f 
  };

#define notes_width 256
#define notes_height 46
static unsigned char notes[] PROGMEM = {
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x40, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x07, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x80, 0x05, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
  0x00, 0xf0, 0x01, 0xc0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x40,
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x40, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x5e, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x60, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x5e, 0x00, 0x80, 0x08, 0x00, 0x00, 0x07, 0x00, 0xe0, 0x01, 0x00,
  0xc0, 0x47, 0x00, 0x08, 0x00, 0x00, 0x07, 0x00, 0xe0, 0x01, 0x00, 0x00,
  0x40, 0x00, 0x18, 0x00, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x4e, 0x00, 0x00,
  0x19, 0x00, 0xf0, 0x07, 0x00, 0x20, 0x01, 0x00, 0xf8, 0x41, 0x00, 0x18,
  0x00, 0xf0, 0x07, 0x00, 0x20, 0x01, 0x00, 0x18, 0x40, 0x00, 0x78, 0x00,
  0x80, 0x00, 0xe0, 0x0f, 0x00, 0x47, 0x00, 0x00, 0x10, 0x00, 0xf0, 0x07,
  0x00, 0x20, 0x03, 0x00, 0x3c, 0x40, 0x00, 0x10, 0x00, 0xf0, 0x07, 0x00,
  0x20, 0x03, 0x00, 0x1c, 0x40, 0x00, 0xe8, 0x00, 0x80, 0x00, 0xe0, 0x0f,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x41, 0x00, 0x00,
  0x10, 0x00, 0x3f, 0x04, 0x00, 0xe0, 0x04, 0xe0, 0x11, 0x40, 0x00, 0x10,
  0x00, 0x3f, 0x04, 0x00, 0xe0, 0x04, 0xe0, 0x11, 0x40, 0x00, 0x38, 0x01,
  0x40, 0x00, 0x7e, 0x08, 0xc0, 0xe0, 0x0f, 0x00, 0x10, 0x00, 0x0f, 0x04,
  0x00, 0xb0, 0x01, 0x78, 0x10, 0x40, 0x00, 0x10, 0x00, 0x0f, 0x04, 0x00,
  0xb0, 0x01, 0x78, 0x10, 0x40, 0x00, 0x68, 0x01, 0x40, 0x00, 0x1e, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x70, 0x3e, 0x00,
  0x1c, 0x00, 0x01, 0x04, 0x00, 0x10, 0x00, 0x0e, 0x10, 0x40, 0x00, 0x18,
  0x00, 0x01, 0x04, 0x00, 0x10, 0x00, 0x0e, 0x10, 0x40, 0x00, 0xa8, 0x00,
  0x2f, 0x00, 0x02, 0x08, 0x40, 0x98, 0x78, 0x00, 0x1f, 0x00, 0x01, 0x04,
  0x00, 0x10, 0x00, 0x06, 0x10, 0x40, 0x00, 0x1f, 0x00, 0x01, 0x04, 0x00,
  0x10, 0x00, 0x06, 0x10, 0x40, 0x00, 0x48, 0xc0, 0x1f, 0x00, 0x02, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x88, 0x70, 0x80,
  0x0f, 0x00, 0x01, 0x04, 0x80, 0x17, 0x00, 0x02, 0x10, 0x7c, 0x80, 0x0f,
  0x00, 0x01, 0x04, 0x80, 0x17, 0x00, 0x02, 0x10, 0x7c, 0x00, 0x08, 0x80,
  0x0f, 0x00, 0x02, 0x08, 0x40, 0x88, 0x70, 0x00, 0x07, 0x00, 0x01, 0x04,
  0xc0, 0x1f, 0x00, 0x02, 0x10, 0x7e, 0x00, 0x07, 0x00, 0x01, 0x04, 0xc0,
  0x1f, 0x00, 0x02, 0x10, 0x7e, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x02, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x71, 0x00,
  0x00, 0x00, 0xc1, 0x07, 0xe0, 0x07, 0x00, 0x02, 0x1e, 0x00, 0x00, 0x00,
  0x00, 0xc1, 0x07, 0xe0, 0x07, 0x00, 0x02, 0x1e, 0x00, 0xc0, 0x07, 0x00,
  0x00, 0x00, 0x82, 0x0f, 0x80, 0x01, 0x39, 0x00, 0x00, 0x00, 0xe1, 0x07,
  0xc0, 0x03, 0x00, 0x02, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x07, 0xc0,
  0x03, 0x00, 0x02, 0x1f, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0xc2, 0x0f,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0x1e, 0x00,
  0x00, 0xf0, 0x81, 0x03, 0x00, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x00, 0x00,
  0xf0, 0x81, 0x03, 0x00, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x03, 0x07, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0xf8, 0x01, 0x00,
  0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00,
  0x00, 0xf0, 0x03, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


uint32_t reverse_bits(uint32_t inval, int bits)
{
if ( bits > 0 )
{
bits--;
return reverse_bits(inval >> 1, bits) | ((inval & 1) << bits);
}
return 0;
}

// Deklaracja obiektu JSON
//StaticJsonDocument<1024> doc;     // Przyjęto rozmiar JSON na 1024 bajty

enum MenuOption 
{
  PLAY_FILES,      // Odtwarzacz plików
  INTERNET_RADIO,  // Radio internetowe
};
MenuOption currentOption = INTERNET_RADIO;  // Aktualnie wybrana opcja menu (domyślnie radio internetowe)

// Funkcja sprawdza, czy plik jest plikiem audio na podstawie jego rozszerzenia
bool isAudioFile(const char *filename) {
  // Dodaj więcej rozszerzeń plików audio, jeśli to konieczne
  //  return (strstr(filename, ".mp3") || strstr(filename, ".MP3") || strstr(filename, ".wav") || strstr(filename, ".WAV") || strstr(filename, ".flac") || strstr(filename, ".FLAC"));
  //}

  // Znajdź ostatni wystąpienie kropki w nazwie pliku
  const char *ext = strrchr(filename, '.');

  // Jeśli nie znaleziono kropki lub nie ma rozszerzenia, zwróć false
  if (!ext) {
    return false;
  }

  // Sprawdź rozszerzenie, ignorując wielkość liter
  return (strcasecmp(ext, ".mp3") == 0 || strcasecmp(ext, ".wav") == 0 || strcasecmp(ext, ".flac") == 0);
}

/*===============    Definicja portu i deklaracje zmiennych do obsługi odbiornika IR    =============*/
int recv_pin = 15;  // Pin odbiornika IR

//volatile unsigned long pulse_start = 0;  // Czas rozpoczęcia impulsu
//volatile unsigned long pulse_end = 0;    // Czas zakończenia impulsu
//volatile bool pulse_ready = false;       // Flaga informująca, że impuls jest gotowy

volatile bool pulse_ready = false;  // Flaga sygnału gotowości
unsigned long pulse_start = 0;      // Czas początkowy impulsu
unsigned long pulse_end = 0;        // Czas końcowy impulsu
unsigned long pulse_duration = 0;   // Czas trwania impulsu
unsigned long pulse_duration_9ms = 0;   // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_4_5ms = 0; // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_690us = 0; // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_1690us = 0; // Tylko do analizy - Czas trwania impulsu

volatile bool pulse_ready9ms = false;  // Flaga sygnału gotowości
volatile bool pulse_ready_low = false;  // Flaga sygnału gotowości
unsigned long pulse_start_low = 0;      // Czas początkowy impulsu
unsigned long pulse_end_low = 0;        // Czas końcowy impulsu
unsigned long pulse_duration_low = 0;   // Czas trwania impulsu


unsigned long ir_code = 0;  // Zmienna do przechowywania kodu IR
int bit_count = 0;          // Licznik bitów w odebranym kodzie

// Próg dla sygnałów czasowych
//const int LEAD_HIGH = 8800;  // 9 ms sygnał wysoki (początkowy)
//const int LEAD_LOW = 4450;   // 4,5 ms sygnał niski (początkowy)
//const int HIGH_THRESHOLD = 1600;  // Sygnał "1"
//const int LOW_THRESHOLD = 560;    // Sygnał "0"

const int LEAD_HIGH = 9000;         // 9 ms sygnał wysoki (początkowy)
const int LEAD_LOW = 4600;          // 4,5 ms sygnał niski (początkowy)
const int TOLERANCE = 160;          // Tolerancja (w mikrosekundach)
const int HIGH_THRESHOLD = 1690;    // Sygnał "1"
const int LOW_THRESHOLD = 600;//560;      // Sygnał "0"


bool data_start_detected = false;  // Flaga dla sygnału wstępnego
//volatile static unsigned long last_pulse_time = 0;


// Funkcja do obsługi przycisków enkoderów, odpowiedzialna za debouncing i wykrywanie długiego naciśnięcia
void handleButtons() {
  static unsigned long buttonPressTime1 = 0;  // Zmienna do przechowywania czasu naciśnięcia przycisku
  static bool isButton1Pressed = false;       // Flaga do śledzenia, czy przycisk jest wciśnięty
  static bool action1Taken = false;           // Flaga do śledzenia, czy akcja została wykonana
                                              // static unsigned long lastPressTime = 0;    // Zmienna do kontrolowania debouncingu (ostatni czas naciśnięcia)

  static unsigned long buttonPressTime2 = 0;  // Zmienna do przechowywania czasu naciśnięcia przycisku enkodera 2
  static bool isButton2Pressed = false;       // Flaga do śledzenia, czy przycisk enkodera 2 jest wciśnięty
  static bool action2Taken = false;           // Flaga do śledzenia, czy akcja dla enkodera 2 została wykonana


  static unsigned long lastPressTime = 0;  // Zmienna do kontrolowania debouncingu (ostatni czas naciśnięcia)
  const unsigned long debounceDelay = 50;  // Opóźnienie debouncingu


  // ===== Obsługa przycisku enkodera 1 =====
  int reading1 = digitalRead(SW_PIN1);

  // Debouncing dla przycisku enkodera 1
  if (reading1 == LOW)  // Przycisk jest wciśnięty (stan niski)
  {
    if (millis() - lastPressTime > debounceDelay) {
      lastPressTime = millis();  // Aktualizujemy czas ostatniego naciśnięcia

      // Sprawdzamy, czy przycisk był wciśnięty przez 3 sekundy
      if (!isButton1Pressed) {
        buttonPressTime1 = millis();  // Ustawiamy czas naciśnięcia
        isButton1Pressed = true;      // Ustawiamy flagę, że przycisk jest wciśnięty
        action1Taken = false;         // Resetujemy flagę akcji dla enkodera 1
      }



      // Jeśli przycisk jest wciśnięty przez co najmniej 3 sekundy i akcja jeszcze nie była wykonana
      if (millis() - buttonPressTime1 >= buttonLongPressTime1 && !action1Taken) {
        timeDisplay = false;
        displayMenu();
        menuEnable = true;
        displayActive = true;
        displayStartTime = millis();

        Serial.println("Wyświetlenie menu po przytrzymaniu przycisku enkodera 1");

        // Ustawiamy flagę, że akcja została wykonana
        action1Taken = true;
      }
    }
  } else {
    isButton1Pressed = false;  // Resetujemy flagę naciśnięcia przycisku enkodera 1
    action1Taken = false;      // Resetujemy flagę akcji dla enkodera 1
  }

  // ===== Obsługa przycisku enkodera 2 =====
  int reading2 = digitalRead(SW_PIN2);

  // Debouncing dla przycisku enkodera 2

  if (reading2 == LOW)  // Przycisk jest wciśnięty (stan niski)
  {
    if (millis() - lastPressTime > debounceDelay) {
      //encoderButton2 = true;  // Ustawiamy flagę, że przycisk został wciśnięty
      lastPressTime = millis();  // Aktualizujemy czas ostatniego naciśnięcia

      // Sprawdzamy, czy przycisk był wciśnięty przez 3 sekundy
      if (!isButton2Pressed) {
        buttonPressTime2 = millis();  // Ustawiamy czas naciśnięcia
        isButton2Pressed = true;      // Ustawiamy flagę, że przycisk jest wciśnięty
        action2Taken = false;         // Resetujemy flagę akcji dla enkodera 2
        action3Taken = false;         // Resetujmy flage akcji Super długiego wcisniecia enkodera 2
        volumeSet = false;
      }

      /*if ((millis() - buttonPressTime2 >= buttonShortPressTime2) && (millis() - buttonPressTime2 < buttonSuperLongPressTime2) &&(millis() - buttonPressTime2 < buttonLongPressTime2))
      {
        volumeSet = true;
        timeDisplay = false;
        displayStartTime = millis();
        Serial.println("debug--krotkie nacisniecie enkodera 2");
      }
      */

      if (millis() - buttonPressTime2 >= buttonLongPressTime2 && millis() - buttonPressTime2 >= buttonSuperLongPressTime2 && action3Taken == false) {
        displayActive = true;
        displayStartTime = millis();
        volumeSet = false;
        timeDisplay = false;
        bankMenuEnable = false;
        //vuMeterOn = !vuMeterOn;
        vuMeterMode = !vuMeterMode;
        audio.setTone(6, -4, 6);
        //u8g2.clearBuffer();
        //u8g2.setCursor(0,13);
        //u8g2.print("VUmeters mode:");
        //u8g2.print(vuMeterOn);
        Serial.print("Wartość flagi VUmeter mode:");
        Serial.println(vuMeterMode);
        action3Taken = true;
      }


      // Jeśli przycisk jest wciśnięty przez co najmniej 3 sekundy i akcja jeszcze nie była wykonana
      if (millis() - buttonPressTime2 >= buttonLongPressTime2 && !action2Taken && millis() - buttonPressTime2 < buttonSuperLongPressTime2) {
        Serial.println("debug--Bank Menu");
        volumeSet = false;
        previous_bank_nr = bank_nr;  // jesli weszlimy do menu "wybór banku" to zapisujemy obecny bank zanim zaczniemy krecic enkoderem
        bankMenuEnable = true;
        timeDisplay = false;
        bankChange = true;
        //currentOption = BANK_LIST;  // Ustawienie listy banków do przewijania i wyboru
        String bankNrStr = String(bank_nr);
        Serial.println("Wyświetlenie listy banków");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_fub14_tf);
        u8g2.drawStr(80, 33, "BANK:");
        u8g2.drawStr(145, 33, String(bank_nr).c_str());  // numer banku

        u8g2.drawRFrame(21, 42, 214, 14, 3);                // Ramka do slidera bankow
        u8g2.drawRBox((bank_nr * 13) + 10, 44, 15, 10, 2);  // wypełnienie slidera
        u8g2.sendBuffer();
        x = 0;

        // Ustawiamy flagę akcji, aby wykonała się tylko raz
        action2Taken = true;
      }
    }
  } else {
    //encoderButton2 = false;  // Przywracamy stan przycisku
    isButton2Pressed = false;  // Resetujemy flagę naciśnięcia przycisku enkodera 2
    action2Taken = false;      // Resetujemy flagę akcji dla enkodera 2
    action3Taken = false;
  }
}


// Funkcja do pobierania danych z API z serwera pogody openweathermap.org
/*
void getWeatherData()
{
  HTTPClient http;  // Utworzenie obiektu HTTPClient
  
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Piła,pl&appid=your_own_API_key";  // URL z danymi do API, na końcu musi być Twój unikalny klucz API otrzymany po resetracji w serwisie openweathermap.org

  http.begin(url);  // Inicjalizacja połączenia HTTP z podanym URL-em, otwieramy połączenie z serwerem.

  int httpCode = http.GET();  // Wysłanie żądanie GET do serwera, aby pobrać dane pogodowe

  if (httpCode == HTTP_CODE_OK)  // Sprawdzenie, czy odpowiedź z serwera była prawidłowa (kod 200 OK)
  {
    String payload = http.getString();  // Pobranie odpowiedzi z serwera w postaci ciągu znaków (JSON)
    Serial.println("Odpowiedź JSON z API:");
    Serial.println(payload); 

    DeserializationError error = deserializeJson(doc, payload);  // Deserializujemy dane JSON do obiektu dokumentu
    if (error)  // Sprawdzamy, czy deserializacja JSON zakończyła się niepowodzeniem
    {
      Serial.print(F("deserializeJson() failed: "));  // Jeśli jest błąd, drukujemy komunikat o błędzie
      Serial.println(error.f_str());  // Wydruk szczegółów błędu deserializacji
      return;  // Zakończenie funkcji w przypadku błędu
    }
    if (timeDisplay == true)
    {
     updateWeather();  // Jeśli deserializacja zakończyła się sukcesem, wywołujemy funkcję `updateWeather`, aby zaktualizować wyświetlacz i serial terminal
    }
  }
  else  // Jeśli połączenie z serwerem nie powiodło się
  {
    Serial.println("Błąd połączenia z serwerem.");
    //u8g2.drawStr(0, 62, "                                           ");
    //u8g2.drawStr(0, 62, "Brak polaczenia z serwerem pogody");
    u8g2.sendBuffer();  
  }

  http.end();  // Zakończenie połączenia HTTP, zamykamy zasoby
}

// Funkcja do aktualizacji danych pogodowych
void updateWeather()
{
  //u8g2.drawStr(0, 62, "                                           "); // Wypełnienie spacjami jako czyszczenie linii

  JsonObject root = doc.as<JsonObject>();  // Konwertuje dokument JSON do obiektu typu JsonObject

  JsonObject main = root["main"];  // Pobiera obiekt "main" zawierający dane główne, takie jak temperatura, wilgotność, ciśnienie
  JsonObject weather = root["weather"][0];  // Pobiera pierwszy element z tablicy "weather", który zawiera dane o pogodzie
  JsonObject wind = root["wind"];  // Pobiera obiekt "wind" zawierający dane o wietrze

  unsigned long timestamp = root["dt"];  // Pobiera timestamp (czas w sekundach) z JSON
  String formattedDate = convertTimestampToDate(timestamp);  // Konwertuje timestamp na sformatowaną datę i godzinę

  float temp = main["temp"].as<float>() - 273.15;  // Pobiera temperaturę w Kelvinach i konwertuje ją na °C
  float feels_like = main["feels_like"].as<float>() - 273.15;  // Pobiera odczuwalną temperaturę i konwertuje ją na °C

  int humidity = main["humidity"];  // Pobiera wilgotność powietrza
  String weatherDescription = weather["description"].as<String>();  // Pobiera opis pogody (np. "light rain")
  String icon = weather["icon"].as<String>();  // Pobiera kod ikony pogody (np. "10d" dla deszczu)
  float windSpeed = wind["speed"];  // Pobiera prędkość wiatru w m/s
  float windGust = wind["gust"];  // Pobiera prędkość podmuchów wiatru w m/s
  float pressure = main["pressure"].as<float>();  // Pobiera ciśnienie powietrza w hPa

  Serial.println("Dane z JSON:");
  Serial.print("Data: ");
  Serial.println(formattedDate);
  Serial.print("Temperatura: ");
  Serial.print(temp, 2);
  Serial.println(" °C");
  tempStr = "Temperatura: " + String(temp, 2) + " C";
  
  Serial.print("Odczuwalna temperatura: ");
  Serial.print(feels_like, 2);
  Serial.println(" °C");
  feels_likeStr = "Odczuwalna: " + String(feels_like, 2) + " C";
  
  Serial.print("Wilgotność: ");
  Serial.print(humidity);
  Serial.println(" %");
  humidityStr = "Wilgotnosc: " + String(humidity) + " %";
  
  Serial.print("Ciśnienie: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  pressureStr = "Cisnienie: " + String(pressure, 2) + " hPa";
  
  Serial.print("Opis pogody: ");
  Serial.println(weatherDescription);
  Serial.print("Ikona: ");
  Serial.println(icon);
  
  Serial.print("Prędkość wiatru: ");
  Serial.print(windSpeed, 2);
  Serial.println(" m/s");
  windStr = "Wiatr: " + String(windSpeed) + " m/s";
  
  Serial.print("Podmuchy wiatru: ");
  Serial.print(windGust, 2);
  Serial.println(" m/s");
  windGustStr = "W podmuchach: " + String(windGust) + " m/s";
}

// Funkcja do przełączania między różnymi danymi pogodowymi, które są wyświetlane na ekranie
void switchWeatherData()
{
  //u8g2.setFont(u8g2_font_spleen6x12_mr);
  u8g2.setFont(spleen6x12PL);
  if (timeDisplay == true)
  {
  if (cycle == 0)
    {
      //u8g2.drawStr(0, 62, "                                           ");
      //u8g2.drawStr(0, 62, tempStr.c_str());
      //u8g2.drawStr(130, 62, feels_likeStr.c_str()); 
    } 
    else if (cycle == 1)
    {
      //u8g2.drawStr(0, 62, "                                           ");
      //u8g2.drawStr(0, 62, windStr.c_str());
      //u8g2.drawStr(110, 62, windGustStr.c_str());
    } 
    else if (cycle == 2)
    {
      //u8g2.drawStr(0, 62, "                                           ");
      //u8g2.drawStr(0, 62, humidityStr.c_str());
      //u8g2.drawStr(115, 62, pressureStr.c_str());
    }

  //  u8g2.sendBuffer();
  }
  // Zmiana cyklu: przechodzimy do następnego zestawu danych
  cycle++;
  if (cycle > 2) 
  {
    cycle = 0;  // Wracamy do cyklu 0 po trzecim cyklu
  }
}
*/

// Funkcja konwertująca timestamp na datę i godzinę w formacie "YYYY-MM-DD HH:MM:SS"
/*
String convertTimestampToDate(unsigned long timestamp)  
{
  int year, month, day, hour, minute, second;  // Deklaracja zmiennych dla roku, miesiąca, dnia, godziny, minuty i sekundy z pogodynki
  time_t rawTime = timestamp;                  // Konwersja timestamp na typ time_t, który jest wymagany przez funkcję localtime()
  struct tm* timeInfo;                         // Wskaźnik na strukturę tm, która zawiera informacje o czasie
  timeInfo = localtime(&rawTime);              // Konwertowanie rawTime na strukturę tm zawierającą szczegóły daty i godziny

  year = timeInfo->tm_year + 1900;             // Rok jest liczony od 1900 roku, więc musimy dodać 1900
  month = timeInfo->tm_mon + 1;                // Miesiąc jest indeksowany od 0, więc dodajemy 1
  day = timeInfo->tm_mday;                     // Dzień miesiąca
  hour = timeInfo->tm_hour;                    // Godzina (0-23)
  minute = timeInfo->tm_min;                   // Minuta (0-59)
  second = timeInfo->tm_sec;                   // Sekunda (0-59)

  // Formatowanie na dwie cyfry (dodawanie zer na początku, jeśli liczba jest mniejsza niż 10)
  String strMonth = (month < 10) ? "0" + String(month) : String(month);            // Dodaje zero przed miesiącem, jeśli miesiąc jest mniejszy niż 10
  String strDay = (day < 10) ? "0" + String(day) : String(day);                    // Dodaje zero przed dniem, jeśli dzień jest mniejszy niż 10
  String strHour = (hour < 10) ? "0" + String(hour) : String(hour);                // Dodaje zero przed godziną, jeśli godzina jest mniejsza niż 10
  String strMinute = (minute < 10) ? "0" + String(minute) : String(minute);        // Dodaje zero przed minutą, jeśli minuta jest mniejsza niż 10
  String strSecond = (second < 10) ? "0" + String(second) : String(second);        // Dodaje zero przed sekundą, jeśli sekunda jest mniejsza niż 10

  // Tworzenie sformatowanej daty w formacie "YYYY-MM-DD HH:MM:SS"
  String date = String(year) + "-" + strMonth + "-" + strDay + " " + strHour + ":" + strMinute + ":" + strSecond;
                
  return date;  // Zwraca sformatowaną datę jako String
}
*/

//Funkcja odpowiedzialna za zapisywanie informacji o stacji do pamięci EEPROM.
void saveStationToEEPROM(const char *station) {
  // Sprawdź, czy istnieje jeszcze miejsce na kolejną stację w pamięci EEPROM.
  if (stationsCount < MAX_STATIONS) {
    int length = strlen(station);

    // Sprawdź, czy długość linku nie przekracza ustalonego maksimum.
    if (length <= STATION_NAME_LENGTH) {
      // Zapisz długość linku jako pierwszy bajt.
      psramData[stationsCount * (STATION_NAME_LENGTH + 1)] = length;
      // Zapisz link jako kolejne bajty w pamięci EEPROM.
      for (int i = 0; i < length; i++) 
      {
        psramData[stationsCount * (STATION_NAME_LENGTH + 1) + 1 + i] = station[i];
      }

      // Potwierdź zapis do pamięci EEPROM.
      //EEPROM.commit();

      // Wydrukuj informację o zapisanej stacji na Serialu.
      Serial.println(String(stationsCount + 1) + "   " + String(station));  // Drukowanie na serialu od nr 1 jak w banku na serwerze

      // Zwiększ licznik zapisanych stacji.
      stationsCount++;

      u8g2.setFont(spleen6x12PL);  // progress bar pobieranych stacji
      u8g2.drawStr(21, 36, "Progress:");
      u8g2.drawStr(75, 36, String(stationsCount).c_str());  // Napisz licznik pobranych stacji

      u8g2.drawRFrame(21, 42, 212, 12, 3);  // Ramka paska postępu ladowania stacji stacji w>8 h>8
      x = (stationsCount * 2) + 8;          // Dodajemy gdy stationCount=1 + 8 aby utrzymac warunek dla zaokrąglonego drawRBox - szerokość W>6 h>6 ma byc W>=2*(r+1), h >= 2*(r+1)
      u8g2.drawRBox(23, 44, x, 8, 2);       // Pasek postepu ladowania stacji z serwera lub karty SD
      u8g2.sendBuffer();
    } else {
      // Informacja o błędzie w przypadku zbyt długiego linku do stacji.
      Serial.println("Błąd: Link do stacji jest zbyt długi");
    }
  } else {
    // Informacja o błędzie w przypadku osiągnięcia maksymalnej liczby stacji.
    Serial.println("Błąd: Osiągnięto maksymalną liczbę zapisanych stacji");
  }
}

// Funkcja odpowiedzialna za zmianę aktualnie wybranej stacji radiowej.
void changeStation() {
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(34, 33, "Loading stream..."); // 8 znakow  x 11 szer
  u8g2.sendBuffer();

  mp3 = flac = aac = false;
  stationFromBuffer = station_nr;
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString

  // Tworzymy nazwę pliku banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzamy, czy plik istnieje
  if (!SD.exists(fileName)) {
    Serial.println("Błąd: Plik banku nie istnieje.");
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File bankFile = SD.open(fileName, FILE_READ);
  if (!bankFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku banku.");
    return;
  }

  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String stationUrl = "";
  while (bankFile.available()) {
    String line = bankFile.readStringUntil('\n');
    currentLine++;

    if (currentLine == station_nr) {
      // Wyciągnij pierwsze 42 znaki i przypisz do stationName
      stationName = line.substring(0, 41);  //42 Skopiuj pierwsze 42 znaki z linii
      Serial.print("Nazwa stacji: ");
      Serial.println(stationName);

      // Znajdź część URL w linii, np. po numerze stacji
      int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
      if (urlStart != -1) {
        stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
        stationUrl.trim();                      // Usuwamy białe znaki na początku i końcu
      }
      break;
    }
  }
  bankFile.close();  // Zamykamy plik po odczycie
  // Sprawdzamy, czy znaleziono stację
  if (stationUrl.isEmpty()) {
    Serial.println("Błąd: Nie znaleziono stacji dla podanego numeru.");
    return;
  }

  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  if (stationUrl.startsWith("http://") || stationUrl.startsWith("https://")) {
    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Aktualnie wybrana stacja: ");
    Serial.println(station_nr);
    Serial.print("Link do stacji: ");
    Serial.println(stationUrl);
    
    u8g2.setFont(spleen6x12PL);  // wypisujemy jaki stream jakie stacji jest ładowany
    u8g2.drawStr(34, 55, String(stationName.substring(0, 23)).c_str());
    u8g2.sendBuffer();
    
    // Połącz z daną stacją
    audio.connecttohost(stationUrl.c_str());
    //seconds = 0;
    stationFromBuffer = station_nr;
    bankFromBuffer = bank_nr;
    saveStationOnSD();
  } else {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
    Serial.println("Odczytany URL: " + stationUrl);
  }
  currentSelection = station_nr - 1; // ustawiamy stacje na liscie na obecnie odtwarzaczną po zmianie stacji
  firstVisibleLine = currentSelection + 1; // pierwsza widoczna lina to grająca stacja przy starcie
  if (currentSelection + 1 >= stationsCount - 1) 
  {
   firstVisibleLine = currentSelection - 3;
  }
}

// Jesli dany bank istnieje juz na karcie SD to odczytujemy tylko dany Bank z karty
void readSDStations() {
  stationsCount = 0;
  Serial.println("Plik Banu isnieje na karcie SD. Czytamy TYLKO z karty");
  mp3 = flac = aac = false;
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString

  // Tworzymy nazwę pliku banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzamy, czy plik istnieje
  if (!SD.exists(fileName)) {
    Serial.println("Błąd: Plik banku nie istnieje.");
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File bankFile = SD.open(fileName, FILE_READ);
  if (!bankFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku banku.");
    return;
  }

  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String stationUrl = "";

  while (bankFile.available())  // & currentLine <= MAX_STATIONS)
  {
    //if (currentLine < MAX_STATIONS)
    //{
    String line = bankFile.readStringUntil('\n');
    currentLine++;

    //currentLine == station_nr
    stationName = line.substring(0, 42);
    int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
    if (urlStart != -1) {
      stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
      stationUrl.trim();                      // Usuwamy białe znaki na początku i końcu
      ////Serial.print(" URL stacji:");
      ///Serial.println(stationUrl);
      //String station = currentLine + "   " + stationName + "  " + stationUrl;
      String station = stationName + "  " + stationUrl;
      sanitizeAndSaveStation(station.c_str());  // przepisanie stacji do EEPROMu  (RAMU)
    }
    //}
  }
  Serial.print("Zamykamy plik bankFile na wartosci currentLine:");
  Serial.println(currentLine);
  bankFile.close();  // Zamykamy plik po odczycie
}

// Funkcja do pobierania listy stacji radiowych z serwera
void fetchStationsFromServer() 
{
  u8g2.setFont(spleen6x12PL);
  u8g2.clearBuffer();
  u8g2.setCursor(15,23);
  //u8g2.print("Bank " + String(bank_nr) + " loading station from:");
  u8g2.drawStr(21, 10, "Bank:");
  u8g2.drawStr(51, 10, String(bank_nr).c_str());
  u8g2.drawStr(21, 23, "Loading station from:");
  u8g2.sendBuffer();
  currentSelection = 0;
  firstVisibleLine = 0;
  station_nr = 1;


  // Utwórz obiekt klienta HTTP
  HTTPClient http;

  // URL stacji dla danego banku
  String url;

  // Wybierz URL na podstawie bank_nr za pomocą switch
  switch (bank_nr) {
    case 1:
      url = STATIONS_URL;
      break;
    case 2:
      url = STATIONS_URL1;
      break;
    case 3:
      url = STATIONS_URL2;
      break;
    case 4:
      url = STATIONS_URL3;
      break;
    case 5:
      url = STATIONS_URL4;
      break;
    case 6:
      url = STATIONS_URL5;
      break;
    case 7:
      url = STATIONS_URL6;
      break;
    case 8:
      url = STATIONS_URL7;
      break;
    case 9:
      url = STATIONS_URL8;
      break;
    case 10:
      url = STATIONS_URL9;
      break;
    case 11:
      url = STATIONS_URL10;
      break;
    case 12:
      url = STATIONS_URL11;
      break;
    case 13:
      url = STATIONS_URL12;
      break;
    case 14:
      url = STATIONS_URL13;
      break;
    case 15:
      url = STATIONS_URL14;
      break;
    case 16:
      url = STATIONS_URL15;
      break;
    default:
      Serial.println("Nieprawidłowy numer banku");
      return;
  }

  // Tworzenie nazwy pliku dla danego banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzenie, czy plik istnieje
  if (SD.exists(fileName) && bankNetworkUpdate == false) 
  {
    Serial.println("Plik banku " + fileName + " już istnieje.");
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(147, 23, "SD card");
    //u8g2.print("SD card");
    u8g2.sendBuffer();
    readSDStations();  // Jesli plik istnieje to odczytujemy go tylko z karty
  } 
  else
  //if (bankNetworkUpdate = true)
  {
    // stworz plik na karcie tylko jesli on nie istnieje GR
    u8g2.drawStr(147, 23, "GitHub server");
    //u8g2.print("GitHub server");
    u8g2.sendBuffer();
    {
      // Próba utworzenia pliku, jeśli nie istnieje
      File bankFile = SD.open(fileName, FILE_WRITE);

      if (bankFile) {
        Serial.println("Utworzono plik banku: " + fileName);
        bankFile.close();  // Zamykanie pliku po utworzeniu
      } else {
        Serial.println("Błąd: Nie można utworzyć pliku banku: " + fileName);
        return;  // Przerwij dalsze działanie, jeśli nie udało się utworzyć pliku
      }
    }
    // Inicjalizuj żądanie HTTP do podanego adresu URL
    http.begin(url);

    // Wykonaj żądanie GET i zapisz kod odpowiedzi HTTP
    int httpCode = http.GET();

    // Wydrukuj dodatkowe informacje diagnostyczne
    Serial.print("Kod odpowiedzi HTTP: ");
    Serial.println(httpCode);

    // Sprawdź, czy żądanie było udane (HTTP_CODE_OK)
    if (httpCode == HTTP_CODE_OK) {
      // Pobierz zawartość odpowiedzi HTTP w postaci tekstu
      String payload = http.getString();
      //Serial.println("Stacje pobrane z serwera:");
      //Serial.println(payload);  // Wyświetlenie pobranych danych (payload)
      // Otwórz plik w trybie zapisu, aby zapisać payload
      File bankFile = SD.open(fileName, FILE_WRITE);
      if (bankFile) {
        bankFile.println(payload);  // Zapisz dane do pliku
        bankFile.close();           // Zamknij plik po zapisaniu
        Serial.println("Dane zapisane do pliku: " + fileName);
      } else {
        Serial.println("Błąd: Nie można otworzyć pliku do zapisu: " + fileName);
      }
      // Zapisz każdą niepustą stację do pamięci EEPROM z indeksem
      int startIndex = 0;
      int endIndex;
      stationsCount = 0;
      // Przeszukuj otrzymaną zawartość w poszukiwaniu nowych linii
      while ((endIndex = payload.indexOf('\n', startIndex)) != -1 && stationsCount < MAX_STATIONS) {
        // Wyodrębnij pojedynczą stację z otrzymanego tekstu
        String station = payload.substring(startIndex, endIndex);

        // Sprawdź, czy stacja nie jest pusta, a następnie przetwórz i zapisz
        if (!station.isEmpty()) {
          // Zapisz stację do pliku na karcie SD
          sanitizeAndSaveStation(station.c_str());
        }
        // Przesuń indeks początkowy do kolejnej linii
        startIndex = endIndex + 1;
      }
    } else {
      // W przypadku nieudanego żądania wydrukuj informację o błędzie z kodem HTTP
      Serial.printf("Błąd podczas pobierania stacji. Kod HTTP: %d\n", httpCode);
    }
    // Zakończ połączenie HTTP
    http.end();
  }
}

// Funkcja przetwarza i zapisuje stację do pamięci EEPROM
void sanitizeAndSaveStation(const char *station) {
  // Bufor na przetworzoną stację - o jeden znak dłuższy niż maksymalna długość linku
  char sanitizedStation[STATION_NAME_LENGTH + 1];

  // Indeks pomocniczy dla przetwarzania
  int j = 0;

  // Przeglądaj każdy znak stacji i sprawdź czy jest to drukowalny znak ASCII
  for (int i = 0; i < STATION_NAME_LENGTH && station[i] != '\0'; i++) {
    // Sprawdź, czy znak jest drukowalnym znakiem ASCII
    if (isprint(station[i])) {
      // Jeśli tak, dodaj do przetworzonej stacji
      sanitizedStation[j++] = station[i];
    }
  }

  // Dodaj znak końca ciągu do przetworzonej stacji
  sanitizedStation[j] = '\0';

  // Zapisz przetworzoną stację do pamięci EEPROM
  saveStationToEEPROM(sanitizedStation);
}



void audio_info(const char *info) {
  // Wyświetl informacje w konsoli szeregowej
  Serial.print("info        ");
  Serial.println(info);
  // Znajdź pozycję "BitRate:" w tekście
  int bitrateIndex = String(info).indexOf("BitRate:");
  bitratePresent = false;
  if (bitrateIndex != -1) {
    // Przytnij tekst od pozycji "BitRate:" do końca linii
    bitrateString = String(info).substring(bitrateIndex + 8, String(info).indexOf('\n', bitrateIndex));
    bitrateStringInt = bitrateString.toInt();  // przliczenie bps na Kbps
    bitrateStringInt = bitrateStringInt / 1000;
    bitrateString = String(bitrateStringInt);
    bitratePresent = true;

    if (currentOption == PLAY_FILES) {
      displayPlayer();
    }
    if (currentOption == INTERNET_RADIO) {
      displayRadio();
    }
  }

  // Znajdź pozycję "SampleRate:" w tekście
  int sampleRateIndex = String(info).indexOf("SampleRate:");
  if (sampleRateIndex != -1) {
    // Przytnij tekst od pozycji "SampleRate:" do końca linii
    sampleRateString = String(info).substring(sampleRateIndex + 11, String(info).indexOf('\n', sampleRateIndex));
  }

  // Znajdź pozycję "BitsPerSample:" w tekście
  int bitsPerSampleIndex = String(info).indexOf("BitsPerSample:");
  if (bitsPerSampleIndex != -1) {
    // Przytnij tekst od pozycji "BitsPerSample:" do końca linii
    bitsPerSampleString = String(info).substring(bitsPerSampleIndex + 15, String(info).indexOf('\n', bitsPerSampleIndex));
  }

  // Znajdź pozycję "skip metadata" w tekście
  int metadata = String(info).indexOf("skip metadata");
  if (metadata != -1) {
    Serial.println("Brak ID3 - nazwa pliku: " + fileNameString);
    if (fileNameString.length() > 84) {
      fileNameString = String(fileNameString).substring(0, 84);  // Przytnij string do 84 znaków, aby zmieścić w 2 liniach z dalszym podziałem na pełne wyrazy
    }
  }

  if (String(info).indexOf("MP3Decoder") != -1) {
    mp3 = true;
    flac = false;
    aac = false;
  }

  if (String(info).indexOf("FLACDecoder") != -1) {
    flac = true;
    mp3 = false;
    aac = false;
  }

  if (String(info).indexOf("AACDecoder") != -1) {
    aac = true;
    flac = false;
    mp3 = false;
  }
}

void audio_id3data(const char *info) {
  Serial.print("id3data     ");
  Serial.println(info);

  // Znajdź pozycję w tekście
  int artistIndex1 = String(info).indexOf("Artist: ");
  int artistIndex2 = String(info).indexOf("ARTIST=");

  if (artistIndex1 != -1) {
    // Przytnij tekst od pozycji "Artist:" do końca linii
    artistString = String(info).substring(artistIndex1 + 8, String(info).indexOf('\n', artistIndex1));
    Serial.println("Znalazłem artystę: " + artistString);
    id3tag = true;
  }
  if (artistIndex2 != -1) {
    // Przytnij tekst od pozycji "ARTIST=" do końca linii
    artistString = String(info).substring(artistIndex2 + 7, String(info).indexOf('\n', artistIndex2));
    Serial.println("Znalazłem artystę: " + artistString);
    id3tag = true;
  }

  // Znajdź pozycję w tekście
  int titleIndex1 = String(info).indexOf("Title: ");
  int titleIndex2 = String(info).indexOf("TITLE=");

  if (titleIndex1 != -1) {
    // Przytnij tekst od pozycji "Title: " do końca linii
    titleString = String(info).substring(titleIndex1 + 7, String(info).indexOf('\n', titleIndex1));
    Serial.println("Znalazłem tytuł: " + titleString);
    id3tag = true;
  }
  if (titleIndex2 != -1) {
    // Przytnij tekst od pozycji "TITLE=" do końca linii
    titleString = String(info).substring(titleIndex2 + 6, String(info).indexOf('\n', titleIndex2));
    Serial.println("Znalazłem tytuł: " + titleString);
    id3tag = true;
  }
}

void audio_bitrate(const char *info) {
  Serial.print("bitrate     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info) {
  fileEnd = true;
  Serial.print("eof_mp3     ");
  Serial.println(info);
}

void audio_showstation(const char *info) {
  Serial.print("station     ");
  Serial.println(info);
  //  StationNameStream = info;
}

void audio_showstreamtitle(const char *info) {
 // u8g2.setFont(spleen6x12PL);
  //u8g2.setFont(u8g2_font_6x12_mf);
 // u8g2.drawStr(0, 27, "                                           ");
 // u8g2.drawStr(0, 39, "                                           ");
 // u8g2.drawStr(0, 51, "                                           ");

  Serial.print("streamtitle ");
  Serial.println(info);
  stationString = String(info);
  if (currentOption == INTERNET_RADIO) {

    ActionNeedUpdateTime = true;
    displayRadio();
  }
}

void audio_commercial(const char *info) {
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info) {
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info) {
  Serial.print("lasthost    ");
  Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  ");
  Serial.println(info);
}

void displayMenu() {
  timeDisplay = false;
  menuEnable = true;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_spleen8x16_mr);
  u8g2.drawStr(65, 20, "MENU");

  switch (currentOption) {
    case PLAY_FILES:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_spleen8x16_mr);
      u8g2.drawStr(65, 20, "MENU");

      u8g2.setDrawColor(1);          // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
      u8g2.drawBox(0, 27, 112, 15);  // Narysuj prostokąt jako tło dla zaznaczonej stacji (x=0, szerokość 256, wysokość 10)
      u8g2.setDrawColor(0);          // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
      u8g2.drawStr(0, 40, " MUSIC PLAYER ");
      u8g2.setDrawColor(1);
      u8g2.drawStr(0, 60, " Net radio    ");
      break;
    case INTERNET_RADIO:
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_spleen8x16_mr);
      u8g2.drawStr(65, 20, "MENU");

      u8g2.setDrawColor(1);  // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
      u8g2.drawStr(0, 40, " Music player ");
      //u8g2.setDrawColor(1);
      u8g2.drawBox(0, 47, 112, 15);  // Narysuj prostokąt jako tło dla zaznaczonej stacji (x=0, szerokość 256, wysokość 10)
      u8g2.setDrawColor(0);          // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
      u8g2.drawStr(0, 60, " NET RADIO    ");
      u8g2.setDrawColor(1);
      break;
  }
  u8g2.sendBuffer();
}

void printDirectoriesAndSavePaths(File dir, int numTabs, String currentPath) {
  directoryCount = 0;
  while (true) {
    // Otwórz kolejny plik w katalogu
    File entry = dir.openNextFile();
    // Jeżeli nie ma więcej plików, przerwij pętlę
    if (!entry) {
      break;
    }

    // Sprawdź, czy to katalog
    if (entry.isDirectory()) {
      // Utwórz pełną ścieżkę do bieżącego katalogu
      String path = currentPath + "/" + entry.name();
      Serial.print("String path:");
      Serial.println(path);
      // Zapisz pełną ścieżkę do tablicy
      directories[directoryCount] = path;

      // Wydrukuj numer indeksu i pełną ścieżkę
      Serial.print(directoryCount);
      Serial.print(": ");
      Serial.println(path.substring(1));

      // Zwiększ licznik katalogów
      directoryCount++;

      // Jeżeli to nie katalog System Volume Information, wydrukuj na ekranie OLED
      if (path != "/System Volume Information") {
        for (int i = 1; i < 7; i++) {
          // Przygotuj pełną ścieżkę dla wyświetlenia
          String fullPath = directories[i];

          // Ogranicz długość do 21 znaków
          fullPath = fullPath.substring(1, 42);
        }
      }
    }
    // Zamknij plik
    entry.close();
  }
}

// Funkcja do wylistowania katalogów z karty
void listDirectories(const char *dirname) {
  File root = SD.open(dirname);
  if (!root) {
    Serial.println("1-Błąd otwarcia katalogu!");
    Serial.print("debug--ER-dirname:");
    Serial.println(dirname);
    return;
  }
  Serial.print("debug--dirname:");
  Serial.println(dirname);

  printDirectoriesAndSavePaths(root, 0, "");  // Początkowo pełna ścieżka jest pusta
  Serial.println("Wylistowano katalogi z karty SD");
  root.close();
  scrollDown();
  displayFolders();
}

// Funkcja do przewijania w górę
void scrollUp() {
  if (currentSelection > 0) {
    currentSelection--;
    if (currentSelection < firstVisibleLine) {
      firstVisibleLine = currentSelection;
    }
  }
  // Dodaj dodatkowy wydruk do diagnostyki
  Serial.print("Scroll Up: CurrentSelection = ");
  Serial.println(currentSelection);
}

// Funkcja do przewijania w dół
void scrollDown() {
  if (currentSelection < maxSelection()) {
    currentSelection++;
    if (currentSelection >= firstVisibleLine + maxVisibleLines) {
      firstVisibleLine++;
    }
    // Dodaj dodatkowy wydruk do diagnostyki
    Serial.print("Scroll Down: CurrentSelection = ");
    Serial.println(currentSelection);
  }
}

int maxSelection() {
  if (currentOption == INTERNET_RADIO) {
    return stationsCount - 1;
  } else if (currentOption == PLAY_FILES) {
    return directoryCount - 1;
  }
  return 0;  // Zwraca 0, jeśli żaden warunek nie jest spełniony
}

// Funkcja do odtwarzania plików z wybranego folderu
void playFromSelectedFolder() {
  folderNameString = currentDirectory + directories[folderIndex];
  Serial.println("Odtwarzanie plików z wybranego folderu: " + folderNameString);

  // Otwórz folder
  File root = SD.open(folderNameString);
  PlayedFolderName = folderNameString;                                           // Aktulanie odtwarzaczny folder
  PlayedFolderName = PlayedFolderName.substring(currentDirectory.length() + 1);  // wycinamy z nazwy folderu informacje o katalogu gdzie trzymamy cała muzyke i wyswietlamy tylko docelowy katalog

  if (!root) {
    Serial.println("2-Błąd otwarcia katalogu!");
    Serial.print("debug--ER_FolderNameString: ");
    Serial.println(folderNameString);
    return;
  }
  Serial.print("debug--FolderNameString: ");
  Serial.println(folderNameString);


  totalFilesInFolder = 0;
  fileIndex = 1;  // Zaczynamy odtwarzanie od pierwszego pliku audio w folderze

  // Zliczanie plików audio w folderze
  while (File entry = root.openNextFile()) {
    String fileName = entry.name();
    Serial.print("debug--fileName: ");
    Serial.println(fileName);
    if (isAudioFile(fileName.c_str())) {
      totalFilesInFolder++;
    }
    entry.close();  // Zamykaj każdy plik natychmiast po zakończeniu przetwarzania
  }
  root.rewindDirectory();  // Przewiń katalog na początek

  bool playNextFolder = false;  // Flaga kontrolująca przejście do kolejnego folderu

  // Odtwarzanie plików
  while (fileIndex <= totalFilesInFolder && !playNextFolder) {
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.sendBuffer();
    File entry = root.openNextFile();
    if (!entry) {
      break;  // Koniec plików w folderze
    }

    String fileName = entry.name();

    // Pomijaj pliki, które nie są w zadeklarowanym formacie audio
    if (!isAudioFile(fileName.c_str())) {
      Serial.println("Pominięto plik: " + fileName);
      entry.close();  // Zamknij pominięty plik
      continue;
    }

    fileNameString = fileName;
    Serial.print("Odtwarzanie pliku: ");
    Serial.print(fileIndex);  // Numeracja pliku
    Serial.print("/");
    Serial.print(totalFilesInFolder);  // Łączna liczba plików w folderze
    Serial.print(" - ");
    Serial.println(fileName);

    // Pełna ścieżka do pliku
    String fullPath = folderNameString + "/" + fileName;

    Serial.print("debug--fullPath: ");
    Serial.println(fullPath);

    // Odtwarzaj plik
    audio.connecttoFS(SD, fullPath.c_str());
    seconds = 0;
    isPlaying = true;
    fileFromBuffer = fileIndex;
    folderFromBuffer = folderIndex;
    entry.close();  // Zamykaj plik po odczytaniu

    // Oczekuj na zakończenie odtwarzania



    while (isPlaying) {
      audio.loop();  // Tutaj obsługujemy odtwarzacz w tle
      button1.loop();
      button2.loop();

      // Jeśli skończył się plik, przejdź do następnego
      if (fileEnd) {
        fileEnd = false;
        id3tag = false;
        fileIndex++;
        break;
      }

      if (button2.isPressed()) {
        audio.stopSong();
        //fileIndex++;
        playNextFolder = true;
        id3tag = false;
        break;
      }

      if (button1.isPressed()) {
        audio.stopSong();
        encoderButton1 = true;
        break;
      }

      handleEncoder1Rotation();  // Obsługa kółka enkodera nr 1
      handleEncoder2Rotation();  // Obsługa kółka enkodera nr 2
      backDisplayPlayer();       // Obsługa bezczynności, przywrócenie wyświetlania danych audio
    }

    // Jeśli encoderButton1 aktywowany, wyjdź z pętli
    if (encoderButton1) {
      encoderButton1 = false;
      displayMenu();
      break;
    }

    // Sprawdź, czy zakończono odtwarzanie plików w folderze
    if (fileIndex > totalFilesInFolder) {
      Serial.println("To był ostatni plik w folderze, przechodzę do kolejnego folderu");
      playNextFolder = true;
      folderIndex++;
    }
  }

  // Przejdź do kolejnego folderu, jeśli ustawiono flagę
  if (playNextFolder) {
    if (folderIndex < directoryCount)  // Upewnij się, że folderIndex nie przekroczy dostępnych folderów
    {
      playFromSelectedFolder();  // Wywołanie funkcji tylko raz
    } else {
      Serial.println("To był ostatni folder.");
    }
  }

  // Po zakończeniu zamknij katalog
  root.close();
}

// Obsługa wyświetlacza dla odtwarzanego strumienia radia internetowego
void displayRadio() {
  //Serial.println("debug---displayRadio inside void");
  u8g2.clearBuffer();

  //u8g2.setFont(DotMatrix13pl);
  u8g2.setFont(u8g2_font_fub14_tf);
  stationName = stationName.substring(0, 22);
  u8g2.drawStr(27, 16, stationName.c_str());
  //u8g2.drawStr(0, 16, stationName.c_str());

  if (vuMeterMode == 1)  // rysujemy linijke pod nazwa stacji tylko w trybie 1 vumeter
  {
    u8g2.drawLine(0, 21, 255, 21);
  }
  //u8g2.drawStr(0, 29, stationName.c_str());

  // Funkcja wyswietlania numeru Banku na dole ekranu
  u8g2.setFont(spleen6x12PL);
  char BankStr[8];  //Formatowanie informacji o Banku do postaci Bank 00
  snprintf(BankStr, sizeof(BankStr), "Bank %02d", bank_nr);

  u8g2.setDrawColor(0);
  u8g2.setCursor(160, 63);  // pozycja napisu Bank 0x na dole ekranu
  u8g2.print(BankStr);

  u8g2.setDrawColor(1);
  u8g2.drawBox(159, 54, 1, 12);  // dorysowujemy 1px pasek przed cziocnką poniewaz cziocnka 6x12 aktywne ma 5 px dla literki i 1px spacji za literką
  //u8g2.drawRBox(225,1,30,16,4); // Rbox pod numerem stacji
  u8g2.drawRBox(1, 1, 21, 16, 4);  // Rbox pod numerem stacji

  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_spleen8x16_mr);
  char StationStr[5];
  snprintf(StationStr, sizeof(StationStr), "%02d", station_nr);  //Formatowanie informacji o stacji i baku do postaci 00
  //u8g2.setCursor(228, 14);  // Pozycja numeru stacji na gorze ekranu S0xx
  u8g2.setCursor(4, 14);  // Pozycja numeru stacji na gorze ekranu S0xx
  u8g2.print(StationStr);

  u8g2.setFont(spleen6x12PL);
  u8g2.setDrawColor(1);

  // Parametry do obługi wyświetlania w 3 kolejnych wierszach z podzialem do pełnych wyrazów
  const int maxLineLength = 41;  // Maksymalna długość jednej linii w znakach
  String currentLine = "";       // Bieżąca linia
  int yPosition = 27;            // Początkowa pozycja Y



  processText(stationString);  // przetwarzamy polsie znaki

  //Liczymy długość napisu StationString i dodajemy separator do przewijanego tekstu
  stationStringScroll = stationString + "      ";
  stationStringWidth = u8g2.getUTF8Width(stationStringScroll.c_str());
  //Serial.print("debug--stationStringWidth:");
  //Serial.println(stationStringWidth);
  //Serial.print("debug--stationStringLength:");
  //Serial.println(stationString.length());


  // Podziel tekst na wyrazy
  String word;
  int wordStart = 0;

  /* for (int i = 0; i <= stationString.length(); i++)
  {
    // Sprawdź, czy dotarliśmy do końca słowa lub do końca tekstu
    if (i == stationString.length() || stationString.charAt(i) == ' ')
    {
      // Pobierz słowo
      String word = stationString.substring(wordStart, i);
      wordStart = i + 1;

      // Sprawdź, czy dodanie słowa do bieżącej linii nie przekroczy maxLineLength
      if (currentLine.length() + word.length() <= maxLineLength)
      {
        // Dodaj słowo do bieżącej linii
        if (currentLine.length() > 0)
        {
          currentLine += " ";  // Dodaj spację między słowami
        }
        currentLine += word;
      }
      else
      {
        // Jeśli słowo nie pasuje, wyświetl bieżącą linię i przejdź do nowej linii
        u8g2.setFont(spleen6x12PL);
        u8g2.drawStr(0, yPosition, currentLine.c_str());
        yPosition += 12;  // Przesunięcie w dół dla kolejnej linii

        // Zresetuj bieżącą linię i dodaj nowe słowo
        currentLine = word;
      }
    }
  }
  // Wyświetl ostatnią linię, jeśli coś zostało
  if (currentLine.length() > 0)
  {
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(0, yPosition, currentLine.c_str());
  }

  */


  u8g2.drawLine(0, 52, 255, 52);
  String displayString = sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit " + bitrateString + "Kbps";
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(0, 63, displayString.c_str());
  //u8g2.drawStr(0, 52, displayString.c_str());
  u8g2.sendBuffer();
  // u8g2.nextPage();
  //u8g2.firstPage();
}







// Funkcja przetwarza tekst, zamieniając polskie znaki diakrytyczne
void processText(String &text) {
  for (int i = 0; i < text.length(); i++) {
    switch (text[i]) {
      case (char)0xC2:
        switch (text[i + 1]) {
          case (char)0xB3: text.setCharAt(i, 0xB3); break;  // Zamiana na "ł"
          case (char)0x9C: text.setCharAt(i, 0x9C); break;  // Zamiana na "ś"
          case (char)0x8C: text.setCharAt(i, 0x8C); break;  // Zamiana na "Ś"
          case (char)0xB9: text.setCharAt(i, 0xB9); break;  // Zamiana na "ą"
          case (char)0x9B: text.setCharAt(i, 0xEA); break;  // Zamiana na "ę"
          case (char)0xBF: text.setCharAt(i, 0xBF); break;  // Zamiana na "ż"
          case (char)0x9F: text.setCharAt(i, 0x9F); break;  // Zamiana na "ź"
        }
        text.remove(i + 1, 1);
        break;
      case (char)0xC3:
        switch (text[i + 1]) {
          case (char)0xB1: text.setCharAt(i, 0xF1); break;  // Zamiana na "ń"
          case (char)0xB3: text.setCharAt(i, 0xF3); break;  // Zamiana na "ó"
          case (char)0xBA: text.setCharAt(i, 0x9F); break;  // Zamiana na "ź"
          case (char)0xBB: text.setCharAt(i, 0xAF); break;  // Zamiana na "Ż"
          case (char)0x93: text.setCharAt(i, 0xD3); break;  // Zamiana na "Ó"
        }
        text.remove(i + 1, 1);
        break;
      case (char)0xC4:
        switch (text[i + 1]) {
          case (char)0x85: text.setCharAt(i, 0xB9); break;  // Zamiana na "ą"
          case (char)0x99: text.setCharAt(i, 0xEA); break;  // Zamiana na "ę"
          case (char)0x87: text.setCharAt(i, 0xE6); break;  // Zamiana na "ć"
          case (char)0x84: text.setCharAt(i, 0xA5); break;  // Zamiana na "Ą"
          case (char)0x98: text.setCharAt(i, 0xCA); break;  // Zamiana na "Ę"
          case (char)0x86: text.setCharAt(i, 0xC6); break;  // Zamiana na "Ć"
        }
        text.remove(i + 1, 1);
        break;
      case (char)0xC5:
        switch (text[i + 1]) {
          case (char)0x82: text.setCharAt(i, 0xB3); break;  // Zamiana na "ł"
          case (char)0x84: text.setCharAt(i, 0xF1); break;  // Zamiana na "ń"
          case (char)0x9B: text.setCharAt(i, 0x9C); break;  // Zamiana na "ś"
          case (char)0xBB: text.setCharAt(i, 0xAF); break;  // Zamiana na "Ż"
          case (char)0xBC: text.setCharAt(i, 0xBF); break;  // Zamiana na "ż"
          case (char)0x83: text.setCharAt(i, 0xD1); break;  // Zamiana na "Ń"
          case (char)0x9A: text.setCharAt(i, 0x97); break;  // Zamiana na "Ś"
          case (char)0x81: text.setCharAt(i, 0xA3); break;  // Zamiana na "Ł"
          case (char)0xB9: text.setCharAt(i, 0xAC); break;  // Zamiana na "Ź"
        }
        text.remove(i + 1, 1);
        break;
    }
  }
}

void rcInputKey(uint8_t i)
{
  timeDisplay = false;
  displayActive = true;
  displayStartTime = millis(); 
  
  if (rcInputDigit1 == 0xFF)
  {
    rcInputDigit1 = i;
  }
  else 
  {
    rcInputDigit2 = i;
  }

  int y = 35;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(65, y, "Station:"); 
  if (rcInputDigit1 != 0xFF)
  {u8g2.drawStr(153, y, String(rcInputDigit1).c_str());} 
  else {u8g2.drawStr(153, x,"_" ); }
  
  if (rcInputDigit2 != 0xFF)
  {u8g2.drawStr(164, y, String(rcInputDigit2).c_str());} 
  else {u8g2.drawStr(164, y,"_" ); }



 if ((rcInputDigit1 != 0xFF) && (rcInputDigit2 != 0xFF)) // jezeli obie wartosci nie są puste
  {
    station_nr = (rcInputDigit1 *10) + rcInputDigit2;
  }
  else if ((rcInputDigit1 != 0xFF) && (rcInputDigit2 == 0xFF))  // jezeli tylko podalismy jedna cyfrę
  { 
  station_nr = rcInputDigit1;
  }

  if (station_nr > stationsCount)  // sprawdzamy czy wprowadzona wartość nie wykracza poza licze stacji w danym banku
  {
    station_nr = stationsCount;  // jesli wpisana wartość jest wieksza niz ilosc stacji to ustawiamy war
  }
  
  // Odczyt stacji pod daną komórka pamieci PSRAM:
  char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
  memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
  int length = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1)];   // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
  
  for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) { // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
    station[j] = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
  }
  u8g2.setFont(spleen6x12PL);
  String stationNameText = String(station);
  stationNameText = stationNameText.substring(0, 25); // Przycinamy do 23 znakow

  u8g2.drawLine(0,48,256,48);
  u8g2.setFont(spleen6x12PL);
  u8g2.setCursor(0, 60);
  u8g2.print("Bank:" + String(bank_nr) + ", 1-" + String(stationsCount) + "     " + stationNameText);
  u8g2.sendBuffer();

  if ((rcInputDigit1 !=0xFF) && (rcInputDigit2 !=0xFF)) // jezeli wpisalismy obie cyfry to czyscimy pola aby mozna bylo je wpisac ponownie
  {
    rcInputDigit1 = 0xFF; // czyscimy cyfre 1, flaga pustej zmiennej F aby naciskajac kolejny raz mozna bylo wypisac cyfre bez czekania 6 sek
    rcInputDigit2 = 0xFF; // czyscimy cyfre 2, flaga pustej zmiennej F
  }
}



void webServer() {
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("debug WEB--New Client.");                                  // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /vumeter/mode1") >= 0) 
            {
              Serial.println("vuMeterMode ustawiony na 1");
              vuMeterMode = 1;
            } 
            
            else if (header.indexOf("GET /vumeter/mode0") >= 0) 
            {
              Serial.println("vuMeterMode ustawiony na 0");
              vuMeterMode = 0;
            } 
            
            else if (header.indexOf("GET /volumeUp") >= 0) 
            {
              Serial.println("debugWEB--volumeUp");
              volumeValue++;
              if (volumeValue > 21) 
              {
                volumeValue = 21;
              }        
              timeDisplay = false;
              displayActive = true;
              displayStartTime = millis();
              
              Serial.print("Wartość głośności: ");
              Serial.println(volumeValue);
              audio.setVolume(volumeValue);                 // zakres 0...21
              String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
              u8g2.clearBuffer();

              u8g2.setFont(u8g2_font_fub14_tf);
              u8g2.drawStr(65, 33, "VOLUME");
              u8g2.drawStr(163, 33, volumeValueStr.c_str());
              u8g2.drawRFrame(21, 42, 214, 14, 3);
              u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);
              u8g2.sendBuffer();
            } 
            
            else if (header.indexOf("GET /volumeDown") >= 0) 
            {
              Serial.println("debugWEB--volumeDown");
              volumeValue--;
              if (volumeValue < 1) 
              {
                volumeValue = 1;
              }
              timeDisplay = false;
              displayActive = true;
              displayStartTime = millis();
              
              Serial.print("Wartość głośności: ");
              Serial.println(volumeValue);
              audio.setVolume(volumeValue);                 // zakres 0...21
              String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
              u8g2.clearBuffer();

              u8g2.setFont(u8g2_font_fub14_tf);
              u8g2.drawStr(65, 33, "VOLUME");
              u8g2.drawStr(163, 33, volumeValueStr.c_str());
              u8g2.drawRFrame(21, 42, 214, 14, 3);
              u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);
              u8g2.sendBuffer();
            } 

            else if (header.indexOf("GET /station/next") >= 0) 
            {
              Serial.println("debugWEB-- station next");
              station_nr++;
              if (station_nr > stationsCount) {
                station_nr = stationsCount;
              }
              changeStation();
            } 
            
            else if (header.indexOf("GET /station/previous") >= 0) 
            {
              Serial.println("debugWEB-- station previous");
              station_nr--;
              if (station_nr < 1) {
                station_nr = 1;
              }
              changeStation();
            }
            else if (header.indexOf("GET /bank/previous") >= 0)
            {
              bank_nr--;
              if (bank_nr < 1) 
              {
                bank_nr = 16;
              }
              //u8g2.setFont(spleen6x12PL);
              //u8g2.clearBuffer();
              //u8g2.drawStr(10, 10, "Bank ");
              //u8g2.drawStr(40, 10, String(bank_nr).c_str());
              //u8g2.drawStr(10, 23, "Loading station from:");
              //u8g2.sendBuffer();
              //currentSelection = 0;
              //firstVisibleLine = 0;
              //station_nr = 1;
              fetchStationsFromServer();
              changeStation();
              u8g2.clearBuffer();

            }
            else if (header.indexOf("GET /bank/next") >= 0)
            {
              bank_nr++;
              if (bank_nr > 16) 
              {
                bank_nr = 1;
              }
              
              fetchStationsFromServer();
              changeStation();
              u8g2.clearBuffer();
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>* { box-sizing: border-box;}");
            client.println(".column {float: left;width: 25%; padding: 5px;}");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: 1; color: white; padding: 10px 20px;");
            client.println("text-decoration: none; font-size: 15px; margin: 2px; cursor: pointer;}");
            client.println("table, th, td { border: 1px solid white; border-collapse: collapse; padding: 0px;}"); 
            client.println("th, td {background-color: LightGray; padding:0px;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP32 Internet Radio</h1>");
            client.println("<p></p>");
            client.println("<p>You are listening: <b>" + String(stationName.substring(0, 23)) + "</b> - " + stationString + "</p>");
            client.println("<p>Station:<b>" + String(station_nr) + "</b>, bank: <b>" + String(bank_nr) + "</b></p>");
            client.println("<p>Volume: <b>" + String(volumeValue) + "</b></p>");
            client.println("<p></p>");

            client.println("<p>VU meter mode: " + String(vuMeterMode) );
            if (vuMeterMode == 0) {
              client.println("<a href=\"/vumeter/mode1\"><button class=\"button\">Set VU Mode 1</button></a></p>");
            } else {
              client.println("<a href=\"/vumeter/mode0\"><button class=\"button button2\">Set VU Mode 0</button></a></p>");
            }

            // Wybór stacji:
            client.println("<p>Control</p>");
            // Przyciski na stronie web
            client.println("<p><a href=\"/volumeDown\"><button class=\"button\">      Volume -      </button></a>");  
            client.println("<a href=\"/bank/previous\"><button class=\"button\">  << Previous Bank  </button></a>");
            client.println("<a href=\"/station/previous\"><button class=\"button\"> < Previous Station </button></a>");
            client.println("<a href=\"/station/next\"><button class=\"button\">   Next Station >   </button></a>");
            client.println("<a href=\"/bank/next\"><button class=\"button\">Next Bank >> </button></a>");
            client.println("<a href=\"/volumeUp\"><button class=\"button\">      Volume +      </button></a></p>");
            client.println("<p></p>");      
            
            //Lista stacji z danego banku:
            client.println("<p style=\"text-align:left\">Bank stations list:</p>");
            //client.println("<div class=\"row\">");
            

            for (int i = 0; i < stationsCount; i++) {
              char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
              memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych

              int length = psramData[i * (STATION_NAME_LENGTH + 1)];

              for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) {
                station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
              }

              if ((i == 0) || (i == 25) || (i == 50) || (i == 75))
              { 
                client.println("<div class=\"column\"><table>");
                //client.println("<tr><th>No</th><th>Station</th><th>Action</th></tr>");
                client.println("<tr><th>No</th><th>Station</th></tr>");
              }


              
              if (i + 1 == station_nr)
              {             
              client.println("<tr>");  // bold <b> dla obecnie odtwarzanej stacji
              client.print("<td><p style=\"text-align:center; margin-top: 3px; margin-bottom:3px; background-color: #4CAF50;\"><b>" + String(i + 1) + "</b></p></td>");
              client.print("<td><p style=\"text-align:left; margin-top: 3px; margin-bottom:3px;  background-color: #4CAF50; width: 250px;\"><b> " + String(station).substring(0, 26) + "</b></p></td>");
              //client.println("<td><p style=\"text-align:center; margin-top: 3px; margin-bottom:3px\"; margin:3px>Playing</b></p></td>");
              client.print("</tr>");
              }
              else
              {
              client.println("<tr>");
              client.print("<td><p style=\"text-align:center; margin-top: 3px; margin-bottom:3px\">" + String(i + 1) + "</p></td>");
              client.print("<td><p style=\"text-align:left; margin-top: 3px; margin-bottom:3px; width: 250px;\"> " + String(station).substring(0, 26) + "</p></td>");
              //client.println("<td><p style=\"text-align:center; margin-top: 3px; margin-bottom:3px\"; margin:3px>Play</p></td>");
              client.print("</tr>");
              }

              if ((i == 24) || (i == 49) || (i == 74) ||(i == 99))
              { 
                client.println("</table></div>");
              }
              


            }
            //client.println("</div>");

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }

        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}






// Obsługa wyświetlacza dla odtwarzanego pliku z karty SD
void displayPlayer() {
  if (id3tag == true) {
    timeDisplay = true;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    u8g2.setCursor(0, 10);
    u8g2.print("PLAYING:");


    Serial.print("DEBUG--PlayedFolderName:");
    Serial.println(PlayedFolderName);

    if (PlayedFolderName.length() > 24) {
      u8g2.print(PlayedFolderName.substring(0, 23));  // Jesli folder muzyk > 16 znakow to wyswietlamy pierwszy 16 i trzy kropki
      u8g2.print("...");
    } else {
      u8g2.print(PlayedFolderName);  // Jesli nazwa folderu miesci sie w 16 znakach wysweitlamy całosc
    }

    u8g2.setCursor(202, 10);
    u8g2.print(" Tr:");
    u8g2.print(fileFromBuffer);
    u8g2.print("/");
    u8g2.print(totalFilesInFolder);
    //u8g2.print(" FOLDER ");
    //u8g2.print(folderFromBuffer);
    //u8g2.print("/");
    //u8g2.print(directoryCount);

    if (artistString.length() > 21) {
      artistString = artistString.substring(0, 21);  // Ogranicz długość tekstu do 33 znaków
    }
    u8g2.setCursor(0, 28);
    u8g2.setFont(u8g2_font_fub14_tf);
    //u8g2.print("Artysta: ");
    u8g2.print(artistString);

    if (titleString.length() > 35) {
      titleString = titleString.substring(0, 35);  // Ogranicz długość tekstu do 35 znaków
    }
    u8g2.setFont(spleen6x12PL);
    u8g2.setCursor(0, 42);
    //u8g2.print("Tytul:");
    u8g2.print(titleString);

    /*if (folderNameString.startsWith("/"))
    {
      folderNameString = folderNameString.substring(1); // Usuń pierwszy ukośnik
    }

    if (folderNameString.length() > 34)
    {
      folderNameString = folderNameString.substring(0, 34); // Ogranicz długość tekstu do 34 znaków
    }
    u8g2.setCursor(0, 41);
    u8g2.print("Folder: ");
    u8g2.print(folderNameString);
    */
    u8g2.drawStr(0, 63, "                                           ");
    u8g2.drawLine(0, 51, 255, 51);
    String displayString = sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit " + bitrateString + "Kbps";
    u8g2.drawStr(0, 63, displayString.c_str());
    u8g2.sendBuffer();
    Serial.println("Tagi ID3 artysty, tytułu i folderu gotowe do wyświetlenia");
  } else {
    // Maksymalna długość wiersza (42 znaki)
    int maxLineLength = 42;
    int maxFirstLineLength = 26;
    int maxFirstLineLengthLongName = 42;
    timeDisplay = true;
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.setCursor(0, 10);
    u8g2.print("PLAYING:                    ");
    u8g2.print(fileFromBuffer);
    u8g2.print(" of ");
    u8g2.print(totalFilesInFolder);
    //u8g2.print(" FOLDER ");
    //u8g2.print(folderFromBuffer);
    //u8g2.print("/");
    //u8g2.print(directoryCount);
    //u8g2.drawStr(0, 21, "Brak danych ID3 utworu, nazwa pliku:");

    // Jeśli długość nazwy pliku przekracza 42 znaki na wiersz
    //if (fileNameString.length() > maxLineLength)
    if (fileNameString.length() > maxFirstLineLength) {

      int FileNameStringIndex = String(fileNameString).indexOf("-");  // Znajdujemy index ile znaków mamy w nazwie pliku do "-"
      // Jeśli nazwa pliku NIE mieści się w jednym wierszu

      // Prcyinamy nazwe artysty aby miesciła sie w pierwszej lini jest jest za długa
      String firstLine = String(fileNameString).substring(0, FileNameStringIndex);
      String secondLine = String(fileNameString).substring(FileNameStringIndex + 2, String(fileNameString).indexOf('.', FileNameStringIndex));

      if (firstLine.length() < 26) {
        firstLine = String(firstLine.substring(0, maxFirstLineLength));  // Nazwe Artysty przycinamy do wartosci FirstLineLenght dla dużej czcionka (długosc do 26 znakow)
        u8g2.setCursor(0, 28);
        u8g2.setFont(u8g2_font_fub14_tf);  // W pierwszej lini jest nazwa Artysty - piszemy duza czcionką
        u8g2.print(firstLine);
      } else {
        firstLine = String(firstLine.substring(0, maxFirstLineLengthLongName));  // Nazwe Artysty przycinamy do wartosci maxFirstLineLengthLongName dla długosci > 26 znakow
        u8g2.setCursor(0, 28);
        u8g2.setFont(spleen6x12PL);  // przy BARDZO długich nazwach (powyzej 26 znakow) pierwsza linia budowana jest mała cziocnka - rozwiazanie tymczasowe
        u8g2.print(firstLine);
      }

      // Drugi wiersz - pozostałe znaki nazwa utworu
      //Wyswietlamy

      u8g2.setFont(spleen6x12PL);
      u8g2.setCursor(0, 42);
      u8g2.print(secondLine);

    } else {
      int FileNameStringIndex = String(fileNameString).indexOf("-");  // Znajdujemy index ile znaków mamy w nazwie pliku do "-"
      // Jeśli nazwa pliku mieści się w jednym wierszu

      u8g2.setCursor(0, 28);
      u8g2.setFont(u8g2_font_fub14_tf);
      u8g2.print(String(fileNameString).substring(0, FileNameStringIndex));  // W pierwszej lini jest nazwa Artysty - piszemy duza czcionką

      //druga linia to nazwa utworu, zmieniamy cziocnke na małą
      u8g2.setFont(spleen6x12PL);
      u8g2.setCursor(0, 42);

      // Składamy nazwe utworu w przypadku braku id3tag.
      //Pierwsza linia wycina z nazwy pliku do znacznika "-" druga zawiera to co jest po znaczniku "-" do krpoki rozszerzenia "."

      u8g2.print(String(fileNameString).substring(FileNameStringIndex + 2, String(fileNameString).indexOf('.', FileNameStringIndex)));
    }
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(0, 63, "                                           ");
    u8g2.drawLine(0, 51, 255, 51);
    String displayString = sampleRateString.substring(1) + "Hz " + bitsPerSampleString + "bit " + bitrateString + "Kbps" + " noID3";
    u8g2.drawStr(0, 63, displayString.c_str());
    u8g2.sendBuffer();
    Serial.println("Brak prawidłowych tagów ID3 do wyświetlenia");
  }
}

// Funkcja przywracająca wyświetlanie danych o utworze po przekroczeniu czasu bezczynności podczas odtwarzania plików audio z karty SD
void backDisplayPlayer() {
  if (displayActive && (millis() - displayStartTime >= displayTimeout)) {
    displayPlayer();
    displayActive = false;
    timeDisplay = true;
  }
}

// Obsługa kółka enkodera 1 podczas dzialania odtwarzacza plików
void handleEncoder1Rotation() {
  CLK_state1 = digitalRead(CLK_PIN1);
  if (CLK_state1 != prev_CLK_state1 && CLK_state1 == HIGH) {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();
    if (digitalRead(DT_PIN1) == HIGH) {
      volumeValue--;
      if (volumeValue < 1) {
        volumeValue = 1;
      }
    } else {
      volumeValue++;
      if (volumeValue > 21) {
        volumeValue = 21;
      }
    }
    Serial.print("Wartość głośności: ");
    Serial.println(volumeValue);
    audio.setVolume(volumeValue);                 // zakres 0...21
    String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_fub14_tf);
    u8g2.drawStr(20, 33, "VOLUME");
    u8g2.drawStr(132, 33, volumeValueStr.c_str());
    u8g2.drawRFrame(21, 42, 214, 14, 3);
    u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);
    u8g2.sendBuffer();
  }
  prev_CLK_state1 = CLK_state1;
}

// Obsługa kółka enkodera 2 podczas dzialania odtwarzacza plików
void handleEncoder2Rotation() {
  CLK_state2 = digitalRead(CLK_PIN2);
  if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH) {
    folderIndex = currentSelection;  // Zaktualizuj indeks folderu
    timeDisplay = false;
    if (digitalRead(DT_PIN2) == HIGH) {
      folderIndex--;
      if (folderIndex < 0) {
        folderIndex = 0;
      }
      Serial.print("Numer folderu do tyłu: ");
      Serial.println(folderIndex);
      scrollUp();
      displayFolders();
    } else {
      folderIndex++;
      if (folderIndex > (directoryCount - 1)) {
        folderIndex = directoryCount - 1;
      }
      Serial.print("Numer folderu do przodu: ");
      Serial.println(folderIndex);

      scrollDown();
      displayFolders();
    }
    displayActive = true;
    displayStartTime = millis();
  }
  prev_CLK_state2 = CLK_state2;
}
// Funkcja do wyświetlania folderów na ekranie OLED z uwzględnieniem zaznaczenia
void displayFolders() {
  u8g2.clearBuffer();
  u8g2.setFont(spleen6x12PL);
  u8g2.setCursor(0, 10);
  u8g2.print("   ODTWARZACZ PLIKOW - LISTA KATALOGOW    ");
  //u8g2.setCursor(0, 21);
  //u8g2.print(currentDirectory);  // Wyświetl bieżący katalog

  int displayRow = 1;  // Zmienna dla numeru wiersza, zaczynając od drugiego (pierwszy to nagłówek)

  // Wyświetlanie katalogów zaczynając od pierwszej widocznej linii
  for (int i = firstVisibleLine; i < min(firstVisibleLine + 4, directoryCount); i++) {
    String fullPath = currentDirectory + directories[i];

    // Pomijaj "System Volume Information"
    if (fullPath != "/System Volume Information") {
      Serial.print("----------------------------------");
      Serial.print("debug--Full path CurretnDirectory:");
      Serial.println(currentDirectory);
      // Sprawdź, czy ścieżka zaczyna się od aktualnego katalogu
      if (fullPath.startsWith(currentDirectory))
      //if (fullPath.startsWith(folderNameString))

      {
        // Ogranicz długość do 42 znaków
        String displayedPath = fullPath.substring(currentDirectory.length() + 1, currentDirectory.length() + 42);
        Serial.print("debug--Displayedpath:");
        Serial.println(displayedPath);
        // Podświetlenie zaznaczonego katalogu
        //if (i == x) {x= i+1 }

        if (i == currentSelection) {
          Serial.print("debug--Full path:");
          Serial.println(fullPath);
          Serial.print("debug--Indeks i:");
          Serial.println(i);
          Serial.print("debug--CurrentDirectory: ");
          Serial.println(currentDirectory);

          u8g2.setFont(spleen6x12PL);
          u8g2.setDrawColor(1);                           // Biały kolor tła
          u8g2.drawBox(0, displayRow * 13 - 2, 256, 13);  // Narysuj prostokąt jako tło dla zaznaczonego folderu
          u8g2.setDrawColor(0);                           // Czarny kolor tekstu
        } else {
          u8g2.setDrawColor(1);
        }
        // Wyświetl ścieżkę
        //  u8g2.setDrawColor(1);
        u8g2.setFont(spleen6x12PL);
        u8g2.drawStr(0, displayRow * 13 + 8, String(displayedPath).c_str());

        // Przesuń się do kolejnego wiersza
        displayRow++;
      }
    } else {
      x == i;
      Serial.println("SystemVOLUME");
    }
  }
  // Przywróć domyślne ustawienia koloru rysowania (biały tekst na czarnym tle)
  u8g2.setDrawColor(1);  // Biały kolor rysowania
  u8g2.sendBuffer();
}


// Funkcja do wyświetlania listy stacji radiowych z opcją wyboru poprzez zaznaczanie w negatywie
void displayStations() {
  listedStations = true;
  u8g2.clearBuffer();  // Wyczyść bufor przed rysowaniem, aby przygotować ekran do nowej zawartości
  u8g2.setFont(spleen6x12PL);
  u8g2.setCursor(60, 10);                                          // Ustaw pozycję kursora (x=60, y=10) dla nagłówka
  u8g2.print("RADIO STATIONS:   ");                                // Wyświetl nagłówek "Radio Stations:"
  u8g2.print(String(station_nr) + " / " + String(stationsCount));  // Dodaj numer aktualnej stacji i licznik wszystkich stacji

  int displayRow = 1;  // Zmienna dla numeru wiersza, zaczynając od drugiego (pierwszy to nagłówek)
  
  //erial.print("FirstVisibleLine:");
  //Serial.print(firstVisibleLine);

  // Wyświetlanie stacji, zaczynając od drugiej linii (y=21)
  for (int i = firstVisibleLine; i < min(firstVisibleLine + maxVisibleLines, stationsCount); i++) {
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych

    // Odczytaj długość nazwy stacji z EEPROM dla bieżącego indeksu stacji
    //int length = EEPROM.read(i * (STATION_NAME_LENGTH + 1));

    int length = psramData[i * (STATION_NAME_LENGTH + 1)];  //----------------------------------------------

    // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) {
      station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }

    // Sprawdź, czy bieżąca stacja to ta, która jest aktualnie zaznaczona
    if (i == currentSelection) {
      u8g2.setDrawColor(1);                           // Ustaw biały kolor rysowania
      u8g2.drawBox(0, displayRow * 13 - 2, 256, 13);  // Narysuj prostokąt jako tło dla zaznaczonej stacji (x=0, szerokość 256, wysokość 10)
      u8g2.setDrawColor(0);                           // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
    } else {
      u8g2.setDrawColor(1);  // Dla niezaznaczonych stacji ustaw zwykły biały kolor tekstu
    }
    // Wyświetl nazwę stacji, ustawiając kursor na odpowiedniej pozycji
    u8g2.drawStr(0, displayRow * 13 + 8, String(station).c_str());
    //u8g2.print(station);  // Wyświetl nazwę stacji

    // Przejdź do następnej linii (następny wiersz na ekranie)
    displayRow++;
  }
  // Przywróć domyślne ustawienia koloru rysowania (biały tekst na czarnym tle)
  u8g2.setDrawColor(1);  // Biały kolor rysowania
  u8g2.sendBuffer();     // Wyślij zawartość bufora do ekranu OLED, aby wyświetlić zmiany
}

void updateTimerFlag() {
  ActionNeedUpdateTime = true;
}

// Funkcja wywoływana co sekundę przez timer do aktualizacji czasu na wyświetlaczu
void updateTimer() {
  // Wypełnij spacjami, aby wyczyścić pole
  //u8g2.drawStr(208, 63, "         "); // czyszczenie pola zegara
  //u8g2.drawStr(128, 63, "    "); // czyszczenie pola FLAC/MP3/AAC

  // Zwiększ licznik sekund
  seconds++;

  // Wyświetl aktualny czas w sekundach
  // Konwertuj sekundy na minutę i sekundy
  unsigned int minutes = seconds / 60;
  unsigned int remainingSeconds = seconds % 60;

  u8g2.setDrawColor(1);  // Ustaw kolor na biały

  if (timeDisplay == true) {
    if (audio.isRunning() == true) {
      if (mp3 == true) {
        u8g2.drawStr(130, 63, " MP3");
        //Serial.println("Gram MP3");
      }
      if (flac == true) {
        u8g2.drawStr(130, 63, "FLAC");
        //Serial.println("Gram FLAC");
      }
      if (aac == true) {
        u8g2.drawStr(130, 63, " AAC");
        //Serial.println("Gram AAC");
      }
    }

    if ((currentOption == PLAY_FILES) && (bitratePresent == true)) {
      // Formatuj czas jako "mm:ss"
      char timeString[10];
      snprintf(timeString, sizeof(timeString), "%02um:%02us", minutes, remainingSeconds);
      u8g2.drawStr(210, 63, timeString);
      u8g2.sendBuffer();
    }

    if ((currentOption == INTERNET_RADIO) && ((mp3 == true) || (flac == true) || (aac == true))) {
      // Struktura przechowująca informacje o czasie
      struct tm timeinfo;

      // Sprawdź, czy udało się pobrać czas z lokalnego zegara czasu rzeczywistego
      if (!getLocalTime(&timeinfo)) {
        // Wyświetl komunikat o niepowodzeniu w pobieraniu czasu
        Serial.println("Nie udało się uzyskać czasu");
        return;  // Zakończ funkcję, gdy nie udało się uzyskać czasu
      }

      // Konwertuj godzinę, minutę i sekundę na stringi w formacie "HH:MM:SS"
      char timeString[9];  // Bufor przechowujący czas w formie tekstowej
      snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      u8g2.setFont(spleen6x12PL);
      u8g2.drawStr(205, 63, timeString);
      //u8g2.sendBuffer(); // nie piszemy po ekranie w tej funkcji tylko przygotowujemy bufor. Nie mozna pisac podczas pracy
    }
  }
}
// Funkcja do zapisywania numeru stacji i numeru banku na karcie SD
void saveStationOnSD() {
  // Sprawdź, czy plik station_nr.txt istnieje

  Serial.print("Zapisany bank: ");
  Serial.println(bank_nr);
  Serial.print("Zapisana stacja: ");
  Serial.println(station_nr);

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt")) {
    Serial.println("Plik station_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Aktualizacja station_nr.txt na karcie SD.");
    } else {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  } else {
    Serial.println("Plik station_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość station_nr
    myFile = SD.open("/station_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano station_nr.txt na karcie SD.");
    } else {
      Serial.println("Błąd podczas tworzenia pliku station_nr.txt.");
    }
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt")) {
    Serial.println("Plik bank_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Aktualizacja bank_nr.txt na karcie SD.");
    } else {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  } else {
    Serial.println("Plik bank_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość bank_nr
    myFile = SD.open("/bank_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano bank_nr.txt na karcie SD.");
    } else {
      Serial.println("Błąd podczas tworzenia pliku bank_nr.txt.");
    }
  }
}
// Funkcja do odczytu danych stacji radiowej z karty SD
void readStationFromSD() {
  // Sprawdź, czy karta SD jest dostępna
  if (!SD.begin(47)) {
    Serial.println("Nie można znaleźć karty SD. Ustawiam domyślne wartości.");
    station_nr = 9;  // Domyślny numer stacji gdy brak karty SD
    bank_nr = 1;     // Domyślny numer banku gdy brak karty SD
    return;
  }

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SD.exists("/station_nr.txt")) {
    myFile = SD.open("/station_nr.txt");
    if (myFile) {
      station_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano station_nr z karty SD: ");
      Serial.println(station_nr);
    } else {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  } else {
    Serial.println("Plik station_nr.txt nie istnieje.");
    station_nr = 9;  // ustawiamy stacje w przypadku braku pliku na karcie
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SD.exists("/bank_nr.txt")) {
    myFile = SD.open("/bank_nr.txt");
    if (myFile) {
      bank_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano bank_nr z karty SD: ");
      Serial.println(bank_nr);
    } else {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  } else {
    Serial.println("Plik bank_nr.txt nie istnieje.");
    bank_nr = 1;  // // ustawiamy bank w przypadku braku pliku na karcie
  }
}

void vuMeter() {
  //uint8_t previousvuMeterL;
  //uint8_t previousvuMeterR;

  vuMeterL = audio.getVUlevel() & 0xFF;  // wyciagamy ze zmiennej typu int16 kanał L
  vuMeterR = (audio.getVUlevel() >> 8);  // z wyzszej polowki wyciagamy kanal P

  //vuMeterL = (vuMeterL >> 1); // dzielimy przez 2 -> przesuniecie o jeden bit abyz  255 -> 64
  //vuMeterR = (vuMeterR >> 1);


  //u8g2.drawFrame(0,39,132,14);
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 41, 253, 3);  //czyszczenie ekranu pod VU meter
  u8g2.drawBox(0, 46, 253, 3);

  u8g2.setDrawColor(1);

  if (vuMeterMode == 1)  // tryb 1 ciagle paski
  {
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 41, vuMeterL, 3);  // rysujemy kreseczki o dlugosci odpowiadajacej wartosci VU
    u8g2.drawBox(0, 46, vuMeterR, 3);

  } else  // vuMeterMode == 0  tryb podstawowy, kreseczki z przerwami
  {
    for (uint8_t vusize = 0; vusize < vuMeterL; vusize++) {
      u8g2.drawBox(vusize, 41, 8, 2);
      vusize = vusize + 8;
    }

    for (uint8_t vusize = 0; vusize < vuMeterR; vusize++) {
      u8g2.drawBox(vusize, 46, 8, 2);
      vusize = vusize + 8;
    }
  }
}

void displayRadioScroller() {


  if (stationString.length() > 42) {

    xPositionStationString = offset;
    u8g2.setFont(spleen6x12PL);
    u8g2.setDrawColor(1);
    do {
      u8g2.drawStr(xPositionStationString, 34, stationStringScroll.c_str());
      xPositionStationString = xPositionStationString + stationStringWidth;
    } while (xPositionStationString < 256);

    offset = offset - 1;
    //if ( (u8g2_uint_t)offset < (u8g2_uint_t)-stationStringWidth )
    if (offset < (65535 - stationStringWidth)) {
      /*  Serial.print("u8g2_uint_t Offset:");
      Serial.println((u8g2_uint_t)offset);

      Serial.print("(u8g2_uint_t)-stationStringWidth:");
      Serial.println((u8g2_uint_t)-stationStringWidth);
      
      Serial.print("Offset:");
      Serial.println(offset);
      
      Serial.print("offset -stationStringWidth:");
      Serial.println(offset - stationStringWidth);   
     */

      offset = 0;
      //Serial.println("debug-- Reset Offsetu -> 0");
    }

  } else {
    xPositionStationString = 0;
    u8g2.setDrawColor(1);
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(xPositionStationString, 34, stationString.c_str());
  }
//u8g2.sendBuffer();  
}
/*
void ir_task(void* parameter){
	parameter = parameter;
  while(1){
    ir_recv.task();
  }
	vTaskDelete(NULL);
}
*/

void bankMenuDisplay()

{
  displayStartTime = millis();
  Serial.println("debug--Bank Menu");
  volumeSet = false;
  previous_bank_nr = bank_nr;  // jesli weszlimy do menu "wybór banku" to zapisujemy obecny bank zanim zaczniemy krecic enkoderem
  bankMenuEnable = true;
  timeDisplay = false;
  bankChange = true;
  //currentOption = BANK_LIST;  // Ustawienie listy banków do przewijania i wyboru
  String bankNrStr = String(bank_nr);
  Serial.println("Wyświetlenie listy banków");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(80, 33, "BANK:");
  u8g2.drawStr(145, 33, String(bank_nr).c_str());  // numer banku

  u8g2.drawRFrame(21, 42, 214, 14, 3);                // Ramka do slidera bankow
  u8g2.drawRBox((bank_nr * 13) + 10, 44, 15, 10, 2);  // wypełnienie slidera
  u8g2.sendBuffer();
  
}


void volumeDisplay()
{
  timeDisplay = false;
  displayActive = true;
  displayStartTime = millis();
  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());
  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
  u8g2.sendBuffer();
}

void volumeUp()
{
  timeDisplay = false;
  displayActive = true;
  displayStartTime = millis();   
  volumeValue++;

  if (volumeValue > 21) 
  {
    volumeValue = 21;
  }

  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());
  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
  u8g2.sendBuffer();
}

void volumeDown()
{
  timeDisplay = false;
  displayActive = true;
  displayStartTime = millis();
  volumeValue--;
  if (volumeValue < 1) 
  {
    volumeValue = 1;
  } 
  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());
  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
  u8g2.sendBuffer();
}

void bufforAudioInfo()
{
Serial.print("debug--Bufor Audio pojemność / zapełniony:");
Serial.print(audio.inBufferSize());
Serial.print(" / ");
Serial.println(audio.inBufferFilled());
//Serial.println(audio.inBufferFree());
//Serial.println(audio.inBufferSize());
//Serial.println(audio.inBufferSize() - audio.inBufferFilled());
}



// Funkcja obsługująca przerwanie (reakcja na zmianę stanu pinu)
//void IRAM_ATTR pulseISR()
void IRAM_ATTR pulseISR()
{
  czas1 = esp_timer_get_time();
  if (digitalRead(recv_pin) == HIGH)
  {
    pulse_start = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end = micros();    // Zapis końca impulsu
    pulse_ready = true;
  }

  if (digitalRead(recv_pin) == LOW)
  {
    pulse_start_low = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end_low = micros();    // Zapis końca impulsu
    pulse_ready_low = true;
  }

  //static bool data_start_detected = false;  // Flaga dla sygnału wstępnego
  //static unsigned long last_pulse_time = 0;
  
  // ----------- ANALIZA PUSLOW -----------------------------
  if (pulse_ready_low) // spradzamy czy jest stan niski przez 9ms - start ramki
  {
    pulse_duration_low = pulse_end_low - pulse_start_low;
  
    if (pulse_duration_low > (LEAD_HIGH - TOLERANCE) && pulse_duration_low < (LEAD_HIGH + TOLERANCE))
    {
      pulse_duration_9ms = pulse_duration_low; // przypisz czas trwania puslu Low do zmiennej puls 9ms
      pulse_ready9ms = true; // flaga poprawnego wykrycia pulsu 9ms w granicach tolerancji
    }

  }
  // Sprawdzenie, czy impuls jest gotowy do analizy
  if ((pulse_ready== true) && (pulse_ready9ms = true))
  {
    pulse_ready = false;
    pulse_ready9ms = false; // kasujemy flage wykrycia pulsu 9ms

    // Obliczenie czasu trwania impulsu
    pulse_duration = pulse_end - pulse_start;
    //Serial.println(pulse_duration); odczyt dlugosci pulsow z pilota - debug
    if (!data_start_detected)
    {
    
      // Oczekiwanie na sygnał 4,5 ms wysoki
      if (pulse_duration > (LEAD_HIGH - TOLERANCE) && pulse_duration < (LEAD_HIGH + TOLERANCE))
      {
       
        // Początek sygnału: 9 ms niski
        //Serial.println("Otrzymano początek sygnału (9 ms niski).");
        //pulse_duration_9ms = pulse_duration; 
      }
      else if (pulse_duration > (LEAD_LOW - TOLERANCE) && pulse_duration < (LEAD_LOW + TOLERANCE))
      {
        
        pulse_duration_4_5ms = pulse_duration;
        // Początek sygnału: 4,5 ms wysoki
        // Serial.println("Otrzymano początek sygnału (4,5 ms wysoki).");
        data_start_detected = true;  // Ustawienie flagi po wykryciu sygnału wstępnego
        bit_count = 0;               // Reset bit_count przed odebraniem danych
        ir_code = 0;                 // Reset kodu IR przed odebraniem danych
      }
    }
    else
    {
      // Sygnały dla bajtów (adresu ADDR, IADDR, komendy CMD, ICMD) zaczynają się po wstępnym sygnale
      if (pulse_duration > (HIGH_THRESHOLD - TOLERANCE) && pulse_duration < (HIGH_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 1;  // Dodanie "1" do kodu IR
        bit_count++;
        pulse_duration_1690us = pulse_duration;
      }
      else if (pulse_duration > (LOW_THRESHOLD - TOLERANCE) && pulse_duration < (LOW_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 0;  // Dodanie "0" do kodu IR
        bit_count++;
        pulse_duration_690us = pulse_duration;
      }

      // Sprawdzenie, czy otrzymano pełny 32-bitowy kod IR
      if (bit_count == 32)
      {
        // Rozbicie kodu na 4 bajty
        uint8_t ADDR = (ir_code >> 24) & 0xFF;  // Pierwszy bajt
        uint8_t IADDR = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
        uint8_t CMD = (ir_code >> 8) & 0xFF;    // Trzeci bajt (komenda)
        uint8_t ICMD = ir_code & 0xFF;          // Czwarty bajt (inwersja komendy)

        // Sprawdzenie poprawności (inwersja) bajtów adresu i komendy
        if ((ADDR ^ IADDR) == 0xFF && (CMD ^ ICMD) == 0xFF)
        {
          data_start_detected = false;
          //bit_count = 0;
        }
        else
        {
          ir_code = 0; 
          //ir_code=0xF0F0F0F0;  // Ustawiamy F0F0F0F0 jako flagę błednego kodu
          data_start_detected = false;
          //bit_count = 0;        
        }

      }
    }
  }
czas2 = esp_timer_get_time();
}



/*
void IRAM_ATTR analyzePulseFromIR()
{
  
  //static bool data_start_detected = false;  // Flaga dla sygnału wstępnego
  //static unsigned long last_pulse_time = 0;
  
  if (pulse_ready_low) // spradzamy czy jest stan niski przez 9ms - start ramki
  {
    pulse_duration_low = pulse_end_low - pulse_start_low;
  
    if (pulse_duration_low > (LEAD_HIGH - TOLERANCE) && pulse_duration_low < (LEAD_HIGH + TOLERANCE))
    {
      pulse_duration_9ms = pulse_duration_low; // przypisz czas trwania puslu Low do zmiennej puls 9ms
      pulse_ready9ms = true; // flaga poprawnego wykrycia pulsu 9ms w granicach tolerancji
    }

  }
  // Sprawdzenie, czy impuls jest gotowy do analizy
  if ((pulse_ready== true) && (pulse_ready9ms = true))
  {
    pulse_ready = false;
    pulse_ready9ms = false; // kasujemy flage wykrycia pulsu 9ms

    // Obliczenie czasu trwania impulsu
    pulse_duration = pulse_end - pulse_start;
    //Serial.println(pulse_duration); odczyt dlugosci pulsow z pilota - debug
    if (!data_start_detected)
    {
    
      // Oczekiwanie na sygnał 4,5 ms wysoki
      if (pulse_duration > (LEAD_HIGH - TOLERANCE) && pulse_duration < (LEAD_HIGH + TOLERANCE))
      {
       
        // Początek sygnału: 9 ms niski
        //Serial.println("Otrzymano początek sygnału (9 ms niski).");
        //pulse_duration_9ms = pulse_duration; 
      }
      else if (pulse_duration > (LEAD_LOW - TOLERANCE) && pulse_duration < (LEAD_LOW + TOLERANCE))
      {
        
        pulse_duration_4_5ms = pulse_duration;
        // Początek sygnału: 4,5 ms wysoki
        // Serial.println("Otrzymano początek sygnału (4,5 ms wysoki).");
        data_start_detected = true;  // Ustawienie flagi po wykryciu sygnału wstępnego
        bit_count = 0;               // Reset bit_count przed odebraniem danych
        ir_code = 0;                 // Reset kodu IR przed odebraniem danych
      }
    }
    else
    {
      // Sygnały dla bajtów (adresu ADDR, IADDR, komendy CMD, ICMD) zaczynają się po wstępnym sygnale
      if (pulse_duration > (HIGH_THRESHOLD - TOLERANCE) && pulse_duration < (HIGH_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 1;  // Dodanie "1" do kodu IR
        bit_count++;
        pulse_duration_1690us = pulse_duration;
      }
      else if (pulse_duration > (LOW_THRESHOLD - TOLERANCE) && pulse_duration < (LOW_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 0;  // Dodanie "0" do kodu IR
        bit_count++;
        pulse_duration_690us = pulse_duration;
      }

      // Sprawdzenie, czy otrzymano pełny 32-bitowy kod IR
      if (bit_count == 32)
      {
        // Rozbicie kodu na 4 bajty
        uint8_t ADDR = (ir_code >> 24) & 0xFF;  // Pierwszy bajt
        uint8_t IADDR = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
        uint8_t CMD = (ir_code >> 8) & 0xFF;    // Trzeci bajt (komenda)
        uint8_t ICMD = ir_code & 0xFF;          // Czwarty bajt (inwersja komendy)

        // Sprawdzenie poprawności (inwersja) bajtów adresu i komendy
        if ((ADDR ^ IADDR) == 0xFF && (CMD ^ ICMD) == 0xFF)
        {
          data_start_detected = false;
          //bit_count = 0;
        }
        else
        {
          ir_code = 0; 
          //ir_code=0xF0F0F0F0;  // Ustawiamy F0F0F0F0 jako flagę błednego kodu
          data_start_detected = false;
          //bit_count = 0;        
        }

      }
    }
  }
  czas2 = esp_timer_get_time();
}
*/

void setup() {
  // Inicjalizuj komunikację szeregową (Serial)
  Serial.begin(115200);
  Serial.println("debug---------START ESP32 Radio----------");
  
  psramData = (unsigned char *)ps_malloc(PSRAM_lenght * sizeof(unsigned char));

  if (psramInit()) {
    Serial.println("debug--pamiec PSRAM zainicjowana poprawnie");
    Serial.print("Dostepna pamiec PSRAM:");
    Serial.println(ESP.getPsramSize());
    Serial.print("Wolna pamiec PSRAM:");
    Serial.println(ESP.getFreePsram());


  } else {
    Serial.println("debug-- BLAD Pamieci PSRAM");
  }
  
  // Ustaw pin CS dla karty SD jako wyjście i ustaw go na wysoki stan
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);


  // Konfiguruj piny enkodera jako wejścia
  pinMode(CLK_PIN1, INPUT);
  pinMode(DT_PIN1, INPUT);
  pinMode(CLK_PIN2, INPUT);
  pinMode(DT_PIN2, INPUT);
  // Inicjalizacja przycisków enkoderów jako wejścia
  pinMode(SW_PIN1, INPUT_PULLUP);
  pinMode(SW_PIN2, INPUT_PULLUP);
  
  pinMode(recv_pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);

  // Odczytaj początkowy stan pinu CLK enkodera
  prev_CLK_state1 = digitalRead(CLK_PIN1);
  prev_CLK_state2 = digitalRead(CLK_PIN2);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);  // Konfiguruj pinout dla interfejsu I2S audio
  audio.setVolume(volumeValue);                  // Ustaw głośność na podstawie wartości zmiennej volumeValue w zakresie 0...21

  // Inicjalizuj interfejs SPI wyświetlacza
  SPI.begin(SPI_SCK_OLED, SPI_MISO_OLED, SPI_MOSI_OLED);
  SPI.setFrequency(1000000);



  // Inicjalizacja SPI z nowymi pinami dla czytnika kart SD
  customSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);  // SCLK = 45, MISO = 21, MOSI = 48, CS = 47
  // Inicjalizuj wyświetlacz i odczekaj 250 milisekund na włączenie
  u8g2.begin();
  delay(250);
  // Powitanie na wyswietlaczu:

  u8g2.drawXBMP(0, 5, notes_width, notes_height, notes);  // obrazek - nutki
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(58, 17, "Internet Radio");
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(226, 62, softwareRev);
  u8g2.sendBuffer();

  // Inicjalizacja karty SD
  if (!SD.begin(SD_CS, customSPI)) {
    // Informacja na wyswietlaczu o problemach lub braku karty SD
    Serial.println("Błąd inicjalizacji karty SD!");
    //u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 62, "SD card error - check slot");
    u8g2.setDrawColor(0);
    u8g2.drawBox(202, 5, 65, 50);
    u8g2.setDrawColor(1);
    u8g2.drawXBMP(220, 3, 30, 40, sdcard);  // ikona SD karty
    u8g2.sendBuffer();
    return;
  }
  Serial.println("Karta SD zainicjalizowana pomyślnie.");
  Serial.print("Numer seryjny ESP:");
  Serial.println(ESP.getEfuseMac());

  //u8g2.setFont(u8g2_font_ncenB18_tr);
  //u8g2.setFont(DotMatrix13pl);
  //u8g2.setFont(u8g2_font_fub14_tf);

  //u8g2.drawStr(5, 32, "Internet Radio");
  //u8g2.sendBuffer();
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(5, 62, "Connecting to network...");
  u8g2.sendBuffer();

  button2.setDebounceTime(50);  // Ustawienie czasu debouncingu dla przycisku enkodera 2
 
  // Inicjalizacja WiFiManagera
  wifiManager.setConfigPortalBlocking(false);

  readStationFromSD();
  previous_bank_nr = bank_nr;  // wyrównanie wartości przy stacie radia aby nie podmienic bank_nr na wartość 0 po pierwszym upływie czasu menu
  Serial.print("debug...wartość bank_nr:");
  Serial.println(bank_nr);
  Serial.print("debug...wartość previous_bank_nr:");
  Serial.println(previous_bank_nr);

  // Rozpoczęcie konfiguracji Wi-Fi i połączenie z siecią, jeśli konieczne
  if (wifiManager.autoConnect("ESP Internet Radio")) {
    Serial.println("Połączono z siecią WiFi");
    //u8g2.clearBuffer();
    //u8g2.setFont(DotMatrix13pl);
    //u8g2.setFont(u8g2_font_fub14_tf);
    //u8g2.drawStr(5, 32, "WiFi Connected");
    currentIP = WiFi.localIP().toString();  //konwersja IP na string
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 62, "                                   ");  // czyszczenie lini spacjami
    u8g2.sendBuffer();
    u8g2.drawStr(5, 62, "WiFi Connected IP:");  //wyswietlenie IP
    u8g2.drawStr(115, 62, currentIP.c_str());   //wyswietlenie IP
    u8g2.sendBuffer();
    delay(2000);  // odczekaj 2 sek przed wymazaniem numeru IP

    u8g2.setFont(spleen6x12PL);
    u8g2.clearBuffer();
    u8g2.drawStr(10, 25, "Time synchronization...");


    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //Serial.print("Syncrhonizacja zegara - status:");
    //Serial.println(sntp_get_sync_status());

    // while (syncStatus != SNTP_SYNC_STATUS_COMPLETED)
    // {
    //  syncStatus = sntp_get_sync_status();
    //}

    timer1.attach(1, updateTimerFlag);  // Ustaw timer, aby wywoływał funkcję updateTimer co sekundę
    //timer1.attach(1, updateTimer);   // Ustaw timer, aby wywoływał funkcję updateTimer co sekundę
    //timer2.attach(0.00005,analyzePulseFromIR); // 100us
    //timer2.attach(60, getWeatherData);   // Ustaw timer, aby wywoływał funkcję getWeatherData co 60 sekund
    //timer3.attach(10, switchWeatherData);   // Ustaw timer, aby wywoływał funkcję switchWeatherData co 10 sekund
   
    //u8g2.setFont(spleen6x12PL);
    //u8g2.clearBuffer();
    //u8g2.drawStr(10, 10, "Bank ");
    //u8g2.drawStr(40, 10, String(bank_nr).c_str());
    //u8g2.drawStr(10, 23, "Loading station from:");
    fetchStationsFromServer();
    changeStation();
    //getWeatherData();
    //wifiManager.startWebPortal();
    //previous_bank_nr = bank_nr; // wyrownanie wartosci pamieci poprzedniego banku
    server.begin();
    currentSelection = station_nr - 1; // ustawiamy stacje na liscie na obecnie odtwarzaczną przy starcie radia
    firstVisibleLine = currentSelection + 1; // pierwsza widoczna lina to grająca stacja przy starcie
    if (currentSelection + 1 >= stationsCount - 1) 
    {
     firstVisibleLine = currentSelection - 3;
    }


  } 
  else 
  {
    Serial.println("Brak połączenia z siecią WiFi");  // W przypadku braku polaczenia wifi - wyslij komunikat na serial
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 13, "No network connection");  // W przypadku braku polaczenia wifi - wyswietl komunikat na wyswietlaczu OLED
    u8g2.drawStr(5, 26, "Connect to WiFi: ESP Internet Radio");
    u8g2.drawStr(5, 39, "Open http://192.168.4.1");
    u8g2.sendBuffer();
  }
  //wifiManager.setConfigPortalBlocking(true);
  //displayRadio();
  
}

void loop() {
  wifiManager.process();  // WiFi manager
  audio.loop();           // Wykonuje główną pętlę dla obiektu audio (np. odtwarzanie dźwięku, obsługa audio)
  button1.loop();         // Wykonuje pętlę dla obiektu button1 (sprawdza stan przycisku z enkodera 1)
  button2.loop();         // Wykonuje pętlę dla obiektu button2 (sprawdza stan przycisku z enkodera 2)
  handleButtons();        // Wywołuje funkcję obsługującą przyciski i wykonuje odpowiednie akcje (np. zmiana opcji, wejście do menu)
  webServer();            // Uruchamiamy Web serwer
 

  CLK_state1 = digitalRead(CLK_PIN1);  // Odczytanie aktualnego stanu pinu CLK enkodera 1
  if (CLK_state1 != prev_CLK_state1 && CLK_state1 == HIGH) {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();
    if (menuEnable == true)  // Przewijanie menu prawym enkoderem
    {
      int DT_state1 = digitalRead(DT_PIN1);
      switch (currentOption) {
        case PLAY_FILES:
          if (DT_state1 == HIGH) {
            currentOption = PLAY_FILES;
          } else {
            currentOption = INTERNET_RADIO;
          }
          break;

        case INTERNET_RADIO:
          if (DT_state1 == HIGH) {
            currentOption = PLAY_FILES;
          } else {
            currentOption = INTERNET_RADIO;
          }
          break;


      }
      displayMenu();
    }

    else  // Regulacja głośności
    {
      if (digitalRead(DT_PIN1) == HIGH) {
        volumeValue--;
        if (volumeValue < 1) {
          volumeValue = 1;
        }
      } else {
        volumeValue++;
        if (volumeValue > 21) {
          volumeValue = 21;
        }
      }
      Serial.print("Wartość głośności: ");
      Serial.println(volumeValue);
      audio.setVolume(volumeValue);  // zakres 0...21

      // Używamy funkcji String() do konwersji liczby na tekst
      String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_fub14_tf);
      u8g2.drawStr(65, 33, "VOLUME");
      u8g2.drawStr(163, 33, volumeValueStr.c_str());
      u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
      u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
      u8g2.sendBuffer();
    }
  }
  prev_CLK_state1 = CLK_state1;

  CLK_state2 = digitalRead(CLK_PIN2);                       // Odczytanie aktualnego stanu pinu CLK enkodera 2
  if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH)  // Sprawdzenie, czy stan CLK zmienił się na wysoki
  {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();

    if ((currentOption == INTERNET_RADIO) && (volumeSet == false) && (bankMenuEnable == false))  // Przewijanie listy stacji radiowych
    {
      station_nr = currentSelection + 1;
      if (digitalRead(DT_PIN2) == HIGH) {
        station_nr--;
        if (station_nr < 1) {
          station_nr = 1;
        }
        Serial.print("Numer stacji do tyłu: ");
        Serial.println(station_nr);
        scrollUp();
      } else {
        station_nr++;
        if (station_nr > stationsCount) {
          station_nr = stationsCount;
        }
        Serial.print("Numer stacji do przodu: ");
        Serial.println(station_nr);
        scrollDown();
      }
      displayStations();
    } 
    else 
    {
      if ((currentOption == INTERNET_RADIO) && (bankMenuEnable == false)) {

        if (digitalRead(DT_PIN2) == HIGH) {
          volumeValue--;
          if (volumeValue < 1) {
            volumeValue = 1;
          }

        } else {
          volumeValue++;
          if (volumeValue > 21) {
            volumeValue = 21;
          }
        }

        Serial.print("Wartość głośności: ");
        Serial.println(volumeValue);
        audio.setVolume(volumeValue);  // zakres 0...21

        // Używamy funkcji String() do konwersji liczby na tekst
        String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_fub14_tf);
        u8g2.drawStr(65, 33, "VOLUME");
        u8g2.drawStr(163, 33, volumeValueStr.c_str());
        u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
        u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
        u8g2.sendBuffer();
        volumeSet = true;
      }
    }

    if (bankMenuEnable == true)  // Przewijanie listy banków stacji radiowych
    {
      if (digitalRead(DT_PIN2) == HIGH) {
        bank_nr--;
        if (bank_nr < 1) {
          bank_nr = 16;
        }
      } else {
        bank_nr++;
        if (bank_nr > 16) {
          bank_nr = 1;
        }
      }
      // Używamy funkcji String() do konwersji liczby na tekst
      String bankNrStr = String(bank_nr);  // Zamiana liczby na ciąg znaków

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_fub14_tf);
      u8g2.drawStr(80, 33, "BANK:");
      u8g2.drawStr(145, 33, String(bank_nr).c_str());

      u8g2.drawRFrame(21, 42, 214, 14, 3);                // Stała ramka z zaokrąglonymi rogami
      u8g2.drawRBox((bank_nr * 13) + 10, 44, 15, 10, 2);  // wypełnienie o szerokosci
      u8g2.sendBuffer();
      x = 0;
      volumeSet = false;
    }
  }
  prev_CLK_state2 = CLK_state2;


  if (displayActive && (millis() - displayStartTime >= displayTimeout))  // Przywracanie poprzedniej zawartości ekranu po 6 sekundach
  {
    displayActive = false;
    timeDisplay = true;
    listedStations = false;
    menuEnable = false;
    volumeSet = false;
    bankMenuEnable = false;
    bankNetworkUpdate = false;
    Serial.print("Wprowadzono cyfry:");
    Serial.print(rcInputDigit1);
    Serial.println(rcInputDigit2); 
    rcInputDigit1 = 0xFF; // czyscimy cyfre 1, flaga pustej zmiennej to FF
    rcInputDigit2 = 0xFF; // czyscimy cyfre 2, flaga pustej zmiennej to FF
    currentOption = INTERNET_RADIO;
    station_nr = stationFromBuffer;
    displayRadio();
  }

  if ((currentOption == PLAY_FILES) && (button1.isPressed()) && (menuEnable == true)) {
    if (!SD.begin(SD_CS)) {
      Serial.println("Błąd inicjalizacji karty SD!");
      return;
    }
    folderIndex = 0;
    currentSelection = 0;
    firstVisibleLine = 1;
    //listDirectories("/music");
    listDirectories(currentDirectory.c_str());
    audio.stopSong();
    playFromSelectedFolder();
  }

  if ((currentOption == INTERNET_RADIO) && (button1.isPressed()) && (menuEnable == true)) {
    menuEnable = false;
    changeStation();
  }

  if ((button1.isPressed()) && (bankMenuEnable == true)) {
    bankMenuEnable = false;
    previous_bank_nr = bank_nr;
    volumeSet = false;

    u8g2.setFont(spleen6x12PL);
    u8g2.clearBuffer();
    u8g2.drawStr(10, 10, "Bank ");
    u8g2.drawStr(40, 10, String(bank_nr).c_str());
    u8g2.drawStr(10, 23, "Update station from:");
    u8g2.sendBuffer();
    bankNetworkUpdate = true;
    currentSelection = 0;
    firstVisibleLine = 0;
    station_nr = 1;
    currentOption = INTERNET_RADIO;

    fetchStationsFromServer();
    changeStation();
    bankNetworkUpdate = false;
    u8g2.clearBuffer();
    //u8g2.setFont(u8g2_font_spleen6x12_mr);
    //u8g2.sendBuffer();
  }


  if ((currentOption == INTERNET_RADIO) && (button2.isReleased()) && (listedStations == true)) {
    listedStations = false;
    volumeSet = false;
    changeStation();
  }

  if ((currentOption == INTERNET_RADIO) && (button2.isPressed()) && (listedStations == false) && (bankMenuEnable == false)) {
    displayStartTime = millis();
    timeDisplay = false;
    volumeSet = true;
    displayActive = true;

    Serial.println("debug--Wcisnieto enkoder 1 - Volume");

    Serial.print("Wartość głośności: ");
    Serial.println(volumeValue);
    audio.setVolume(volumeValue);  // zakres 0...21

    // Używamy funkcji String() do konwersji liczby na tekst
    String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_fub14_tf);
    u8g2.drawStr(65, 33, "VOLUME");
    u8g2.drawStr(163, 33, volumeValueStr.c_str());
    u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
    u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);  // Progress bar głosnosci
    u8g2.sendBuffer();
  }


  if ((button2.isPressed()) && (bankMenuEnable == true)) {
    previous_bank_nr = bank_nr;


    //u8g2.setFont(spleen6x12PL);
    //u8g2.clearBuffer();
    //u8g2.drawStr(10, 10, "Bank ");
    //u8g2.drawStr(40, 10, String(bank_nr).c_str());
    //u8g2.drawStr(10, 23, "Loading station from:");
    //u8g2.sendBuffer();

    //currentSelection = 0;
    //firstVisibleLine = 0;
    //station_nr = 1;
    currentOption = INTERNET_RADIO;

    fetchStationsFromServer();
    changeStation();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_spleen6x12_mr);
    Serial.print("debug--wartość flagi Volume set:");
    Serial.println(volumeSet);
    volumeSet = false;
    bankMenuEnable = false;
  }

  if (action4Taken = false)  // aktualizujemy zegar i wyswietlacz ale tylko raz przy starcie zanim wejdziemy do funkcji millis odswiezania scrollera
  {
    displayRadio();
    updateTimer();
    action4Taken = true;
  }
  
 
  
  if (bit_count == 32) // sprawdzamy czy odczytalismy w przerwaniu pełne 32 bity kodu IR NEC
  {
    if (ir_code != 0) // && (ir_code != 0xF0F0F0F0)
    {
      detachInterrupt(recv_pin); // rozpinay przerwanie
      Serial.print("Kod NEC OK:");
      Serial.print(ir_code, HEX);
      ir_code = reverse_bits(ir_code,32);     // rotacja bitów zmiana z LSB-MSB na MSB-LSB
      Serial.print("  MSB-LSB: ");
      Serial.print(ir_code, HEX);
    
      uint8_t CMD = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
      uint8_t ADDR = ir_code & 0xFF;        // Czwarty bajt (inwersja komendy)
      
      Serial.print("  ADR:");
      Serial.print(ADDR, HEX);
      Serial.print(" CMD:");
      Serial.println(CMD, HEX);
      ir_code = ADDR << 8 | CMD;      // Łączymy ADDR i CMD w jedną zmienną 0xDDRCMD

      Serial.print("debug-- puls 9ms:");
      Serial.print(pulse_duration_9ms);
      Serial.print("  4.5ms:");
      Serial.print(pulse_duration_4_5ms);
      Serial.print("  1690us:");
      Serial.print(pulse_duration_1690us);
      Serial.print("  690us:");
      Serial.println(pulse_duration_690us);
      
      if (ir_code == rcCmdVolumeUp)      // V+
      { 
        //Serial.println("debug--IR przycisk Vol+");
        volumeUp();
      }
      else if (ir_code == rcCmdVolumeDown) // V-
      {  
        //Serial.println("debug--IR przycisk Vol-");
        volumeDown();
      }
      else if (ir_code == rcCmdArrowRight) // Przycisk w górę 9D62D02F  9D62E11E
      {  
        if (bankMenuEnable == true)
        {
          bank_nr++;
          if (bank_nr > 16) 
          {
            bank_nr = 1;
          }
        bankMenuDisplay();
        }
        else
        {
          station_nr++;
          if (station_nr > stationsCount) 
          {
            station_nr = stationsCount;
          }
        changeStation();
        }
      }
      else if (ir_code == rcCmdArrowLeft) // Przycisk w dół
      {  
        if (bankMenuEnable == true)
        {
          bank_nr--;
          if (bank_nr < 1) 
          {
            bank_nr = 16;
          }
        bankMenuDisplay();  
        }
        else
        {        
          station_nr--;
          if (station_nr < 1) 
          {
           station_nr = 1;
          }
        changeStation();
        }
      }
      else if ((ir_code == rcCmdArrowUp) && (currentOption == INTERNET_RADIO) && (volumeSet == false))// Przycisk w góre
      {  
        //Serial.println("debug--IR strzalka w gore");
        timeDisplay = false;
        displayActive = true;
        displayStartTime = millis();
        station_nr = currentSelection + 1;
        station_nr--;
        if (station_nr < 1) 
        {
          station_nr = 1;
        }
        //Serial.print("Numer stacji do tyłu: ");
        //Serial.println(station_nr);
        scrollUp(); 
        displayStations();
      }
      else if ((ir_code == rcCmdArrowDown) && (currentOption == INTERNET_RADIO) && (volumeSet == false)) // Przycisk w dół
      {  
        timeDisplay = false;
        displayActive = true;
        displayStartTime = millis();
        station_nr = currentSelection + 1;
        
        station_nr++;
        if (station_nr > stationsCount) 
	      {
          station_nr = stationsCount;
        }
        
        Serial.println(station_nr);
        scrollDown(); 
        displayStations();
      }
      else if (ir_code == rcCmdOk)
      {
        if (bankMenuEnable == true)
        {
          //u8g2.setFont(spleen6x12PL);
          //u8g2.clearBuffer();
          //u8g2.drawStr(10, 10, "Bank ");
          //u8g2.drawStr(40, 10, String(bank_nr).c_str());
          //u8g2.drawStr(10, 23, "Loading station from:");
          //u8g2.sendBuffer();
          fetchStationsFromServer();
          bankMenuEnable = false;
        }
      changeStation(); 
      u8g2.clearBuffer();
       
      } 
      else if (ir_code == rcCmdKey0) {rcInputKey(0);}
      else if (ir_code == rcCmdKey1) {rcInputKey(1);}     
      else if (ir_code == rcCmdKey2) {rcInputKey(2);}     
      else if (ir_code == rcCmdKey3) {rcInputKey(3);}     
      else if (ir_code == rcCmdKey4) {rcInputKey(4);}     
      else if (ir_code == rcCmdKey5) {rcInputKey(5);}     
      else if (ir_code == rcCmdKey6) {rcInputKey(6);}     
      else if (ir_code == rcCmdKey7) {rcInputKey(7);}     
      else if (ir_code == rcCmdKey8) {rcInputKey(8);}     
      else if (ir_code == rcCmdKey9) {rcInputKey(9);}     
      else if (ir_code == rcCmdDirect) {vuMeterOn = !vuMeterOn; displayRadio();}     
      else if (ir_code == rcCmdBankMinus){bankMenuDisplay();}
      else if (ir_code == rcCmdBankPlus){bankMenuDisplay();}
      else if (ir_code == rcCmdMute){debugAudioBuffor = !debugAudioBuffor; }
      else
      {
        Serial.println("Inny przycisk");
      }
    }
    else
    {
      Serial.println("Błąd - kod NEC jest niepoprawny!");

    }
  ir_code = 0;
  bit_count = 0;
  Serial.println("Czas2 - Czas1:");
  Serial.println(czas1);
  Serial.println(czas2);
  czas2 = czas2 - czas1;
  Serial.println(czas2);
  
  attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);
  Serial.print("debug-- Kontrola stosu:");
  Serial.println(uxTaskGetStackHighWaterMark(NULL));

  }

// petla millis oswiezajaca scroller i VU metter, dodatkowo sprawdza flage odswiezenia czasu zegara na ekranie OLED
  if ((millis() - scrollingStationStringTime > scrollingRefresh) && (bankMenuEnable == false) && (menuEnable == false) && (listedStations == false) && (timeDisplay == true)) 
  {
    scrollingStationStringTime = millis();
        
    if (ActionNeedUpdateTime == true) {
      ActionNeedUpdateTime = false;
      updateTimer();
      if (debugAudioBuffor == true)
      { 
        bufforAudioInfo();
      }
    }
  
    displayRadioScroller();  // wykonujemy przewijanie tekstu station stringi przygotowujemy bufor ekranu
    
    if (vuMeterOn == true && flac == false)  //&& (flac == false) jesli właczone sa wskazniki VU to rysujemy, dla stacji FLAC wyłaczamy aby nie bylo cieci w streamie
    {
      vuMeter();
    }
    u8g2.sendBuffer();  // rysujemy zawartosc Scrollera i VU jesli właczone
  }  
}