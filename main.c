#include <stdio.h> //function like printf, scanf..
#include <stdlib.h>// functions like malloc, free...
#include <unistd.h> //function like fork, pipe, read, write, close...
#include <string.h>
#include <sys/types.h> // defines primitive data types like pid_t..
#include <sys/mman.h> // allows functions such as mmap for mapping memory for
                        //files and dircectives 
#include <dirent.h> // defines DIR which represents a data stream to/from file
#include <sys/wait.h> // provides process mangement -> functions such as wait
#include <sys/time.h> // for gettimeofday function

#define FOLDER1_PATH "Datasets/folder1"
#define FOLDER2_PATH "Datasets/folder2"
#define FOLDER3_PATH "Datasets/folder3"

void print_photo_names(char *folder_path){
    DIR *dir; // is a pointer to a directory stream used to read directory 
              // contents
    struct dirent *file_entry; //pointer to a structure representing a diretory enertry 
                                // d_type -> gives the type of file
                                // d_name -> name of the file
    dir = opendir(folder_path); // opens specified directory
    if (dir == NULL){
        perror("Unable to open directory\n");
        return;
    }

    printf("Directory opened successfully\n");
    while ((file_entry = readdir(dir)) != NULL){ // Read each entry in the directory stream until all entries are processed
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0){
            continue;
        }
        printf("%s\n", file_entry->d_name); // print the name of the directory entry (file or folder)
    }

    closedir(dir); // closes the directory stream
}

void rename_photos(char *folder_path, int* shared_mem){
    DIR *dir;
    struct dirent *file_entry;
    char old_name[500];
    char new_name[500];

    dir = opendir(folder_path);

    print_photo_names(folder_path);

    rewinddir(dir); // reset the position of the directory stream

    int count = 0;
    while ((file_entry = readdir(dir)) != NULL){ // Read each entry in the directory stream until all entries are processed
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0){
            continue;
        }
        snprintf(new_name, sizeof(new_name), "%s/photo%d.jpg", folder_path, count); // composes a string with the same text that would ne printed if format was to print, otherwise, saves it as the new string fro our variable
        snprintf(old_name, sizeof(old_name), "%s/%s", folder_path, file_entry->d_name);
        rename(old_name, new_name); // renames the file
        count++;

    }

    rewinddir(dir);
    print_photo_names(folder_path);

    *shared_mem += count; //accenssed by all processes; shared memory region
    closedir(dir); // closes the directory stream

}

int main(){
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    pid_t p1,p2,p3;

    int *shared_mem = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // creates our shared memory region for one integer between all child processes

    p1 = fork();
    if (p1 == 0){
        rename_photos(FOLDER1_PATH, shared_mem);
        exit(0);
    }
    p2 = fork();
    if (p2 == 0){
        rename_photos(FOLDER2_PATH, shared_mem);
        exit(0);
    }
    p3 = fork();
    if (p3 == 0){
        rename_photos(FOLDER3_PATH, shared_mem);
        exit(0);
    }
    wait(NULL);
    wait(NULL);
    wait(NULL);

    printf("Total photos updated: %d\n", *shared_mem);

    gettimeofday(&stop, NULL);
    printf("Editing the photos using multiprocessing and tracking a counter using shared memory took %lu milliseconds \n", (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec));

    return 0;
}