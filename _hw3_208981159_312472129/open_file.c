#include "types.h"
#include "user.h"
#include "fcntl.h"

#define FILE1 "file1.txt"
#define FILE2 "file2.txt"
#define FILE3 "file3.txt"

void delete_files() {
  unlink(FILE1);
  unlink(FILE2);
  unlink(FILE3);
}

int main(void) {
  int pid;
  int fd1, fd2, fd3;

  pid = fork();

  if (pid < 0) {
    printf(1, "Fork failed\n");
    exit();
  }

  if (pid == 0) {
    // Child process

    // Open the files
    fd1 = open(FILE1, O_CREATE | O_WRONLY);
    fd2 = open(FILE2, O_CREATE | O_WRONLY);
    fd3 = open(FILE3, O_CREATE | O_WRONLY);

    // Check if file opening was successful
    if (fd1 < 0 || fd2 < 0 || fd3 < 0) {
      printf(1, "Failed to open files\n");
      exit();
    }

    printf(1, "Opened files successfully\n");

    // Wait for 10 seconds
    sleep(10000);

    // Close the files
    close(fd1);
    close(fd2);
    close(fd3);

    // Delete the files
    delete_files();

    printf(1, "Closed and deleted files\n");

    exit();
  } else {
    

    exit();
  }
}
