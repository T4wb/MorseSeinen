// PIT2
// Serial Seinen
// Tawwab Djalielie

#include <LiquidCrystal.h>
#include <Wire.h>

//Define unit length in ms
const int UNIT_LENGTH = 250;

//Build a struct with the morse code mapping
static const struct {
  const char letter, *code;
} MorseMap[] =
{
  { 'A', ".-" },
  { 'B', "-..." },
  { 'C', "-.-." },
  { 'D', "-.." },
  { 'E', "." },
  { 'F', "..-." },
  { 'G', "--." },
  { 'H', "...." },
  { 'I', ".." },
  { 'J', ".---" },
  { 'K', ".-.-" },
  { 'L', ".-.." },
  { 'M', "--" },
  { 'N', "-." },
  { 'O', "---" },
  { 'P', ".--." },
  { 'Q', "--.-" },
  { 'R', ".-." },
  { 'S', "..." },
  { 'T', "-" },
  { 'U', "..-" },
  { 'V', "...-" },
  { 'W', ".--" },
  { 'X', "-..-" },
  { 'Y', "-.--" },
  { 'Z', "--.." },
  { ' ', "     " }, //Gap between word, seven units

  { '1', ".----" },
  { '2', "..---" },
  { '3', "...--" },
  { '4', "....-" },
  { '5', "....." },
  { '6', "-...." },
  { '7', "--..." },
  { '8', "---.." },
  { '9', "----." },
  { '0', "-----" },

  { '.', "·–·–·–" },
  { ',', "--..--" },
  { '?', "..--.." },
  { '!', "-.-.--" },
  { ':', "---..." },
  { ';', "-.-.-." },
  { '(', "-.--." },
  { ')', "-.--.-" },
  { '"', ".-..-." },
  { '@', ".--.-." },
  { '&', ".-..." },
};

bool verzendmodus = false;
String bericht_morse;
String bericht_tekst;
String letter;
String woord_morse;
unsigned long changeTime;

const int led = 1;
const int ontvang_beschikbaar = 8;
const int verzend_beschikbaar = 9;
const int morse_punt = 10;
const int morse_streep = 11;
const int pauze = 12;
const int backspace = 13;

LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

void setup()
{
  pinMode(morse_punt, INPUT);
  pinMode(morse_streep, INPUT);
  pinMode(pauze, INPUT);
  pinMode(backspace, INPUT);
  pinMode(ontvang_beschikbaar, INPUT);
  pinMode(verzend_beschikbaar, OUTPUT);
  pinMode(led, OUTPUT);
  
  lcd.begin(16, 2);
  lcd.blink();

  digitalWrite(led, LOW );
}

void loop()
{
  if (digitalRead(backspace) == HIGH && digitalRead(pauze) == HIGH && (millis() - changeTime) > 200) {
    verzendmodus = !verzendmodus;
    changeTime = millis();
  }

  if (verzendmodus == true) {
    verzendmodus_geactiveerd();

    if (verzendmodus == false) {
      ontvangstmodus_geactiveerd();
    }
  }
}

void verzendmodus_geactiveerd() {
  if ((millis() - changeTime) > 150)
  {
    if (digitalRead(morse_punt) == HIGH && (millis() - changeTime) < 750) {
      controleer_invoer('.');
      changeTime = millis();
    }

    if (digitalRead(morse_streep) == HIGH && (millis() - changeTime) < 750) {
      controleer_invoer('-');
      changeTime = millis();
    }

    if (digitalRead(pauze) == HIGH && (millis() - changeTime) < 750) {
      if (bericht_tekst[bericht_tekst.length() - 1] != ' ')
      {
        letter = "     ";
        karakter_toevoegen();
        update_scherm();
        letter = "";
      }
      changeTime = millis();
    }

    if (digitalRead(backspace) == HIGH && (millis() - changeTime) < 750 && bericht_morse != "") {
      karakter_verwijderen();
      update_scherm();
      changeTime = millis();
    }

    if (digitalRead(morse_punt) == HIGH && digitalRead(morse_streep) == HIGH && bericht_morse != "")
    {
      bericht_opsplitsen();
      reset_na_verzonden();
    }

    if ((millis() - changeTime) > 750)
    {
      if (letter[letter.length() - 1] == '.' || letter[letter.length() - 1] == '-') // bericht_morse.length() != 0 && bericht_morse[bericht_morse.length() - 1] != ' ', voorkomt dat tekst op scherm standaard start met spatie
      {
        controleer_invoer(' ');
      }
      changeTime = millis();
    }

    if (digitalRead(backspace) == HIGH && digitalRead(pauze) == HIGH && bericht_morse != "") {
      verzendmodus = !verzendmodus;
      changeTime = millis();
    }
  }
}

