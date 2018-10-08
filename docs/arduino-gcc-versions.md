Arduino ships also avr-gcc. This is matrix of versions and changes
based on Linux versions.


| Arduino | avr-gcc | differences | test? |
|---------|---------|-------------|-------|
|<=1.0    |none?    | | |
|1.0.1    |4.3.2    |new| |
|1.0.2    |4.3.2    |same as 1.0.1| |
|1.0.3    |4.3.2    |same as 1.0.1| |
|1.0.4    |4.3.2    |same as 1.0.1| |
|1.0.5    |4.3.2    |same as 1.0.1| |
|1.0.6    |4.3.2    |same as 1.0.1| yes |
|1.5      |4.3.2    |same as 1.0.1| |
|1.5.1    |4.3.2    |same as 1.0.1| |
|1.5.2    |4.3.2    |same as 1.0.1| |
|1.5.3    |4.3.2    |same as 1.0.1| |
|1.5.4    |4.3.2    |same as 1.0.1| |
|1.5.5    |4.3.2    |same as 1.0.1| |
|1.5.6-r2 |4.3.2    |same as 1.0.1| |
|1.5.7    |4.8.1    |toolchains upgrade, avrdude 6.0.1avrdude| |
|1.5.8    |4.8.1    |same as 1.5.7| |
|1.6.0    |4.8.1    |same as 1.5.7| |
|1.6.1    |4.8.1    |toolchains upgrade, added ATmega48/88/168PB, ATA5702M322, ATA5782; added RAMSTART to io*.h| |
|1.6.2    |packed    | | |
|1.6.3    |4.8.1    |toolchains upgrade, cleaned| |
|1.6.4    |4.8.1    |almost the same as 1.6.1| |
|1.6.5-r5 |4.8.1    |same as 1.6.4| |
|1.6.6    |4.8.1    |same as 1.6.4| |
|1.6.7    |4.8.1    |same as 1.6.4| |
|1.6.8    |4.8.1    |same as 1.6.4| |
|1.6.9    |4.8.1    |same as 1.6.4| yes |
|1.6.10   |4.9.2    |toolchains upgrade, , avrdude 6.3 | |
|1.6.11   |4.9.2    |same as 1.6.10, back to avrdude 6.0.1| |
|1.6.12   |4.9.2    |same as 1.6.10, patched avrdude 6.3| |
|1.6.13   |4.9.2    |same as 1.6.12| yes |
|1.8.0    |4.9.2    |same as 1.6.12, another patch for avrdude 6.3 | |
|1.8.1    |4.9.2    |same as 1.8.0| |
|1.8.2    |4.9.2    |recompiled, new toolchains, lot of changes| |
|1.8.3    |4.9.2    |same as 1.8.2| |
|1.8.4    |4.9.2    |same as 1.8.2| |
|1.8.5    |4.9.2    |same as 1.8.2| yes |
|1.8.6    |5.4.0    |new toolchains, added ATmega328PB and ATmega324PB among others| |
|1.8.7    |5.4.0    |same as 1.8.6| yes |
