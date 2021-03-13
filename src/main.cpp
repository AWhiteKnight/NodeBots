/**
 *  Wrapper to the Arduino framwork
 */
#include <Arduino.h>
#include <logging.h>

#include "NodeBot.h"
#include "BotMesh.h"

// rtc stuff
#ifdef WITH_RTC_PCF8523
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    static RTC_PCF8523 rtc;
#endif
#ifdef WITH_RTC_DS1307
    #ifndef WITH_RTC
        #define WITH_RTC
    #endif
    #include "RTClib.h"
    static RTC_DS1307 rtc;
#endif

#ifdef HAS_WEB_SERVER
    #ifndef HAS_INTERNET_ACCESS
        #define HAS_INTERNET_ACCESS
    #endif
#endif

void serialPrintDateTime();
#ifdef WITH_RTC
    void serialPrintRtcDateTime();
#endif

// Task definitions
#ifdef HAS_INTERNET_ACCESS
    static const char * ntpServer = "pool.ntp.org";
    void getNtpTime();
    Task taskGetNtpTime( 1000UL * 3600UL * 24UL * 5UL, TASK_FOREVER, &getNtpTime );
#endif
#ifdef SERIAL_DEBUG
    void serialPrint();
    Task taskSerialPrint( 1000UL * 60UL * 10UL, TASK_FOREVER, &serialPrint );
#endif

// to control tasks
static Scheduler defaultScheduler;

static timezone tz = { 3600, 3600 };
#ifdef WITH_RTC
    static timeval tv = { 0, 0 };
#endif



void setup() {
    // use default serial speed of boards to see boot messages
    // take care that monitor_speed in platformio.ini is set accordingly
    #ifdef ESP32
        Serial.begin( 115200 );
    #elif defined(ESP8266)
        Serial.begin( 76800 );
    #else
        #error "unknown board"
    #endif

    SERIAL_PRINTLN("setup begin");

    // time setup
    // set timezone values
    configTime(tz.tz_minuteswest, tz.tz_dsttime, nullptr );

    #ifdef WITH_RTC
        rtc.begin();
        // adjust time from compiletime if not already set
        #ifdef WITH_RTC_PCF8523
            if ( !rtc.initialized() || rtc.lostPower() )
        #elif defined(WITH_RTC_DS1307)
            if ( !rtc.isrunning() )
        #else
            #error "unknown RTC"
        #endif
        {
            rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
            SERIAL_PRINTLN( "RTC Time adjusted" );
            #ifdef WITH_RTC_PCF8523
                rtc.start();
            #endif
        }
        #ifdef SERIAL_DEBUG
            serialPrintRtcDateTime();
        #endif
        // set system time from rtc
        tv.tv_sec = rtc.now().unixtime();
        settimeofday( &tv, &tz );
    #endif
    #ifdef SERIAL_DEBUG
        serialPrintDateTime();
    #endif

    // add Tasks
    #ifdef HAS_INTERNET_ACCESS
        defaultScheduler.addTask( taskGetNtpTime );
        taskGetNtpTime.enableDelayed( 90000UL );
    #endif
    #ifdef SERIAL_DEBUG
        defaultScheduler.addTask( taskSerialPrint );
        taskSerialPrint.enableDelayed( 30000UL );
    #endif

    // setup mesh
    BotMesh::getInstance().setup( &defaultScheduler );

    // setup of the bot
    NodeBot::getInstance().setup( &defaultScheduler );

    SERIAL_PRINTLN("setup end");
}

void loop() {
    // this will run the schedulers as well
    BotMesh::getInstance().update();
}

#ifdef SERIAL_DEBUG
    size_t _print( struct tm * timeinfo, const char * format )
    {
        char buf[64];
        size_t written = strftime( buf, 63, format, timeinfo );
        if(written == 0)  return written;
        return Serial.print(buf);
    }

    size_t _println( struct tm * timeinfo, const char * format )
    {
        size_t n = _print( timeinfo, format );
        n += Serial.println();
        return n;
    }
#endif

bool _getLocalTime( struct tm * info, uint32_t ms = 5000 )
{
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms)
    {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)) return true;
        delay(10);
    }
    return false;
}

#ifdef SERIAL_DEBUG
    void serialPrintDateTime()
    {
        // set system time from rtc
        #ifdef WITH_RTC
            Serial.println( rtc.now().unixtime() );
            tv.tv_sec = rtc.now().unixtime();
            settimeofday( &tv, &tz );
        #endif
        struct tm timeinfo;
        _getLocalTime( &timeinfo );
        _println( &timeinfo, "%A, %B %d %Y %H:%M:%S" );
    }

    #ifdef WITH_RTC
        void serialPrintRtcDateTime()
        {
            Serial.print("RTC: ");

            DateTime now = rtc.now();
        
            Serial.print(now.year(), DEC);
            Serial.print('-');
            Serial.print(now.month(), DEC);
            Serial.print('-');
            Serial.print(now.day(), DEC);
            Serial.print(' ');
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.print(now.second(), DEC);
            Serial.println();
        }
    #endif

    void serialPrint()
    {
        #ifdef WITH_RTC
            serialPrintRtcDateTime();
        #endif
        serialPrintDateTime();
    }
#endif

#ifdef HAS_INTERNET_ACCESS
    void getNtpTime()
    {
        SERIAL_PRINTLN( "NTP-Request" );
        // update time from ntp server
        configTime(tz.tz_minuteswest, tz.tz_dsttime, ntpServer);
        #ifdef WITH_RTC
            rtc.adjust( DateTime( time( nullptr ) ) );
            #ifdef SERIAL_DEBUG
                serialPrintRtcDateTime();
            #endif
        #endif
        #ifdef SERIAL_DEBUG
            serialPrintDateTime();
        #endif
    }
#endif

