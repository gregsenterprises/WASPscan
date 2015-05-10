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

#define LIGHT_CURVE_LENGTH 256

/* temporary files used to create plots */
char * plot_script_filename = "/tmp/SuperWASP.plot";
char * plot_data_filename = "/tmp/SuperWASP.dat";

/**
* @brief creates a gnuplot script
* @param plot_scipt_filename Filename for the plot script
* @param plot_data_filename Filename for the data to be plotted
* @param title Title of the plot
* @param subtitle Subtitle of the plot
* @param subtitle_indent_horizontal X coordinate of the subtitle (0.0-1.0)
* @param subtitle_indent_vertical Y coordinate of the subtitle (0.0-1.0)
* @param min_x The minimum horizontal value
* @param max_x The maximum horizontal value
* @param min_y The minimum vertical value
* @param max_y The maximum vertical value
* @param x_label Label for the horizontal axis
* @param y_label Label for the vertical axis
* @param image_filename Filename to save the plot as
* @param image_width Width of the image to be saved
* @param image_height Height of the image to be saved
* @param field_name
* @param field_number
* @param show_minmax Show minimum and maximum values
* @param plot_points Whether to plot individual samples
* @param runningaverage Whether to show a running average
* @param returns 0 on success
*/
int gnuplot_create_script(char * plot_script_filename,
                          char * plot_data_filename,
                          char * title, char * subtitle,
                          float subtitle_indent_horizontal,
                          float subtitle_indent_vertical,
                          float min_x, float max_x,
                          float min_y, float max_y,
                          char * x_label, char * y_label,
                          char* image_filename,
                          int image_width, int image_height,
                          char * field_name, int field_number,
                          int show_minmax, int plot_points,
                          int runningaverage)
{
    char draw_type[16];
    char file_type[8];
    FILE * fp;

    sprintf(draw_type,"%s","lines");
    if (plot_points != 0) sprintf(draw_type,"%s","points");

    fp = fopen(plot_script_filename,"w");
    if (!fp) return -1;

    fprintf(fp,"%s","reset\n");
    fprintf(fp,"set title \"%s\"\n",title);
    if (strlen(subtitle) > 0) {
        fprintf(fp,"set label \"%s\" at screen %f, screen %f\n",
                subtitle,subtitle_indent_horizontal,
                subtitle_indent_vertical);
    }
    fprintf(fp,"set xrange [%f:%f]\n", min_x, max_x);
    fprintf(fp,"set yrange [%f:%f]\n", min_y, max_y);
    fprintf(fp,"set lmargin 9\n");
    fprintf(fp,"set rmargin 2\n");
    fprintf(fp,"set xlabel \"%s\"\n", x_label);
    fprintf(fp,"set ylabel \"%s\"\n", y_label);
    fprintf(fp,"set grid\n");
    fprintf(fp,"set key right bottom\n");

    sprintf(file_type,"%s","png");
    if (strlen(image_filename) > 0) {
        if (strstr(image_filename,".jp")!=NULL) {
            sprintf(file_type,"%s","jpeg");
        }

        fprintf(fp,"set terminal %s size %d,%d\n",
                file_type, image_width, image_height);

        fprintf(fp,"set output \"%s\"\n", image_filename);
        fprintf(fp,"plot ");

        fprintf(fp,"\"%s\" using 1:%d ",plot_data_filename, field_number);
        if ((show_minmax != 0) || (runningaverage != 0)) {
            fprintf(fp,"title \"%s\" with %s", field_name, draw_type);
        }
        else {
            fprintf(fp,"notitle with %s",draw_type);
        }

        if (runningaverage != 0) {
            fprintf(fp,", \"%s\" using 1:5 title \"Running\" with %s",
                    plot_data_filename, draw_type);
        }

        if (show_minmax != 0) {
            fprintf(fp,", \"%s\" using 1:3 title \"Min\" with %s",
                    plot_data_filename, draw_type);
            fprintf(fp,", \"%s\" using 1:4 title \"Max\" with %s",
                    plot_data_filename, draw_type);
        }

        fprintf(fp,"\n");
    }

    fclose(fp);
    return 0;
}

/**
* @brief Saves a data series to file
* @param timestamp Array containing times for each entry
* @param series Array containing values for each entry
* @param series_length Length of the Array
* @param plot_data_filename Filename to save as
* @returns 0 on success
*/
int gnuplot_save_data(float timestamp[], float series[],
                      int series_length,
                      char * plot_data_filename)
{
    FILE * fp = fopen(plot_data_filename,"w");
    if (!fp) return -1;
    for (int i = 0; i < series_length; i++) {
        fprintf(fp,"%.8f %.8f\n", timestamp[i], series[i]);
    }
    fclose(fp);
    return 0;
}

