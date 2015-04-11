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

#include "waspscan.h"

void show_help()
{
    printf("WASPscan: Detection of exoplanet transits\n\n");
    printf(" -f  --filename              Log filename\n");
    printf(" -0  --min                   Minimum orbital period in days\n");
    printf(" -1  --max                   Maximum orbital period in days\n");
    printf(" -h  --help                  Show help\n");
}

int main(int argc, char* argv[])
{
    int i, series_length;
    float timestamp[MAX_SERIES_LENGTH];
    float series[MAX_SERIES_LENGTH];
    int endpoints[MAX_SERIES_LENGTH];
    int no_of_sections;
    char log_filename[256];
    char name[256], title[256];
    float orbital_period_days;
    float minimum_period_days = 0;
    float maximum_period_days = 0;

    /* if no options given then show help */
    if (argc <= 1) {
        show_help();
        return 1;
    }

    /* no filename specified */
    log_filename[0]=0;

    /* parse the options */
    for (i = 1; i < argc; i++) {
        /* log filename */
        if ((strcmp(argv[i],"-f")==0) ||
            (strcmp(argv[i],"--filename")==0)) {
            i++;
            if (i < argc) {
                sprintf(log_filename,"%s",argv[i]);
            }
        }
        /* Minimum orbital period */
        if ((strcmp(argv[i],"-0")==0) ||
            (strcmp(argv[i],"--min")==0)) {
            i++;
            if (i < argc) {
                minimum_period_days = atof(argv[i]);
            }
        }
        /* Maximum orbital period */
        if ((strcmp(argv[i],"-1")==0) ||
            (strcmp(argv[i],"--max")==0)) {
            i++;
            if (i < argc) {
                maximum_period_days = atof(argv[i]);
            }
        }
        /* show help */
        if ((strcmp(argv[i],"-h")==0) ||
                (strcmp(argv[i],"--help")==0)) {
            show_help();
            return 0;
        }
        if ((strcmp(argv[i],"-v")==0) ||
                (strcmp(argv[i],"--version")==0)) {
            printf("Version %.2f\n",VERSION);
            return 0;
        }
    }

    if (log_filename[0]==0) {
        printf("No log file specified\n");
        return -1;
    }

    if (maximum_period_days == 0) {
        printf("No maximum orbital period specified\n");
        return -2;
    }

    if (maximum_period_days <= minimum_period_days) {
        printf("Maximum orbital period must be greater ");
        printf("than the minimum orbital period\n");
        return -3;
    }

    /* get the name of the scan from the log filename */
    scan_name(log_filename, name);

    /* read the data */
    series_length = logfile_load(log_filename,
                                 timestamp,
                                 series,
                                 MAX_SERIES_LENGTH);
    printf("%d values loaded\n", series_length);

    no_of_sections = detect_endpoints(timestamp, series_length,
                                      endpoints);
    if (no_of_sections == 0) {
        printf("No sections detected in the time series\n");
        return -4;
    }

    /*orbital_period_days = 1.3382282f;*/

    orbital_period_days =
        detect_orbital_period(timestamp,
                              series, series_length,
                              minimum_period_days,
                              maximum_period_days,
                              SEARCH_INCREMENT_DAYS);
    if (orbital_period_days == 0) {
        printf("No transits detected\n");
        return -5;
    }

    printf("orbital_period_days %.6f\n",orbital_period_days);

	orbital_period_days = 2.5199464f;

    sprintf(title,"SuperWASP Light Curve for %s",name);
    gnuplot_light_curve_distribution(title,
                                     timestamp, series, series_length,
                                     "result_light_curve.png",
                                     1024, 640,
                                     0.44,0.93,
                                     "TAMUZ corrected processed flux (micro Vega)",
                                     orbital_period_days);
    gnuplot_light_curve(title,
                        timestamp, series, series_length,
                        "result_light_curve_simple.png",
                        1024, 640,
                        0.44,0.93,
                        "TAMUZ corrected processed flux (micro Vega)",
                        orbital_period_days);

    return 0;
}
