#include "dateTime.h"
#include "loggerSampleData.h"
#include "loggerHardware.h"
#include "loggerConfig.h"
#include "loggerData.h"
#include "virtual_channel.h"
#include "imu.h"
#include "ADC.h"
#include "timer.h"
#include "PWM.h"
#include "GPIO.h"
#include "OBD2.h"
#include "sampleRecord.h"
#include "gps.h"
#include "geopoint.h"
#include "predictive_timer_2.h"
#include "linear_interpolate.h"
#include "printk.h"


int populate_sample_buffer(ChannelSample * samples,  size_t count, size_t currentTicks){
	unsigned short highestRate = SAMPLE_DISABLED;
	for (size_t i = 0; i < count; i++){
		unsigned short sampleRate = samples->sampleRate;
		if (currentTicks % sampleRate == 0){
			highestRate = HIGHER_SAMPLE_RATE(sampleRate, highestRate);
			size_t channelIndex = samples->channelIndex;
			float value = samples->get_sample(channelIndex); //polymorphic behavior
			if (get_channel(samples->channelId)->precision == 0){
				samples->intValue = (int)value;
			}
			else{
				samples->floatValue = value;
			}
		}
		else{
			samples->intValue = NIL_SAMPLE;
		}
		samples++;
	}
	return highestRate;
}


