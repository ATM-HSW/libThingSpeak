/*
  ThingSpeak(TM) Communication Library for Mbed OS

  Enables Mbed OS compatible hardware to write or read data to or from ThingSpeak,
  an open data platform for the Internet of Things with MATLAB analytics and visualization.

  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and
  analyze live data streams in the cloud.

  Copyright 2018, The MathWorks, Inc.
            2019-2021, Dr. Olaf Hagendorf, HS Wismar

  See the accompaning licence file for licensing information.
*/

//#define THINGSPEAK_DBG_HTTP
//#define THINGSPEAK_DBG_MSG

#ifdef THINGSPEAK_DBG_MSG
#define PRINT_DEBUG_MESSAGES
#endif
#ifdef THINGSPEAK_DBG_HTTP
#define PRINT_HTTP
#endif

#ifndef ThingSpeak_h
#define ThingSpeak_h

#define TS_VER "2.0.0"


#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "mbed.h"
#include "http_request.h"

#define THINGSPEAK_URL "api.thingspeak.com"
#define THINGSPEAK_PORT_NUMBER 80

#define TS_USER_AGENT "tslib-mbed/" TS_VER " (mbed)"

#define FIELDNUM_MIN 1
#define FIELDNUM_MAX 8
#define FIELDLENGTH_MAX 255  // Max length for a field in ThingSpeak is 255 bytes (UTF-8)

#define TIMEOUT_MS_SERVERRESPONSE 5000  // Wait up to five seconds for server to respond

#define OK_SUCCESS              200     // OK / Success
#define ERR_BADAPIKEY           400     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_BADURL              404     // Incorrect API key (or invalid ThingSpeak server address)
#define ERR_OUT_OF_RANGE        -101    // Value is out of range or string is too long (> 255 bytes)
#define ERR_INVALID_FIELD_NUM   -201    // Invalid field number specified
#define ERR_SETFIELD_NOT_CALLED -210    // setField() was not called before writeFields()
#define ERR_CONNECT_FAILED      -301    // Failed to connect to ThingSpeak
#define ERR_UNEXPECTED_FAIL     -302    // Unexpected failure during write to ThingSpeak
#define ERR_BAD_RESPONSE        -303    // Unable to parse response
#define ERR_TIMEOUT             -304    // Timeout waiting for server to respond
#define ERR_NOT_INSERTED        -401    // Point was not inserted (most probable cause is the rate limit of once every 15 seconds)

// Enables an Arduino, ESP8266, ESP32 or other compatible hardware to write or read data to or from ThingSpeak, an open data platform for the Internet of Things with MATLAB analytics and visualization.
class ThingSpeak
{
  public:
  ThingSpeak() {
    resetWriteFields();
    this->lastReadStatus = OK_SUCCESS;
    this->net = NULL;
  };


  /*
  Function: begin

  Summary:
  Initializes the ThingSpeak library and network settings using the ThingSpeak.com service.

  Parameters:
  client - EthernetClient, YunClient, TCPClient, or WiFiClient created earlier in the sketch

  Returns:
  Always returns true

  Notes:
  This does not validate the information passed in, or generate any calls to ThingSpeak.

  */
  bool begin(NetworkInterface * net) {
    resetWriteFields();
    this->lastReadStatus = OK_SUCCESS;
    this->net = net;

    return true;
  };

  /*
  Function: writeField

  Summary:
  Write an integer value to a single field in a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to write to.
  value - Integer value (from -32,768 to 32,767) to write.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  HTTP status code of 200 if successful.

  Notes:
  See getLastReadStatus() for other possible return values.
  */
  int writeField(unsigned long channelNumber, unsigned int field, int value, const char * writeAPIKey) {
    return writeField(channelNumber, field, std::to_string(value), writeAPIKey);
  };


  /*
  Function: writeField

  Summary:
  Write a long value to a single field in a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to write to.
  value - Long value (from -2,147,483,648 to 2,147,483,647) to write.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  HTTP status code of 200 if successful.

  Notes:
  See getLastReadStatus() for other possible return values.
  */
  int writeField(unsigned long channelNumber, unsigned int field, long value, const char * writeAPIKey) {
    return writeField(channelNumber, field, std::to_string(value), writeAPIKey);
  };

  /*
  Function: writeField

  Summary:
  Write a floating point value to a single field in a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to write to.
  value - Floating point value (from -999999000000 to 999999000000) to write.  If you need more accuracy, or a wider range, you should format the number using <tt>dtostrf</tt> and writeField().
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  HTTP status code of 200 if successful.

  Notes:
  See getLastReadStatus() for other possible return values.
  */
  int writeField(unsigned long channelNumber, unsigned int field, float value, const char * writeAPIKey) {
    return writeField(channelNumber, field, std::to_string(value), writeAPIKey);
  };


