waspscan
========
This utility takes log files produced by SuperWASP and searches for exoplanet transits within them, producing plots if any are found. A range of orbital periods can be specified in days within which the search will take place, of if the period is known it can be specified.

For more information see:

```
https://en.wikipedia.org/wiki/SuperWASP
http://wasp-planets.net
```

Installation
------------
Installation is pretty conventional:

```
sudo apt-get install build-essential gnuplot
make
sudo make install
```

You will also need to obtain the SuperWASP log files. These can be ontained from:

```
http://exoplanetarchive.ipac.caltech.edu/docs/SuperWASPBulkDownload.html
```

There are a couple of scripts which contain many *wget* commands which download individual observation series for particular stars. For the currently known exoplanets:

```
mkdir data
cd data
wget http://exoplanetarchive.ipac.caltech.edu/data/ETSS/SuperWASP/metadata/SuperWASP_confirmed_wget.bat
````

And for all the available data:

```
wget http://exoplanetarchive.ipac.caltech.edu/bulk_data_download/SuperWASP_wget.tar.gz
tar -xzvf SuperWASP_wget.tar.gz
```

Once you've downloaded those you can open *SuperWASP_confirmed_wget.bat* in your favourite editor and manually run individual wgets, or make it executable and download all the data sets. Please be warned that downloading all of the data series could take a very long time, so you may want to be selective.

Usage
-----
To search for a transit within a range of orbital periods:

```
waspscan -f data/1SWASP_xyz.tbl --min 0.5 --max 3.0
```

The above will search a particular log file for orbits in the range 0.5 to 3 days. If a transit is found then it will be plotted as *png* files saved to the current directory.

If you already know that a log file contains a transit, and you know the orbital period, then you can produce plots as follows:

```
waspscan -f data/1SWASP_xyz.tbl -p [days]
```
