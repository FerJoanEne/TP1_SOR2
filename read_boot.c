#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    char start_sector[4];
    char length_sectors[4];
} __attribute((packed)) PartitionTable;

typedef struct {
    unsigned char jmp[3]; //Assembly code instructions to jump to boot code (mandatory in bootable partition) 
    char oem[8]; //OEM name in ASCII 
    unsigned short sector_size; // Bytes per sector (512, 1024, 2048, or 4096) 
    char sectors_per_cluster; //Sectors per cluster (Must be a power of 2 and cluster size must be <=32 KB) 
    unsigned short reserved_sectors; //Size of reserved area, in sectors 
    char number_of_fats; //Number of FATs (usually 2) 
    unsigned short root_dir_entries; //Maximum number of files in the root directory (FAT12/16; 0 for FAT32) 
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

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    
    fseek(in, 446 , SEEK_SET); // Ir al inicio de la tabla de particiones.
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 
    
    for(i=0; i<4; i++) {        
        printf("Partition type: %d\n", pt[i].partition_type);
        if(pt[i].partition_type == 1) {
            printf("Encontrado FAT12 %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No se encontró filesystem FAT12, saliendo ...\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("  Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("  OEM code: [%.8s]\n", bs.oem);
    printf("  sector_size: %d\n", bs.sector_size);
	// {...} COMPLETAR
    printf("  volume_id: 0x%08X\n", bs.volume_id);
    printf("  Volume label: [%.11s]\n", bs.volume_label);
    printf("  Filesystem type: [%.8s]\n", bs.fs_type);
    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);
    
    fclose(in);
    return 0;
}
