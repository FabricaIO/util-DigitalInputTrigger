#include"DigitalInputTrigger.h"

/// @brief Creates a generic analog input
/// @param Pin Pin to use
/// @param configFile Full path to config file to use
DigitalInputTrigger::DigitalInputTrigger(int Pin) {
	digital_config.Pin = Pin;
}

/// @brief Starts the Digital Input Trigger
/// @return True on success
bool DigitalInputTrigger::begin() {
	// Waits for time to be set
	int timeout = 0;
	do {
		delay(1000);
		timeout++;
	} while (TimeInterface::getEpoch() < 10000 && timeout < 10);
	// Check if time was set
	if (TimeInterface::getEpoch() < 10000) {
		return false;
	}
	// Set initial time information
	clearTrigger();
	return true;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DigitalInputTrigger::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["Pin"] = digital_config.Pin;
	doc["Mode"]["current"] = digital_config.mode;
	doc["Mode"]["options"][0] = "Input";
	doc["Mode"]["options"][1] = "Pullup";
	doc["Mode"]["options"][2] = "Pulldown";
	doc["Mode"]["options"][3] = "Open Drain";
	doc["Trigger"]["current"] = digital_config.trigger;
	doc["Trigger"]["options"][0] = "Disabled";
	doc["Trigger"]["options"][1] = "Rising";
	doc["Trigger"]["options"][2] = "Falling";
	doc["Trigger"]["options"][3] = "Change";
	doc["Trigger"]["options"][4] = "Low";
	doc["Trigger"]["options"][5] = "High";
	doc["Trigger"]["options"][6] = "Low with Wakeup";
	doc["Trigger"]["options"][7] = "High with Wakeup";
	doc["id"] = digital_config.id;
	doc["taskName"] = task_config.taskName;
	doc["taskPeriod"] = task_config.taskPeriod;
	doc["taskEnabled"] = digital_config.taskEnabled;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @return True on success
bool DigitalInputTrigger::setConfig(String config) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	// Assign loaded values
	digital_config.Pin = doc["Pin"].as<int>();
	digital_config.mode = doc["Mode"]["current"].as<String>();
	digital_config.trigger = doc["Trigger"]["current"].as<String>();
	digital_config.id = doc["id"].as<int>();
	digital_config.taskEnabled = doc["taskEnabled"].as<bool>();
	task_config.taskName = doc["taskName"].as<std::string>();
	task_config.taskPeriod = doc["taskPeriod"].as<long>();

	if(configureInput()) {
		return enableTask(digital_config.taskEnabled);
	}
	return false;
}

/// @brief Configures the pin for use
/// @return True on success
bool DigitalInputTrigger::configureInput() {
	pinMode(digital_config.Pin, modes[digital_config.mode]);
	if (digitalPinToInterrupt(digital_config.Pin) == -1) {
		Logger.println("Pin does not support interrupts");
		return false;
	}
	attachInterrupt(digital_config.Pin, std::bind(&DigitalInputTrigger::trigger, this), triggers[digital_config.trigger]);
	return true;
}

/// @brief Runs the task wanted by the input trigger
/// @param elapsed The time elapsed since last checked
void DigitalInputTrigger::runTask(long elapsed) {
	if (elapsed >= task_config.taskPeriod) {
		if (triggered) {
			ulong time = lastRunTime + elapsedMillis / 1000;
			Logger.println("Event " + String(digital_config.id) + " triggered at " + String(time) + " " + String(elapsedMillis % 1000) + "ms");
			clearTrigger();
		}
	}
}

/// @brief Clears a triggered event
void DigitalInputTrigger::clearTrigger() {
	currentMillis = millis();
	lastRunTime = TimeInterface::getEpoch();
	triggered = false;
}

/// @brief ISR for a triggered event
void DigitalInputTrigger::trigger() {
	if (!triggered) {
		triggered = true;
		elapsedMillis = millis() - currentMillis;
	}
}