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
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size; // 2 bytes
    char sectors_per_cluster; //Sectors per cluster (Must be a power of 2 and cluster size must be <=32 KB) 
    unsigned short reserved_sectors; //Size of reserved area, in sectors 
    char number_of_fats; //Number of FATs (usually 2) 
    unsigned short root_dir_entries; //Maximum number of files in the root directory (FAT12/16; 0 for FAT32) 
    unsigned short sectors_fs; // Cant of sectors in the FS; if 2 B is not large enough, set to 0 and use 4 B value in bytes 32-35 below 
    unsigned char media_type; //Media type (0xf0=removable disk, 0xf8=fixed disk) 
    unsigned short fat_size_sectors; //Size of each FAT, in sectors, for FAT12/16; 0 for FAT32 
    unsigned short sectors_per_track; //Sectors per track in storage device 
    unsigned short heads_stgdev; //Number of heads in storage device 
    unsigned int sectors_before_start; //Cant of sectors before the start partition 
    unsigned int sectors_fylesystem; //Cant of sectors in the FS; this field will be 0 if the 2B field above (bytes 19-20) is non-zero 
    unsigned char bios_int; //BIOS INT 13h (low level disk services) drive number 
    char unused; //Not used 
    char extended_boot_signature;  //Extended boot signature to validate next three fields (0x29)
    unsigned int volume_id;
    char volume_label[11];
    char fs_type[8]; // Type en ascii
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

int main() {
    FILE * in = fopen("test.img", "rb");
    int i;
    PartitionTable pt[4];
    Fat12BootSector bs;
    int offset = 446;
    
    fseek(in, offset , SEEK_SET); // Ir al inicio de la tabla de particiones.
    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 
    
    printf("\nPARTITION TABLES AVAILABLE: \n\n");
    
    for(i=0; i<4; i++) {        
        printf("Partition type: %d\n", pt[i].partition_type);
        if(pt[i].partition_type == 1) {
            printf("Encontrado FAT12, partition table %d\n", i);
            break;
        }
    }
    
    if(i == 4) {
        printf("No se encontró filesystem FAT12, saliendo ...\n");
        return -1;
    }
    
    fseek(in, 0, SEEK_SET);
    fread(&bs, sizeof(Fat12BootSector), 1, in);
    
    printf("\nBOOT SECTOR: \n\n");
    
    printf("Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);
    printf("OEM code: [%.8s]\n", bs.oem);
    printf("Sector size: %d\n", bs.sector_size);
    printf("Sectors per cluster: %d\n", bs.sectors_per_cluster);
    printf("Reserved sectors: %d\n", bs.reserved_sectors);
    printf("Number of FATs: %d\n", bs.number_of_fats);
    printf("Max files in root directory: %d\n", bs.root_dir_entries);
    printf("Sectors in FS: %d\n", bs.sectors_fs);
    printf("Media type: 0x%x\n", bs.media_type);
    printf("FAT size in sectors: %d\n", bs.fat_size_sectors);
    printf("Sectors per track: %d\n", bs.sectors_per_track);
    printf("Heads in storage dev: %d\n", bs.heads_stgdev);
    printf("Sectors before start: %d\n", bs.sectors_before_start);
    printf("Extended sectors in FS: %d\n", bs.sectors_fylesystem);
    printf("Bios int: 0x%x\n", bs.bios_int);
//  printf("unused: 0x%x\n", bs.unused);
    printf("Extended boot signature: 0x%x\n", bs.extended_boot_signature);
    printf("Volume id: 0x%08X\n", bs.volume_id);
    printf("Volume label: [%.11s]\n", bs.volume_label);
    printf("Filesystem type: [%.8s]\n", bs.fs_type);
    printf("Boot sector signature: 0x%04X\n", bs.boot_sector_signature);
    
    fclose(in);
    return 0;
}
