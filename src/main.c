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
    printf(" -h  --help                  Show help\n");
}

int main(int argc, char* argv[])
{
    int i, series_length;
	float series[MAX_SERIES_LENGTH];
	char log_filename[256];

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

	series_length = logfile_load(log_filename, series, MAX_SERIES_LENGTH);
	printf("%d values loaded\n", series_length);

	for (i = 0; i < 10; i++) {
		printf("%.4f\n",series[i]);
	}

    return 0;
}
