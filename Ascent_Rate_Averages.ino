static float rate = 0;
static float ratetotal = 0;
static float prev_alt_array[5];
static unsigned long prev_time_array[5];


void AscentRate_Array_Update() {        //Updates the values in the array with most recent data
  for (int i=4; i>0; i--) {
    prev_alt_array[i] = prev_alt_array[i-1];
    prev_time_array[i] = prev_time_array[i-1];
  }

  prev_alt_array[0] = alt_feet;   // most recent alt array value is the one just calculated
  prev_time_array[0] = millis();  // most recent time array value set to millis()
}

float get_altrate_avg() {         // returns the recent average ascent/descent rate in feet/sec
  ratetotal = 0;
  
  for (int i=0; i<4; i++) {
    ratetotal += (prev_alt_array[i] - prev_alt_array[i+1])/(prev_time_array[i] - prev_time[i+1])*1000;
    rate = ratetotal/(i+1);         //given in feet per second
  }

  return rate;
}
