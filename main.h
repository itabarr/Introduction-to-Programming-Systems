#define MAX_LINE_LEN	48 // 39 (data + dir / size) + 8 (max name length) + 1 (null terminator)

void time_to_string(__le16 ctime , char* time_str);
void date_to_string(__le16 cdate , char* date_str);
void fat_name_to_normal_name(char* FAT_name, char* normal_name);
void format_number(__le32 num, char *str);
void print_root_dir(FILE* img_file, struct fat_boot_sector* boot_sector);
int check_if_file_exists(FILE* img_file, struct fat_boot_sector* boot_sector, char* file_name);
