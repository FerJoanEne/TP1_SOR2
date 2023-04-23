Se colocan algunos comandos:

para montar el archivo del tp me posiciono en la carpeta en la que se encuentra el archivo como en el siguiente ejemplo
$ sudo mount $(pwd)/test.img /mnt -o loop,umask=000

para recuperar un archivo (en progreso):

$ gcc recuperar.c -o recuperar
$ ./recuperar  /root/lapapa.txt

ver archivo a bajo nivel (sin sudo porque pincha)
$ ghex nombre_ arhivo_con_extension