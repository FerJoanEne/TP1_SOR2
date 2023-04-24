#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <string.h> 

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

enum TipoEntry {
  ARCHIVO,
  DIRECTORIO,
  LONGFILENAME
}; 
enum EstadoEntradaFAT { //Dentro de la Fat table, los estados que puede tomar un valor
  LIBRE,
  RESERVADO,
  EN_USO,
  MAL_SECTOR,
  EOF_
}; 
typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char starting_cluster[4];
    char file_size[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3]; //Assembly code instructions to jump to boot code (mandatory in bootable partition) 
    char oem[8]; //OEM name in ASCII 
    unsigned short sector_size; // Bytes per sector (512, 1024, 2048, or 4096) //512 en el TP
    char sectors_per_cluster; //Sectors per cluster (Must be a power of 2 and cluster size must be <=32 KB) //4 en el TP
    unsigned short reserved_sectors; //Size of reserved area, in sectors //1 en el TP
    char number_of_fats; //Number of FATs (usually 2) //2 en el TP
    unsigned short root_dir_entries; //Maximum number of files in the root directory (FAT12/16; 0 for FAT32) //512 en el TP
    unsigned short sectors_fs; // Number of sectors in the file system; if 2 B is not large enough, set to 0 and use 4 B value in bytes 32-35 below 
    char media_type; //Media type (0xf0=removable disk, 0xf8=fixed disk) 
    unsigned short fat_size_sectors; //Size of each FAT, in sectors, for FAT12/16; 0 for FAT32 
    unsigned short sectors_per_track; //Sectors per track in storage device 
    unsigned short heads_stgdev; //Number of heads in storage device 
    unsigned int sectors_before_start; //Number of sectors before the start partition 
    unsigned int sectors_fylesystem; //Number of sectors in the file system; this field will be 0 if the 2B field above (bytes 19-20) is non-zero 
    char bios_int; //BIOS INT 13h (low level disk services) drive number 
    char unused; //Not used 
    char extended_boot_signature;  //Extended boot signature to validate next three fields (0x29)
    unsigned int volume_id; //Volume serial number 
    char volume_label[11]; //Volume label, in ASCII 
    char fs_type[8]; // File system type level, in ASCII. (Generally "FAT", "FAT12", or "FAT16") 
    char boot_code[448]; //Not used 
    unsigned short boot_sector_signature; //Signature value (0xaa55) 
} __attribute((packed)) Fat12BootSector;

typedef struct {
    unsigned char filename[11]; //Characters 2 to 11 of file name in ASCII  //First character of file name in ASCII and allocation status (0xe5 if deleted or 0x00 if unallocated)
    char attributes; //File Attributes (see below) 
    char reserved; //Reserved 
    unsigned char file_creation_time; //Created time (tenths of second) 
    unsigned short creation_time; //Created time (hours, minutes, seconds) 
    unsigned short creation_date; //Created day 
    unsigned short access_date; //Accessed day 
    unsigned short cluster_adress_high; //High 2 bytes of first cluster address (0 for FAT12 and FAT16) 
    unsigned short modified_time; //Written time (hours, minutes, seconds) 
    unsigned short modified_date; //Written day 
    unsigned short cluster_adress_low; //Low 2 bytes of first cluster address 
    unsigned int file_size; //Size of file (0 for directories) 
//Flag Value (in bits) 	Description 
//0000 0001 (0x01) 	Read only 	
//0000 0010 (0x02) 	Hidden file 	
//0000 0100 (0x04) 	System file 	
//0000 1000 (0x08) 	Volume label 	
//0000 1111 (0x0f) 	Long file name
//0001 0000 (0x10) 	Directory 
//0010 0000 (0x20) 	Archive 	
} __attribute((packed)) Fat12Entry;

typedef struct {
    char sequence_number; // 	Sequence number (ORed with 0x40) and allocation status (0xe5 if unallocated) 
    char filename_1[10]; //File name characters 1-5 (Unicode)
    char attributes; //File attributes (0x0f)  
    char reserved; //Reserved 
    char checksum; //Checksum
    char filename_2[12]; //File name characters 6-11 (Unicode) 
    unsigned short reserved_2; //Reserved 
    char filename_3[4]; //File name characters 12-13 (Unicode) 
} __attribute((packed)) Fat12LongFileNameEntry;

void obtenerNombreLongFilenameEntry(char dest[14], Fat12LongFileNameEntry *entry){
    dest[0]=entry->filename_1[0];
    dest[1]=entry->filename_1[2];
    dest[2]=entry->filename_1[4];
    dest[3]=entry->filename_1[6];
    dest[4]=entry->filename_1[8];
    dest[5]=entry->filename_2[0];
    dest[6]=entry->filename_2[2];
    dest[7]=entry->filename_2[4];
    dest[8]=entry->filename_2[6];
    dest[9]=entry->filename_2[8];
    dest[10]=entry->filename_2[10];
    dest[11]=entry->filename_3[0];
    dest[12]=entry->filename_3[2];
    dest[13]='\0';
    return;
}

char caracterBorrado(bool esBorrado){
    if (esBorrado){
        return '?';
    }
    return ' ';
}
char caracterOculto(bool esOculto){
    if (esOculto){
        return 'O';
    }
    return ' ';
}
unsigned short getValorEnTablaFat(Fat12BootSector bs, FILE * in, int posicion){
    long int inicio_tabla_fat = sizeof(Fat12BootSector);
    unsigned short valor;
    fseek(in, inicio_tabla_fat + (posicion/2)*3+posicion%2, SEEK_SET); //formula para buscar los bytes a leer
    fread(&valor, 2, 1, in);
    if (posicion%2 == 0){
        posicion = posicion<<4;
        posicion = posicion>>4;
    } else {
        posicion = posicion>>4;
    }
    return posicion;
}

