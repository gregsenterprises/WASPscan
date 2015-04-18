WASPscan
========
<img src="https://github.com/bashrc/WASPscan/blob/master/images/J233415.png?raw=true" width=640/>

This utility takes log files produced by SuperWASP and searches for exoplanet transits within them, producing plots if any are found. A range of possible orbital periods can be given, or if the period is already known then that can be specified.

For more information see:

    https://en.wikipedia.org/wiki/SuperWASP
    http://wasp-planets.net

Installation
------------
Installation is pretty conventional:

    sudo apt-get install build-essential gnuplot python-astropy
    make
    sudo make install

You will also need to obtain the SuperWASP log files. These can be ontained from:

    http://exoplanetarchive.ipac.caltech.edu/docs/SuperWASPBulkDownload.html

There are a couple of scripts which contain many *wget* commands which download individual observation series for particular stars. For the currently known exoplanets:

    mkdir data
    cd data
    wget http://exoplanetarchive.ipac.caltech.edu/data/ETSS/SuperWASP/metadata/SuperWASP_confirmed_wget.bat

And for all the available data:

    wget http://exoplanetarchive.ipac.caltech.edu/bulk_data_download/SuperWASP_wget.tar.gz
    tar -xzvf SuperWASP_wget.tar.gz

Once you've downloaded those you can open *SuperWASP_confirmed_wget.bat* in your favourite editor and manually run individual wgets, or make it executable and download all the data sets. Please be warned that downloading all of the data series could take a very long time, so you may want to be selective.

File Formats
------------
The two file formats used are *fits* and *tbl*. waspscan only uses the tbl format, so if you have fits files those can be converted with:

    fits2tbl [fits filename] > [table filename]

Or for non-WASP fits files:

    fits2tbl [fits filename] [table index] > [table filename]

Where the table index is a number of the table inside of the fits file.

Usage
-----
To search for a transit within a range of orbital periods:

    waspscan -f data/1SWASP_xyz.tbl --min 0.5 --max 3.0

The above will search a particular log file for orbits in the range 0.5 to 3 days. If a transit is found then it will be plotted as *png* files saved to the current directory.

If you already know that a log file contains a transit, and you know the orbital period, then you can produce plots as follows:

    waspscan -f data/1SWASP_xyz.tbl -p [days]

If you want to scan multiple *tbl* files or *fits* files within a directory there's also a helper script for that purpose:

    waspscandir [minimum period] [maximum period]

Log files will be scanned one by one and if transits are found then plot images will be generated for them within the same directory for subsequent manual review.

Scaling up the search
---------------------
It is also possible to install a daemon which will search through light curves and report its results. The daemon currently only runs with a systemd init system (Fedora, Ubuntu 15.04+, Debian 8+).

    sudo startwaspd --archive [bulk data download url]
                    --start [percent] --end [percent]
                    --min [period days] --max [period days]
                    --email [email address]

The start and end percent values indicate where within the total data set the daemon will begin searching and where it will end. Hence you can distribute the search across multiple machines, each searching only a portion of the data. The --email parameter is optional and will only work if the machine has an email server installed. If an email address is set then when a candidate transit is detected a corresponding email will be sent.

You can check whether the daemon is running with:

    systemctl status waspd

Any candidate transits will be saved into the directory */home/wasp/candidates*
