// Muiltithreading or Multiprocessing:
// Multithreading is used here because renaming files is an I/O bound operation.
// Threads share memory, reducing overhead and improving in a while for I/O tasks.
//Multiprocessing would be helpful for CPU-intensive tasks because it seperate processes...
// that use multiple CPU cores more effectively
// Because we are involving disk I/O, multithreading is faster.

#include <stdio.h> //function like printf, scanf..
#include <stdlib.h>// functions like malloc, free...
#include <string.h>
#include <dirent.h> // defines DIR which represents a data stream to/from file
#include <pthread.h>
#include <sys/time.h> // for gettimeofday function

#define FOLDER1_PATH "Datasets/folder1"
#define FOLDER2_PATH "Datasets/folder2"
#define FOLDER3_PATH "Datasets/folder3"

int photo_counter = 0;
pthread_mutex_t lock;

// Function to print all photo names in the given directory
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

// Function executed by threads to rename photos in the given directory
void *rename_photos(void *path) {
    char *folder_path = (char *)path;
    DIR *dir;
    struct dirent *file_entry;
    char old_name[500];
    char new_name[500];
    int count = 0;

    dir = opendir(folder_path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    print_photo_names(folder_path);
    rewinddir(dir); // reset the position of the directory stream
    
    while ((file_entry = readdir(dir)) != NULL){ // Read each entry in the directory stream until all entries are processed
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0){
            continue;
        }
        snprintf(new_name, sizeof(new_name), "%s/photo%d.jpg", folder_path, count); // composes a string with the same text that would ne printed if format was to print, otherwise, saves it as the new string fro our variable
        snprintf(old_name, sizeof(old_name), "%s/%s", folder_path, file_entry->d_name);
        rename(old_name, new_name); // renames the file
        
        // Increment the photo counter safely using a mutex
        pthread_mutex_lock(&lock);
        photo_counter++;
        pthread_mutex_unlock(&lock);
        count++;
    }
    closedir(dir); // closes the directory stream
    return NULL;
}

int main() {
    struct timeval start, stop;
    pthread_t threads[3];
    char *folders[] = {FOLDER1_PATH, FOLDER2_PATH, FOLDER3_PATH};
    
    pthread_mutex_init(&lock, NULL);
    gettimeofday(&start, NULL);
    
    // Creating threads for each folder
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, rename_photos, (void *)folders[i]);
    }
    
    // Joining threads to ensure completion before proceeding
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    gettimeofday(&stop, NULL);
    
    // Display the total number of updated photos and time taken
    printf("Total photos updated: %d\n", photo_counter);
    printf("Editing photos with multithreading took %lu milliseconds\n", 
           (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec));
    
    pthread_mutex_destroy(&lock);
    return 0;
}