enum EstadoEntradaFAT valorFatAEstadoFat(unsigned short valor){
    if (valor == 0x0000){
        return LIBRE;
    } else if (valor == 0x0001){
        return RESERVADO;
    } else if ((valor == 0x0000)||((valor >= 0x0FF0)&&(valor <= 0x0FF6))){
        return LIBRE;
    } else if (valor == 0x0FF7){
        return MAL_SECTOR;
    } else if (valor >= 0x0FF8){
        return EOF_;
    } else{
        return EN_USO;
    }    
}

void mostrarDirectorios(char ruta[], unsigned short cant_entries, FILE * in, Fat12BootSector bs, long int offset) {
    fseek(in, offset, SEEK_SET);
    Fat12Entry entry;
    Fat12Entry* pentry = &entry;
    long int inicio_clusters = sizeof(Fat12BootSector)+(bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) * bs.sector_size + bs.root_dir_entries*sizeof(Fat12Entry);
    enum TipoEntry tipo;
    bool esBorrado;
    bool esOculto;
    char nombre[256] = ""; //Maxima longitud de nombre en FAT12
    char nombreAux[256] = "";
    char nombreLongAux[14];
    char rutaAux[512];
    unsigned short numero_cluster;
    unsigned long posicionActual;
    int i;
    for(i=0; i<cant_entries; i++) {      
        posicionActual = ftell(in); 
        fread(&entry, sizeof(entry), 1, in);
        switch(pentry->filename[0]) {
        case 0x00:
            continue; // unused entry
        case 0x2E:
            continue; // son los directorios que se autoreferencian o a su origen
        case 0xE5: //borrado
            esBorrado = true;
            break;            
        default: 
            esBorrado = false;
        }
        tipo = ARCHIVO;
        if (CHECK_BIT(pentry->attributes, 4)){ //bit que indica si es directorio
            tipo = DIRECTORIO;
        }
        esOculto = false;
        if (CHECK_BIT(pentry->attributes, 0) && CHECK_BIT(pentry->attributes, 1) && CHECK_BIT(pentry->attributes, 2) && CHECK_BIT(pentry->attributes, 3)){ //Si todos están en 1 es LongFileNameEntry
            tipo = LONGFILENAME;
        } else if (CHECK_BIT(pentry->attributes, 1)){
            esOculto = true; 
        }
        //Ya sabemos el tipo de la entry
        switch(tipo) {
        case ARCHIVO:
            printf("A%c%c:0x%08lX:%s%s\n", caracterBorrado(esBorrado),caracterOculto(esOculto), posicionActual, ruta, nombre);
            nombre[0] = '\0';
            break;
        case LONGFILENAME:            
            strcpy(nombreAux, nombre);
            nombre[0] = '\0';
            obtenerNombreLongFilenameEntry(nombreLongAux,   (Fat12LongFileNameEntry*)pentry);
            strcat(nombre, nombreLongAux); //va obteniendo el nombre de estas entries
            strcat(nombre, nombreAux);
            break;
        case DIRECTORIO:
            printf("D%c%c:0x%08lX:%s%s\n", caracterBorrado(esBorrado),caracterOculto(esOculto), posicionActual,ruta, nombre);
            strcpy(rutaAux, ruta);
            strcat(rutaAux, nombre);
            strcat(rutaAux, "/");
            posicionActual = ftell(in);
            numero_cluster = pentry->cluster_adress_low;
            mostrarDirectorios(rutaAux, (unsigned short)((bs.sectors_per_cluster * bs.sector_size)/sizeof(Fat12Entry)), in, bs, inicio_clusters+ ((numero_cluster)-2)*bs.sectors_per_cluster* bs.sector_size);
            while (valorFatAEstadoFat(getValorEnTablaFat(bs, in, numero_cluster) == EN_USO)){
                numero_cluster = getValorEnTablaFat(bs, in, numero_cluster);
                mostrarDirectorios(rutaAux, (unsigned short)((bs.sectors_per_cluster * bs.sector_size)/sizeof(Fat12Entry)), in, bs, inicio_clusters+ ((numero_cluster)-2)*bs.sectors_per_cluster* bs.sector_size);
            }            
            fseek(in, posicionActual, SEEK_SET); //volvemos a donde estabamos
            nombre[0] = '\0';
            break;
        default:
            continue;
        }
    }
}

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    Fat12Entry entry;
   
	fseek(in, 446 , SEEK_SET); // Ir al inicio de la tabla de particiones.
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 
    
    for(i=0; i<4; i++) {        
        if(pt[i].partition_type == 1) {
            //printf("Encontrada particion FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        //printf("No encontrado filesystem FAT12, saliendo...\n");
        return -1;
    }

    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("---Estructura de archivo---\n");
    printf("Tipo De Archivo: Dirección Inicial del Directory Entry: Ruta y nombre\n\n");

    printf("---Tipos de archivo---\n");
    printf("D  : Directorio\n");
    printf("A  : Archivo\n");
    printf(" ? : Borrado\n");
    printf("  O: Oculto\n\n");
           
    printf("---Mostrando Archivos en el FileSystem---\n");
    long int offset = ftell(in) + (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) * bs.sector_size;
    mostrarDirectorios("/", bs.root_dir_entries, in, bs, offset);

    fclose(in);
    return 0;
}