/**
* @brief Returns the min and max values within a data series
* @param series Array containing values
* @param series_length Length of the array
* @param range_min Returned minimum value
* @param range_max Returned maximum value
*/
void gnuplot_get_range(float series[], int series_length,
                       float * range_min, float * range_max)
{
    for (int i = 0; i < series_length; i++) {
        if (i > 0) {
            if (series[i] < *range_min) {
                *range_min = series[i];
            }
            if (series[i] > *range_max) {
                *range_max = series[i];
            }
        }
        else {
            *range_min = series[i];
            *range_max = series[i];
        }
    }
}

/**
* @brief Plots the full series of points
* @param title Title for the plot
* @param timestamp Array containing times for each entry
* @param series Array containing values for each entry
* @param series_length Length of the Array
* @param image_filename Filename for the image to save as
* @param image_width Width of the image to be saved
* @param image_height Height of the image to be saved
* @param subtitle_indent_horizontal X coordinate of the subtitle (0.0-1.0)
* @param subtitle_indent_vertical Y coordinate of the subtitle (0.0-1.0)
* @param axis_label Label for the vertical axis
* @returns result of the call to system()
*/
int gnuplot_distribution(char * title,
                         float timestamp[],
                         float series[], int series_length,
                         char * image_filename,
                         int image_width, int image_height,
                         float subtitle_indent_horizontal,
                         float subtitle_indent_vertical,
                         char * axis_label)
{
    char subtitle[256];
    float mean, variance;
    float range_min=0;
    float range_max=0;
    float time_min=0;
    float time_max=0;
    char commandstr[256];

    sprintf(subtitle,"%s","");

    if (gnuplot_save_data(timestamp, series, series_length,
                          (char*)plot_data_filename) != 0) {
        return -1;
    }

    gnuplot_get_range(timestamp, series_length,
                      &time_min, &time_max);
    if (time_max == time_min) {
        return -2;
    }

    mean = detect_mean(series, series_length);
    variance = detect_variance(series, series_length, mean);
    range_min = mean - variance*4;
    range_max = mean + variance*4;

    if (gnuplot_create_script((char*)plot_script_filename,
                              (char*)plot_data_filename,
                              title, subtitle,
                              subtitle_indent_horizontal,
                              subtitle_indent_vertical,
                              time_min, time_max,
                              range_min, range_max,
                              "Time", axis_label,
                              image_filename,
                              image_width, image_height,
                              "Flux", 2, 0, 1, 0) != 0) {
        return -4;
    }

    sprintf(commandstr,"gnuplot %s", plot_script_filename);
    return system(commandstr);
}

/**
* @brief Plots a light curve
* @param title Title for the plot
* @param timestamp Array containing times for each entry
* @param series Array containing values for each entry
* @param series_length Length of the Array
* @param image_filename Filename for the image to save as
* @param image_width Width of the image to be saved
* @param image_height Height of the image to be saved
* @param subtitle_indent_horizontal X coordinate of the subtitle (0.0-1.0)
* @param subtitle_indent_vertical Y coordinate of the subtitle (0.0-1.0)
* @param axis_label Label for the vertical axis
* @param period_days Orbital period in days
* @param vertical_scale Vertical scaling factor
* @returns result of the call to system()
*/
int gnuplot_light_curve(char * title,
                        float timestamp[],
                        float series[], int series_length,
                        char * image_filename,
                        int image_width, int image_height,
                        float subtitle_indent_horizontal,
                        float subtitle_indent_vertical,
                        char * axis_label,
                        float period_days,
                        float vertical_scale)
{
    char subtitle[256];
    float mean, variance;
    float range_min=0;
    float range_max=0;
    float time_min=0;
    float time_max=0;
    char commandstr[LIGHT_CURVE_LENGTH];
    float curve[LIGHT_CURVE_LENGTH];
    float density[LIGHT_CURVE_LENGTH];
    float phase[LIGHT_CURVE_LENGTH];
    int i, offset;

    sprintf(subtitle,"Orbital Period %.5f days",period_days);

    for (i = 0; i < LIGHT_CURVE_LENGTH; i++) {
        phase[i] = (i*360.0f/LIGHT_CURVE_LENGTH)-180.0f;
    }

    light_curve(timestamp, series, series_length,
                period_days, curve, density, LIGHT_CURVE_LENGTH);

    offset = detect_phase_offset(curve, LIGHT_CURVE_LENGTH);
    adjust_curve(curve, LIGHT_CURVE_LENGTH, offset);

    if (gnuplot_save_data(phase, curve, LIGHT_CURVE_LENGTH,
                          (char*)plot_data_filename) != 0) {
        return -1;
    }

    gnuplot_get_range(timestamp, series_length,
                      &time_min, &time_max);
    if (time_max == time_min) {
        return -2;
    }

    mean = detect_mean(curve, LIGHT_CURVE_LENGTH);
    variance = detect_variance(curve, LIGHT_CURVE_LENGTH, mean);
    range_min = mean - (variance*8*vertical_scale);
    range_max = mean + (variance*8*vertical_scale);

    if (gnuplot_create_script((char*)plot_script_filename,
                              (char*)plot_data_filename,
                              title, subtitle,
                              subtitle_indent_horizontal,
                              subtitle_indent_vertical,
                              -180, 180,
                              range_min, range_max,
                              "Phase", axis_label,
                              image_filename,
                              image_width, image_height,
                              "Magnitude", 2, 0, 0, 0) != 0) {
        return -4;
    }

    sprintf(commandstr,"gnuplot %s", plot_script_filename);
    return system(commandstr);
}

