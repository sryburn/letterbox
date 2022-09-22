SYSTEM_THREAD(ENABLED);

#define lid D1
#define door D0

bool lidOpenFlag = false;
bool doorOpenFlag = false;
bool publishMail = false;
bool publishCleared = false;
bool hasPublished = false;
bool battPublished = false;
bool readyToPublish = false;
int version = 34;
unsigned long lastPublish = 0;
char soc[10]; //soc- state of charge

Timer publishTimer(1000, publishData); //publish every second (to avoid issues with switch bounce)

void setup() {
    pinMode(lid, INPUT_PULLUP);
    pinMode(door, INPUT_PULLUP);
    Particle.variable("version", version);
    publishTimer.start();
}

void loop() {
    
    if (digitalRead(lid) == HIGH && !lidOpenFlag){
        lidOpenFlag = true;
        publishMail = true;
    }
    
    if (digitalRead(lid) == LOW && lidOpenFlag){
        lidOpenFlag = false;
    }
    
    if (digitalRead(door) == HIGH && !doorOpenFlag){
        doorOpenFlag = true;
        publishCleared = true;
    }
    
    if (digitalRead(door) == LOW && doorOpenFlag){
        doorOpenFlag = false;
    }
    
    if (publishMail && readyToPublish){  
        readyToPublish = false;
        bool mailSuccess = Particle.publish("mail", "true"); //returns quickly but success != delivered, can be rate limited by particle server.
        if (mailSuccess){
            publishMail = false;
            lastPublish = millis();
            hasPublished = true;
            
        }
    }
    
    if (publishCleared && readyToPublish){ 
        readyToPublish = false;
        bool clearedSuccess = Particle.publish("mail", "false");
        if (clearedSuccess){
            publishCleared = false;
            lastPublish = millis();
            hasPublished = true;
            
        }
    }
    
    //publish the battery state once, 3 secs after every wakeup
    if (((millis()-lastPublish) > 3000) && !battPublished){
        float batterySoc = System.batteryCharge();
        sprintf(soc, "%.0f", batterySoc);
        bool battSuccess = Particle.publish("batt", soc);
        if (battSuccess){
          battPublished = true;  
        }
        
    }
    
    // sleep if it's been 1 min since anything happened, or otherwise after 5 mins (in case of restarts)
    if ((((millis() - lastPublish) > 60000) && hasPublished == true) || ((millis() - lastPublish) > 300000)) {
        lastPublish = millis();
        hasPublished = false;
        battPublished = false;
        SystemSleepConfiguration config;
        config.mode(SystemSleepMode::ULTRA_LOW_POWER)
        .gpio(lid, CHANGE)
        .gpio(door, CHANGE);
        System.sleep(config);
    }

}

void publishData(){
    readyToPublish = true;
}