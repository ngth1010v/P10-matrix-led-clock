// ================================
// DOM elements
// ================================

const internetWifiName = document.getElementById("internetWifiName");
const internetWifiPassword = document.getElementById("internetWifiPassword");

const configWifiName = document.getElementById("configWifiName");
const configWifiPassword = document.getElementById("configWifiPassword");

const timezone = document.getElementById("timezone");
const timeOffset = document.getElementById("timeOffset");

const sleepModeEnable = document.getElementById("sleepModeEnable");

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

const saveButton = document.querySelector(".save-button");


// ================================
// Load config from ESP32
// ================================

async function loadConfig() {
  try {
    const response = await fetch("/api/config");

    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const config = await response.json();

    // Internet WiFi
    internetWifiName.value =
      config.internetWifiName ?? "";

    internetWifiPassword.value =
      config.internetWifiPassword ?? "";


    // Config WiFi
    configWifiName.value =
      config.configWifiName ?? "";

    configWifiPassword.value =
      config.configWifiPassword ?? "";


    // Time
    timezone.value =
      config.timezone ?? 0;

    timeOffset.value =
      config.timeOffset ?? 0;


    // Sleep mode
    sleepModeEnable.value =
      String(config.sleepModeEnable ?? false);


    // Sleep mode - From
    if (config.sleepModeFrom) {
      sleepModeFromHour.value =
        config.sleepModeFrom.hour ?? 0;

      sleepModeFromMinute.value =
        config.sleepModeFrom.minute ?? 0;

      sleepModeFromSecond.value =
        config.sleepModeFrom.second ?? 0;
    }


    // Sleep mode - To
    if (config.sleepModeTo) {
      sleepModeToHour.value =
        config.sleepModeTo.hour ?? 0;

      sleepModeToMinute.value =
        config.sleepModeTo.minute ?? 0;

      sleepModeToSecond.value =
        config.sleepModeTo.second ?? 0;
    }

  } catch (error) {
    console.error("Failed to load config:", error);

    alert("Failed to load configuration.");
  }
}


// ================================
// Build config from HTML
// ================================

function getConfigFromForm() {
  return {
    // Internet WiFi
    internetWifiName:
      internetWifiName.value,

    internetWifiPassword:
      internetWifiPassword.value,


    // Config WiFi
    configWifiName:
      configWifiName.value,

    configWifiPassword:
      configWifiPassword.value,


    // Time
    timezone:
      Number(timezone.value),

    timeOffset:
      Number(timeOffset.value),


    // Sleep mode
    sleepModeEnable:
      sleepModeEnable.value === "true",


    // Sleep mode - From
    sleepModeFrom: {
      hour:
        Number(sleepModeFromHour.value),

      minute:
        Number(sleepModeFromMinute.value),

      second:
        Number(sleepModeFromSecond.value)
    },


    // Sleep mode - To
    sleepModeTo: {
      hour:
        Number(sleepModeToHour.value),

      minute:
        Number(sleepModeToMinute.value),

      second:
        Number(sleepModeToSecond.value)
    }
  };
}


// ================================
// Save config to ESP32
// ================================

async function saveConfig() {
  try {
    const config = getConfigFromForm();

    console.log("Saving config:", config);

    const response = await fetch("/api/config", {
      method: "POST",

      headers: {
        "Content-Type": "application/json"
      },

      body: JSON.stringify(config)
    });


    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }

    const result = await response.json();

    console.log("Save result:", result);

    alert("Configuration saved.");

  } catch (error) {
    console.error("Failed to save config:", error);

    alert("Failed to save configuration.");
  }
}


// ================================
// Event listeners
// ================================

saveButton.addEventListener("click", saveConfig);


// ================================
// Initial load
// ================================

loadConfig();

