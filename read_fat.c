#include <stdlib.h>
#include <stdio.h>

#include "filesystem.h"
#include "time.h"
#include "dir.h"
#include "utils-f.h"

/* header */
void print_boot_sector(struct fat16_filesystem *);
void print_root_dir(struct fat16_filesystem *);
void print_dir_entry(struct fat16_filesystem *, struct fat_dir_entry *);

/* implementation */
int main(int argc, char ** argv) {

  if (argc != 2) {
    fprintf(stderr, "argc %d, expected 2\n", argc);
    exit(1);
  }

  struct fat16_filesystem fs;
  fat_open_filesystem(&fs, argv[1]);

  print_boot_sector(&fs);
  print_root_dir(&fs);
  putchar('\n');

  fat_close_filesystem(&fs);

  return EXIT_SUCCESS;
}

void print_root_dir(struct fat16_filesystem * fs) {
  struct fat_dir_entry entry;
  int i;

  fat_seek_to_root_dir(fs);

  putchar('\n');
  printf("Root directory:\n");
  for (i = 0; i < fs->root_dir_entries; i++) {
    fread(&entry, sizeof(entry), 1, fs->fd);

    /* skip empty/unused entries */
    if (!fat_dir_entry_exists(&entry)) continue;

    /* skip volume label */
    if (fat_is_volume_label(&entry)) continue;

    print_dir_entry(fs, &entry);
  }
}

void print_dir_entry(struct fat16_filesystem * fs, struct fat_dir_entry * de) {
  char filename[13]; /* "FILENAME.EXT\0" */
  struct fat_date date_created, date_modified, date_accessed;
  struct fat_time time_created, time_modified;
  char * file_content;

  /* read filename */
  fat_read_filename(filename, de);

  /* read dates and times from bitfields */
  fat_read_date(&date_created, de->create_date);
  fat_read_date(&date_modified, de->modify_date);
  fat_read_date(&date_accessed, de->access_date);
  fat_read_time(&time_created, de->create_time);
  fat_read_time(&time_modified, de->modify_time);

  printf("  %s\n", filename);
  printf("    bytes: %u  cluster: %u\n", de->size, de->start_cluster);
  printf("    created:  %4u-%02u-%02u %02u:%02u:%02u\n",
      date_created.year, date_created.month, date_created.day,
      time_created.hour, time_created.minute, time_created.second);
  printf("    modified: %4u-%02u-%02u %02u:%02u:%02u\n",
      date_modified.year, date_modified.month, date_modified.day,
      time_modified.hour, time_modified.minute, time_modified.second);
  printf("    accessed: %4u-%02u-%02u\n",
      date_accessed.year, date_accessed.month, date_accessed.day);
  printf("   ");
  printf(" ro:%s", de->attributes & 0x01 ? "yes" : "no");
  printf(" hide:%s", de->attributes & 0x02 ? "yes" : "no");
  printf(" sys:%s", de->attributes & 0x03 ? "yes" : "no");
  printf(" dir:%s", de->attributes & 0x10 ? "yes" : "no");
  printf(" arch:%s", de->attributes & 0x20 ? "yes" : "no");
  putchar('\n');

  if (fat_is_file(de)) {
    file_content = fat_read_file_from_dir_entry(fs, de);
    printf("    Content:\n%s    <EOF>\n", file_content);
    free(file_content);
  }
}

void print_boot_sector(struct fat16_filesystem * fs) {
  struct fat16_boot_sector * bs;
  struct fat_preamble * pre;
  struct fat_bios_parameter_block * bp;
  struct fat16_extended_bios_parameter_block * ebp;
  int fat_i;

  /* presentation variables */
  int total_sectors;
  char oem_name[FAT_OEM_NAME_LENGTH + 1];
  char label[FAT_LABEL_LENGTH + 1];
  char fs_type[FAT_FS_TYPE_LENGTH + 1];

  bs = &fs->boot_sector;
  pre = &bs->preamble;
  bp = &bs->bios_params;
  ebp = &bs->ext_bios_params;

  total_sectors = bp->total_sectors == 0 ? (int)bp->total_sectors_large : (int)bp->total_sectors;

  /* null terminate strings */
  fat_string_copy(oem_name, pre->oem_name, FAT_OEM_NAME_LENGTH);
  fat_string_copy(label, ebp->label, FAT_LABEL_LENGTH);
  fat_string_copy(fs_type, ebp->fs_type, FAT_FS_TYPE_LENGTH);

  putchar('\n');
  printf("Boot sector size: %d bytes\n", (int)sizeof(bs));
  printf("Jump instruction: %X %X %X\n", pre->jump_instruction[0], pre->jump_instruction[1], pre->jump_instruction[2]);
  printf("OEM name: %s\n", oem_name);

  printf("\nBIOS parameters:\n");
  printf("  Bytes per sector: %d\n", bp->bytes_per_sector);
  printf("  Sectors per cluster: %d\n", bp->sectors_per_cluster);
  printf("  Reserved sectors: %d\n", bp->reserved_sector_count);
  printf("  Number of FATs: %d\n", bp->fat_count);
  printf("  Maximum root entries: %d\n", bp->max_root_entries);
  printf("  Total sectors: %d (small: %d, large: %d)\n", total_sectors, bp->total_sectors, bp->total_sectors_large);
  printf("  Media descriptor: 0x%02X%s\n", bp->media_descriptor, bp->media_descriptor == 0xf8 ? " (fixed disk)" : "");
  printf("  Sectors per FAT: %d\n", bp->sectors_per_fat);
  printf("  Sectors per track: %d\n", bp->sectors_per_track);
  printf("  Number of heads: %d\n", bp->head_count);
  printf("  Sectors before partition: %d\n", bp->sectors_before_partition);

  printf("\nExtended BIOS parameters:\n");
  printf("  Physical drive code: 0x%02X (%s)\n", ebp->physical_drive_number, ebp->physical_drive_number ? "physical" : "removable");
  printf("  Reserved byte (WinNT): 0x%02X\n", ebp->current_head);
  printf("  Extended boot signature: 0x%02X\n", ebp->extended_boot_signature);
  printf("  Volume serial number: 0x%08X\n", ebp->serial_number);
  printf("  Volume label: %11s\n", label);
  printf("  FAT type: %s\n", fs_type);
  printf("  OS boot code: %d bytes\n", (int)sizeof(ebp->os_boot_code));
  printf("  Boot sector signature: 0x%04X\n", ebp->boot_sector_signature);
  putchar('\n');
  printf("Calculated parameters:\n");
  printf("  FAT size: %u bytes\n", fs->fat_size);
  for (fat_i = 0; fat_i < bp->fat_count; fat_i++)
    printf("  FAT #%u offset: 0x%08X \n", fat_i + 1, fs->fat_offset + fs->fat_size * fat_i);
  printf("  Root directory offset: 0x%08X\n", fs->root_dir_offset);
  printf("  Root directory size: %u bytes\n", fs->root_dir_size);
  printf("  Data offset: 0x%08X\n", fs->data_offset);
  printf("  Cluster size: %u bytes\n", fs->cluster_size);
}
