/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
*
* Contributors: Sam Groveman
*/
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PeriodicTask.h>
#include <TimeInterface.h>
#include <FunctionalInterrupt.h>
#include <Configuration.h>
#include <map>
#include <atomic>

/// @brief Class describing a generic output on a GPIO pin
class DigitalInputTrigger : public PeriodicTask {
	// All methods are protected since this should be inherited not instanced
	protected:
		DigitalInputTrigger(int Pin);
		bool begin();
		String getConfig();
		bool setConfig(String config);
		
		// Mutex for thread safe ISR
		portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

		/// @brief Map for input modes
		std::map<String, int> modes = {{"Input", INPUT}, {"Pullup", INPUT_PULLUP}, {"Pulldown", INPUT_PULLDOWN}, {"Open Drain", OPEN_DRAIN}};

		/// @brief Map for trigger modes
		std::map<String, int> triggers = {{"Disabled", DISABLED}, {"Rising", RISING}, {"Falling", FALLING}, {"Change", CHANGE}, {"Low", ONLOW}, {"High", ONHIGH}, {"Low with Wakeup", ONLOW_WE}, {"High with Wakeup", ONHIGH_WE}};

		/// @brief Output configuration
		struct {
			/// @brief The pin number attached to the output
			int Pin;
			
			/// @brief The mode of the pin
			String mode;
			
			/// @brief The trigger mode of the interrupt
			String trigger;
			
			/// @brief The ID of the input
			int id;

			/// @brief Enables or disables the task
			bool taskEnabled = false;
		} digital_config;

		/// @brief Button triggered event
		std::atomic<bool> triggered{false};

		/// @brief Records time of last interrupt call
		std::atomic<uint64_t> interrupt_time{0};

		/// @brief Milliseconds elapsed since last input trigger
		std::atomic<ulong> elapsedMillis{0};
		
		/// @brief Curent Milliseconds since at last input trigger
		ulong currentMillis = 0;

		/// @brief Unix timestamp at last input trigger
		ulong lastRunTime = 0;

		bool configureInput();
		void clearTrigger();
		void IRAM_ATTR trigger();
		void runTask(ulong elapsed);
		JsonDocument addAdditionalConfig();
};