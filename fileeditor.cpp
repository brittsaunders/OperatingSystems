/*Brittany Saunders, Samuel Akinmulero
 * June 20th, 2018
 * CS570 Summer 2018
 * Guy Leonard
 * Assignment #2
 */

#include <fcntl.h>
#include <dirent.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include <limits.h>
#include <semaphore.h>

#define THREADS 3


using namespace std;


//GLOBAL VARIABLES
int choice;
static ofstream rfile; //Regular file
struct stat buf;  //Stat function buffer
static sem_t WAIT; //Semaphore
static string file; //Global file variable to access in threads
static int status;

/*-----------------------------------------------------------------------------------------------------------------------*/

//HELPER FUNCTIONS

//File size helper function
size_t getFilesize(const string& filename) {
    if (stat(filename.c_str(), &buf) != 0) {
        return 0;
    }
    return buf.st_size;
    //return buf.st_size/1024; // Size of file in Kilobytes
}
//Checks to see if directory/file exists
static bool exists_check(const string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
//Prints directory content helper function
static void printdircontent(string fd){
    DIR *dir;  //Creates directory pointer
    struct dirent *direct; //Used to get information about entries

    dir = opendir(fd.c_str());  //Opens directory
    if(dir != NULL ){
        while(direct = readdir(dir)){ //Reads through directory
            puts(direct->d_name);  //Prints entry
        }
        (void) closedir(dir);  //Closes the directory
    }
    else{
        perror("Directory failed to open");  //Error check
    }
}

void *child_copy(void *tid) {  //Start routine for making shadow file
    sem_wait(&WAIT); //Unlocks semaphore

    string fname = file + ".bak"; //Appends a .bak to filename
    std::ifstream src(file.c_str(), std::ios::binary); //Inputting real file
    std::ofstream dst(fname.c_str(), std::ios::binary); //Outputting shadow file

    dst << src.rdbuf(); //Copying contents of file

    sem_post(&WAIT); //Locks semaphore
}

void *child_sorted(void *tid) { //Start routine for sorting file contents
    sem_wait(&WAIT);

    string name = "SORTED.txt";
    ofstream myfile(name.c_str());
    fstream is(file.c_str()); //Readies file for iterator input stream
    istream_iterator<string> start(is), end; // Creates iterator string
    vector<string> letters(start,end); //Puts file into vector
    cout << "\n\n";

    sort(letters.begin(), letters.end()); //Sorts the file content in ABC order by words
    copy(letters.begin(), letters.end(), ostream_iterator<string>(myfile, " ")); //Copies new output into file

    sem_post(&WAIT);
}

void *child_reverseSort(void *tid) { //Start routine for reverse sorting file contents
    sem_wait(&WAIT);

    string name = "REVERSE_SORTED.txt";
    ofstream myfile(name.c_str());
    fstream is(file.c_str());
    istream_iterator<string> start(is), end;
    vector<string> letters(start,end);
    cout << "\n\n";

    sort(letters.rbegin(), letters.rend()); //Sorts the file content in reverse ABC order by words
    copy(letters.begin(), letters.end(), ostream_iterator<string>(myfile, " "));

    sem_post(&WAIT);
}

/*-----------------------------------------------------------------------------------------------------------------------*/
//GLOBAL Methods
static bool exists_check(const string& name);

static void printdircontent(string fd);

static void main_menu();

/*-----------------------------------------------------------------------------------------------------------------------*/

//Main program
void modes(){

    switch(choice){
        //Creates new directories
        case 1: {
            string dirname; //Directory name
            cout << "Enter path folder name or enter '9' to return to the main menu. " << endl;
            cin.ignore(); //Clears stream
            getline(cin, dirname); //Accepts user input
            if (exists_check(dirname)) {
                cout << "Directory already exists." << endl;
            } else if (dirname == "9") {
                main_menu(); //Returns to main menu
            } else {
                mkdir(dirname.c_str(), S_IRWXU); //Makes directory
            }
            break;
        }
            //Creates new regular files
        case 2: {

            string filename;
            file = filename;
            cout << "Enter file name in the format of FILE.txt or enter '9' to return to the main menu: " << endl;
            cin.ignore();
            cin >> filename;
            if (filename == "9")
                main_menu();
            rfile.open(filename.c_str()); //Opens full path
            rfile.close(); //Closes file
        }
            break;
            //Read from a file
        case 3:
        {
            string filename;
            cout << "Enter file name to read or enter '9' to return to the main menu: " << endl;
            cin.ignore(1000, '\n');
            getline(cin, filename);
            if(filename == "9")
                main_menu();

            ifstream myfile(filename.c_str());
            if(!myfile.is_open()) {
                cout << "Sorry, that file does not exist. Redirecting to main menu.\n\n";
                sleep(3);
                main_menu();
                break;
            }
            else {
                sem_init(&WAIT, 0, 50); //Initializes semaphore
                pthread_t threads[THREADS]; //Initializes threads
                file = filename;

                string line;
                cout << "\n" << filename << endl; //Prints name of file
                cout << "-------------------------------------------------------------------------------------" << endl;

                while (getline(myfile, line)) {
                    cout << line << "\n"; //Prints contents line by line
                }
                myfile.close();

                status =  pthread_create(&threads[0], NULL, child_copy, (void *) 1); //Creates shadow file process

                pthread_join(threads[0], NULL); //Waits

                sem_destroy(&WAIT); //Destroys semaphore
            }
        }
            break;
            //Write to file: insert
        case 4: {
            string filename;
            int start;
            int bytes;
            string message;
            cout << "Enter file name to insert to or enter '9' to return to the main menu: " << endl;
            cin.ignore(1000, '\n');
            getline(cin, filename);
            ifstream myfile(filename.c_str());
            if (!myfile.good()) {
                cout << "Sorry, that file does not exist. Redirecting to main menu.\n\n";
                sleep(3);
                main_menu();
                break;
            }
            if (filename == "9") {
                main_menu();
                break;
            } else {
                sem_init(&WAIT, 0, 50);
                pthread_t threads[THREADS];
                file = filename;

                ifstream in(filename.c_str());
                string cont((istreambuf_iterator<char>(in)),
                            istreambuf_iterator<char>()); //Converts file contents to string
                cout << "Where would you like to start writing? Example: '4'" << endl;
                cin >> start;
                cout << "What do you want to write?" << endl;
                cin.ignore();
                getline(cin, message);
                cout << "How many bytes will you be overwriting? Example '12'" << endl;
                cin >> bytes;
                cont.replace(start, bytes, message); //Insertion occurs
                ofstream out(filename.c_str()); //Opens file for output operations
                out << cont; //Reads string into file
                myfile.close();
                in.close();
                out.close();

                status = pthread_create(&threads[0], NULL, child_copy, (void *) 1);
                status = pthread_create(&threads[1], NULL, child_sorted, (void *) 2); //Creates sorted process
                //Creates reverse sorted process
                status = pthread_create(&threads[2], NULL, child_reverseSort, (void *) 3);

                pthread_join(threads[0], NULL);
                pthread_join(threads[1], NULL);
                pthread_join(threads[2], NULL);

                sem_destroy(&WAIT);

            }
        }
            break;
            //Write to file: append
        case 5:
        {
            string filename;
            cout << "Enter file name to append to or enter '9' to return to the main menu: " << endl;
            cin.ignore(1000, '\n');
            getline(cin, filename);
            ifstream myfile(filename.c_str());
            if(!myfile.good()) {
                cout << "Sorry, that file does not exist. Redirecting to main menu.\n\n";
                sleep(3);
                main_menu();
                break;
            }
            if (filename == "9") {
                main_menu();
                break;
            }
            else {
                sem_init(&WAIT, 0, 50);
                pthread_t threads[THREADS];
                file = filename;

                rfile.open(filename.c_str(), fstream::app); //Opens file in append mode
                cout << "What would you like to write to the file?: " << endl;
                string write;
                getline(cin, write);
                rfile << write << endl;
                rfile.close();

                status = pthread_create(&threads[0], NULL, child_copy, (void *) 1);
                status = pthread_create(&threads[1], NULL, child_sorted, (void *) 2);
                status = pthread_create(&threads[2], NULL, child_reverseSort, (void *) 3);

                pthread_join(threads[0], NULL);
                pthread_join(threads[1], NULL);
                pthread_join(threads[2], NULL);

                sem_destroy(&WAIT);
            }
        }
            break;

            //Write to file: overwrite
        case 6: {
            string filename;
            cout << "Enter file name overwrite to or enter '9' to return to the main menu: " << endl;
            cin.ignore(1000, '\n');
            getline(cin, filename);
            ifstream myfile(filename.c_str());
            if (!myfile.good()) {
                cout << "Sorry, that file does not exist. Redirecting to main menu.\n\n";
                sleep(3);
                main_menu();
                break;
            }
            if (filename == "9") {
                main_menu();
                break;
            } else {
                sem_init(&WAIT, 0, 50);
                pthread_t threads[THREADS];
                file = filename;

                rfile.open(filename.c_str(), fstream::trunc); //Opens file in overwrite mode
                cout << "What would you like to write to the file?: " << endl;
                string write;
                getline(cin, write);
                rfile << write << endl;
                rfile.close();

                status = pthread_create(&threads[0], NULL, child_copy, (void *) 1);
                status = pthread_create(&threads[1], NULL, child_sorted, (void *) 2);
                status = pthread_create(&threads[2], NULL, child_reverseSort, (void *) 3);

                pthread_join(threads[0], NULL);
                pthread_join(threads[1], NULL);
                pthread_join(threads[2], NULL);

                sem_destroy(&WAIT);
            }
        }
            break;

            //Prints file status
        case 7:
        {
            string filename;
            cout << "What file would you like information about? " << endl;
            cin.ignore();
            getline(cin, filename);
            if(filename == "9")
                main_menu();

            ifstream myfile(filename.c_str());
            if(!myfile.good()){
                cout << "File does not exist." << endl;
                sleep(3);
                main_menu();
                break;
            }

            if(rfile.is_open() == true){
                cout << "File is open." << endl;
            }
            else if(rfile.is_open() != true){
                cout << "File is not open." << endl;
            }

            stat(filename.c_str(),&buf); //Uses stat function to get file information

            cout << "ID of device containing file: " << buf.st_dev << endl;
            cout << "Inode number: " << buf.st_ino << endl;
            cout << "Protection: " << buf.st_mode << endl;
            cout << "# of hard links: " << buf.st_nlink << endl;
            cout << "User ID: " << buf.st_uid << endl;
            cout << "Group ID: " << buf.st_gid << endl;
            cout << "Special file device ID: " << buf.st_rdev << endl;
            cout << "File size: " << getFilesize(filename) << " B" << endl;
            printf("Last time file was accessed: %s", ctime(&buf.st_atime));
            printf("Last time file was modified: %s", ctime(&buf.st_mtime));
            printf("Last time file status was changed: %s", ctime(&buf.st_ctime));
        }

            break;

            //Prints file status
        case 8:
        {
            string path;
            cout << "What directory would you like to look into?" << endl;
            cin.ignore();
            getline(cin,path);
            if(!exists_check(path)) {
                cout << "Directory does not exist. Redirecting to main menu.\n\n";
                sleep(3);
                main_menu();
                break;
            }
            else {
                printdircontent(path); //Refers to helper function to print contents
                printf(path.c_str()); //ACTUALLY print contents to console
            }
        }
            break;

        case 9:{
            main_menu(); //Prints main menu to stdout
        }
            break;

        default: cout << "Please refer to menu options." << endl;
    }

    cout << "\n";
}


static void main_menu() {
    cout << "\t      Main Menu" << endl;
    cout << "-----------------------------------------" << endl;
    cout << "Press 1 to create a new directory file" << endl;
    cout << "\n";
    cout << "Press 2 to create a new regular file" << endl;
    cout << "\n";
    cout << "Press 3 to read from a file" << endl;
    cout << "\n";
    cout << "Press 4 to write to a file in Insert mode" << endl;
    cout << "\n";
    cout << "Press 5 to write to a file in Append mode" << endl;
    cout << "\n";
    cout << "Press 6 to write to a file in Overwrite mode" << endl;
    cout << "\n";
    cout << "Press 7 to print the file status" << endl;
    cout << "\n";
    cout << "Press 8 to print directory listing" << endl;
    cout << "\n";
    cout << "Press 9 to print out menu options again." << endl;
    cout << "\n";
    cout << "Program is sensitive, therefore, ANYTHING ELSE will terminate the program." << endl;
    cout << "--> ";
    cin >> choice;
    modes();

    string ans;
    cout << "Press 9 if you want to return to the main menu. " << endl;
    cin >> ans;
    if(ans == "9")
        main_menu();
}

int main() {
    main_menu();
    return 0;
}