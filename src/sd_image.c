#include "sd_image.h"
#include "xil_io.h"
#include <string.h>

/* Registros del controlador SPI de Xilinx (AXI Quad SPI IP) */
#define SPI_BASE   0x44A00000
#define SPI_SRR    (SPI_BASE + 0x40)   /* Software Reset Register */
#define SPI_SPICR  (SPI_BASE + 0x60)   /* Control Register */
#define SPI_SPISR  (SPI_BASE + 0x64)   /* Status Register */
#define SPI_DTR    (SPI_BASE + 0x68)   /* Data Transmit Register */
#define SPI_DRR    (SPI_BASE + 0x6C)   /* Data Receive Register */
#define SPI_SPISSR (SPI_BASE + 0x70)   /* Slave Select Register */

/* Bits del registro de control SPI */
#define SPICR_SPE    (1 << 1)   /* SPI habilitado */
#define SPICR_MASTER (1 << 2)   /* modo maestro */
#define SPICR_MANUAL (1 << 7)   /* control manual de SS */
#define SPICR_MTI    (1 << 8)   /* inhibir transaccion (TX inactivo) */

/* Bits del registro de estado SPI */
#define SPISR_RX_EMPTY (1 << 0)   /* FIFO de recepcion vacia */
#define SPISR_TX_EMPTY (1 << 2)   /* FIFO de transmision vacia */

static unsigned char *image_buffer = (unsigned char *)DDR2_IMAGE_BASE;

/*  Capa SPI / SD (bajo nivel) */

/*
 * Transmite un byte por SPI y retorna el byte recibido.
 * Drena el FIFO RX antes de transmitir para evitar leer datos residuales.
 * Retorna 0xFF si se agota el timeout de recepcion.
 */
 
static unsigned char spi_byte(unsigned char tx) {
    int t;

    while (!(Xil_In32(SPI_SPISR) & SPISR_RX_EMPTY))
        (void)Xil_In32(SPI_DRR);

    Xil_Out32(SPI_DTR, (unsigned int)tx);
    Xil_Out32(SPI_SPICR, SPICR_MASTER | SPICR_SPE | SPICR_MANUAL);

    t = 2000000;
    while ((Xil_In32(SPI_SPISR) & SPISR_RX_EMPTY) && --t);

    Xil_Out32(SPI_SPICR, SPICR_MASTER | SPICR_SPE | SPICR_MANUAL | SPICR_MTI);

    if (!t) return 0xFF;
    return (unsigned char)(Xil_In32(SPI_DRR) & 0xFF);
}

/*
 * Envia un comando SD en formato SPI (CMD frame de 6 bytes).
 * Formato: 0x40|cmd, arg[31:24], arg[23:16], arg[15:8], arg[7:0], crc|0x01
 * Sondea hasta 10 veces esperando respuesta R1 valida (bit 7 = 0).
 * Retorna 0xFF si no hay respuesta.
 */
static unsigned char sd_cmd(unsigned char cmd, unsigned int arg, unsigned char crc) {
    unsigned char r;
    int i;

    spi_byte(0x40 | cmd);
    spi_byte((unsigned char)((arg >> 24) & 0xFF));
    spi_byte((unsigned char)((arg >> 16) & 0xFF));
    spi_byte((unsigned char)((arg >>  8) & 0xFF));
    spi_byte((unsigned char)( arg        & 0xFF));
    spi_byte(crc | 0x01);

    for (i = 0; i < 10; i++) {
        r = spi_byte(0xFF);
        if (!(r & 0x80)) return r;
    }
    return 0xFF;
}

/*
 * Lee un bloque de 512 bytes de la SD en block_addr (direccion de bloque).
 * Usa CMD17 (READ_SINGLE_BLOCK). Espera el token de inicio 0xFE antes
 * de leer los datos. Los 2 bytes finales son el CRC (se descartan).
 * Retorna 0 en exito, -1 en error.
 */
