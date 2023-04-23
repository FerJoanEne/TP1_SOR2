Se colocan algunos comandos:

-para montar el archivo del tp me posiciono en la carpeta en la que se encuentra el archivo y puedo usar cualquiera de los siguientes comandos:

--$ sudo mount $(pwd)/test.img /mnt -o loop,umask=000   <--(VM lubuntu)

--$ sudo mount -t vfat -o umask=000 test.img /mnt   <--(en debian 11)

-para recuperar un archivo (en progreso):

-- $ gcc recuperar.c -o recuperar
-- $ ./recuperar  /root/lapapa.txt

-ver archivo a bajo nivel (sin sudo)
--$ ghex nombre_ arhivo_con_extension