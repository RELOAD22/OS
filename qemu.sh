set -v 
cp /mnt/shared/project3/image /mnt/shared/QEMULoongson
cd /mnt/shared/QEMULoongson/
dd if=image of=disk conv=notrunc
sh run_pmon.sh