static int sd_read_block(unsigned int block_addr, unsigned char *buffer) {
    unsigned char r;
    int i;

    Xil_Out32(SPI_SPISSR, 0x00000000);   /* SS activo */
    spi_byte(0xFF);
    r = sd_cmd(17, block_addr, 0x00);
    Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);   /* SS inactivo */
    spi_byte(0xFF);

    if (r != 0x00) return -1;

    /* Esperar token de inicio de bloque 0xFE */
    Xil_Out32(SPI_SPISSR, 0x00000000);
    for (i = 0; i < 100000; i++) {
        r = spi_byte(0xFF);
        if (r == 0xFE) break;
    }

    if (r != 0xFE) {
        Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
        return -1;
    }

    for (i = 0; i < 512; i++)
        buffer[i] = spi_byte(0xFF);

    /* Descartar CRC de 2 bytes */
    spi_byte(0xFF);
    spi_byte(0xFF);

    Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
    spi_byte(0xFF);

    return 0;
}

/*
 * Inicializa el controlador SPI y la SD en modo SPI.
 * Secuencia: reset SPI, 80 ciclos de reloj con SS inactivo (CMD0),
 * CMD0 (reset), CMD8 (detectar SDHC), ACMD41 (inicializacion).
 * is_hcs indica si la tarjeta es SDHC/SDXC (argumento de ACMD41 distinto).
 * Retorna 0 en exito, -1 en error.
 */
int sd_image_init(void) {
    Xil_Out32(SPI_SRR, 0x0000000A);   /* reset del controlador SPI */
    for (volatile int i = 0; i < 10000000; i++);

    Xil_Out32(SPI_SPICR, SPICR_MASTER | SPICR_SPE | SPICR_MANUAL | SPICR_MTI);
    Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);

    /* 80 ciclos de reloj con SS inactivo para que la SD entre en modo SPI */
    for (int i = 0; i < 10; i++)
        spi_byte(0xFF);

    /* CMD0: reset (GO_IDLE_STATE); CRC 0x95 requerido en modo SPI */
    Xil_Out32(SPI_SPISSR, 0x00000000);
    spi_byte(0xFF);
    unsigned char r = sd_cmd(0, 0x00000000, 0x95);
    Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
    spi_byte(0xFF);

    if (r != 0x01) return -1;

    /* CMD8: detectar soporte de voltaje y SDHC (CRC 0x87 requerido) */
    Xil_Out32(SPI_SPISSR, 0x00000000);
    spi_byte(0xFF);
    r = sd_cmd(8, 0x000001AA, 0x87);
    if (r == 0x01) {
        /* Consumir los 4 bytes de la respuesta R7 */
        for (int i = 0; i < 4; i++)
            spi_byte(0xFF);
    }
    Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
    spi_byte(0xFF);

    if (r != 0x01 && r != 0x05) return -1;

    /* r==0x01: SD v2 (SDHC posible); r==0x05: SD v1 (no SDHC) */
    int is_hcs = (r == 0x01);

    /* ACMD41: inicializacion; bit HCS en argumento si es SDHC */
    unsigned int acmd41_arg = is_hcs ? 0x40000000 : 0x00000000;
    for (int i = 0; i < 1000; i++) {
        Xil_Out32(SPI_SPISSR, 0x00000000);
        spi_byte(0xFF);
        r = sd_cmd(55, 0x00000000, 0x00);   /* CMD55: prefijo de ACMD */
        Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
        spi_byte(0xFF);

        if (r > 0x01) return -1;

        Xil_Out32(SPI_SPISSR, 0x00000000);
        spi_byte(0xFF);
        r = sd_cmd(41, acmd41_arg, 0x00);
        Xil_Out32(SPI_SPISSR, 0xFFFFFFFF);
        spi_byte(0xFF);

        if (r == 0x00) break;   /* inicializacion completa */
        if (r != 0x01) return -1;

        for (volatile int i = 0; i < 100000; i++);
    }

    if (r != 0x00) return -1;
    return 0;
}

/*  Capa FAT16/FAT32 con soporte MBR */

