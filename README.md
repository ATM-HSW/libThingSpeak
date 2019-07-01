# ThingSpeak Communication Library for ~~Arduino, ESP8266 and ESP32~~ Mbed OS

This library enables an Mbed compatible board to write or read data to or from ThingSpeak, an open data platform for the Internet of Things with MATLAB analytics and visualization.

Examples are found here. But to give you an idea of usage examples for <a href="#typical_write">writing</a> and <a href="#typical_read">reading</a> are shown below. Complete <a href="#documentation">documentation</a> in also shown below.

ThingSpeak offers free data storage and analysis of time-stamped numeric or alphanumeric data. Users can access ThingSpeak by visiting http://thingspeak.com and creating a ThingSpeak user account.

ThingSpeak stores data in channels. Channels support an unlimited number of timestamped observations (think of these as rows in a spreadsheet). Each channel has up to 8 fields (think of these as columns in a speadsheet). Check out this [video](http://www.mathworks.com/videos/introduction-to-thingspeak-107749.html) for an overview.

Channels may be public, where anyone can see the data, or private, where only the owner and select users can read the data. Each channel has an associated Write API Key that is used to control who can write to a channel. In addition, private channels have one or more Read API Keys to control who can read from private channel. An API Key is not required to read from public channels.  Each channel can have up to 8 fields. One field is created by default.

You can visualize and do online analytics of your data on ThingSpeak using the built in version of MATLAB, or use the desktop version of MATLAB to get deeper historical insight. Visit https://www.mathworks.com/hardware-support/thingspeak.html to learn more.

Libraries and examples for Particle devices can be found here: https://github.com/mathworks/thingspeak-particle

## Installation
Please see the examples in this github account.

## Compatible Hardware:

Examples Tested with NUCLEO_F767 - an Mbed compatible board with ethernet connection and Mbed OS 5.12.4. With minor adaption of network initialization code other network connectable boards should be working as well.

# Examples

The library includes several <a href="http://github.com/mathworks/thingspeak-arduino/tree/master/examples">examples</a> to help you get started.

* **ReadField:** Reading from a channel on ThingSpeak (own channel has to be configured).
* **ReadWeatherStation:** Reading weather station data from Mathwork's headquarter weather channel (public channel)
* **WriteField:** Writing a value to a single field on ThingSpeak (own channel has to be configured).
* **WriteChannel:** Writing values to multiple fields of a channel on ThingSpeak (own channel has to be configured).

## <a id="typical_write">Typical Write Example</a>
In this case, write to a field with an incrementing number.   

```
#include "mbed.h"
#include "EthernetInterface.h"

#include "arduino_WString.h"
#include "secrets.h"
//#define THINGSPEAK_DBG_MSG
//#define THINGSPEAK_DBG_HTTP
#include "ThingSpeak.h"

Serial pc(USBTX, USBRX);
NetworkInterface *net;
ThingSpeak thingSpeak;
TCPSocket socket;

nsapi_size_or_error_t result;

int setup() {
#ifdef MBED_MAJOR_VERSION
  pc.printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

  net = new EthernetInterface();
  if (!net) {
      pc.printf("Error! No network inteface found.\n");
      return -1;
  }

  result = net->connect();
  if (result != 0) {
      pc.printf("Error! net->connect() returned: %d\n", result);
      return result;
  }

  nsapi_error_t open_result = socket.open(net);
  if (open_result != 0) {
      pc.printf("Error! socket.open(net) returned: %d\n", open_result);
      return open_result;
  }

  thingSpeak.setSerial(&pc);
  thingSpeak.begin(&socket);  // Initialize ThingSpeak

  return 0;
}

void loop() {
  static uint8_t number=0;

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = thingSpeak.writeField(SECRET_CH_ID, 1, number, SECRET_WRITE_APIKEY);
  if(x == 200){
    pc.printf("Channel update successful.\n");
  }
  else{
    pc.printf("Problem updating channel. HTTP error code %d\n",x);
  }

  // change the value
  number++;
  if(number > 99){
    number = 0;
  }

  wait(20); // Wait 20 seconds to update the channel again
}

int main() {
  if(setup() == 0) 
    while(true) loop();
  else
    while(true) wait(1);
}
```

## <a id="typical_read">Typical Read Example</a>
In this case, read from a public channel and a private channel with an ESP8266.  The public channel is the temperature(F) at MathWorks headquarters.  The private channel is a counter that increments.

 ```
#include "mbed.h"
#include "EthernetInterface.h"
#include "arduino_WString.h"
#include "secrets.h"
//#define THINGSPEAK_DBG_MSG
//#define THINGSPEAK_DBG_HTTP
#include "ThingSpeak.h"

Serial pc(USBTX, USBRX);
NetworkInterface *net;
ThingSpeak thingSpeak;
TCPSocket socket;

// Weather station channel details
unsigned long weatherStationChannelNumber = SECRET_CH_ID_WEATHER_STATION;
unsigned int temperatureFieldNumber = 4;

// Counting channel details
unsigned long counterChannelNumber = SECRET_CH_ID_COUNTER;
const char * myCounterReadAPIKey = SECRET_READ_APIKEY_COUNTER;
unsigned int counterFieldNumber = 1;
nsapi_size_or_error_t result;

int setup() {
#ifdef MBED_MAJOR_VERSION
  pc.printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#endif

  net = new EthernetInterface();
  if (!net) {
      pc.printf("Error! No network interface found.\n");
      return -1;
  }

  result = net->connect();
  if (result != 0) {
      pc.printf("Error! net->connect() returned: %d\n", result);
      return result;
  }

  nsapi_error_t open_result = socket.open(net);
  if (open_result != 0) {
      pc.printf("Error! socket.open(net) returned: %d\n", open_result);
      return open_result;
  }

  thingSpeak.setSerial(&pc);
  thingSpeak.begin(&socket);  // Initialize ThingSpeak

  return 0;
}

void loop() {

  int statusCode = 0;

  // Read in field 4 of the public channel (temperature)
  float fTemperatureInF = thingSpeak.readFloatField(weatherStationChannelNumber, temperatureFieldNumber);

  // Check the status of the read operation to see if it was successful
  statusCode = thingSpeak.getLastReadStatus();
  if(statusCode == 200){
    pc.printf("Temperature at MathWorks HQ: %f deg F\n", fTemperatureInF);
  } else{
    pc.printf("Problem reading channel. HTTP error code %d\n", statusCode);
  }

  wait(15); // No need to read the temperature too often.

  // Read in field 1 of the private channel which is a counter
  long count = thingSpeak.readLongField(counterChannelNumber, counterFieldNumber, myCounterReadAPIKey);

   // Check the status of the read operation to see if it was successful
  statusCode = thingSpeak.getLastReadStatus();
  if(statusCode == 200){
    pc.printf("Counter: %ld\n", count);
  }
  else{
    pc.printf("Problem reading channel. HTTP error code %d\n", statusCode);
  }

  wait(15); // No need to read the counter too often.
}

int main() {
  if(setup() == 0) 
    while(true) loop();
  else
    while(true) wait(1);
}
 ```

# <a id="documentation">Documentation</a>

## begin
Initializes the ThingSpeak library and network settings.
```
bool begin (client) // defaults to ThingSpeak.com
```
```
bool begin (client, port)
```
| Parameter      | Type         | Description                                            |          
|----------------|:-------------|:-------------------------------------------------------|
| client         | Client &     | TCPClient created earlier in the sketch                |
| port           | unsigned int | Port number to use with a custom install of ThingSpeak |

### Returns
Always returns true. This does not validate the information passed in, or generate any calls to ThingSpeak.

## writeField
Write a value to a single field in a ThingSpeak channel.
```
int writeField(channelNumber, field, value, writeAPIKey)
```
| Parameter     | Type          | Description                                                                                     |          
|---------------|:--------------|:------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                  |
| field         | unsigned int  | Field number (1-8) within the channel to write to.                                              |
| value         | int           | Integer value (from -32,768 to 32,767) to write.                                                |
|               | long          | Long value (from -2,147,483,648 to 2,147,483,647) to write.                                     |
|               | float         | Floating point value (from -999999000000 to 999999000000) to write.                             |
|               | String        | String to write (UTF8 string). ThingSpeak limits this field to 255 bytes.                       |
|               | const char *  | Character array (zero terminated) to write (UTF8). ThingSpeak limits this field to 255 bytes.   |
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Special characters will be automatically encoded by this method. See the note regarding special characters below.

## writeFields
Write a multi-field update. Call setField() for each of the fields you want to write first. 
```
int writeFields (channelNumber, writeAPIKey)	
```
| Parameter     | Type          | Description                                                                                     |          
|---------------|:--------------|:------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                  |
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Special characters will be automatically encoded by this method. See the note regarding special characters below.

## writeRaw
Write a raw POST to a ThingSpeak channel. 
```
int writeRaw (channelNumber, postMessage, writeAPIKey)	
```

| Parameter     | Type          | Description                                                                                                                                       |          
|---------------|:--------------|:--------------------------------------------------------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                                                                    |
| postMessage   | const char *  | Raw URL to write to ThingSpeak as a String. See the documentation at https://thingspeak.com/docs/channels#update_feed.                            |
|               | String        | Raw URL to write to ThingSpeak as a character array (zero terminated). See the documentation at https://thingspeak.com/docs/channels#update_feed. | 
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key                                                   |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
This method will not encode special characters in the post message.  Use '%XX' URL encoding to send special characters. See the note regarding special characters below.

## setField
Set the value of a single field that will be part of a multi-field update.
```
int setField (field, value)
```

| Parameter | Type         | Description                                                                                   |          
|-----------|:-------------|:----------------------------------------------------------------------------------------------|
| field     | unsigned int | Field number (1-8) within the channel to set                                                  |
| value     | int          | Integer value (from -32,768 to 32,767) to write.                                              |
|           | long         | Long value (from -2,147,483,648 to 2,147,483,647) to write.                                   |
|           | float        | Floating point value (from -999999000000 to 999999000000) to write.                           |
|           | String       | String to write (UTF8 string). ThingSpeak limits this field to 255 bytes.                     |
|           | const char * | Character array (zero terminated) to write (UTF8). ThingSpeak limits this field to 255 bytes. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setStatus
Set the status of a multi-field update. Use status to provide additonal details when writing a channel update. Additionally, status can be used by the ThingTweet App to send a message to Twitter.
```
int setStatus (status)	
```

| Parameter | Type      | Description                                                                   |          
|--------|:-------------|:------------------------------------------------------------------------------|
| status | const char * | String to write (UTF8). ThingSpeak limits this to 255 bytes.                  |
|        | String       | const character array (zero terminated). ThingSpeak limits this to 255 bytes. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setLatitude
Set the latitude of a multi-field update.
```
int setLatitude	(latitude)	
```

| Parameter | Type  | Description                                                                |          
|-----------|:------|:---------------------------------------------------------------------------|
| latitude  | float | Latitude of the measurement (degrees N, use negative values for degrees S) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setLongitude
Set the longitude of a multi-field update.
```
int setLongitude (longitude)	
```

| Parameter | Type  | Description                                                                 |          
|-----------|:------|:----------------------------------------------------------------------------|
| longitude | float | Longitude of the measurement (degrees E, use negative values for degrees W) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setElevation
Set the elevation of a multi-field update.
```
int setElevation (elevation)	
```

| Parameter | Type      | Description                                         |          
|-----------|:------|:--------------------------------------------------------|
| elevation | float | 	Elevation of the measurement (meters above sea level) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setCreatedAt
Set the created-at date of a multi-field update. The timestamp string must be in the ISO 8601 format. Example "2017-01-12 13:22:54"
```
int setCreatedAt (createdAt)
```

| Parameter | Type         | Description                                                                                      |          
|-----------|:-------------|:-------------------------------------------------------------------------------------------------|
| createdAt | String       | Desired timestamp to be included with the channel update as a String.                            |
|           | const char * | Desired timestamp to be included with the channel update as a character array (zero terminated). |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Timezones can be set using the timezone hour offset parameter. For example, a timestamp for Eastern Standard Time is: "2017-01-12 13:22:54-05". If no timezone hour offset parameter is used, UTC time is assumed.

## setTwitterTweet
Set the Twitter account and message to use for an update to be tweeted.
```
int setTwitterTweet	(twitter, tweet)	
```

| Parameter | Type         | Description                                                                      |          
|-----------|:-------------|:---------------------------------------------------------------------------------|
| twitter   | String       | Twitter account name as a String.                                                |
|           | const char * | Twitter account name as a character array (zero terminated).                     |
| tweet     | String       | Twitter message as a String (UTF-8) limited to 140 character.                    |
|           | const char * | Twitter message as a character array (zero terminated) limited to 140 character. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Prior to using this feature, a twitter account must be linked to your ThingSpeak account. To link your twitter account. login to ThingSpeak and go to Apps -> ThingTweet and click Link Twitter Account.

## readStringField
Read the latest string from a channel. Include the readAPIKey to read a private channel.
```
String readStringField (channelNumber, field, readAPIKey)	
```
```
String readStringField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read (UTF8 string), or empty string if there is an error.

## readFloatField
Read the latest float from a channel. Include the readAPIKey to read a private channel.
```
float readFloatField (channelNumber, field, readAPIKey)	
```
```
float readFloatField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. Note that NAN, INFINITY, and -INFINITY are valid results. 

## readLongField
Read the latest long from a channel. Include the readAPIKey to read a private channel.
```
long readLongField (channelNumber, field, readAPIKey)	
```
```
long readLongField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. 

## readIntField
Read the latest int from a channel. Include the readAPIKey to read a private channel.
```
int readIntField (channelNumber, field, readAPIKey)		
```
```
int readIntField (channelNumber, field)		
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. If the value returned is out of range for an int, the result is undefined. 

## readStatus
Read the latest status from a channel. Include the readAPIKey to read a private channel.
```
String readStatus (channelNumber, readAPIKey)	
```
```
String readStatus (channelNumber)
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Returns the status field as a String.

## String readCreatedAt()
Read the created-at timestamp associated with the latest update to a channel. Include the readAPIKey to read a private channel.
```
String readCreatedAt (channelNumber, readAPIKey)
```
```
String readCreatedAt (channelNumber)	
```

| channelNumber | unsigned long | Channel number                                                                                 |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Returns the created-at timestamp as a String.

## readRaw
Read a raw response from a channel. Include the readAPIKey to read a private channel.
```
String readRaw (channelNumber, URLSuffix, readAPIKey)	
```
```
String readRaw	(channelNumber, URLSuffix)
```

| Parameter     | Type          | Description                                                                                                        |          
|---------------|:--------------|:-------------------------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                                     |
| URLSuffix     | String        | Raw URL to write to ThingSpeak as a String. See the documentation at https://thingspeak.com/docs/channels#get_feed |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key.                    |     

### Returns
Returns the raw response from a HTTP request as a String.

## getLastReadStatus
Get the status of the previous read.
```
int getLastReadStatus ()	
```

### Returns
See Return Codes below for other possible return values.

## Return Codes
| Value | Meaning                                                                                   |
|-------|:----------------------------------------------------------------------------------------|
| 200   | OK / Success                                                                            |
| 404   | Incorrect API key (or invalid ThingSpeak server address)                                |
| -101  | Value is out of range or string is too long (> 255 characters)                          |
| -201  | Invalid field number specified                                                          |
| -210  | setField() was not called before writeFields()                                          |
| -301  | Failed to connect to ThingSpeak                                                         |
| -302  | Unexpected failure during write to ThingSpeak                                           |
| -303  | Unable to parse response                                                                |
| -304  | Timeout waiting for server to respond                                                   |
| -401  | Point was not inserted (most probable cause is the rate limit of once every 15 seconds) |
|    0  | Other error                                                                             |

## Special Characters
Some characters require '%XX' style URL encoding before sending to ThingSpeak.  The writeField() and writeFields() methods will perform the encoding automatically.  The writeRaw() method will not.

| Character  | Encoding |
|------------|:---------|
|     "      | %22      |
|     %      | %25      |
|     &      | %26      |
|     +      | %2B      |
|     ;      | %3B      |

Control characters, ASCII values 0 though 31, are not accepted by ThingSpeak and will be ignored.  Extended ASCII characters with values above 127 will also be ignored. 
