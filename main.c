#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "msdos_fs.h"
#include "main.h"

//TODO : handle lower case for naming (dir and cp)
//TODO : check if there is an EOF inside the sector - and it needs to be handled

// Function to read the directory entries
void print_root_dir(FILE* img_file, struct fat_boot_sector* boot_sector) {
    char line[MAX_LINE_LEN];
    char name[MSDOS_NAME + 1];
    char time_str[9]; // enough space for "HH:MM AM\0"
    char date_str[11]; // enough space for "MM/DD/YYYY\0"
    int name_len;
    char size_str[19];

    time_str[8] = '\0';
    date_str[10] = '\0';

    fseek(img_file, (boot_sector->reserved + boot_sector->fats * boot_sector->fat_length) * SECTOR_SIZE, SEEK_SET);

    for (int i = 0; i < boot_sector->dir_entries[0]; i++) {
        struct msdos_dir_entry dir_entry;
        fread(&dir_entry, sizeof(struct msdos_dir_entry), 1, img_file);

        if (dir_entry.name[0] == 0x00) {  // End of directory
            break;
        }
        if (dir_entry.name[0] == 0xE5) {  // Deleted file, skip it
            continue;
        }
        if (dir_entry.attr & ATTR_VOLUME) {  // Volume label, skip it
            continue;
        }

        fat_name_to_normal_name(dir_entry.name, name);
        name_len = strlen(name);
        
        time_to_string(dir_entry.ctime, time_str);
        date_to_string(dir_entry.cdate, date_str);
        
        strncpy(line, date_str, strlen(date_str));
        strncpy(line + 10, "  ", 2);
        strncpy(line + 12, time_str, strlen(time_str));

        // format dir example:
        // 02/23/2022  11:07 PM    <DIR>            WebstormProjects
        if (dir_entry.attr & ATTR_DIR) {
            // do some string formatting
            strncpy(line + 20, "    <DIR>          ", 19);
            strncpy(line + 39, name, name_len);
            line[MAX_LINE_LEN - 9 + name_len] = '\0';
        }

        // format file example:
        // 09/06/2021  10:01 PM         1,957,374 get-pip.py
         else {
            format_number(dir_entry.size, size_str);

            // do some string formatting
            strncpy(line + 20, "                   ", 19);
            strncpy(line + (MAX_LINE_LEN - 10 - strlen(size_str)), size_str, strlen(size_str));
            strncpy(line + 39, name, name_len);
            line[MAX_LINE_LEN - 9 + name_len] = '\0';
        }

        printf("%s\n", line);
    }
}

// check if a file exists in the root directory. returns 1 if exists, 0 if not
__le16 get_file_entry(FILE* img_file, struct fat_boot_sector* boot_sector, char* file_name){
    char name[MSDOS_NAME + 1];

    fseek(img_file, (boot_sector->reserved + boot_sector->fats * boot_sector->fat_length) * SECTOR_SIZE, SEEK_SET);

    for (int i = 0; i < boot_sector->dir_entries[0]; i++) {
        struct msdos_dir_entry dir_entry;
        fread(&dir_entry, sizeof(struct msdos_dir_entry), 1, img_file);

        if (dir_entry.name[0] == 0x00) {  // End of directory
            break;
        }
        if (dir_entry.name[0] == 0xE5) {  // Deleted file, skip it
            continue;
        }
        if (dir_entry.attr & ATTR_VOLUME) {  // Volume label, skip it
            continue;
        }

        if (dir_entry.attr & ATTR_DIR){ // Directory, skip it
            continue;
        }

        fat_name_to_normal_name(dir_entry.name, name);

        if (strcmp(name, file_name) == 0) {
            return dir_entry.start;
        }
    }
    return 0;

}

// get the next entry in the FAT table
__le16 get_next_entry(FILE* img_file, struct fat_boot_sector* boot_sector, __le16 current_entry){
    __le32 next_entry;
    __le16 fat_entry = boot_sector->reserved * SECTOR_SIZE;
    __le16 entry_offset = (current_entry / 2) * 3;
    __le16 rem = current_entry % 2;
    __le16 fat_entry_offset = fat_entry + entry_offset;

    fseek(img_file, fat_entry_offset , SEEK_SET);
    fread(&next_entry, sizeof(__le32), 1, img_file);

    next_entry = next_entry & 0x00FFFFFF;
    next_entry = (next_entry >> (12*rem)) & 0x0FFF;

    return next_entry;
}

