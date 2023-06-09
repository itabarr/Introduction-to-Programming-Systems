#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "msdos_fs.h"
#include "main.h"

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

    struct msdos_dir_entry dir_entry;
    for (int i = 0; i < boot_sector->dir_entries[0]; i++) {
        
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
void get_file_info(FILE* img_file, struct fat_boot_sector* boot_sector, char* file_name , __le16* start_entry , __le32* file_size){
    char name[MSDOS_NAME + 1];

    fseek(img_file, (boot_sector->reserved + boot_sector->fats * boot_sector->fat_length) * SECTOR_SIZE, SEEK_SET);

    struct msdos_dir_entry dir_entry;
    for (int i = 0; i < boot_sector->dir_entries[0]; i++) {
        
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
            *start_entry = dir_entry.start;
            *file_size = dir_entry.size;
            return;
        }
    }
    return;

}

// get the next entry in the FAT table
__le32 get_next_entry(FILE* img_file, struct fat_boot_sector* boot_sector, __le16 current_entry){
    __le32 next_entry;

    // calculate the offset of the current entry in the FAT table
    __le16 fat_entry = boot_sector->reserved * SECTOR_SIZE;
    __le16 entry_offset = (current_entry / 2) * 3;
    __le16 rem = current_entry % 2;
    __le16 fat_entry_offset = fat_entry + entry_offset;
    
    // read the entry from the FAT table
    fseek(img_file, fat_entry_offset , SEEK_SET);
    fread(&next_entry, sizeof(__le32), 1, img_file);

    // bit manipulation to get the next entry
    next_entry = next_entry & 0x00FFFFFF;
    next_entry = (next_entry >> (12*rem)) & 0x0FFF;

    return next_entry;
}

// extract file from fat12 img, given the start entry
// do the whole sectors "logic thing"
void extract_file_from_fat12(FILE* img_file, struct fat_boot_sector* boot_sector, __le16 start_entry , char * name , __le32 file_size){
    // start entry is the first entry of the file - do everything in 32 bit for simplicity
    __le32 current_entry = (__le32)start_entry;
    __le32 next_entry;
    __le32 rem = file_size % SECTOR_SIZE;

    char buffer[SECTOR_SIZE];
    FILE* new_file = fopen(name, "w");
    
    // loop until the end of the file
    while (current_entry < 0xFF8 & current_entry != 0 & current_entry != 1){
        next_entry = get_next_entry(img_file, boot_sector, current_entry);
        
        // if the next entry is the last one, write the remaining bytes and return
        if (next_entry >= 0xFF8 & next_entry != 0 & next_entry != 1){
            fseek(img_file, (FIXED_OFFSET + current_entry) * SECTOR_SIZE , SEEK_SET);
            fread(buffer, sizeof(char), rem, img_file);
            fwrite(buffer, sizeof(char), rem, new_file);
            return;
        }
        // print current index , cluster in int and in 3 hex digits
        //printf("Current entry: %d, Next entry: %d, 0x%03X\n", current_entry, next_entry, next_entry);

        // write the current entry to the new file
        fseek(img_file, (FIXED_OFFSET + current_entry) * SECTOR_SIZE , SEEK_SET);
        fread(buffer, sizeof(char), SECTOR_SIZE, img_file);
        fwrite(buffer, sizeof(char), SECTOR_SIZE, new_file);

        current_entry = next_entry;

    }

    fclose(new_file);
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

// convert string to upper case
void str_to_upper_case(char* str) {
    int i = 0;
    while(str[i] != '\0') {
        str[i] = toupper(str[i]);
        i++;
    }
    return;
}

// Main function
int main(int argc, char* argv[]) {

    // check number of arguments
    if (argc < 3 || argc > 5) {
        printf("Program takes 2 to 4 arguments.\n");
        return 1;
    }

    // check if second argument is dir or cp
    if (strcmp(argv[2], "dir") != 0 && strcmp(argv[2], "cp") != 0) {
        printf("Second argument must be either 'dir' or 'cp'.\n");
        return 1;
    }

    // Open the image file
    FILE* img_file = fopen(argv[1], "rb");
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
        
        // check number of arguments
        if (argc !=5){
            printf("For 'cp' cammnd, program needs to have <src> and <dest> arguments.\n");
            return 1;
        }

        // convert file name to upper case
        str_to_upper_case(argv[3]);

        // check if file exists in root dir
        if (check_if_file_in_root_dir(img_file, &boot_sector, argv[3]) == 0) {
            printf("File %s does not exist in the root directory\n", argv[3]);
            return 1;
        }

        // get file info  
        __le16 start_entry;
        __le32 file_size;
        get_file_info(img_file, &boot_sector, argv[3], &start_entry, &file_size);

        // extract file from fat12
        extract_file_from_fat12(img_file, &boot_sector, start_entry , argv[4] , file_size);
    }
    
    fclose(img_file);
    return 0;
}