  /*
  Function: writeField

  Summary:
  Write a string to a single field in a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to write to.
  value - String to write (UTF8 string).  ThingSpeak limits this field to 255 bytes.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  HTTP status code of 200 if successful.

  Notes:
  See getLastReadStatus() for other possible return values.
  */
  int writeField(unsigned long channelNumber, unsigned int field, const char * value, const char * writeAPIKey) {
    return writeField(channelNumber, field, string(value), writeAPIKey);
  };

  /*
  Function: writeField

  Summary:
  Write a string to a single field in a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to write to.
  value - String to write (UTF8 string).  ThingSpeak limits this field to 255 bytes.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  HTTP status code of 200 if successful.

  Notes:
  See getLastReadStatus() for other possible return values.
  */
  int writeField(unsigned long channelNumber, unsigned int field, string value, const char * writeAPIKey) {
    // Invalid field number specified
    if(field < FIELDNUM_MIN || field > FIELDNUM_MAX)
      return ERR_INVALID_FIELD_NUM;
    // Max # bytes for ThingSpeak field is 255
    if(value.length() > FIELDLENGTH_MAX)
      return ERR_OUT_OF_RANGE;

    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::writeField (channelNumber: %lu writeAPIKey: %s field: %d value: \"%s\")\n", channelNumber, writeAPIKey, field, value.c_str());
    #endif
    string postMessage = string("field");
    postMessage +=  std::to_string(field);
    postMessage += "=";
    postMessage += value;
    return writeRaw(channelNumber, postMessage, writeAPIKey);
   };


  /*
  Function: setField

  Summary:
  Set the value of a single field that will be part of a multi-field update.

  Parameters:
  field - Field number (1-8) within the channel to set.
  value - Integer value (from -32,768 to 32,767) to set.

  Returns:
  Code of 200 if successful.
  Code of -101 if value is out of range or string is too long (> 255 bytes)

  */
  int setField(unsigned int field, int value) {
    return setField(field, std::to_string(value));
  };

  /*
  Function: setField

  Summary:
  Set the value of a single field that will be part of a multi-field update.

  Parameters:
  field - Field number (1-8) within the channel to set.
  value - Long value (from -2,147,483,648 to 2,147,483,647) to write.

  Returns:
  Code of 200 if successful.
  Code of -101 if value is out of range or string is too long (> 255 bytes)

  */
  int setField(unsigned int field, long value) {
    return setField(field, std::to_string(value));
  };

  /*
  Function: setField

  Summary:
  Set the value of a single field that will be part of a multi-field update.

  Parameters:
  field - Field number (1-8) within the channel to set.
  value - Floating point value (from -999999000000 to 999999000000) to write.  If you need more accuracy, or a wider range, you should format the number yourself (using <tt>dtostrf</tt>) and setField() using the resulting string.

  Returns:
  Code of 200 if successful.
  Code of -101 if value is out of range or string is too long (> 255 bytes)

  */
  int setField(unsigned int field, float value) {
    return setField(field, std::to_string(value));
  };


  /*
  Function: setField

  Summary:
  Set the value of a single field that will be part of a multi-field update.

  Parameters:
  field - Field number (1-8) within the channel to set.
  value - String to write (UTF8).  ThingSpeak limits this to 255 bytes.

  Returns:
  Code of 200 if successful.
  Code 0f -101 if value is out of range or string is too long (> 255 bytes)

  */
  int setField(unsigned int field, const char * value) {
    return setField(field, string(value));
  };


