set -v 
cp /mnt/shared/Project2/project2-simple-kernel/start_code/image /mnt/shared/QEMULoongson
cd /mnt/shared/QEMULoongson/
dd if=image of=disk conv=notrunc
sh run_pmon.sh