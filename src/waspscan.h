/*
    WASPscan: Detection of exoplanet transits
    Copyright (C) 2015 Bob Mottram
    fuzzgun@gmail.com

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

#ifndef WASPSCAN_H
#define WASPSCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define VERSION 0.01

#define SEARCH_INCREMENT_DAYS 0.00001
#define MAX_SEARCH_STEPS      1000000

#define MAX_SERIES_LENGTH 100000

float detect_mean(float series[], int series_length);
float detect_variance(float series[], int series_length, float mean);
int logfile_load(char * filename, float timestamp[],
                 float series[], int max_series_length);
int gnuplot_distribution(char * title,
                         float timestamp[],
                         float series[], int series_length,
                         char * image_filename,
                         int image_width, int image_height,
                         float subtitle_indent_horizontal,
                         float subtitle_indent_vertical,
                         char * axis_label);
int gnuplot_light_curve(char * title,
                        float timestamp[],
                        float series[], int series_length,
                        char * image_filename,
                        int image_width, int image_height,
                        float subtitle_indent_horizontal,
                        float subtitle_indent_vertical,
                        char * axis_label,
                        float period_days);
int gnuplot_light_curve_distribution(char * title,
                                     float timestamp[],
                                     float series[], int series_length,
                                     char * image_filename,
                                     int image_width, int image_height,
                                     float subtitle_indent_horizontal,
                                     float subtitle_indent_vertical,
                                     char * axis_label,
                                     float period_days);
void fft1D(float series[], int series_length, float freq[]);
int detect_endpoints(float timestamp[], int series_length,
                     int endpoints[]);
void light_curve(float timestamp[],
                 float series[], int series_length,
                 float period_days,
                 float curve[], float density[], int curve_length);
void scan_name(char * filename, char * result);
float detect_orbital_period(float timestamp[],
                            float series[], int series_length,
                            float min_period_days,
                            float max_period_days,
                            float increment_days);
int detect_phase_offset(float curve[], int curve_length);
void adjust_curve(float curve[], int curve_length, int offset);

#endif
