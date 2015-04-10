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

char * plot_script_filename = "/tmp/SuperWASP.plot";
char * plot_data_filename = "/tmp/SuperWASP.dat";

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
            fprintf(fp,", \"%s\" using 1:5 title \"Running\" with %s", plot_data_filename, draw_type);
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

int gnuplot_save_data(float series[], int series_length,
					  char * plot_data_filename)
{
	int i;
	FILE * fp;

	fp = fopen(plot_data_filename,"w");
	if (!fp) return -1;
	for (i = 0; i < series_length; i++) {
        fprintf(fp,"%.8f %.8f\n", (float)i, series[i]);
	}
	fclose(fp);
	return 0;
}

void gnuplot_get_range(float series[], int series_length,
					   float * range_min, float * range_max)
{
	int i;

	for (i = 0; i < series_length; i++) {
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

int gnuplot_distribution(char * title,
						 float series[], int series_length,
						 char * image_filename,
						 int image_width, int image_height,
						 float subtitle_indent_horizontal,
						 float subtitle_indent_vertical,
						 char * axis_label)
{
    char * subtitle = "";
    float range_min=0;
    float range_max=0;
	char commandstr[256];

	if (gnuplot_save_data(series, series_length,
						  (char*)plot_data_filename) != 0) {
		return -1;
	}

    gnuplot_get_range(series, series_length,
					  &range_min, &range_max);
	if (range_max == range_min) {
		return -2;
	}

    if (gnuplot_create_script((char*)plot_script_filename,
							  (char*)plot_data_filename,
							  title, subtitle,
							  subtitle_indent_horizontal,
							  subtitle_indent_vertical,
							  0, series_length,
							  range_min, range_max,
							  "Time", axis_label,
							  image_filename,
							  image_width, image_height,
							  "Flux", 2, 0, 1, 0) != 0) {
		return -3;
	}

	sprintf(commandstr,"gnuplot %s", plot_script_filename);
    return system(commandstr);
}
