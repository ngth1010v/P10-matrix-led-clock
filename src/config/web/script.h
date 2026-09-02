#pragma once
#include <Arduino.h>

const char script_js[] PROGMEM = R"rawliteral(

// ========================================
// DOM elements
// ========================================

// Internet WiFi
const internetWifiName =
  document.getElementById("internetWifiName");

const internetWifiPassword =
  document.getElementById("internetWifiPassword");


// Config WiFi
const configWifiName =
  document.getElementById("configWifiName");

const configWifiPassword =
  document.getElementById("configWifiPassword");


// Time
const timezone =
  document.getElementById("timezone");

const timeOffset =
  document.getElementById("timeOffset");


// Sleep mode
const sleepModeEnable =
  document.getElementById("sleepModeEnable");

const sleepModeFromHour =
  document.getElementById("sleepModeFromHour");

const sleepModeFromMinute =
  document.getElementById("sleepModeFromMinute");

const sleepModeFromSecond =
  document.getElementById("sleepModeFromSecond");

const sleepModeToHour =
  document.getElementById("sleepModeToHour");

const sleepModeToMinute =
  document.getElementById("sleepModeToMinute");

const sleepModeToSecond =
  document.getElementById("sleepModeToSecond");


// Save button
const saveButton =
  document.querySelector(".save-button");


// ========================================
// Load configuration from ESP32
// ========================================

async function loadConfig() {
  try {
    const response = await fetch("/api/config");

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const config = await response.json();


    // ------------------------------------
    // Internet WiFi
    // ------------------------------------

    internetWifiName.value =
      config.internetWifi?.name ?? "";

    internetWifiPassword.value =
      config.internetWifi?.password ?? "";


    // ------------------------------------
    // Config WiFi
    // ------------------------------------

    configWifiName.value =
      config.configWifi?.name ?? "";

    configWifiPassword.value =
      config.configWifi?.password ?? "";


    // ------------------------------------
    // Time
    // ------------------------------------

    timezone.value =
      config.timezone ?? 0;

    timeOffset.value =
      config.timeOffset ?? 0;


    // ------------------------------------
    // Sleep mode
    // ------------------------------------

    const sleepMode =
      config.sleepMode ?? {};


    sleepModeEnable.value =
      String(sleepMode.enable ?? false);


    // From
    sleepModeFromHour.value =
      sleepMode.from?.hour ?? 0;

    sleepModeFromMinute.value =
      sleepMode.from?.minute ?? 0;

    sleepModeFromSecond.value =
      sleepMode.from?.second ?? 0;


    // To
    sleepModeToHour.value =
      sleepMode.to?.hour ?? 0;

    sleepModeToMinute.value =
      sleepMode.to?.minute ?? 0;

    sleepModeToSecond.value =
      sleepMode.to?.second ?? 0;


  } catch (error) {

    console.error(
      "Failed to load configuration:",
      error
    );

    alert("Failed to load configuration.");
  }
}


// ========================================
// Get SleepMode from form
// ========================================

function getSleepModeFromForm() {
  return {
    enable:
      sleepModeEnable.value === "true",

    from: {
      hour:
        Number(sleepModeFromHour.value),

      minute:
        Number(sleepModeFromMinute.value),

      second:
        Number(sleepModeFromSecond.value)
    },

    to: {
      hour:
        Number(sleepModeToHour.value),

      minute:
        Number(sleepModeToMinute.value),

      second:
        Number(sleepModeToSecond.value)
    }
  };
}


// ========================================
// Get complete configuration from form
// ========================================

function getConfigFromForm() {
  return {

    internetWifi: {
      name:
        internetWifiName.value,

      password:
        internetWifiPassword.value
    },


    configWifi: {
      name:
        configWifiName.value,

      password:
        configWifiPassword.value
    },


    timezone:
      Number(timezone.value),

    timeOffset:
      Number(timeOffset.value),


    sleepMode:
      getSleepModeFromForm()
  };
}


// ========================================
// Save configuration to ESP32
// ========================================

async function saveConfig() {
  try {

    const config =
      getConfigFromForm();


    console.log(
      "Saving configuration:",
      config
    );


    const response = await fetch(
      "/api/config",
      {
        method: "POST",

        headers: {
          "Content-Type":
            "application/json"
        },

        body:
          JSON.stringify(config)
      }
    );


    if (!response.ok) {
      throw new Error(
        `HTTP ${response.status}`
      );
    }


    const result =
      await response.json();


    console.log(
      "Save result:",
      result
    );


    alert("Configuration saved.");


  } catch (error) {

    console.error(
      "Failed to save configuration:",
      error
    );

    alert(
      "Failed to save configuration."
    );
  }
}


// ========================================
// Events
// ========================================

saveButton.addEventListener(
  "click",
  saveConfig
);


// ========================================
// Initial load
// ========================================

loadConfig();


)rawliteral";