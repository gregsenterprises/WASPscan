/*
  Copyright (C) 2015 Bob Mottram
  bob@robotics.uk.to

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "waspscan.h"

/**
 * @brief Detects the starting and ending indexes of active
 *        data sections within a time series
 * @param timestamp A series of timestamps
 * @param series_length The number of entries in the series
 * @param endpoints An array of returned start and end indexes
 * @returns The number of data sections within the series
 */
int detect_endpoints(float timestamp[], int series_length,
                     int endpoints[])
{
    int start_index = 0;
    float variance;
    float threshold, dt, mean_dt = 0, dt_variance = 0;
    int i,ctr=0;

    for (i = 1; i < series_length; i++) {
        mean_dt += timestamp[i] - timestamp[i-1];
    }
    mean_dt /= (series_length-1);

    for (i = 1; i < series_length; i++) {
        variance = (timestamp[i] - timestamp[i-1]) - mean_dt;
        dt_variance += variance*variance;
    }
    dt_variance = (float)sqrt(dt_variance/(series_length-1));
    printf("dt_mean = %f\n",mean_dt);
    printf("dt_variance = %f\n",dt_variance);
    threshold = dt_variance*10;

    for (i = 1; i < series_length-1; i++) {
        dt = timestamp[i] - timestamp[i-1];
        if (dt > threshold) {
            endpoints[ctr*2] = start_index;
            endpoints[ctr*2+1] = i-1;
            ctr++;
            start_index = i+1;
        }
    }
    endpoints[ctr*2] = -1;
    return ctr;
}
