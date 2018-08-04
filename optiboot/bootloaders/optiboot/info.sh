#%/bin/bash
for f in $*; do
    echo
    echo $f
    avr-objcopy -I ihex -O binary $f /tmp/__optiboot_tmp.bin
    avr-strings -a /tmp/__optiboot_tmp.bin
done
rm /tmp/__optiboot_tmp.bin