typedef struct {
    unsigned int bytes_per_sector;
    unsigned int sectors_per_cluster;
    unsigned int reserved_sectors;
    unsigned int num_fats;
    unsigned int root_entries;      /* solo FAT16; 0 en FAT32 */
    unsigned int fat_size;          /* sectores por tabla FAT */
    unsigned int root_cluster;      /* solo FAT32 */
    unsigned int first_fat_sector;
    unsigned int root_dir_sector;
    unsigned int root_dir_sectors;
    unsigned int first_data_sector;
    unsigned int partition_base;    /* LBA de inicio de la particion */
    int is_fat32;
} fat_info_t;

static fat_info_t fs;

/* Lectura de enteros en little-endian desde un buffer de bytes */
static unsigned short rd16(const unsigned char *p) {
    return (unsigned short)(p[0] | (p[1] << 8));
}

static unsigned int rd32(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] <<  8) |
           ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}

/* Lee un sector relativo al inicio del volumen (suma partition_base) */
static int read_vol_sector(unsigned int rel_sector, unsigned char *buf) {
    return sd_read_block(fs.partition_base + rel_sector, buf);
}

/*
 * Interpreta el BPB (BIOS Parameter Block) del sector de arranque.
 * Determina si el sistema de archivos es FAT16 o FAT32 segun fat_size_16.
 * Valida bytes_per_sector, num_fats y sectors_per_cluster.
 * Retorna 0 si el BPB es valido, -1 si no.
 */
static int parse_bpb(const unsigned char *boot_sector) {
    fs.bytes_per_sector    = rd16(&boot_sector[11]);
    fs.sectors_per_cluster = boot_sector[13];
    fs.reserved_sectors    = rd16(&boot_sector[14]);
    fs.num_fats            = boot_sector[16];
    fs.root_entries        = rd16(&boot_sector[17]);

    unsigned int fat_size_16 = rd16(&boot_sector[22]);
    unsigned int fat_size_32 = rd32(&boot_sector[36]);

    if (fat_size_16 != 0) {
        /* FAT16: fat_size en offset 22 */
        fs.is_fat32     = 0;
        fs.fat_size     = fat_size_16;
        fs.root_cluster = 0;
    } else {
        /* FAT32: fat_size en offset 36, root cluster en offset 44 */
        fs.is_fat32     = 1;
        fs.fat_size     = fat_size_32;
        fs.root_cluster = rd32(&boot_sector[44]);
    }

    int bps_ok = (fs.bytes_per_sector == 512  || fs.bytes_per_sector == 1024 ||
                  fs.bytes_per_sector == 2048 || fs.bytes_per_sector == 4096);
    int nf_ok  = (fs.num_fats == 1 || fs.num_fats == 2);
    int spc_ok = (fs.sectors_per_cluster != 0);

    return (bps_ok && nf_ok && spc_ok) ? 0 : -1;
}

/*
 * Monta el sistema de archivos FAT.
 * Intenta parsear el BPB del sector 0. Si falla, asume que el sector 0
 * es un MBR y lee la primera entrada de la tabla de particiones (offset 446)
 * para obtener el LBA de inicio de la particion y reintenta el parseo.
 * Calcula los sectores clave: FAT, directorio raiz y primer sector de datos.
 */
static int fat_mount(void) {
    unsigned char sector0[512];

    fs.partition_base = 0;

    if (sd_read_block(0, sector0) != 0) return -1;
    if (sector0[510] != 0x55 || sector0[511] != 0xAA) return -1;

    if (parse_bpb(sector0) != 0) {
        /* Sector 0 es MBR: leer LBA y tipo de la primera particion */
        unsigned char ptype = sector0[446 + 4];
        unsigned int  plba  = rd32(&sector0[446 + 8]);

        if (ptype == 0x00 || plba == 0) return -1;

        unsigned char sector_p[512];
        if (sd_read_block(plba, sector_p) != 0) return -1;
        if (sector_p[510] != 0x55 || sector_p[511] != 0xAA || parse_bpb(sector_p) != 0) return -1;

        fs.partition_base = plba;
    }

    fs.first_fat_sector  = fs.reserved_sectors;
    fs.root_dir_sectors  = ((fs.root_entries * 32) + (fs.bytes_per_sector - 1)) / fs.bytes_per_sector;
    fs.root_dir_sector   = fs.reserved_sectors + (fs.num_fats * fs.fat_size);
    fs.first_data_sector = fs.root_dir_sector + fs.root_dir_sectors;

    return 0;
}

