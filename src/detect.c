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

/* length of the light curve used for transit detection */
#define DETECT_CURVE_LENGTH 256

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

/**
 * @brief Calculates the amount of variance for a light curve
 * @param period_days Orbital period in days
 * @param timestamp
 */
static float light_curve_variance(float period_days,
                                  float timestamp[],
                                  float series[],
                                  int series_length,
                                  float curve[], int curve_length)
{
    int i, index;
    float days, variance = 0;

    for (i = 0; i < series_length; i++) {
        days = timestamp[i] / (60.0f*60.0f*24.0f);
        index = (int)(fmod(days,period_days) * curve_length / period_days);
        variance += (series[i] - curve[index])*(series[i] - curve[index]);
    }
    return (float)sqrt(variance/series_length);
}

/**
 * @brief Returns an array containing a light curve for the given orbital period_days
 * @param timestamp Array of imaging times
 * @param series Array containing magnitudes
 * @param series_length The length of the data series
 * @param period_days The expected orbital period
 * @param curve Returned light curve Array
 * @param density Density of samples
 * @param curve_length The number of buckets within the curve
 */
static void light_curve_base(float timestamp[],
                             float series[], int series_length,
                             float period_days,
                             float curve[], float density[], int curve_length)
{
    int i, index;
    float days, max_samples=0;

    for (i = 0; i < curve_length; i++) {
        curve[i] = 0;
        density[i] = 0;
    }

    for (i = 0; i < series_length; i++) {
        days = timestamp[i] / (60.0f*60.0f*24.0f);
        index = (int)(fmod(days,period_days) * curve_length / period_days);
        curve[index] += series[i];
        density[index]++;
    }

    for (i = 0; i < curve_length; i++) {
        if (density[i] > 0) {
            curve[i] /= density[i];
        }
        if (density[i] > max_samples) {
            max_samples = density[i];
        }
    }
    /* normalise */
    for (i = 0; i < curve_length; i++) {
        density[i] /= max_samples;
    }
}

/**
 * @brief Resamples a light curve within bounds.
 *        This is used to disguard outliers which otherwise
 *        cause distraction.
 * @param min_value Minimum value
 * @param max_value Maximum value
 * @param timestamp Array of imaging times
 * @param series Array containing magnitudes
 * @param series_length The length of the data series
 * @param period_days The expected orbital period
 * @param curve Returned light curve Array
 * @param density Density of samples
 * @param curve_length The number of buckets within the curve
 */
static void light_curve_resample(float min_value, float max_value,
                                 float timestamp[],
                                 float series[], int series_length,
                                 float period_days,
                                 float curve[], int curve_length)
{
    int i, index;
    float days;
    int hits[512];

    for (i = 0; i < curve_length; i++) {
        curve[i] = 0;
        hits[i] = 0;
    }

    for (i = 0; i < series_length; i++) {
        if ((series[i] < min_value) ||
            (series[i] > max_value)) {
            continue;
        }
        days = timestamp[i] / (60.0f*60.0f*24.0f);
        index = (int)(fmod(days,period_days) * curve_length / period_days);
        curve[index] += series[i];
        hits[index]++;
    }
    for (i = 0; i < curve_length; i++) {
        if (curve[i] > 0) {
            curve[i] /= hits[i];
        }
    }

    /* fill any holes */
    for (i = 0; i < curve_length; i++) {
        if (curve[i] == 0) {
            if (i > 0) {
                curve[i] = curve[i-1];
            }
            else {
                curve[i] = curve[curve_length-1];
            }
        }
    }
}

/**
 * @brief Returns the average value for all data points
 * @param series Array containing the data points
 * @param series_length Length of the array
 * @returns Average value
 */
float detect_mean(float series[], int series_length)
{
    int i;
    double mean = 0;

    for (i = 0; i < series_length; i++) {
        mean += series[i];
    }
    return (float)(mean/series_length);
}

/**
 * @brief Returns the variance for all data points in a series
 * @param series Array containing data points
 * @param series_length Length of the Array
 * @param mean Pre-calculated mean value
 * @returns Variance value
 */
float detect_variance(float series[], int series_length, float mean)
{
    int i;
    double variance = 0;

    for (i = 0; i < series_length; i++) {
        variance += (series[i] - mean)*(series[i] - mean);
    }
    return (float)sqrt(variance/series_length);
}

/**
 * @brief Returns an array containing a light curve for the given orbital period_days
 * @param timestamp Array of imaging times
 * @param series Array containing magnitudes
 * @param series_length The length of the data series
 * @param period_days The expected orbital period
 * @param curve Returned light curve Array
 * @param density Density of samples
 * @param curve_length The number of buckets within the curve
 */
void light_curve(float timestamp[],
                 float series[], int series_length,
                 float period_days,
                 float curve[], float density[], int curve_length)
{
    float mean, variance;

    light_curve_base(timestamp, series, series_length,
                     period_days, curve, density, curve_length);

    mean = detect_mean(series, series_length);
    variance = detect_variance(series, series_length, mean);

    light_curve_resample(mean - variance, mean + variance,
                         timestamp, series, series_length,
                         period_days, curve, curve_length);
}

/**
 * @brief Detects the array index of the centre of the transit
 * @param curve Array containing light curve magnitudes
 * @param curve_length Length of the array
 * @returns Array index of the centre of the transit
 */
int detect_phase_offset(float curve[], int curve_length)
{
    int i, j, l, offset = 0;
    float minimum = 0, v;
    int search_radius = curve_length*5/100;

    for (i = 0; i < curve_length; i++) {
        v = 0;
        for (j = i-search_radius; j <= i+search_radius; j++) {
            l = j;
            if (l < 0) l += curve_length;
            if (l >= curve_length) l -= curve_length;
            v += curve[l];
        }
        if ((v < minimum) || (minimum == 0)) {
            minimum = v;
            offset = i;
        }
    }
    return offset;
}

