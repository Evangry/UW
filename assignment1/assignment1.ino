const int BUTTON_PIN = 13;
const int DEBOUNCE_MS = 30;
const int MODE_COUNT = 3;

const int RED_PIN = 6;
const int GREEN_PIN  = 5;
const int BLUE_PIN  = 3;

const int COLOR_TIMER_MS = 20;
const int COLOR_MAX_VALUE = 255;
const int PHOTO_PIN = A0;
const float PHOTO_INPUT_MULTIPLIER = 0.04;

const int MIC_PIN = A2;
const int SOUND_MEMORY_SIZE = 500;
const float AUDIO_INPUT_MULTIPLIER = 0.001;

const int PENCIL_PIN = A4;
const int RED_BORDER = 1;
const int YELLOW_BORDER = 2;
const int GREEN_BORDER = 3;
const int CYAN_BORDER = 5;
const int BLUE_BORDER = 9;
const int MAGENTA_BORDER = 14;


int savedButtonValue;
unsigned long buttonReadMillis;
int mode;

unsigned long colorChangeMillis;
int currentColor[] = {255, 0, 0};
int favoriteColor[] = {255, 255, 255};
float currentBrightness = 1.0;

int fadingInRGB = 1;
int fadeSpeed = 5;

int soundMemory[SOUND_MEMORY_SIZE];
unsigned long numSoundsRecorded = 0;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);
  pinMode(MIC_PIN, INPUT);
  pinMode(PENCIL_PIN, INPUT);

  mode = 0;
  colorChangeMillis = millis();
}

//currently not setting timers only at the end. could be a bug if a loop takes too long.
void loop() {
  if (mode == 0) {
    crossFadeLoop();
  }
  else if (mode == 1) {
    audioResponseLoop();
  }
  else if (mode ==2) {
    //creative
    pencilLoop();
  }


  if (millis() >= buttonReadMillis) {
    int buttonValue = digitalRead(BUTTON_PIN);
    if (buttonValue != savedButtonValue) {
      //Serial.print("new button read: ");
      //Serial.println(buttonValue);
      savedButtonValue = buttonValue;
      if (buttonValue == 1) {
        mode = (mode + 1) % MODE_COUNT;
        //Serial.print("entering mode ");
        //Serial.println(mode);
        if (mode==2) {
          audioResponseBegin();
        }
      }
      buttonReadMillis = millis() + DEBOUNCE_MS;
    }
  }
}

  

void crossFadeLoop() {
  if (millis() >= colorChangeMillis) {
    currentColor[fadingInRGB] += fadeSpeed;
    currentColor[(fadingInRGB + 2) % 3] -= fadeSpeed;
    currentBrightness = max(0.0, 1.0 - analogRead(PHOTO_PIN) * PHOTO_INPUT_MULTIPLIER);

    //fadingInRGB has peaked, so it's time for it to fade out and have another fade in.
    if(currentColor[fadingInRGB] > COLOR_MAX_VALUE){
      //Serial.print( "brightness:");
      //Serial.println(analogRead(PHOTO_PIN) * PHOTO_INPUT_MULTIPLIER);

      fadingInRGB = (fadingInRGB + 1) % 3;
      currentColor[(fadingInRGB) % 3] = 0;
      currentColor[(fadingInRGB + 1) % 3] = 0;
      currentColor[(fadingInRGB + 2) % 3] = COLOR_MAX_VALUE;
    }
    //set the color and schedule the next change
    writeColor(currentColor, max(0.0, currentBrightness));
    colorChangeMillis = millis() + COLOR_TIMER_MS;
  }
}

void audioResponseBegin() {
  memset(soundMemory, 0, sizeof(soundMemory));
  numSoundsRecorded = 0;
}

void audioResponseLoop() {

  int soundIndex = numSoundsRecorded % SOUND_MEMORY_SIZE;
  int read = analogRead(MIC_PIN);
  soundMemory[soundIndex] = read;
  numSoundsRecorded ++;
  if (millis() >= colorChangeMillis) {
    int highest = 0;
    int lowest = 0;
    for (int i = 0; i < min(numSoundsRecorded, SOUND_MEMORY_SIZE); i++) {
      if (soundMemory[i] > highest) {
        highest = soundMemory[i];
      }
      else if (soundMemory[i] < lowest)  {
        lowest = soundMemory[i];
      }
    }
    int loudest = highest - lowest;
    //Serial.print("micLoudest:");
    //Serial.println(loudest);

    currentBrightness = min(1.0, loudest * AUDIO_INPUT_MULTIPLIER);
    writeColor(favoriteColor, max(0.0, currentBrightness));
    colorChangeMillis = millis() + COLOR_TIMER_MS;
  }
}

void pencilLoop() {
  if (millis() >= colorChangeMillis) {
    int pencilRead = analogRead(PENCIL_PIN);
    Serial.print("pencilPin:");
    Serial.println(pencilRead);

    if (pencilRead < RED_BORDER) {
      favoriteColor[0] = 255;
      favoriteColor[1] = 0;
      favoriteColor[2] = 0;
    }
    else if (pencilRead < YELLOW_BORDER) {
      favoriteColor[0] = 255;
      favoriteColor[1] = 255;
      favoriteColor[2] = 0;
    }
    else if (pencilRead < GREEN_BORDER) {
      favoriteColor[0] = 0;
      favoriteColor[1] = 255;
      favoriteColor[2] = 0;
    }
    else if (pencilRead < CYAN_BORDER) {
      favoriteColor[0] = 0;
      favoriteColor[1] = 255;
      favoriteColor[2] = 255;
    }
    else if (pencilRead < BLUE_BORDER) {
      favoriteColor[0] = 0;
      favoriteColor[1] = 0;
      favoriteColor[2] = 255;
    }
    else if (pencilRead < MAGENTA_BORDER) {
      favoriteColor[0] = 255;
      favoriteColor[1] = 0;
      favoriteColor[2] = 255;
    }
    else {
      favoriteColor[0] = 255;
      favoriteColor[1] = 255;
      favoriteColor[2] = 255;
    }
    currentBrightness = 1.0;
    writeColor(favoriteColor, max(0.0, currentBrightness));
    colorChangeMillis = millis() + COLOR_TIMER_MS;
  }
}

//analog write to 3 color pins in one method. multiplier must be 1.0 or less.
void writeColor(int color[], float multiplier)
{
  analogWrite(RED_PIN, (int) (multiplier * color[0]));
  analogWrite(GREEN_PIN, (int) (multiplier * color[1]));
  analogWrite(BLUE_PIN, (int) (multiplier * color[2]));  
}