void ontvangstmodus_geactiveerd() {
  while (digitalRead(ontvang_beschikbaar) == HIGH) {
    bericht_lezen();
    update_scherm();
  }

  if (digitalRead(ontvang_beschikbaar) == LOW) {
    led_knipperen();
    bericht_morse = "";
  }
}

void bericht_lezen() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int howMany) {
  while (1 <= Wire.available()) {
    char c = Wire.read();
    woord_morse += c;
    bericht_morse += woord_morse;
    woord_morse = "";
  }
}

void led_knipperen() {
  for (int i = 0; i < bericht_morse.length(); i++)
  {
    if (bericht_morse[i] == '.')
    {
      digitalWrite(led, HIGH );
      delay( UNIT_LENGTH );
      digitalWrite(led, LOW );
      delay( UNIT_LENGTH );
    }
    else if (bericht_morse[i] == '-')
    {
      digitalWrite(led, HIGH );
      delay( UNIT_LENGTH * 3 );
      digitalWrite(led, LOW );
      delay( UNIT_LENGTH );
    }
    else if (bericht_morse[i] == ' ')
    {
      delay( UNIT_LENGTH );
    }
  }
}

void bericht_opsplitsen()
{
  digitalWrite(verzend_beschikbaar, HIGH);

  for (int j = 0; j < bericht_morse.length() - 1; j += 31)
  {
    woord_morse = bericht_morse.substring(j, j + 31);
    bericht_verzenden();
  }
  digitalWrite(verzend_beschikbaar, LOW);
}

void bericht_verzenden()
{
  Wire.begin();
  char buffer[32];
  Wire.beginTransmission(8);
  woord_morse.toCharArray(buffer, 32);
  Wire.write(buffer);
  Wire.endTransmission();
  delay(200);
}

void reset_na_verzonden()
{
  letter = "";
  bericht_morse = "";
  bericht_tekst = "";
  lcd.clear();
  lcd.print("Verzonden");
  delay(2000);
}

void controleer_invoer(char input)
{
  if (input == ' ')
  {
    for ( int i = 0; i < sizeof MorseMap / sizeof * MorseMap; ++i )
    {
      if (letter == MorseMap[i].code)
      {
        karakter_toevoegen();
        update_scherm();
        break;
      }

    }
    letter = "";
  }
  else {
    letter += input;
  }
}

void karakter_toevoegen()
{
  bericht_morse += letter;
  bericht_morse += ' ';
}

void karakter_verwijderen()
{
  int k = bericht_morse.length() - 1;

  if (bericht_morse[k - 1] == ' ') {
    k -= 5;
  }
  else {
    // karakter wissen (geen spatie)
    while (k > 0)
    {
      if (bericht_morse[k - 1] == ' ')
      {
        break;
      }
      k--;
    }
  }
  bericht_morse = bericht_morse.substring(0, k);;
}

void update_scherm()
{
  lcd.clear();
  bericht_tekst = decode(bericht_morse);
  lcd.print(bericht_tekst);
}

String decode(String morse)
{
  String msg = "";

  if (morse[morse.length() - 1] != ' ')
  {
    morse += ' ';
  }

  int lastPos = 0;
  int pos = morse.indexOf(' ');
  while ( lastPos <= morse.lastIndexOf(' ') )
  {
    for ( int i = 0; i < sizeof MorseMap / sizeof * MorseMap; ++i )
    {
      if ( morse.substring(lastPos, pos) == MorseMap[i].code )
      {
        msg += MorseMap[i].letter;
      }
    }

    lastPos = pos + 1;
    pos = morse.indexOf(' ', lastPos);

    // Handle white-spaces between words (7 spaces)
    while ( morse[lastPos] == ' ' && morse[pos + 1] == ' ' )
    {
      pos ++;
    }
  }
  return msg;
}
