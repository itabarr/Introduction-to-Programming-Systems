#define MAX_LINE_LEN	48 // 39 (data + dir / size) + 8 (max name length) + 1 (null terminator)

void time_to_string(__le16 ctime , char* time_str);
void date_to_string(__le16 cdate , char* date_str);
void fat_name_to_normal_name(char* FAT_name, char* normal_name);
void format_number(__le32 num, char *str);
void print_root_dir(FILE* img_file, struct fat_boot_sector* boot_sector);
__le16 get_file_entry(FILE* img_file, struct fat_boot_sector* boot_sector, char* file_name);
void extract_file_from_fat12(FILE* img_file, struct fat_boot_sector* boot_sector, __le16 start_entry , char * name);
void str_to_upper_case(char* str);