// extract file from fat12 img, given the start entry
// do the whole sectors "logic thing"
void extract_file_from_fat12(FILE* img_file, struct fat_boot_sector* boot_sector, __le16 start_entry , char * name){
    __le16 current_entry = start_entry;
    __le16 next_entry;

    char* buffer = malloc(sizeof(char) * 512);
    //create new file 
    FILE* new_file = fopen(name, "w");

    // write first entry to file
    fseek(img_file, (33 - 2 + current_entry) * SECTOR_SIZE , SEEK_SET);
    fread(buffer, sizeof(char), 512, img_file);
    fwrite(buffer, sizeof(char), 512, new_file);
    
    while (current_entry < 0xFF8){
        next_entry = get_next_entry(img_file, boot_sector, current_entry);
        
        if (next_entry == 0 | next_entry == 1){
            //printf("Current entry: %d, Next entry: %d, 0x%03X\n", current_entry, next_entry, next_entry);
            //printf("Unused Entry.\n");
            break;
        }

        if (next_entry >= 0xFF8){
            //printf("Current entry: %d, Next entry: %d, 0x%03X\n", current_entry, next_entry, next_entry);
            //printf("End of file.\n");
            break;
        }

        // print current index , cluster in int and in 3 hex digits
        //printf("Current entry: %d, Next entry: %d, 0x%03X\n", current_entry, next_entry, next_entry);

        current_entry = next_entry;

        fseek(img_file, (33 - 2 + current_entry) * SECTOR_SIZE , SEEK_SET);
        fread(buffer, sizeof(char), 512, img_file);
        fwrite(buffer, sizeof(char), 512, new_file);

    }

    fclose(new_file);
    free(buffer);
    return;
}

// convert ctime to a string
void time_to_string(__le16 ctime , char* time_str) {
    unsigned int hours = ctime >> 11;
    unsigned int minutes = (ctime >> 5) & 0x3F;
    
    char am_pm[3];
    if (hours >= 12) {
        strcpy(am_pm, "PM");
        if (hours > 12) { hours -= 12; }
    } else {
        strcpy(am_pm, "AM");
        if (hours == 0) { hours = 12; }
    }
    sprintf(time_str, "%02u:%02u %s", hours, minutes, am_pm);
}

// convert cdate to a string
void date_to_string(__le16 cdate , char* date_str) {
    unsigned int day = cdate & 0x1F;
    unsigned int month = (cdate >> 5) & 0x0F;
    unsigned int year = ((cdate >> 9) & 0x7F) + 1980;

    sprintf(date_str, "%02u/%02u/%04u", month, day, year);
}

// convert a FAT file name to "normal" string
void fat_name_to_normal_name(char* FAT_name, char* normal_name) {
    int i = 0;
    while (i < 8 && FAT_name[i] != ' ') {
        normal_name[i] = FAT_name[i];
        i++;
    }
    if (FAT_name[8] != ' ') {
        normal_name[i] = '.';
        i++;
        for (int j = 8; j < 11 && FAT_name[j] != ' '; j++) {
            normal_name[i] = FAT_name[j];
            i++;
        }
    }
    normal_name[i] = '\0';
}

// convert bytes size to string
void format_number(__le32 num, char *str) {
    char temp[20];
    int count = 0;
    int length = 0;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (num > 0) {
        if (count % 3 == 0 && count != 0) {
            temp[length++] = ',';
        }
        temp[length++] = '0' + num % 10;
        num /= 10;
        count++;
    }

    temp[length] = '\0';

    // Reverse the string
    for (int i = 0; i < length; i++) {
        str[i] = temp[length - 1 - i];
    }
    str[length] = '\0';
}

// check if a file is in the root directory
int check_if_file_in_root_dir(FILE* img_file, struct fat_boot_sector* boot_sector , char *file_name) {
    char name[MSDOS_NAME + 1];
    
    fseek(img_file, (boot_sector->reserved + boot_sector->fats * boot_sector->fat_length) * SECTOR_SIZE, SEEK_SET);

    for (int i = 0; i < boot_sector->dir_entries[0]; i++) {
        struct msdos_dir_entry dir_entry;
        fread(&dir_entry, sizeof(struct msdos_dir_entry), 1, img_file);

        if (dir_entry.name[0] == 0x00) {  // End of directory
            break;
        }
        if (dir_entry.name[0] == 0xE5) {  // Deleted file, skip it
            continue;
        }
        if (dir_entry.attr & ATTR_VOLUME) {  // Volume label, skip it
            continue;
        }

        if(dir_entry.attr & ATTR_DIR) {  // Directory, skip it
            continue;
        }

        fat_name_to_normal_name(dir_entry.name, name);
        if (strcmp(file_name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

// Main function
int main(int argc, char* argv[]) {

    // Open the image file
    FILE* img_file = fopen("floppy.img", "rb");
    if (!img_file) {
        perror("Error opening image file");
        return 1;
    }

    // Read the boot sector 
    struct fat_boot_sector boot_sector;
    fread(&boot_sector, sizeof(struct fat_boot_sector), 1, img_file);

    // check if dir - if so print root dir
    if (strcmp(argv[2], "dir") == 0) {
        print_root_dir(img_file, &boot_sector);
    }

    // check if cp - if so do the copy
    if (strcmp(argv[2], "cp") == 0) {

        // check if file exists in root dir
        if (check_if_file_in_root_dir(img_file, &boot_sector, argv[3]) == 0) {
            printf("File %s does not exist in the root directory\n", argv[3]);
            return 1;
        }

        __le16 start_entry = get_file_entry(img_file, &boot_sector, argv[3]);
        extract_file_from_fat12(img_file, &boot_sector, start_entry , argv[4]);
    }
    
    fclose(img_file);
    return 0;
}
