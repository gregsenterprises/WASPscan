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

#define VERSION 0.01

#define MAX_SERIES_LENGTH 640000

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

#endif
