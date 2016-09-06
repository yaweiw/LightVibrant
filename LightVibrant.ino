#include "LightVibrant.h"

// here our mmapped data will be accessed
MMAPDATA_HANDLE p_mmapData;

// define pins
const int ledPinGND = 2;
const int ledPinSig = 3;
const int ledPin = 13;

// global variable
const int threshold = 100;
float resistance;

void setup() {
  // serial
  Serial.begin(9600);
  // led pins
  pinMode(ledPin,OUTPUT);
  pinMode(ledPinGND,OUTPUT);
  pinMode(ledPinSig,OUTPUT);
  // file descriptor for memory mapped file
  int fd_mmapFile;
  // open file and mmap mmapData
  fd_mmapFile = open(mmapFilePath, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd_mmapFile == -1) Serial.println("couldn't open mmap file"); 
  // make the file the right size - exit if this fails
  if (ftruncate(fd_mmapFile, sizeof(MMAPDATA)) == -1) Serial.println("couldn' modify mmap file");
  // memory map the file to the data
  // assert(filesize not modified during execution)
  p_mmapData = (MMAPDATA_HANDLE)(mmap(NULL, sizeof(MMAPDATA), PROT_READ | PROT_WRITE, MAP_SHARED, fd_mmapFile, 0));  
  if (p_mmapData == MAP_FAILED) Serial.println("couldn't mmap"); 
  // initialize mutex
  pthread_mutexattr_t mutexattr; 
  if (pthread_mutexattr_init(&mutexattr) == -1) Serial.println("pthread_mutexattr_init");
  if (pthread_mutexattr_setrobust(&mutexattr, PTHREAD_MUTEX_ROBUST) == -1) Serial.println("pthread_mutexattr_setrobust");
  if (pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) == -1) Serial.println("pthread_mutexattr_setpshared");
  if (pthread_mutex_init(&(p_mmapData->mutex), &mutexattr) == -1) Serial.println("pthread_mutex_init");

  // initialize condition variable
  pthread_condattr_t condattr;
  if (pthread_condattr_init(&condattr) == -1) Serial.println("pthread_condattr_init");
  if (pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED) == -1) Serial.println("pthread_condattr_setpshared");
  if (pthread_cond_init(&(p_mmapData->cond), &condattr) == -1) Serial.println("pthread_cond_init");

} 
 
void loop() {
  int vibrantVal = 0;
  int lightVal = 0;
  sample(1000, &vibrantVal, &lightVal);
  // float resistance = (float)(1023-lightVal)*10/lightVal;
  // digitalWrite(ledPinGND,LOW);
  Serial.println("vibration sensor value:");
  Serial.println(vibrantVal);
  Serial.println("light sensor resistance value:");
  Serial.println(lightVal);
  
  Serial.println("in front of pthread_mutex_lock");
  // block until we are signalled from native code
  if (pthread_mutex_lock(&(p_mmapData->mutex)) != 0) Serial.println("pthread_mutex_lock");
  Serial.println("Write light and vibrant value to p_mmapData");
  p_mmapData->light = lightVal;
  p_mmapData->vibrant = vibrantVal;
  if (pthread_mutex_unlock(&(p_mmapData->mutex)) != 0) Serial.println("pthread_mutex_unlock");
  if (pthread_cond_signal(&(p_mmapData->cond)) != 0) Serial.println("pthread_cond_signal");
}

void sample(int interval, int* vibrant, int* light)
{
  int i, j = 0;
 
  // sample every 10 ms
  for(i = 0; i < interval; i = i + 10)
  {
    *vibrant = *vibrant + analogRead(A0);
    *light = *light + analogRead(A1);
    j++;
    delay(10);
  }
  *vibrant = (int)(*vibrant/j);
  *light = (int)(*light/j);
}

