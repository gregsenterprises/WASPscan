/*
  Copyright (C) 2015 Bob Mottram
  This is based on the example from Rosetta Code
  http://rosettacode.org/wiki/Fast_Fourier_transform#C
  which was originally under GNU Free Documentation License 1.2
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

double PI_LOCAL;
typedef double complex cplx;

void _fft(cplx buf[], cplx out[], int n, int step)
{
    if (step < n) {
        _fft(out, buf, n, step * 2);
        _fft(out + step, buf + step, n, step * 2);

        for (int i = 0; i < n; i += 2 * step) {
            cplx t = cexp(-I * PI_LOCAL * i / n) * out[i + step];
            buf[i / 2]     = out[i] + t;
            buf[(i + n)/2] = out[i] - t;
        }
    }
}

void fft(cplx buf[], int n)
{
    cplx out[n];
    for (int i = 0; i < n; i++) out[i] = buf[i];

    _fft(buf, out, n, 1);
}

void fft1D(float series[], int series_length, float freq[])
{
    int i;
    cplx buf[series_length];

    for (i = 0; i < series_length; i++) {
        buf[i] = series[i] + 0.0f*_Complex_I;
    }
    PI_LOCAL = atan2(1, 1) * 4;
    fft(buf,series_length);
    for (i = 0; i < series_length; i++) {
        freq[i] = creal(buf[i]);
    }
}