  /*
  Function: setField

  Summary:
  Set the value of a single field that will be part of a multi-field update.

  Parameters:
  field - Field number (1-8) within the channel to set.
  value - string to write (UTF8).  ThingSpeak limits this to 255 bytes.

  Returns:
  Code of 200 if successful.
  Code of -101 if value is out of range or string is too long (> 255 bytes)

  */
  int setField(unsigned int field, string value) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setField   (field: %d value: \"%s\")\n", field, value.c_str());
    #endif
    if(field < FIELDNUM_MIN || field > FIELDNUM_MAX) return ERR_INVALID_FIELD_NUM;
    // Max # bytes for ThingSpeak field is 255 (UTF-8)
    if(value.length() > FIELDLENGTH_MAX) return ERR_OUT_OF_RANGE;
    this->nextWriteField[field - 1] = value;
    return OK_SUCCESS;
  };


  /*
  Function: setLatitude

  Summary:
  Set the latitude of a multi-field update.

  Parameters:
  latitude - Latitude of the measurement as a floating point value (degrees N, use negative values for degrees S)

  Returns:
  Always return 200

  Notes:
  To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()

  */
  int setLatitude(float latitude) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setLatitude(latitude: %f\")\n", latitude);
    #endif
    this->nextWriteLatitude = latitude;
    return OK_SUCCESS;
  };


  /*
  Function: setLongitude

  Summary:
  Set the longitude of a multi-field update.

  Parameters:
  longitude - Longitude of the measurement as a floating point value (degrees E, use negative values for degrees W)

  Returns:
  Always return 200

  Notes:
  To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()

  */
  int setLongitude(float longitude) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setLongitude(longitude: %f\")\n", longitude);
    #endif
    this->nextWriteLongitude = longitude;
    return OK_SUCCESS;
  };


  /*
  Function: setElevation

  Summary:
  Set the elevation of a multi-field update.

  Parameters:
  elevation - Elevation of the measurement as a floating point value (meters above sea level)

  Returns:
  Always return 200

  Notes:
  To record latitude, longitude and elevation of a write, call setField() for each of the fields you want to write. Then setLatitude(), setLongitude(), setElevation() and then call writeFields()

  */
  int setElevation(float elevation) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setElevation(elevation: %f\")\n", elevation);
    #endif
    this->nextWriteElevation = elevation;
    return OK_SUCCESS;
  };


  /*
  Function: setStatus

  Summary:
  Set the status field of a multi-field update.

  Parameters:
  status - String to write (UTF8).  ThingSpeak limits this to 255 bytes.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To record a status message on a write, call setStatus() then call writeFields().
  Use status to provide additonal details when writing a channel update.
  Additonally, status can be used by the ThingTweet App to send a message to Twitter.

  */
  int setStatus(const char * status) {
    return setStatus(string(status));
  };


  /*
  Function: setStatus

  Summary:
  Set the status field of a multi-field update.

  Parameters:
  status - String to write (UTF8).  ThingSpeak limits this to 255 bytes.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To record a status message on a write, call setStatus() then call writeFields().
  Use status to provide additonal details when writing a channel update.
  Additonally, status can be used by the ThingTweet App to send a message to Twitter.

  */
  int setStatus(string status) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setStatus(status: %s\")\n", status.c_str());
    #endif
    // Max # bytes for ThingSpeak field is 255 (UTF-8)
    if(status.length() > FIELDLENGTH_MAX)
      return ERR_OUT_OF_RANGE;
    this->nextWriteStatus = status;
    return OK_SUCCESS;
  };


  /*
  Function: setTwitterTweet

  Summary:
  Set the Twitter account and message to use for an update to be tweeted.

  Parameters:
  twitter - Twitter account name as a String.
  tweet - Twitter message as a String (UTF-8) limited to 140 character.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To send a message to twitter call setTwitterTweet() then call writeFields().
  Prior to using this feature, a twitter account must be linked to your ThingSpeak account. Do this by logging into ThingSpeak and going to Apps, then ThingTweet and clicking Link Twitter Account.
  */
  int setTwitterTweet(const char * twitter, const char * tweet) {
    return setTwitterTweet(string(twitter), string(tweet));
  };

  /*
  Function: setTwitterTweet

  Summary:
  Set the Twitter account and message to use for an update to be tweeted.

  Parameters:
  twitter - Twitter account name as a String.
  tweet - Twitter message as a String (UTF-8) limited to 140 character.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To send a message to twitter call setTwitterTweet() then call writeFields().
  Prior to using this feature, a twitter account must be linked to your ThingSpeak account. Do this by logging into ThingSpeak and going to Apps, then ThingTweet and clicking Link Twitter Account.
  */
  int setTwitterTweet(string twitter, const char * tweet) {
    return setTwitterTweet(twitter, string(tweet));
  };

  /*
  Function: setTwitterTweet

  Summary:
  Set the Twitter account and message to use for an update to be tweeted.

  Parameters:
  twitter - Twitter account name as a String.
  tweet - Twitter message as a String (UTF-8) limited to 140 character.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To send a message to twitter call setTwitterTweet() then call writeFields().
  Prior to using this feature, a twitter account must be linked to your ThingSpeak account. Do this by logging into ThingSpeak and going to Apps, then ThingTweet and clicking Link Twitter Account.
  */
  int setTwitterTweet(const char * twitter, string tweet) {
    return setTwitterTweet(string(twitter), tweet);
  };

  /*
  Function: setTwitterTweet

  Summary:
  Set the Twitter account and message to use for an update to be tweeted.

  Parameters:
  twitter - Twitter account name as a String.
  tweet - Twitter message as a String (UTF-8) limited to 140 character.

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  To send a message to twitter call setTwitterTweet() then call writeFields().
  Prior to using this feature, a twitter account must be linked to your ThingSpeak account. Do this by logging into ThingSpeak and going to Apps, then ThingTweet and clicking Link Twitter Account.
  */
  int setTwitterTweet(string twitter, string tweet) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setTwitterTweet(twitter: %s, tweet: %s\")\n", twitter.c_str(), tweet.c_str());
    #endif
    // Max # bytes for ThingSpeak field is 255 (UTF-8)
    if((twitter.length() > FIELDLENGTH_MAX) || (tweet.length() > FIELDLENGTH_MAX))
      return ERR_OUT_OF_RANGE;

    this->nextWriteTwitter = twitter;
    this->nextWriteTweet = tweet;

    return OK_SUCCESS;
  };


  /*
  Function: setCreatedAt

  Summary:
  Set the created-at date of a multi-field update.

  Parameters:
  createdAt - Desired timestamp to be included with the channel update as a String.  The timestamp string must be in the ISO 8601 format. Example "2017-01-12 13:22:54"

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  Timezones can be set using the timezone hour offset parameter. For example, a timestamp for Eastern Standard Time is: "2017-01-12 13:22:54-05".
  If no timezone hour offset parameter is used, UTC time is assumed.

  */
  int setCreatedAt(const char * createdAt) {
    return setCreatedAt(string(createdAt));
  }


  /*
  Function: setCreatedAt

  Summary:
  Set the created-at date of a multi-field update.

  Parameters:
  createdAt - Desired timestamp to be included with the channel update as a String.  The timestamp string must be in the ISO 8601 format. Example "2017-01-12 13:22:54"

  Returns:
  Code of 200 if successful.
  Code of -101 if string is too long (> 255 bytes)

  Notes:
  Timezones can be set using the timezone hour offset parameter. For example, a timestamp for Eastern Standard Time is: "2017-01-12 13:22:54-05".
  If no timezone hour offset parameter is used, UTC time is assumed.

  */
  int setCreatedAt(string createdAt) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::setCreatedAt(createdAt: %s\")\n", createdAt.c_str());
    #endif

    // the ISO 8601 format is too complicated to check for valid timestamps here
    // we'll need to reply on the api to tell us if there is a problem
    // Max # bytes for ThingSpeak field is 255 (UTF-8)
    if(createdAt.length() > FIELDLENGTH_MAX)
      return ERR_OUT_OF_RANGE;
    this->nextWriteCreatedAt = createdAt;

    return OK_SUCCESS;
  }


  /*
  Function: writeFields

  Summary:
  Write a multi-field update.

  Parameters:
  channelNumber - Channel number
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  200 - successful.
  404 - Incorrect API key (or invalid ThingSpeak server address)
  -101 - Value is out of range or string is too long (> 255 characters)
  -201 - Invalid field number specified
  -210 - setField() was not called before writeFields()
  -301 - Failed to connect to ThingSpeak
  -302 - Unexpected failure during write to ThingSpeak
  -303 - Unable to parse response
    -304 - Timeout waiting for server to respond
  -401 - Point was not inserted (most probable cause is the rate limit of once every 15 seconds)


  Notes:
  Call setField(), setLatitude(), setLongitude(), setElevation() and/or setStatus() and then call writeFields()

  */
  int writeFields(unsigned long channelNumber, const char * writeAPIKey) {
    int status;

    // Get the content length of the payload
    int contentLen = getWriteFieldsContentLength();

    if(contentLen == 0) {
      // setField was not called before writeFields
      printf("ERR_SETFIELD_NOT_CALLED\n");
      return ERR_SETFIELD_NOT_CALLED;
    }

    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::writeFields   (channelNumber: %lu writeAPIKey: %s", channelNumber, writeAPIKey);
    #endif



    // create Post message for thingspeak
    HttpRequest* request = new HttpRequest(this->net, HTTP_POST, "http://api.thingspeak.com/update");
    request->set_header("User-Agent", TS_USER_AGENT);
    request->set_header("X-THINGSPEAKAPIKEY", writeAPIKey);
    request->set_header("Content-Type", "application/x-www-form-urlencoded");

    string body = "";
    bool fFirstItem = true;
    for(size_t iField = 0; iField < FIELDNUM_MAX; iField++) {
      if(this->nextWriteField[iField].length() > 0) {
        if(!fFirstItem)
          body += "&";
        body += "field";
        body += std::to_string((int)(iField + 1));
        body += "=";
        body += this->nextWriteField[iField];
        fFirstItem = false;
      }
    }

    /*
     ToDo oh
    if(!isnan(this->nextWriteLatitude)) {
      if(!fFirstItem)
        body.concat("&");
      body.concat("lat=");
      body.concat(this->nextWriteLatitude);
      fFirstItem = false;
    }

    if(!isnan(this->nextWriteLongitude)) {
      if(!fFirstItem)
        body.concat("&");
      body.concat("long=");
      body.concat(this->nextWriteLongitude);
      fFirstItem = false;
    }


    if(!isnan(this->nextWriteElevation)) {
      if(!fFirstItem)
        body.concat("&");
      body.concat("elevation=");
      body.concat(this->nextWriteElevation);
      fFirstItem = false;
    }
    */

    if(this->nextWriteStatus.length() > 0) {
      if(!fFirstItem)
        body += "&";
      body += "status=";
      body += this->nextWriteStatus;
      fFirstItem = false;
    }

    if(this->nextWriteTwitter.length() > 0) {
      if(!fFirstItem)
        body += "&";
      body += "twitter=";
      body += this->nextWriteTwitter;
      fFirstItem = false;
    }

    if(this->nextWriteTweet.length() > 0) {
      if(!fFirstItem)
        body += "&";
      body += "tweet=";
      body += this->nextWriteTweet;
      fFirstItem = false;
    }

    if(this->nextWriteCreatedAt.length() > 0) {
      if(!fFirstItem)
        body += "&";
      body += "created_at=";
      body += this->nextWriteCreatedAt;
      fFirstItem = false;
    }

    body += "&headers=false";

    resetWriteFields();

    #ifdef PRINT_DEBUG_MESSAGES
      printf("               POST \"%s\"\n", body.c_str());
    #endif

    HttpResponse* response = request->send(body.c_str(), strlen(body.c_str()));
    #ifdef PRINT_HTTP
      printf("\n----- HTTP POST response -----\n");
      printf("Status: %d - %s\n", response->get_status_code(), response->get_status_message().c_str());

      printf("Headers:\n");
      for (size_t ix = 0; ix < response->get_headers_length(); ix++) {
        printf("\t%s: %s\n", response->get_headers_fields()[ix]->c_str(), response->get_headers_values()[ix]->c_str());
      }
      printf("\nBody (%d bytes):\n\n%s\n", response->get_body_length(), response->get_body_as_string().c_str());
    #endif

    status = response->get_status_code();
    if(status != OK_SUCCESS) {
      delete request;
      printf("get_status_code %d\n", status);
      return status;
    }

    long entryID = stoi(response->get_body_as_string());

    #ifdef PRINT_DEBUG_MESSAGES
      printf("               Entry ID \"%s\" (%ld)", response->get_body_as_string().c_str(), entryID);
    #endif

    #ifdef PRINT_DEBUG_MESSAGES
      printf("disconnected.\n");
    #endif
    if(entryID == 0) {
      // ThingSpeak did not accept the write
      printf("ERR_NOT_INSERTED\n");
      status = ERR_NOT_INSERTED;
    }

    resetWriteFields();
    delete request;
    return status;
  }


  /*
  Function: writeRaw

  Summary:
  Write a raw POST to a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  postMessage - Raw URL to write to ThingSpeak as a string.  See the documentation at https://thingspeak.com/docs/channels#update_feed.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  200 - successful.
  404 - Incorrect API key (or invalid ThingSpeak server address)
  -101 - Value is out of range or string is too long (> 255 characters)
  -201 - Invalid field number specified
  -210 - setField() was not called before writeFields()
  -301 - Failed to connect to ThingSpeak
  -302 - Unexpected failure during write to ThingSpeak
  -303 - Unable to parse response
    -304 - Timeout waiting for server to respond
  -401 - Point was not inserted (most probable cause is the rate limit of once every 15 seconds)

  Notes:
  This is low level functionality that will not be required by most users.

  */
  int writeRaw(unsigned long channelNumber, const char * postMessage, const char * writeAPIKey) {
    return writeRaw(channelNumber, string(postMessage), writeAPIKey);
  };


  /*
  Function: writeRaw

  Summary:
  Write a raw POST to a ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  postMessage - Raw URL to write to ThingSpeak as a string.  See the documentation at https://thingspeak.com/docs/channels#update_feed.
  writeAPIKey - Write API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  200 - successful.
  404 - Incorrect API key (or invalid ThingSpeak server address)
  -101 - Value is out of range or string is too long (> 255 characters)
  -201 - Invalid field number specified
  -210 - setField() was not called before writeFields()
  -301 - Failed to connect to ThingSpeak
  -302 - Unexpected failure during write to ThingSpeak
  -303 - Unable to parse response
    -304 - Timeout waiting for server to respond
  -401 - Point was not inserted (most probable cause is the rate limit of once every 15 seconds)

  Notes:
  This is low level functionality that will not be required by most users.

  */
  int writeRaw(unsigned long channelNumber, string postMessage, const char * writeAPIKey) {
    int status;

    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::writeRaw   (channelNumber: %lu writeAPIKey: %s", channelNumber, writeAPIKey);
    #endif

    // Post data to thingspeak
    string body = postMessage + string("&headers=false");

    HttpRequest* request = new HttpRequest(this->net, HTTP_POST, "http://api.thingspeak.com/update");
    request->set_header("User-Agent", TS_USER_AGENT);
    request->set_header("X-THINGSPEAKAPIKEY", writeAPIKey);
    request->set_header("Content-Type", "application/x-www-form-urlencoded");

    #ifdef PRINT_DEBUG_MESSAGES
      printf("               POST \"%s\"\n", body.c_str());
    #endif

    HttpResponse* response = request->send(body.c_str(), strlen(body.c_str()));
    #ifdef PRINT_HTTP
      printf("\n----- HTTP POST response -----\n");
      printf("Status: %d - %s\n", response->get_status_code(), response->get_status_message().c_str());

      printf("Headers:\n");
      for (size_t ix = 0; ix < response->get_headers_length(); ix++) {
        printf("\t%s: %s\n", response->get_headers_fields()[ix]->c_str(), response->get_headers_values()[ix]->c_str());
      }
      printf("\nBody (%d bytes):\n\n%s\n", response->get_body_length(), response->get_body_as_string().c_str());
    #endif

    status = response->get_status_code();
    if(status != OK_SUCCESS) {
      delete request;
      return status;
    }

    long entryID = stoi(response->get_body_as_string());

    #ifdef PRINT_DEBUG_MESSAGES
      printf("               Entry ID \"%s\" (%ld)", response->get_body_as_string().c_str(), entryID);
    #endif

    #ifdef PRINT_DEBUG_MESSAGES
      printf("disconnected.\n");
    #endif
    if(entryID == 0) {
      // ThingSpeak did not accept the write
      status = ERR_NOT_INSERTED;
    }

    resetWriteFields();
    delete request;
    return status;
  };


  /*
  Function: readStringField

  Summary:
  Read the latest string from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  Value read (UTF8 string), or empty string if there is an error.  Use getLastReadStatus() to get more specific information.

  */
  string readStringField(unsigned long channelNumber, unsigned int field, const char * readAPIKey) {
    if(field < FIELDNUM_MIN || field > FIELDNUM_MAX) {
      this->lastReadStatus = ERR_INVALID_FIELD_NUM;
      return("");
    }
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::readStringField(channelNumber: %lu", channelNumber);
      if(NULL != readAPIKey) {
        printf(" readAPIKey: %s", readAPIKey);
      }
      printf(" field: %d)\n", field);
    #endif
    return readRaw(channelNumber, string("/fields/") + std::to_string(field) + string("/last"), readAPIKey);
  }



  /*
  Function: readStringField

  Summary:
  Read the latest string from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.

  Returns:
  Value read (UTF8 string), or empty string if there is an error.  Use getLastReadStatus() to get more specific information.

  */
  string readStringField(unsigned long channelNumber, unsigned int field) {
    return readStringField(channelNumber, field, NULL);
  };


  /*
  Function: readFloatField

  Summary:
  ead the latest floating point value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  float readFloatField(unsigned long channelNumber, unsigned int field, const char * readAPIKey) {
    return stof(readStringField(channelNumber, field, readAPIKey));
  };


  /*
  Function: readFloatField

  Summary:
  Read the latest floating point value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  float readFloatField(unsigned long channelNumber, unsigned int field) {
    return readFloatField(channelNumber, field, NULL);
  };


  /*
  Function: readLongField

  Summary:
  Read the latest long value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  long readLongField(unsigned long channelNumber, unsigned int field, const char * readAPIKey) {
    // Note that although the function is called "toInt" it really returns a long.
    return stoi(readStringField(channelNumber, field, readAPIKey));
  }


  /*
  Function: readLongField

  Summary:
  Read the latest long value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  long readLongField(unsigned long channelNumber, unsigned int field) {
    return readLongField(channelNumber, field, NULL);
  };


  /*
  Function: readIntField

  Summary:
  Read the latest int value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  int readIntField(unsigned long channelNumber, unsigned int field, const char * readAPIKey) {
    return readLongField(channelNumber, field, readAPIKey);
  }


  /*
  Function: readIntField

  Summary:
  Read the latest int value from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  field - Field number (1-8) within the channel to read from.

  Returns:
  Value read, or 0 if the field is text or there is an error.  Use getLastReadStatus() to get more specific information.  Note that NAN, INFINITY, and -INFINITY are valid results.

  */
  int readIntField(unsigned long channelNumber, unsigned int field) {
    return readLongField(channelNumber, field, NULL);
  };


  /*
  Function: readStatus

  Summary:
  Read the latest status from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Results:
  Value read (UTF8 string). An empty string is returned if there was no status written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.

  */
  string readStatus(unsigned long channelNumber, const char * readAPIKey) {
    string content = readRaw(channelNumber, "/feeds/last.txt?status=true", readAPIKey);

    if(getLastReadStatus() != OK_SUCCESS) {
      return string("");
    }

    return getJSONValueByKey(content, "status");
  };


  /*
  Function: readStatus

  Summary:
  Read the latest status from a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number

  Results:
  Value read (UTF8 string). An empty string is returned if there was no status written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.

  */
  string readStatus(unsigned long channelNumber) {
    return readStatus(channelNumber, NULL);
  };


  /*
  Function: readCreatedAt

  Summary:
  Read the created-at timestamp associated with the latest update to a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Results:
  Value read (UTF8 string). An empty string is returned if there was no created-at timestamp written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.

  */
  string readCreatedAt(unsigned long channelNumber, const char * readAPIKey) {
    string content = readRaw(channelNumber, "/feeds/last.txt", readAPIKey);

    if(getLastReadStatus() != OK_SUCCESS) {
      return string("");
    }

    return getJSONValueByKey(content, "created_at");
  };


  /*
  Function: readCreatedAt

  Summary:
  Read the created-at timestamp associated with the latest update to a private ThingSpeak channel

  Parameters:
  channelNumber - Channel number

  Results:
  Value read (UTF8 string). An empty string is returned if there was no created-at timestamp written to the channel or in case of an error.  Use getLastReadStatus() to get more specific information.

  */
  string readCreatedAt(unsigned long channelNumber) {
    return readCreatedAt(channelNumber, NULL);
  };


  /*
  Function: readRaw

  Summary:
  Read a raw response from a public ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  URLSuffix - Raw URL to write to ThingSpeak as a String.  See the documentation at https://thingspeak.com/docs/channels#get_feed

  Returns:
  Response if successful, or empty string. Use getLastReadStatus() to get more specific information.

  Notes:
  This is low level functionality that will not be required by most users.

  */
  string readRaw(unsigned long channelNumber, string URLSuffix) {
    return readRaw(channelNumber, URLSuffix, NULL);
  }


  /*
  Function: readRaw

  Summary:
  Read a raw response from a public ThingSpeak channel

  Parameters:
  channelNumber - Channel number
  URLSuffix - Raw URL to write to ThingSpeak as a String.  See the documentation at https://thingspeak.com/docs/channels#get_feed
  readAPIKey - Read API key associated with the channel.  *If you share code with others, do _not_ share this key*

  Returns:
  Response if successful, or empty string. Use getLastReadStatus() to get more specific information.

  Notes:
  This is low level functionality that will not be required by most users.

  */
  string readRaw(unsigned long channelNumber, string URLSuffix, const char * readAPIKey) {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ts::readRaw   (channelNumber: %lu", channelNumber);
      if(NULL != readAPIKey) {
        printf(" readAPIKey: %s", readAPIKey);
      }
      printf(" URLSuffix: \"%s\")\n", URLSuffix.c_str());
    #endif

    string URL = string("http://api.thingspeak.com/channels/") + std::to_string(channelNumber) + URLSuffix;

    #ifdef PRINT_DEBUG_MESSAGES
      printf("               GET \"%s\"\n", URL.c_str());
    #endif

    // Post data to thingspeak
    HttpRequest* request = new HttpRequest(this->net, HTTP_GET, URL.c_str());
    request->set_header("User-Agent", TS_USER_AGENT);
    if(NULL != readAPIKey)
      request->set_header("X-THINGSPEAKAPIKEY", readAPIKey);

    const char body[] = "";
    HttpResponse* response = request->send(body, strlen(body));
    #ifdef PRINT_HTTP
      printf("\n----- HTTP GET response -----\n");
      printf("Status: %d - %s\n", response->get_status_code(), response->get_status_message().c_str());

      printf("Headers:\n");
      for (size_t ix = 0; ix < response->get_headers_length(); ix++) {
        printf("\t%s: %s\n", response->get_headers_fields()[ix]->c_str(), response->get_headers_values()[ix]->c_str());
      }
      printf("\nBody (%d bytes):\n%s\n", response->get_body_length(), response->get_body_as_string().c_str());
    #endif

    this->lastReadStatus = response->get_status_code();

    #ifdef PRINT_DEBUG_MESSAGES
      if(this->lastReadStatus == OK_SUCCESS) {
        printf("Read: \"%s\"\n", response->get_body_as_string().c_str());
      }
    #endif
    string ret =string(response->get_body_as_string().c_str());
    delete request;

    #ifdef PRINT_DEBUG_MESSAGES
      printf("disconnected.\n");
    #endif

    if(this->lastReadStatus != OK_SUCCESS) {
      return string("");
    }

    return ret;
  };


  /*
  Function: getLastReadStatus

  Summary:
  Get the status of the previous read.

  Returns:
  Generally, these are HTTP status codes.  Negative values indicate an error generated by the library.
  Possible response codes...
  200 - OK / Success
  404 - Incorrect API key (or invalid ThingSpeak server address)
  -101 - Value is out of range or string is too long (> 255 characters)
  -201 - Invalid field number specified
  -210 - setField() was not called before writeFields()
  -301 - Failed to connect to ThingSpeak
  -302 -  Unexpected failure during write to ThingSpeak
  -303 - Unable to parse response
    -304 - Timeout waiting for server to respond
  -401 - Point was not inserted (most probable cause is exceeding the rate limit)

  Notes:
  The read functions will return zero or empty if there is an error.  Use this function to retrieve the details.
  */
  int getLastReadStatus() {
    return this->lastReadStatus;
  };

  
  private:

  int getWriteFieldsContentLength(){
    size_t iField;
    int contentLen = 0;

    for(iField = 0; iField < FIELDNUM_MAX; iField++) {
      if(this->nextWriteField[iField].length() > 0) {
        contentLen = contentLen + 8 + this->nextWriteField[iField].length();  // &fieldX=[value]

        // future-proof in case ThingSpeak allows 999 fields someday
        if(iField > 9) {
          contentLen = contentLen + 1;
        }
        else if(iField > 99) {
          contentLen = contentLen + 2;
        }
      }
    }

    /*
     * ToDo oh
    if(!isnan(this->nextWriteLatitude)){
      contentLen = contentLen + 5 + string(this->nextWriteLatitude).length(); // &lat=[value]
    }

    if(!isnan(this->nextWriteLongitude)){
      contentLen = contentLen + 6 + string(this->nextWriteLongitude).length(); // &long=[value]
    }

    if(!isnan(this->nextWriteElevation)){
      contentLen = contentLen + 11 + string(this->nextWriteElevation).length(); // &elevation=[value]
    }
    */

    if(this->nextWriteStatus.length() > 0){
      contentLen = contentLen + 8 + this->nextWriteStatus.length();  // &status=[value]
    }

    if(this->nextWriteTwitter.length() > 0){
      contentLen = contentLen + 9 + this->nextWriteTwitter.length();  // &twitter=[value]
    }

    if(this->nextWriteTweet.length() > 0){
      contentLen = contentLen + 7 + this->nextWriteTweet.length();  // &tweet=[value]
    }

    if(this->nextWriteCreatedAt.length() > 0){
      contentLen = contentLen + 12 + this->nextWriteCreatedAt.length();  // &created_at=[value]
    }

    if(contentLen == 0){
      return 0;
    }

    contentLen = contentLen + 13; // add 14 for '&headers=false', subtract 1 for missing first '&'

    return contentLen;

  }

  string getJSONValueByKey(string textToSearch, string key) {
    if(textToSearch.length() == 0) {
      return string("");
    }

    string searchPhrase = string("\"") + key + string("\":\"");

    int fromPosition = textToSearch.find(searchPhrase);

    if(fromPosition == string::npos) {
      // return because there is no status or it's null
      return string("");
    }

    fromPosition = fromPosition + searchPhrase.length();

    int toPosition = textToSearch.find("\"", fromPosition);

    if(toPosition == string::npos) {
      // return because there is no end quote
      return string("");
    }

    textToSearch.erase(toPosition);

    return textToSearch.substr(fromPosition);
  }

  int abortWriteRaw() {
    resetWriteFields();
    return ERR_UNEXPECTED_FAIL;
  }

  string abortReadRaw() {
    #ifdef PRINT_DEBUG_MESSAGES
      printf("ReadRaw abort - disconnected.\n");
    #endif
    this->lastReadStatus = ERR_UNEXPECTED_FAIL;
    return string("");
  }

  void resetWriteFields() {
    for(size_t iField = 0; iField < FIELDNUM_MAX; iField++) {
      this->nextWriteField[iField] = "";
    }
    this->nextWriteLatitude = NAN;
    this->nextWriteLongitude = NAN;
    this->nextWriteElevation = NAN;
    this->nextWriteStatus = "";
    this->nextWriteTwitter = "";
    this->nextWriteTweet = "";
    this->nextWriteCreatedAt = "";
  };

  TCPSocket *socket;
  NetworkInterface *net;
  string nextWriteField[8];
  float nextWriteLatitude;
  float nextWriteLongitude;
  float nextWriteElevation;
  int lastReadStatus;
  string nextWriteStatus;
  string nextWriteTwitter;
  string nextWriteTweet;
  string nextWriteCreatedAt;
};

extern ThingSpeak thingSpeak;

#endif //ThingSpeak_h