/* Convierte numero de cluster a sector absoluto dentro del volumen */
static unsigned int cluster_to_sector(unsigned int cluster) {
    return fs.first_data_sector + (cluster - 2) * fs.sectors_per_cluster;
}

/* Retorna 1 si el valor de cluster es End-Of-Chain (EOC) segun FAT16/FAT32 */
static int is_eoc(unsigned int val) {
    if (fs.is_fat32) return val >= 0x0FFFFFF8;
    return val >= 0xFFF8;
}

/* Obtiene el siguiente cluster en la cadena leyendo la tabla FAT */
static unsigned int get_next_cluster(unsigned int cluster) {
    unsigned char sector_buf[512];
    unsigned int fat_offset = fs.is_fat32 ? (cluster * 4) : (cluster * 2);
    unsigned int fat_sector = fs.first_fat_sector + (fat_offset / fs.bytes_per_sector);
    unsigned int ent_offset = fat_offset % fs.bytes_per_sector;

    if (read_vol_sector(fat_sector, sector_buf) != 0)
        return 0x0FFFFFFF;   /* error: retornar EOC para detener la cadena */

    if (fs.is_fat32)
        return rd32(&sector_buf[ent_offset]) & 0x0FFFFFFF;
    return rd16(&sector_buf[ent_offset]);
}

/*
 * Busca target_name (formato 8.3 en mayusculas, 11 bytes) en un sector
 * de directorio. Ignora entradas eliminadas (0xE5), LFN (0x0F),
 * etiquetas de volumen (bit3) y subdirectorios (bit4).
 * Retorna 1 si encuentra el archivo y rellena start_cluster y file_size.
 * Pone end_of_dir=1 si encuentra una entrada vacia (0x00) que marca el fin.
 */
static int scan_dir_sector(const unsigned char *target_name, const unsigned char *dir_sector,
                            unsigned int *start_cluster, unsigned int *file_size, int *end_of_dir) {
    unsigned int entries_per_sector = fs.bytes_per_sector / 32;

    for (unsigned int j = 0; j < entries_per_sector; j++) {
        const unsigned char *entry = dir_sector + (j * 32);

        if (entry[0] == 0x00) { *end_of_dir = 1; return 0; }  /* fin de directorio */
        if (entry[0] == 0xE5) continue;   /* entrada eliminada */
        if (entry[11] == 0x0F) continue;  /* entrada LFN */
        if (entry[11] & 0x08) continue;   /* etiqueta de volumen */
        if (entry[11] & 0x10) continue;   /* subdirectorio */

        int match = 1;
        for (int k = 0; k < 11; k++) {
            if (entry[k] != target_name[k]) { match = 0; break; }
        }

        if (match) {
            /* Cluster alto (FAT32 en offset 20) y bajo (offset 26) */
            unsigned int cl_hi = rd16(&entry[20]);
            unsigned int cl_lo = rd16(&entry[26]);
            *start_cluster = (cl_hi << 16) | cl_lo;
            *file_size     = rd32(&entry[28]);
            return 1;
        }
    }
    return 0;
}

/*
 * Busca un archivo por nombre en el directorio raiz.
 * Convierte filename al formato 8.3 en mayusculas antes de comparar.
 * FAT16: directorio raiz en sectores fijos (root_dir_sector).
 * FAT32: directorio raiz es una cadena de clusters (root_cluster).
 * Rellena start_cluster y file_size si lo encuentra.
 * Retorna 0 si lo encuentra, -1 si no.
 */
