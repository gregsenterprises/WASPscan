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

/* field index */
#define TAMFLUX2 3

#include "waspscan.h"

int logfile_load(char * filename, float * series, int max_series_length)
{
	FILE * fp;
	char linestr[512],valuestr[512];
	char * retval = NULL;
	int i,ctr=0,field_index;
	int series_length=0;

	fp = fopen(filename,"r");
	if (!fp) return -1;

	while (!feof(fp)) {
		retval = fgets(linestr,511,fp);
		if (retval != NULL) {
			if (strlen(linestr) < 1) continue;
			if ((linestr[0]=='\\') ||
				(linestr[0]=='|')) {
				continue;
			}
			field_index = 0;
			ctr=0;
			for (i = 0; i < strlen(linestr); i++) {
				if (!((linestr[i]==' ') || (linestr[i]=='\t'))) {
					valuestr[ctr++] = linestr[i];
				}
				else {
					if (ctr > 0) {
						valuestr[ctr]=0;
						if (field_index == TAMFLUX2) {
							series[series_length++] = atof(valuestr);
							if (series_length >= max_series_length) {
								fclose(fp);
								return(series_length);
							}
							break;
						}
						ctr = 0;
						field_index++;
					}
				}
			}
		}
	}

	fclose(fp);
	return series_length;
}