/**
* @brief Plots a light curve as a distribution of samples
* @param title Title for the plot
* @param timestamp Array containing times for each entry
* @param series Array containing values for each entry
* @param series_length Length of the Array
* @param image_filename Filename for the image to save as
* @param image_width Width of the image to be saved
* @param image_height Height of the image to be saved
* @param subtitle_indent_horizontal X coordinate of the subtitle (0.0-1.0)
* @param subtitle_indent_vertical Y coordinate of the subtitle (0.0-1.0)
* @param axis_label Label for the vertical axis
* @param period_days Orbital period in days
* @param vertical_scale Vertical scaling factor
* @returns result of the call to system()
*/
int gnuplot_light_curve_distribution(char * title,
                                     float timestamp[],
                                     float series[], int series_length,
                                     char * image_filename,
                                     int image_width, int image_height,
                                     float subtitle_indent_horizontal,
                                     float subtitle_indent_vertical,
                                     char * axis_label,
                                     float period_days,
                                     float vertical_scale)
{
    char subtitle[256];
    float mean, variance, adjust;
    float range_min=0;
    float range_max=0;
    float time_min=0;
    float time_max=0;
    char commandstr[256];
    float timestamp_curve[MAX_SERIES_LENGTH];
    int i, offset;
    float curve[LIGHT_CURVE_LENGTH];
    float density[LIGHT_CURVE_LENGTH];

    sprintf(subtitle,"Orbital Period %.5f days",period_days);

    light_curve(timestamp, series, series_length,
                period_days, curve, density, LIGHT_CURVE_LENGTH);
    offset = detect_phase_offset(curve, LIGHT_CURVE_LENGTH);
    adjust = (period_days/2) - (offset*period_days/LIGHT_CURVE_LENGTH);

    for (i = 0; i < series_length; i++) {
        timestamp_curve[i] =
            (fmod((timestamp[i]/(60*60*24))+adjust,period_days) * 360 /
             period_days) - 180.0f;
    }

    if (gnuplot_save_data(timestamp_curve, series, series_length,
                          (char*)plot_data_filename) != 0) {
        return -1;
    }

    gnuplot_get_range(timestamp, series_length,
                      &time_min, &time_max);
    if (time_max == time_min) {
        return -2;
    }

    mean = detect_mean(series, series_length);
    variance = detect_variance(series, series_length, mean);
    range_min = mean - variance*3*vertical_scale;
    range_max = mean + variance*3*vertical_scale;

    if (gnuplot_create_script((char*)plot_script_filename,
                              (char*)plot_data_filename,
                              title, subtitle,
                              subtitle_indent_horizontal,
                              subtitle_indent_vertical,
                              -180, 180,
                              range_min, range_max,
                              "Phase", axis_label,
                              image_filename,
                              image_width, image_height,
                              "Magnitude", 2, 0, 1, 0) != 0) {
        return -4;
    }

    sprintf(commandstr,"gnuplot %s", plot_script_filename);
    return system(commandstr);
}