/**
 * @brief Adjust the light curve so that the transit is at the centre
 * @param curve Array containing light curve magnitudes
 * @param curve_length Length of the array
 * @param offset Array index of the centre of the transit
 */
void adjust_curve(float curve[], int curve_length, int offset)
{
    int i, index;
    int adjust = (curve_length/2) - offset;
    float new_curve[512];

    for (i = 0; i < curve_length; i++) {
        index = i + adjust;
        if (index < 0) index += curve_length;
        if (index >= curve_length) index -= curve_length;
        new_curve[index] = curve[i];
    }
    for (i = 0; i < curve_length; i++) {
        curve[i] = new_curve[i];
    }
}

/**
 * @brief Attempts to detect the orbital period via the transit method.
 *        This tries many possible periods and looks for a dip in
 *        magnitude.
 * @param timestamp Times for observations
 * @param series Magnitude observations
 * @param series_length Length of the Array
 * @param min_period_days The minimum orbital period in days
 * @param max_period_days The maximum orbital period in days
 * @param increment_days The time increment used within the min/max range
 * @returns The best candidate orbital period, or zero if no transit found
 */
float detect_orbital_period(float timestamp[],
                            float series[], int series_length,
                            float min_period_days,
                            float max_period_days,
                            float increment_days)
{
    float period_days=0;
    float max_response = 0;
    const int expected_width = DETECT_CURVE_LENGTH*2/100;
    const int max_dipped = DETECT_CURVE_LENGTH*9/100;
    const int max_nondipped = DETECT_CURVE_LENGTH*9/100;
    int step = 0;
    int steps = (int)((max_period_days - min_period_days)/increment_days);
    float response[MAX_SEARCH_STEPS];

    if (steps > MAX_SEARCH_STEPS) {
        printf("Maximum number of time steps exceeded\n");
        return 0;
    }

    /* Try different orbital periods in parallel */
#pragma omp parallel for
    for (step = 0; step < steps; step++) {
        float curve[DETECT_CURVE_LENGTH];
        float density[DETECT_CURVE_LENGTH];
        float orbital_period_days = min_period_days + (step*increment_days);

        light_curve(timestamp, series, series_length,
                    orbital_period_days,
                    curve, density, DETECT_CURVE_LENGTH);
        float variance =
            light_curve_variance(orbital_period_days,
                                 timestamp, series,
                                 series_length,
                                 curve, DETECT_CURVE_LENGTH);

        /* calculate the mean */
        float mean = 0;
        int hits = 0;
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if (curve[j]>0) {
                mean += curve[j];
                hits++;
            }
        }
        /* there should be no gaps in the series */
        if (hits < DETECT_CURVE_LENGTH) {
            response[step] = 0;
            continue;
        }
        mean /= hits;

        /* average density of samples */
        float mean_density = 0;
        hits = 0;
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if (density[j] > 0) {
                mean_density += density[j];
                hits++;
            }
        }
        mean_density /= hits;

        /* variation in the density of samples */
        float density_variance = 0;
        hits = 0;
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if (density[j] > 0) {
                density_variance +=
                    (density[j] - mean_density)*
                    (density[j] - mean_density);
                hits++;
            }
        }
        density_variance =
            (float)(density_variance/hits);

        /* find the minimum */
        float minimum = 0;
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            float v = 0;
            hits = 0;
            for (int k = j-expected_width; k <= j+expected_width; k++) {
                int l = k;
                if (l < 0) l += DETECT_CURVE_LENGTH;
                if (l >= DETECT_CURVE_LENGTH) l -= DETECT_CURVE_LENGTH;
                if (curve[l] > 0) {
                    v += curve[l];
                    hits++;
                    if (k == j) {
                        v += curve[l];
                        hits++;
                    }
                }
            }
            if (hits > 0) {
                v /= hits;
                if ((v < minimum) || (minimum == 0)) {
                    minimum = v;
                }
            }
        }

        /* How much difference from the mean? */
        int dipped = 0;
        float threshold_dipped = minimum + ((mean-minimum)*0.2);
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if (curve[j] < threshold_dipped) {
                dipped++;
                if (dipped > max_dipped) {
                    break;
                }
            }
        }
        /* we only expect a small percentage
           of the curve to be dipped */
        if (dipped > max_dipped) {
            dipped = 0;
        }
        if (dipped == 0) {
            response[step] = 0;
            continue;
        }

        /* How much difference from the mean? */
        int nondipped = 0;
        float threshold_upper = mean - ((mean-minimum)*0.2);
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if ((curve[j] < threshold_upper) &&
                (curve[j] > threshold_dipped)) {
                nondipped++;
                if (nondipped > max_nondipped) {
                    break;
                }
            }
        }
        if (nondipped > max_nondipped) {
            response[step] = 0;
            continue;
        }

        variance = 0;
        hits = 0;
        for (int j = 0; j < DETECT_CURVE_LENGTH; j++) {
            if (curve[j] > 0) {
                variance += (curve[j] - mean)*(curve[j] - mean);
                hits++;
            }
        }
        response[step] = 0;
        if (hits > 0) {
            response[step] =
                (mean-minimum) /
                (float)sqrt(variance/hits);
            response[step] =
                (mean-minimum)*dipped*100/(mean*(1+nondipped));
            response[step] /= (density_variance*variance);
        }
    }

    for (int i = 0; i < steps; i++) {
        if (response[i] > max_response) {
            max_response = response[i];
            period_days = min_period_days + (i*increment_days);
        }
    }
    return period_days;
}
