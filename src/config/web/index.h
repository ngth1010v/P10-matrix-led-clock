#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<!DOCTYPE html>
<html>
  <head>
    <title>Schedule clock config</title>
    <link rel="stylesheet" href="styles.css" />
  </head>

  <body>
    <div class="main-container">
      <span class="container-header">SCHEDULE CLOCK CONFIG</span>

      <div class="container">
        <span class="title">Internet WiFi</span>

        <div class="input-container">
          <label for="internetWifiName" class="label">Name</label>
          <input
            type="text"
            class="input"
            name="internetWifiName"
            id="internetWifiName"
            value=""
          />
        </div>

        <div class="input-container">
          <label for="internetWifiPassword" class="label">Password</label>
          <input
            type="password"
            class="input"
            name="internetWifiPassword"
            id="internetWifiPassword"
            value=""
          />
        </div>
      </div>

      <div class="container">
        <span class="title">Config WiFi</span>

        <div class="input-container">
          <label for="configWifiName" class="label">Name</label>
          <input
            type="text"
            class="input"
            name="configWifiName"
            id="configWifiName"
            value=""
          />
        </div>

        <div class="input-container">
          <label for="configWifiPassword" class="label">Password</label>
          <input
            type="password"
            class="input"
            name="configWifiPassword"
            id="configWifiPassword"
            value=""
          />
        </div>
      </div>

      <div class="container">
        <span class="title">Time</span>

        <div class="input-container">
          <label for="timezone" class="label">Timezone</label>
          <select
            id="timezone"
            class="input"
            name="timezone"
          >
            <option value="-10">UTC-10</option>
            <option value="-9">UTC-9</option>
            <option value="-8">UTC-8</option>
            <option value="-7">UTC-7</option>
            <option value="-6">UTC-6</option>
            <option value="-5">UTC-5</option>
            <option value="-4">UTC-4</option>
            <option value="-3">UTC-3</option>
            <option value="-2">UTC-2</option>
            <option value="-1">UTC-1</option>
            <option value="0">UTC</option>
            <option value="1">UTC+1</option>
            <option value="2">UTC+2</option>
            <option value="2">UTC+2</option>
            <option value="3">UTC+3</option>
            <option value="4">UTC+4</option>
            <option value="5">UTC+5</option>
            <option value="6">UTC+6</option>
            <option value="7">UTC+7</option>
            <option value="8">UTC+8</option>
            <option value="9">UTC+9</option>
            <option value="10">UTC+10</option>
            <option value="11">UTC+11</option>
            <option value="12">UTC+12</option>
            <option value="13">UTC+13</option>
          </select>
        </div>

        <div class="input-container">
          <label for="timeOffset" class="label">
            Time offset (in second)
          </label>
          <input
            type="number"
            class="input"
            name="timeOffset"
            id="timeOffset"
            value="0"
          />
        </div>
      </div>


      <div class="container">
        <span class="title">Sleep mode</span>

        <div class="input-container">
          <label for="sleepModeEnable" class="label">Enable</label>
          <select
            id="sleepModeEnable"
            class="input"
          >
            <option value="true">Yes</option>
            <option value="false">No</option>
          </select>
        </div>

        <div class="input-container">
          <label for="" class="label">From</label>
          <div class="time-input-container">
            <input class="time-segment-input" value="0" type="number" max="23" min="0" id="sleepModeFromHour"/>:
            <input class="time-segment-input" value="0" type="number" max="59" min="0" id="sleepModeFromMinute"/>:
            <input class="time-segment-input" value="0" type="number" max="59" min="0" id="sleepModeFromSecond"/>
          </div>
        </div>

        <div class="input-container">
          <label for="" class="label">To</label>
          <div class="time-input-container">
            <input class="time-segment-input" value="0" type="number" max="23" min="0" id="sleepModeToHour"/>:
            <input class="time-segment-input" value="0" type="number" max="59" min="0" id="sleepModeToMinute"/>:
            <input class="time-segment-input" value="0" type="number" max="59" min="0" id="sleepModeToSecond"/>
          </div>
        </div>
      </div>

      <div class="save-button-container">
        <div class="save-button">Save</div>
      </div>
    </div>

    <script src="script.js"></script>
  </body>
</html>


)rawliteral";