static int fat_find_file(const char *filename, unsigned int *start_cluster, unsigned int *file_size) {
    unsigned char target_name[11];
    unsigned char sector_buf[512];
    int dot_pos = -1;
    int name_len = 0;

    for (int i = 0; i < 11; i++) target_name[i] = ' ';

    /* Separar nombre y extension */
    for (int i = 0; filename[i]; i++) {
        if (filename[i] == '.') { dot_pos = i; break; }
        name_len++;
    }

    /* Copiar nombre (hasta 8 caracteres) en mayusculas */
    for (int i = 0; i < name_len && i < 8; i++) {
        char c = filename[i];
        target_name[i] = (c >= 'a' && c <= 'z') ? (unsigned char)(c - 'a' + 'A') : (unsigned char)c;
    }

    /* Copiar extension (hasta 3 caracteres) en mayusculas */
    if (dot_pos >= 0) {
        int ext_idx = dot_pos + 1;
        for (int i = 0; i < 3 && filename[ext_idx]; i++, ext_idx++) {
            char c = filename[ext_idx];
            target_name[8 + i] = (c >= 'a' && c <= 'z') ? (unsigned char)(c - 'a' + 'A') : (unsigned char)c;
        }
    }

    if (!fs.is_fat32) {
        /* FAT16: recorrer sectores fijos del directorio raiz */
        for (unsigned int i = 0; i < fs.root_dir_sectors; i++) {
            if (read_vol_sector(fs.root_dir_sector + i, sector_buf) != 0) return -1;
            int end_of_dir = 0;
            if (scan_dir_sector(target_name, sector_buf, start_cluster, file_size, &end_of_dir)) return 0;
            if (end_of_dir) return -1;
        }
        return -1;
    } else {
        /* FAT32: recorrer la cadena de clusters del directorio raiz */
        unsigned int cluster = fs.root_cluster;
        while (cluster >= 2 && !is_eoc(cluster)) {
            unsigned int first_sector = cluster_to_sector(cluster);
            for (unsigned int s = 0; s < fs.sectors_per_cluster; s++) {
                if (read_vol_sector(first_sector + s, sector_buf) != 0) return -1;
                int end_of_dir = 0;
                if (scan_dir_sector(target_name, sector_buf, start_cluster, file_size, &end_of_dir)) return 0;
                if (end_of_dir) return -1;
            }
            cluster = get_next_cluster(cluster);
        }
        return -1;
    }
}

/*
 * Monta el sistema de archivos, localiza filename y copia su contenido
 * a DDR2_IMAGE_BASE. Lee la cadena de clusters del archivo sector a sector.
 * Copia como maximo IMAGE_SIZE bytes aunque el archivo sea mas grande.
 * Retorna 0 si se copio la cantidad esperada, -1 en cualquier error.
 */
int sd_image_load(const char *filename) {
    unsigned char *ddr2 = (unsigned char *)DDR2_IMAGE_BASE;
    unsigned char sector_buf[512];
    unsigned int start_cluster, file_size;

    if (fat_mount() != 0) return -1;
    if (fat_find_file(filename, &start_cluster, &file_size) != 0) return -1;
    if (start_cluster < 2) return -1;

    unsigned int to_copy     = (file_size != 0 && file_size < IMAGE_SIZE) ? file_size : IMAGE_SIZE;
    unsigned int bytes_copied = 0;
    unsigned int cluster      = start_cluster;

    while (cluster >= 2 && !is_eoc(cluster) && bytes_copied < to_copy) {
        unsigned int first_sector = cluster_to_sector(cluster);

        for (unsigned int s = 0; s < fs.sectors_per_cluster && bytes_copied < to_copy; s++) {
            if (read_vol_sector(first_sector + s, sector_buf) != 0) return -1;
            unsigned int chunk = (to_copy - bytes_copied > 512) ? 512 : (to_copy - bytes_copied);
            memcpy(ddr2 + bytes_copied, sector_buf, chunk);
            bytes_copied += chunk;
        }

        cluster = get_next_cluster(cluster);
    }

    return (bytes_copied >= to_copy) ? 0 : -1;
}

/* Retorna el puntero al buffer de imagen en DDR2 */
unsigned char* sd_image_get_buffer(void) {
    return image_buffer;
}