void init_channel_sample_buffer(LoggerConfig *loggerConfig, ChannelSample * samples, size_t channelCount){
	ChannelSample *sample = samples;

	for (int i=0; i < CONFIG_ADC_CHANNELS; i++){
		ADCConfig *config = &(loggerConfig->ADCConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_analog_sample;
			sample++;
		}
	}

	for (int i = 0; i < CONFIG_IMU_CHANNELS; i++){
		ImuConfig *config = &(loggerConfig->ImuConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_imu_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_TIMER_CHANNELS; i++){
		TimerConfig *config = &(loggerConfig->TimerConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_timer_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_GPIO_CHANNELS; i++){
		GPIOConfig *config = &(loggerConfig->GPIOConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_gpio_sample;
			sample++;
		}
	}

	for (int i=0; i < CONFIG_PWM_CHANNELS; i++){
		PWMConfig *config = &(loggerConfig->PWMConfigs[i]);
		if (config->cfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(config->cfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_pwm_sample;
			sample++;
		}
	}

	{
		OBD2Config *obd2Config = &(loggerConfig->OBD2Configs);
		size_t enabledPids = obd2Config->enabledPids;
		for (size_t i = 0; i < enabledPids; i++){
			ChannelConfig *cc = &obd2Config->pids[i].cfg;
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_obd2_sample;
			sample++;
		}
	}
	{
		size_t virtualChannelCount = get_virtual_channel_count();
		for (size_t i = 0; i < virtualChannelCount; i++){
			VirtualChannel *vc = get_virtual_channel(i);
			ChannelConfig *cc = &vc->config;
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = i;
			sample->get_sample = get_virtual_channel_value;
			sample++;
		}
	}
	{
		GPSConfig *gpsConfig = &(loggerConfig->GPSConfigs);
		unsigned short gpsSampleRate = gpsConfig->sampleRate;
		if (gpsSampleRate != SAMPLE_DISABLED){
			if (gpsConfig->positionEnabled){
				sample->channelId = CHANNEL_Latitude;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_latitude;
				sample->get_sample = get_gps_sample;
				sample++;

				sample->channelId = CHANNEL_Longitude;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_longitude;
				sample->get_sample = get_gps_sample;
				sample++;
			}

			if (gpsConfig->speedEnabled){
				sample->channelId = CHANNEL_Speed;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_speed;
				sample->get_sample = get_gps_sample;
				sample++;
			}

			if (gpsConfig->timeEnabled){
				sample->channelId = CHANNEL_Time;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_time;
				sample->get_sample = get_gps_sample;
				sample++;
			}

			if (gpsConfig->satellitesEnabled){
				sample->channelId = CHANNEL_GPSSats;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_satellites;
				sample->get_sample = get_gps_sample;
				sample++;
			}

			if (gpsConfig->distanceEnabled){
				sample->channelId = CHANNEL_Distance;
				sample->sampleRate = gpsSampleRate;
				sample->intValue = NIL_SAMPLE;
				sample->channelIndex = gps_channel_distance;
				sample->get_sample = get_gps_sample;
				sample++;
			}
		}
	}

	{
		LapConfig *trackConfig = &(loggerConfig->LapConfigs);
		if (trackConfig->lapCountCfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(trackConfig->lapCountCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_lapcount;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->lapTimeCfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(trackConfig->lapTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_laptime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->sectorCfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(trackConfig->sectorCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_sector;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->sectorTimeCfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(trackConfig->sectorTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_sectortime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}

		if (trackConfig->predTimeCfg.sampleRate != SAMPLE_DISABLED){
			ChannelConfig *cc = &(trackConfig->predTimeCfg);
			sample->channelId = cc->channeId;
			sample->sampleRate = cc->sampleRate;
			sample->intValue = NIL_SAMPLE;
			sample->channelIndex = lap_stat_channel_predtime;
			sample->get_sample = get_lap_stat_sample;
			sample++;
		}
	}
}


float get_mapped_value(float value, ScalingMap *scalingMap){
	unsigned short *bins;
	unsigned int bin, nextBin;

	bins = scalingMap->rawValues + ANALOG_SCALING_BINS - 1;
	bin = nextBin = ANALOG_SCALING_BINS - 1;

	while (value < *bins && bin > 0){
		bins--;
		bin--;
	}
	if (bin == 0 && value < *bins){
		return scalingMap->scaledValues[0];
	}
	else{
		nextBin = bin;
		if (bin < ANALOG_SCALING_BINS - 1){
			nextBin++;
		}
		else{
			return scalingMap->scaledValues[ANALOG_SCALING_BINS - 1];
		}
	}
	float x1 = (float)scalingMap->rawValues[bin];
	float y1 = scalingMap->scaledValues[bin];
	float x2 = (float)scalingMap->rawValues[nextBin];
	float y2 = scalingMap->scaledValues[nextBin];
	float scaled = LinearInterpolate(value,x1,y1,x2,y2);
	return scaled;
}

float get_analog_sample(int channelId){
	LoggerConfig * loggerConfig = getWorkingLoggerConfig();
	ADCConfig *ac = &(loggerConfig->ADCConfigs[channelId]);
	unsigned int value = ADC_read(channelId);
	float analogValue = 0;
	switch(ac->scalingMode){
		case SCALING_MODE_RAW:
			analogValue = value * SCALING_5V;
			break;
		case SCALING_MODE_LINEAR:
			analogValue = (ac->linearScaling * (float)value);
			break;
		case SCALING_MODE_MAP:
			analogValue = get_mapped_value((float)value,&(ac->scalingMap));
			break;
		default:
			analogValue = -1;
			break;
	}
	return analogValue;
}

float get_timer_sample(int channelId){
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	TimerConfig *c = &(loggerConfig->TimerConfigs[channelId]);
	unsigned int value = timer_get_period(channelId);
	float timerValue = 0;
	unsigned int scaling = c->calculatedScaling;
	switch (c->mode){
		case MODE_LOGGING_TIMER_RPM:
			timerValue = TIMER_PERIOD_TO_RPM(value, scaling);
			break;
		case MODE_LOGGING_TIMER_FREQUENCY:
			timerValue = TIMER_PERIOD_TO_HZ(value, scaling);
			break;
		case MODE_LOGGING_TIMER_PERIOD_MS:
			timerValue = TIMER_PERIOD_TO_MS(value, scaling);
			break;
		case MODE_LOGGING_TIMER_PERIOD_USEC:
			timerValue = TIMER_PERIOD_TO_USEC(value, scaling);
			break;
		default:
			timerValue = -1;
			break;
	}
	return timerValue;
}

float get_pwm_sample(int channelId){
	LoggerConfig *loggerConfig = getWorkingLoggerConfig();
	PWMConfig *c = &(loggerConfig->PWMConfigs[channelId]);
	float pwmValue = 0;
	switch (c->loggingMode){
		case MODE_LOGGING_PWM_PERIOD:
			pwmValue = PWM_channel_get_period(channelId);
			break;
		case MODE_LOGGING_PWM_DUTY:
			pwmValue = PWM_get_duty_cycle(channelId);
			break;
		case MODE_LOGGING_PWM_VOLTS:
			pwmValue = PWM_get_duty_cycle(channelId) * PWM_VOLTAGE_SCALING;
			break;
		default:
			pwmValue = -1;
			break;
	}
	return pwmValue;
}

float get_obd2_sample(int channelId){
	return (float)OBD2_get_current_PID_value(channelId);
}

float get_gpio_sample(int channelId){
	int gpioValue = GPIO_get(channelId);
	return (float)gpioValue;
}

float get_imu_sample(int channelId){
	LoggerConfig *config = getWorkingLoggerConfig();
	ImuConfig *c = &(config->ImuConfigs[channelId]);
	float value = imu_read_value(channelId, c);
	return value;
}

float get_gps_sample(int channelId){
	float value = 0;
	switch(channelId){
		case gps_channel_latitude:
			value = getLatitude();
			break;
		case gps_channel_longitude:
			value = getLongitude();
			break;
		case gps_channel_speed:
			value = getGPSSpeed() *  0.621371192; //convert to MPH
			break;
		case gps_channel_time:
         {
            /*
             * XXX: Hack.  Doing this until backend can take uint64_t and we
             *      can give time since epoch.  getUTCTime returned time as
             *      HHMMSS.MMM
             */
            const DateTime dt = getLastFixDateTime();
            value = ((float) dt.millisecond) / 1000;
            value += (float) dt.second;
            value += ((float) dt.minute) * 100;
            value += ((float) dt.hour) * 10000;
            break;
         }
		case gps_channel_distance:
			value = getGpsDistance();
			break;
		case gps_channel_satellites:
			value = getSatellitesUsedForPosition();
			break;
		default:
			value = -1;
			break;
	}
	return value;
}

float get_lap_stat_sample(int channelId){
   float value = 0;
   switch(channelId){
   case lap_stat_channel_lapcount:
      value = (float) getLapCount();
      break;
   case lap_stat_channel_laptime:
      value = getLastLapTime();
      break;
   case lap_stat_channel_sector:
      value = (float) getLastSector();
      break;
   case lap_stat_channel_sectortime:
      value = getLastSectorTime();
      break;
   case lap_stat_channel_predtime:
      {
         GeoPoint gp;
         populateGeoPoint(&gp);
         float ssff = getSecondsSinceFirstFix();
         // pred time is in seconds, value is in minutes.
         value = getPredictedTime(gp, ssff) / 60;
         break;
      }
   default:
      value = -1;
      break;
   }
   return value;
}
