#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "msdos_fs.h"
#include "main.h"


// Function to read the directory entries
void read_root_dir(FILE* img_file, struct fat_boot_sector* boot_sector) {
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
        

        

        // format dir example:
        // 02/23/2022  11:07 PM    <DIR>            WebstormProjects
        if (dir_entry.attr & ATTR_DIR) {
            sprintf(line, "%s  %s  <DIR>            %s", date_str, time_str, name);
            line[MAX_LINE_LEN - 9 + name_len] = '\0';
        }

        // format file example:
        // 09/06/2021  10:01 PM         1,957,374 get-pip.py
         else {
            sprintf(line, "%s  %s                   %s", date_str, time_str, name);
            
            format_number(dir_entry.size, size_str);

            int size_str_len = strlen(size_str);
            
            strncpy(line + MAX_LINE_LEN - 10 - size_str_len, size_str, size_str_len);

            line[MAX_LINE_LEN - 9 + name_len] = '\0';
        }

        printf("%s\n", line);
    }
}


// Function to convert ctime to a string
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

// Function to convert cdate to a string
void date_to_string(__le16 cdate , char* date_str) {
    unsigned int day = cdate & 0x1F;
    unsigned int month = (cdate >> 5) & 0x0F;
    unsigned int year = ((cdate >> 9) & 0x7F) + 1980;

    sprintf(date_str, "%02u/%02u/%04u", month, day, year);
}

// Function to convert a FAT file name to "normal" string
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

//Function to convert bytes size to string
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

// Main function
int main(int argc, char* argv[]) {

    FILE* img_file = fopen("floppy.img", "rb");
    if (!img_file) {
        perror("Error opening image file");
        return 1;
    }

    struct fat_boot_sector boot_sector;
    fread(&boot_sector, sizeof(struct fat_boot_sector), 1, img_file);

    read_root_dir(img_file, &boot_sector);

    fclose(img_file);
    return 0;
}
