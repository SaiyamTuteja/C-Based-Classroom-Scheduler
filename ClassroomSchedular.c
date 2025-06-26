#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_LEN 100

// ANSI color codes for vibrant CLI
#define COLOR_RESET   "\033[0m"
#define COLOR_HEADER  "\033[1;38;5;199m" // Vibrant pink
#define CLI_COLOR_MENU    "\033[1;38;5;45m"  // Cyan
#define COLOR_INPUT   "\033[1;38;5;226m" // Yellow
#define COLOR_SUCCESS "\033[1;38;5;82m"  // Green
#define COLOR_ERROR   "\033[1;38;5;196m" // Red
#define COLOR_HILITE  "\033[1;38;5;208m" // Orange

typedef struct Lecture {
    char day[MAX_LEN];
    char time[MAX_LEN];
    char subject[MAX_LEN];
    char faculty[MAX_LEN];
    char section;
    struct Lecture* next;
} Lecture;

Lecture* head = NULL;

// Stack for undo
Lecture* stack[100];
int top = -1;

// Queue for swap
Lecture* queue[100];
int front = -1, rear = -1;

// Structure to store teacher workload
typedef struct {
    char name[MAX_LEN];
    int lectureCount;
    char sections[4];  // To store which sections they teach
    int sectionCount;
} TeacherLoad;

// Structure to store subject information
typedef struct {
    char code[MAX_LEN];
    char name[MAX_LEN];
    int isLab;
} SubjectInfo;

// Structure to store time slot information
typedef struct {
    char day[4];
    char time[MAX_LEN];
    char section;
} TimeSlot;

// Global variables
SubjectInfo curriculum[] = {
    {"TMC201", "Advanced Database Management Systems", 0},
    {"TMC202", "Advanced Java Programming", 0},
    {"TMC203", "Data Structures", 0},
    {"TMC215", "DSE-II: Human-Computer Interaction", 0},
    {"TMC221", "GE-I: Research Methodology", 0},
    {"XMC201", "Career Skills-II", 0},
    {"PMC201", "ADBMS Laboratory", 1},
    {"PMC202", "Advanced Java Programming Lab", 1},
    {"PMC203", "Data Structures Laboratory", 1},
    {"GP201", "General Proficiency", 0},
    {"PBL", "PROJECT BASED LEARNING", 0}
};
const int CURRICULUM_SIZE = sizeof(curriculum) / sizeof(curriculum[0]);

// Multi-language support (English/Hindi demo)
#define LANG_EN 0
#define LANG_HI 1
int currentLang = LANG_EN;

// UI strings (expand as needed)
const char* UI_MENU[][2] = {
    {"1. Display Timetable", "1. समय सारणी देखें"},
    {"2. Swap Classes", "2. कक्षाएं बदलें"},
    {"3. Change Class Section", "3. कक्षा अनुभाग बदलें"},
    {"4. Teacher Load Analysis", "4. शिक्षक भार विश्लेषण"},
    {"5. Assign New Teacher", "5. नया शिक्षक असाइन करें"},
    {"6. Save Timetable", "6. समय सारणी सहेजें"},
    {"7. Load Timetable", "7. समय सारणी लोड करें"},
    {"8. Exit", "8. बाहर निकलें"},
    {"10. Search Timetable", "10. समय सारणी खोजें"},
    {"12. Set Notification/Reminder", "12. अधिसूचना/अनुस्मारक सेट करें"},
    {"13. Statistics Dashboard", "13. सांख्यिकी डैशबोर्ड"}
};
const char* UI_ENTER_CHOICE[] = {"Enter choice: ", "विकल्प दर्ज करें: "};
const char* UI_INVALID_CHOICE[] = {"Invalid choice. Please try again.", "अमान्य विकल्प। कृपया पुनः प्रयास करें।"};
const char* UI_HEADER[] = {"====== Classroom Scheduler ======", "====== कक्षा अनुसूचक ======"};
const char* UI_LANG_PROMPT = "Select Language / भाषा चुनें:\n1. English\n2. हिन्दी\nEnter choice: ";

// Helper to print UI string by index
#define UI(idx) UI_MENU[idx][currentLang]

// Function declarations
void insertLecture(char* day, char* time, char* subject, char* faculty, char section);
const char* getSubjectName(const char* subjectCode);
void printTimetable(char section);
void initializeTimetable(void);
void swapTeachersFlexible(char section1, char section2, int swapTeacher);
void swapWithinSection(char section, const char* day, int slot1, int slot2, int swapTeacher);
void saveTimetableAs(const char* filename);
void url_encode(const char* src, char* dest, int max_len);
void shareLinkToWhatsAppGroup(const char* link);

// Insert a new lecture
void insertLecture(char* day, char* time, char* subject, char* faculty, char section) {
    Lecture* newLecture = (Lecture*)malloc(sizeof(Lecture));
    if (newLecture == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    strcpy(newLecture->day, day);
    strcpy(newLecture->time, time);
    strcpy(newLecture->subject, subject);
    strcpy(newLecture->faculty, faculty);
    newLecture->section = section;
    newLecture->next = head;
    head = newLecture;
}

// Push to undo stack
void push(Lecture* lec) {
    stack[++top] = lec;
}

// Pop from undo stack
Lecture* pop() {
    if (top == -1) return NULL;
    return stack[top--];
}

// Add to queue
void enqueue(Lecture* lec) {
    if (front == -1) front = 0;
    queue[++rear] = lec;
}

// Remove from queue
Lecture* dequeue() {
    if (front == -1 || front > rear) return NULL;
    return queue[front++];
}

// Print timetable for a section
void printTimetable(char section) {
    Lecture* temp = head;
    printf("\n================================================================\n");
    printf("                    TIMETABLE FOR SECTION %c                        \n", section);
    printf("================================================================\n\n");

    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char* fullDays[] = {"MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};

    // For each day
    for(int i = 0; i < 6; i++) {
        int hasLecture = 0;
        temp = head;
        while(temp) {
            if(temp->section == section && strcmp(temp->day, days[i]) == 0) {
                hasLecture = 1;
                break;
            }
            temp = temp->next;
        }

        if(hasLecture) {
            printf("\n%s:\n", fullDays[i]);
            printf("----------------------------------------------------------------\n");
            printf("%-13s | %-7s | %-35s | %-15s |\n", 
                   "Time", "Code", "Subject", "Faculty");
            printf("----------------------------------------------------------------\n");

            // Sort lectures by time for this day
            Lecture* sortedLectures[20];
            int lectureCount = 0;
            
            temp = head;
            while(temp) {
                if(temp->section == section && strcmp(temp->day, days[i]) == 0) {
                    sortedLectures[lectureCount++] = temp;
                }
                temp = temp->next;
            }
            
            // Simple bubble sort by time
            for(int j = 0; j < lectureCount-1; j++) {
                for(int k = 0; k < lectureCount-j-1; k++) {
                    if(strcmp(sortedLectures[k]->time, sortedLectures[k+1]->time) > 0) {
                        Lecture* temp = sortedLectures[k];
                        sortedLectures[k] = sortedLectures[k+1];
                        sortedLectures[k+1] = temp;
                    }
                }
            }
            
            // Print sorted lectures
            for(int j = 0; j < lectureCount; j++) {
                const char* subjectName = getSubjectName(sortedLectures[j]->subject);
                char truncatedSubject[36] = "";
                char truncatedFaculty[16] = "";
                
                // Truncate subject if necessary
                strncpy(truncatedSubject, subjectName, 35);
                truncatedSubject[35] = '\0';
                
                // Truncate faculty if necessary
                strncpy(truncatedFaculty, sortedLectures[j]->faculty, 15);
                truncatedFaculty[15] = '\0';
                
                printf("%-13s | %-7s | %-35s | %-15s |\n", 
                       sortedLectures[j]->time, 
                       sortedLectures[j]->subject,
                       truncatedSubject,
                       truncatedFaculty);
            }
            printf("----------------------------------------------------------------\n");
        }
    }
    printf("\n================================================================\n");
}

// Mark teacher unavailable and swap
void handleUnavailable(char* faculty, char section) {
    Lecture* temp = head;
    while (temp) {
        if (strcmp(temp->faculty, faculty) == 0 && temp->section == section) {
            push(temp); // Save for undo
            printf("\nTeacher %s unavailable for %s (%s).\n", faculty, temp->subject, temp->time);

            // Try to swap with a lecture not using same teacher
            Lecture* trySwap = head;
            while (trySwap) {
                if (trySwap->section == section &&
                    strcmp(trySwap->faculty, faculty) != 0 &&
                    strcmp(trySwap->time, temp->time) != 0) {

                    printf("Swapped with %s (%s).\n", trySwap->subject, trySwap->faculty);

                    // Swap contents
                    char tmpSubject[MAX_LEN], tmpFaculty[MAX_LEN];
                    strcpy(tmpSubject, temp->subject);
                    strcpy(tmpFaculty, temp->faculty);
                    strcpy(temp->subject, trySwap->subject);
                    strcpy(temp->faculty, trySwap->faculty);
                    strcpy(trySwap->subject, tmpSubject);
                    strcpy(trySwap->faculty, tmpFaculty);

                    return;
                }
                trySwap = trySwap->next;
            }

            printf("No swap possible. Marked as Free Period.\n");
            strcpy(temp->subject, "Free Period");
            strcpy(temp->faculty, "-");
        }
        temp = temp->next;
    }
}

// Undo last change
void undo() {
    Lecture* lec = pop();
    if (lec == NULL) {
        printf("Nothing to undo.\n");
        return;
    }
    printf("Undo: Restored %s (%s).\n", lec->subject, lec->faculty);
}

// Initial timetable data
void initializeTimetable() {
    // MONDAY
    // Section A & B (LT-402)
    insertLecture("MON", "8:00-8:55", "TMC202", "Mr. Amit Juyal", 'A');
    insertLecture("MON", "8:55-9:50", "TMC201", "Dr. Udham Singh", 'A');
    insertLecture("MON", "10:10-11:05", "PBL", "-", 'A');
    insertLecture("MON", "11:05-12:00", "XMC201", "Mr. Digamber", 'A');

    insertLecture("MON", "8:00-8:55", "TMC202", "Mr. Amit Juyal", 'B');
    insertLecture("MON", "8:55-9:50", "TMC201", "Dr. Udham Singh", 'B');
    insertLecture("MON", "10:10-11:05", "PBL", "-", 'B');
    insertLecture("MON", "11:05-12:00", "XMC201", "Mr. Digamber", 'B');

    // Section C & D (LT-501)
    insertLecture("MON", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'C');
    insertLecture("MON", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'C');
    insertLecture("MON", "10:10-11:05", "PBL", "-", 'C');
    insertLecture("MON", "11:05-12:00", "PBL", "-", 'C');

    insertLecture("MON", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'D');
    insertLecture("MON", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'D');
    insertLecture("MON", "10:10-11:05", "PBL", "-", 'D');
    insertLecture("MON", "11:05-12:00", "PBL", "-", 'D');

    // TUESDAY
    // Section A & B (LT-402)
    insertLecture("TUE", "8:00-9:50", "PMC201", "Sec. A (Lab 7)", 'A');
    insertLecture("TUE", "8:00-9:50", "PMC202", "Sec. B (IoT Lab)", 'B');
    insertLecture("TUE", "10:10-11:05", "TMC203", "Mr. Neeraj", 'A');
    insertLecture("TUE", "10:10-11:05", "TMC203", "Mr. Neeraj", 'B');

    // Section C & D (LT-501)
    insertLecture("TUE", "12:00-12:55", "PMC201", "Sec. C (Lab 7)", 'C');
    insertLecture("TUE", "12:00-12:55", "PMC202", "Sec. D (IoT Lab)", 'D');
    insertLecture("TUE", "2:10-3:05", "PMC203", "Sec. D (IoT Lab)", 'D');

    // WEDNESDAY
    // Section A & B (LT-402)
    insertLecture("WED", "8:00-8:55", "XMC201", "Mr. Aanand", 'A');
    insertLecture("WED", "8:55-9:50", "TMC201", "Dr. Udham Singh", 'A');
    insertLecture("WED", "10:10-11:05", "TMC203", "Mr. Neeraj", 'A');
    insertLecture("WED", "11:05-12:00", "PDP PRACTICAL", "Ms. Sakshi", 'A');
    insertLecture("WED", "12:00-12:55", "TMC202", "Mr. Amit Juyal", 'A');
    insertLecture("WED", "2:10-3:05", "PMC201", "Sec. B (Lab 7)", 'A');

    insertLecture("WED", "8:00-8:55", "XMC201", "Mr. Aanand", 'B');
    insertLecture("WED", "8:55-9:50", "TMC201", "Dr. Udham Singh", 'B');
    insertLecture("WED", "10:10-11:05", "TMC203", "Mr. Neeraj", 'B');
    insertLecture("WED", "11:05-12:00", "PDP PRACTICAL", "Ms. Sakshi", 'B');
    insertLecture("WED", "12:00-12:55", "TMC202", "Mr. Amit Juyal", 'B');
    insertLecture("WED", "2:10-3:05", "PMC201", "Sec. B (Lab 7)", 'B');

    // Section C & D (LT-501)
    insertLecture("WED", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'C');
    insertLecture("WED", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'C');
    insertLecture("WED", "11:05-12:00", "TMC203", "Mr. Neeraj", 'C');
    insertLecture("WED", "12:00-12:55", "PMC203", "Sec. C (IoT Lab)", 'C');

    insertLecture("WED", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'D');
    insertLecture("WED", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'D');
    insertLecture("WED", "11:05-12:00", "TMC203", "Mr. Neeraj", 'D');

    // THURSDAY
    // Section A & B (LT-402)
    insertLecture("THU", "8:00-9:50", "PMC202", "Sec. A (IoT Lab)", 'A');
    insertLecture("THU", "8:00-9:50", "PMC203", "Sec. B (Lab 7)", 'B');
    insertLecture("THU", "10:10-11:05", "TMC201", "Dr. Udham Singh", 'A');
    insertLecture("THU", "10:10-11:05", "TMC201", "Dr. Udham Singh", 'B');

    // Section C & D (LT-501)
    insertLecture("THU", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'C');
    insertLecture("THU", "8:55-9:50", "XMC201", "Mr. Aanand", 'C');
    insertLecture("THU", "11:05-12:00", "TMC203", "Mr. Neeraj", 'C');
    insertLecture("THU", "12:00-12:55", "PMC201", "Sec. D (Lab 7)", 'C');
    insertLecture("THU", "12:00-12:55", "PMC202", "Sec. C (IoT Lab)", 'C');
    insertLecture("THU", "2:10-3:05", "PDP PRACTICAL", "Ms. Sakshi", 'C');

    insertLecture("THU", "8:00-8:55", "TMC201", "Dr. Udham Singh", 'D');
    insertLecture("THU", "8:55-9:50", "XMC201", "Mr. Aanand", 'D');
    insertLecture("THU", "11:05-12:00", "TMC203", "Mr. Neeraj", 'D');
    insertLecture("THU", "12:00-12:55", "PMC201", "Sec. D (Lab 7)", 'D');
    insertLecture("THU", "12:00-12:55", "PMC202", "Sec. C (IoT Lab)", 'D');
    insertLecture("THU", "2:10-3:05", "PDP PRACTICAL", "Ms. Sakshi", 'D');

    // FRIDAY
    // Section A & B (LT-402)
    insertLecture("FRI", "8:00-8:55", "TMC203", "Mr. Neeraj", 'A');
    insertLecture("FRI", "8:55-9:50", "XMC201", "Mr. Aanand", 'A');
    insertLecture("FRI", "10:10-11:05", "TMC202", "Mr. Amit Juyal", 'A');
    insertLecture("FRI", "11:05-12:00", "PDP PRACTICAL", "Ms. Sakshi", 'A');

    insertLecture("FRI", "8:00-8:55", "TMC203", "Mr. Neeraj", 'B');
    insertLecture("FRI", "8:55-9:50", "XMC201", "Mr. Aanand", 'B');
    insertLecture("FRI", "10:10-11:05", "TMC202", "Mr. Amit Juyal", 'B');
    insertLecture("FRI", "11:05-12:00", "PDP PRACTICAL", "Ms. Sakshi", 'B');

    // Section C & D (LT-501)
    insertLecture("FRI", "8:00-8:55", "XMC201", "Mr. Aanand", 'C');
    insertLecture("FRI", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'C');
    insertLecture("FRI", "10:10-11:05", "TMC203", "Mr. Neeraj", 'C');
    insertLecture("FRI", "12:00-12:55", "XMC201", "Mr. Vishal", 'C');

    insertLecture("FRI", "8:00-8:55", "XMC201", "Mr. Aanand", 'D');
    insertLecture("FRI", "8:55-9:50", "TMC202", "Mr. Amit Juyal", 'D');
    insertLecture("FRI", "10:10-11:05", "TMC203", "Mr. Neeraj", 'D');
    insertLecture("FRI", "12:00-12:55", "XMC201", "Mr. Vishal", 'D');

    // SATURDAY
    // Section A & B (LT-402)
    insertLecture("SAT", "11:05-12:00", "PBL", "-", 'A');
    insertLecture("SAT", "12:55-1:50", "PMC203", "Sec. A (Lab 7)", 'A');

    // Section C & D (LT-501)
    // No classes shown in timetable
}

// Function to print timetable for a specific day
void printDayTimetable(char section, const char* day) {
    Lecture* temp = head;
    int hasLecture = 0;
    
    while(temp) {
        if(temp->section == section && strcmp(temp->day, day) == 0) {
            hasLecture = 1;
            break;
        }
        temp = temp->next;
    }

    if(!hasLecture) {
        printf("\nNo lectures scheduled for this day in Section %c\n", section);
        return;
    }

    printf("\n================================================================\n");
    printf("              TIMETABLE FOR SECTION %c - %s                        \n", section, day);
    printf("================================================================\n");
    printf("----------------------------------------------------------------\n");
    printf("%-13s | %-7s | %-35s | %-15s |\n", 
           "Time", "Code", "Subject", "Faculty");
    printf("----------------------------------------------------------------\n");

    // Sort lectures by time
    Lecture* sortedLectures[20];
    int lectureCount = 0;
    
    temp = head;
    while(temp) {
        if(temp->section == section && strcmp(temp->day, day) == 0) {
            sortedLectures[lectureCount++] = temp;
        }
        temp = temp->next;
    }
    
    // Simple bubble sort by time
    for(int i = 0; i < lectureCount-1; i++) {
        for(int j = 0; j < lectureCount-i-1; j++) {
            if(strcmp(sortedLectures[j]->time, sortedLectures[j+1]->time) > 0) {
                Lecture* temp = sortedLectures[j];
                sortedLectures[j] = sortedLectures[j+1];
                sortedLectures[j+1] = temp;
            }
        }
    }
    
    // Print sorted lectures
    for(int i = 0; i < lectureCount; i++) {
        const char* subjectName = getSubjectName(sortedLectures[i]->subject);
        char truncatedSubject[36] = "";
        char truncatedFaculty[16] = "";
        
        // Truncate subject if necessary
        strncpy(truncatedSubject, subjectName, 35);
        truncatedSubject[35] = '\0';
        
        // Truncate faculty if necessary
        strncpy(truncatedFaculty, sortedLectures[i]->faculty, 15);
        truncatedFaculty[15] = '\0';
        
        printf("%-13s | %-7s | %-35s | %-15s |\n", 
               sortedLectures[i]->time, 
               sortedLectures[i]->subject,
               truncatedSubject,
               truncatedFaculty);
    }
    printf("----------------------------------------------------------------\n");
}

// Function to get available time slots for a section
void getAvailableTimeSlots(char section, const char* day, int showAll) {
    Lecture* temp = head;
    int found = 0;
    
    printf("\nAvailable time slots for Section %c on %s:\n", section, day);
    printf("----------------------------------------\n");
    
    // First pass: collect all time slots
    char times[20][20];
    int timeCount = 0;
    while(temp) {
        if(temp->section == section && strcmp(temp->day, day) == 0) {
            strcpy(times[timeCount], temp->time);
            timeCount++;
        }
        temp = temp->next;
    }
    
    // Sort time slots
    for(int i = 0; i < timeCount-1; i++) {
        for(int j = 0; j < timeCount-i-1; j++) {
            if(strcmp(times[j], times[j+1]) > 0) {
                char temp[20];
                strcpy(temp, times[j]);
                strcpy(times[j], times[j+1]);
                strcpy(times[j+1], temp);
            }
        }
    }
    
    // Display time slots
    for(int i = 0; i < timeCount; i++) {
        temp = head;
        while(temp) {
            if(temp->section == section && strcmp(temp->day, day) == 0 && strcmp(temp->time, times[i]) == 0) {
                if(showAll || strcmp(temp->faculty, "-") == 0) {
                    printf("%d. %s - %s - %s\n", 
                           i+1, 
                           times[i], 
                           temp->subject,
                           strcmp(temp->faculty, "-") == 0 ? "Free" : temp->faculty);
                }
                break;
            }
            temp = temp->next;
        }
    }
    printf("----------------------------------------\n");
}

// Function to swap teachers between two sections
void swapTeachers(char section1, char section2) {
    if(section1 == section2) {
        printf("\nCannot swap teachers within the same section!\n");
        return;
    }
    
    // Show day options
    printf("\nSelect Day:\n");
    printf("1. Monday\n");
    printf("2. Tuesday\n");
    printf("3. Wednesday\n");
    printf("4. Thursday\n");
    printf("5. Friday\n");
    printf("6. Saturday\n");
    printf("Enter choice (1-6): ");
    
    int dayChoice;
    scanf("%d", &dayChoice);
    
    if(dayChoice < 1 || dayChoice > 6) {
        printf("Invalid day choice!\n");
        return;
    }
    
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char* fullDays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* selectedDay = days[dayChoice-1];
    
    // Show available time slots for both sections
    printf("\nTime slots for Section %c:\n", section1);
    getAvailableTimeSlots(section1, selectedDay, 1);
    
    printf("\nTime slots for Section %c:\n", section2);
    getAvailableTimeSlots(section2, selectedDay, 1);
    
    // Get time slot choice
    printf("\nEnter time slot number for Section %c: ", section1);
    int timeChoice1;
    scanf("%d", &timeChoice1);
    
    printf("Enter time slot number for Section %c: ", section2);
    int timeChoice2;
    scanf("%d", &timeChoice2);
    
    // Find the lectures
    Lecture* temp = head;
    Lecture* lec1 = NULL;
    Lecture* lec2 = NULL;
    char time1[20] = "", time2[20] = "";
    
    // Get time slots
    int count = 0;
    while(temp) {
        if(temp->section == section1 && strcmp(temp->day, selectedDay) == 0) {
            count++;
            if(count == timeChoice1) {
                strcpy(time1, temp->time);
            }
        }
        temp = temp->next;
    }
    
    count = 0;
    temp = head;
    while(temp) {
        if(temp->section == section2 && strcmp(temp->day, selectedDay) == 0) {
            count++;
            if(count == timeChoice2) {
                strcpy(time2, temp->time);
            }
        }
        temp = temp->next;
    }
    
    // Find lectures for swapping
    temp = head;
    while(temp) {
        if(strcmp(temp->day, selectedDay) == 0) {
            if(temp->section == section1 && strcmp(temp->time, time1) == 0) {
                lec1 = temp;
            }
            if(temp->section == section2 && strcmp(temp->time, time2) == 0) {
                lec2 = temp;
            }
        }
        temp = temp->next;
    }
    
    if(lec1 && lec2) {
        // Swap faculty members
        char tempFaculty[MAX_LEN];
        strcpy(tempFaculty, lec1->faculty);
        strcpy(lec1->faculty, lec2->faculty);
        strcpy(lec2->faculty, tempFaculty);
        
        printf("\nTeachers swapped successfully!\n");
        printf("Section %c (%s): %s - %s\n", section1, time1, lec1->subject, lec1->faculty);
        printf("Section %c (%s): %s - %s\n", section2, time2, lec2->subject, lec2->faculty);
    } else {
        printf("\nCould not find lectures for both sections at the specified times.\n");
    }
}

// Function to change class of a section
void changeClassSection(char oldSection, char newSection) {
    Lecture* temp = head;
    int found = 0;
    
    printf("\nEnter the day (MON/TUE/WED/THU/FRI/SAT): ");
    char day[10];
    scanf("%s", day);
    
    printf("Enter the time slot (e.g., 8:00-8:55): ");
    char time[20];
    scanf("%s", time);
    
    // Find the lecture to change
    while(temp) {
        if(temp->section == oldSection && strcmp(temp->day, day) == 0 && strcmp(temp->time, time) == 0) {
            temp->section = newSection;
            found = 1;
            printf("\nClass changed successfully!\n");
            printf("Changed from Section %c to Section %c\n", oldSection, newSection);
            printf("Details: %s - %s - %s\n", temp->subject, temp->faculty, temp->time);
            break;
        }
        temp = temp->next;
    }
    
    if(!found) {
        printf("\nNo lecture found for the specified details.\n");
    }
}

// Function to check if a string contains "Lab" or "Sec"
int isLabOrSection(const char* str) {
    return (strstr(str, "Lab") != NULL || strstr(str, "Sec") != NULL);
}

// Function to normalize teacher names
void normalizeTeacherName(char* name) {
    // Convert "Mr. Neeraj" and "Mr. Neeraj Panwar" to "Mr. Neeraj Panwar"
    if(strcmp(name, "Mr. Neeraj") == 0) {
        strcpy(name, "Mr. Neeraj Panwar");
    }
}

// Function to analyze teacher workload
void analyzeTeacherLoad() {
    TeacherLoad teachers[20];  // Assuming max 20 teachers
    int teacherCount = 0;
    const int MAX_LECTURES = 15;  // Threshold for overloaded teachers
    
    // Initialize teacher array
    for(int i = 0; i < 20; i++) {
        teachers[i].lectureCount = 0;
        teachers[i].sectionCount = 0;
        memset(teachers[i].sections, 0, sizeof(teachers[i].sections));
    }
    
    // First pass: collect all unique teachers (excluding labs)
    Lecture* temp = head;
    while(temp) {
        if(strcmp(temp->faculty, "-") != 0 && !isLabOrSection(temp->faculty)) {  // Skip free periods and labs
            char normalizedName[MAX_LEN];
            strcpy(normalizedName, temp->faculty);
            normalizeTeacherName(normalizedName);
            
            int found = 0;
            for(int i = 0; i < teacherCount; i++) {
                if(strcmp(teachers[i].name, normalizedName) == 0) {
                    found = 1;
                    break;
                }
            }
            if(!found) {
                strcpy(teachers[teacherCount].name, normalizedName);
                teacherCount++;
            }
        }
        temp = temp->next;
    }
    
    // Second pass: count lectures and collect sections (excluding labs)
    temp = head;
    while(temp) {
        if(strcmp(temp->faculty, "-") != 0 && !isLabOrSection(temp->faculty)) {  // Skip free periods and labs
            char normalizedName[MAX_LEN];
            strcpy(normalizedName, temp->faculty);
            normalizeTeacherName(normalizedName);
            
            for(int i = 0; i < teacherCount; i++) {
                if(strcmp(teachers[i].name, normalizedName) == 0) {
                    teachers[i].lectureCount++;
                    
                    // Add section if not already present
                    int sectionExists = 0;
                    for(int j = 0; j < teachers[i].sectionCount; j++) {
                        if(teachers[i].sections[j] == temp->section) {
                            sectionExists = 1;
                            break;
                        }
                    }
                    if(!sectionExists) {
                        teachers[i].sections[teachers[i].sectionCount++] = temp->section;
                    }
                    break;
                }
            }
        }
        temp = temp->next;
    }
    
    // Sort teachers by lecture count (descending)
    for(int i = 0; i < teacherCount-1; i++) {
        for(int j = 0; j < teacherCount-i-1; j++) {
            if(teachers[j].lectureCount < teachers[j+1].lectureCount) {
                TeacherLoad temp = teachers[j];
                teachers[j] = teachers[j+1];
                teachers[j+1] = temp;
            }
        }
    }
    
    // Display results
    printf("\n================================================================\n");
    printf("                    TEACHER WORKLOAD ANALYSIS                      \n");
    printf("================================================================\n\n");
    printf("%-25s | %-15s | %-15s | %s\n", "Teacher Name", "Lectures/Week", "Sections", "Status");
    printf("----------------------------------------------------------------\n");
    
    for(int i = 0; i < teacherCount; i++) {
        char sections[10] = "";
        // Sort sections alphabetically
        for(int j = 0; j < teachers[i].sectionCount-1; j++) {
            for(int k = 0; k < teachers[i].sectionCount-j-1; k++) {
                if(teachers[i].sections[k] > teachers[i].sections[k+1]) {
                    char temp = teachers[i].sections[k];
                    teachers[i].sections[k] = teachers[i].sections[k+1];
                    teachers[i].sections[k+1] = temp;
                }
            }
        }
        
        for(int j = 0; j < teachers[i].sectionCount; j++) {
            char temp[3];
            sprintf(temp, "%c ", teachers[i].sections[j]);
            strcat(sections, temp);
        }
        
        printf("%-25s | %-15d | %-15s | %s\n",
               teachers[i].name,
               teachers[i].lectureCount,
               sections,
               teachers[i].lectureCount > MAX_LECTURES ? "OVERLOADED" : "Normal");
    }
    printf("----------------------------------------------------------------\n");
    printf("Note: Teachers with more than %d lectures per week are marked as OVERLOADED\n", MAX_LECTURES);
    printf("================================================================\n");
}

// Function to check if time slot is available
int isTimeSlotAvailable(const char* day, const char* time, char section) {
    Lecture* temp = head;
    while(temp) {
        if(temp->section == section && 
           strcmp(temp->day, day) == 0 && 
           strcmp(temp->time, time) == 0) {
            return 0; // Time slot is occupied
        }
        temp = temp->next;
    }
    return 1; // Time slot is available
}

// Function to get time string from choice
void getTimeString(int choice, char* time) {
    switch(choice) {
        case 1: strcpy(time, "8:00-8:55"); break;
        case 2: strcpy(time, "8:55-9:50"); break;
        case 3: strcpy(time, "10:10-11:05"); break;
        case 4: strcpy(time, "11:05-12:00"); break;
        case 5: strcpy(time, "12:00-12:55"); break;
        case 6: strcpy(time, "2:10-3:05"); break;
        case 7: strcpy(time, "3:05-4:00"); break;
        case 8: strcpy(time, "4:00-4:55"); break;
        case 9: strcpy(time, "4:55-5:50"); break;
    }
}

// Function to get unique subjects from timetable
void getUniqueSubjects(SubjectInfo subjects[], int* count) {
    *count = 0;
    
    // Copy from curriculum
    for(int i = 0; i < CURRICULUM_SIZE; i++) {
        strcpy(subjects[*count].code, curriculum[i].code);
        strcpy(subjects[*count].name, curriculum[i].name);
        subjects[*count].isLab = curriculum[i].isLab;
        (*count)++;
    }
}

// Function to find existing time slots for a subject
void findExistingTimeSlots(const char* subject, TimeSlot slots[], int* count) {
    *count = 0;
    Lecture* temp = head;
    
    while(temp) {
        if(strcmp(temp->subject, subject) == 0) {
            strcpy(slots[*count].day, temp->day);
            strcpy(slots[*count].time, temp->time);
            slots[*count].section = temp->section;
            (*count)++;
        }
        temp = temp->next;
    }
}

// Function to assign new teacher
void assignNewTeacher() {
    char teacherName[MAX_LEN];
    char subject[MAX_LEN];
    char time[MAX_LEN];
    char day[4];
    int dayChoice;
    int timeChoice;
    int subjectChoice;
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    
    printf("\nEnter new teacher name: ");
    scanf(" %[^\n]s", teacherName);
    
    // Get list of existing subjects
    SubjectInfo subjects[50];
    int subjectCount = 0;
    getUniqueSubjects(subjects, &subjectCount);
    
    printf("\nDo you want to:\n");
    printf("1. Assign to an existing subject\n");
    printf("2. Create a new subject\n");
    printf("Enter choice (1-2): ");
    scanf("%d", &subjectChoice);
    
    if(subjectChoice == 1) {
        if(subjectCount == 0) {
            printf("\nNo existing subjects found. Please create a new subject.\n");
            printf("Enter new subject code: ");
            scanf(" %[^\n]s", subject);
        } else {
            printf("\nExisting subjects:\n");
            printf("%-8s | %-40s | %s\n", "Code", "Subject Name", "Type");
            printf("----------------------------------------------------------\n");
            for(int i = 0; i < subjectCount; i++) {
                printf("%d. %-6s | %-40s | %s\n", 
                       i + 1,
                       subjects[i].code,
                       subjects[i].name,
                       subjects[i].isLab ? "Lab" : "Theory");
            }
            
            int choice;
            do {
                printf("\nEnter subject number (1-%d): ", subjectCount);
                scanf("%d", &choice);
                if(choice < 1 || choice > subjectCount) {
                    printf("Invalid choice. Please try again.\n");
                }
            } while(choice < 1 || choice > subjectCount);
            
            strcpy(subject, subjects[choice - 1].code);
            printf("\nSelected: %s - %s\n", subjects[choice - 1].code, subjects[choice - 1].name);
            
            // Find existing time slots for this subject
            TimeSlot existingSlots[20];
            int slotCount = 0;
            
            findExistingTimeSlots(subject, existingSlots, &slotCount);
            
            if(slotCount > 0) {
                printf("\nFound existing time slots for %s:\n", subject);
                for(int i = 0; i < slotCount; i++) {
                    printf("Section %c: %s %s\n", 
                           existingSlots[i].section,
                           existingSlots[i].day,
                           existingSlots[i].time);
                }
                
                printf("\nAssigning %s to these time slots...\n", teacherName);
                
                // Update all existing slots with the new teacher
                Lecture* temp = head;
                while(temp) {
                    if(strcmp(temp->subject, subject) == 0) {
                        strcpy(temp->faculty, teacherName);
                    }
                    temp = temp->next;
                }
                
                printf("\nTeacher assigned successfully!\n");
                return;
            } else {
                printf("\nNo existing time slots found for this subject.\n");
                printf("Proceeding with manual time slot assignment...\n");
            }
        }
    } else if(subjectChoice == 2) {
        char subjectName[MAX_LEN];
        printf("Enter new subject code (max 8 chars): ");
        scanf(" %[^\n]s", subject);
        printf("Enter subject name (max 40 chars): ");
        scanf(" %[^\n]s", subjectName);
        printf("Is this a lab subject? (1 for Yes, 0 for No): ");
        int isLab;
        scanf("%d", &isLab);
    } else {
        printf("Invalid choice. Operation cancelled.\n");
        return;
    }
    
    // Only proceed with manual time slot assignment for new subjects
    // or existing subjects with no time slots
    
    // For each section
    char sections[] = {'A', 'B', 'C', 'D'};
    for(int i = 0; i < 4; i++) {
        char currentSection = sections[i];
        int validTimeSlot = 0;
        
        while(!validTimeSlot) {
            // Select day for current section
            printf("\nSection %c - Select Day:\n", currentSection);
            printf("1. Monday\n");
            printf("2. Tuesday\n");
            printf("3. Wednesday\n");
            printf("4. Thursday\n");
            printf("5. Friday\n");
            printf("6. Saturday\n");
            printf("Enter choice (1-6): ");
            scanf("%d", &dayChoice);
            
            if(dayChoice < 1 || dayChoice > 6) {
                printf("Invalid day choice! Please try again.\n");
                continue;
            }
            
            strcpy(day, days[dayChoice-1]);
            
            // Show available time slots and mark occupied ones
            printf("\nAvailable Time Slots for Section %c on %s:\n", currentSection, days[dayChoice-1]);
            for(int j = 1; j <= 9; j++) {
                char tempTime[MAX_LEN];
                getTimeString(j, tempTime);
                printf("%d. %s %s\n", j, tempTime, 
                       isTimeSlotAvailable(day, tempTime, currentSection) ? "(Available)" : "(Occupied)");
            }
            
            printf("\nEnter time slot number (1-9): ");
            scanf("%d", &timeChoice);
            
            if(timeChoice < 1 || timeChoice > 9) {
                printf("Invalid time choice! Please try again.\n");
                continue;
            }
            
            getTimeString(timeChoice, time);
            
            // Check if time slot is available
            if(!isTimeSlotAvailable(day, time, currentSection)) {
                printf("\nError: Time slot %s on %s is already occupied for Section %c!\n", 
                       time, day, currentSection);
                printf("Please choose a different time slot.\n");
                continue;
            }
            
            // Time slot is valid and available
            validTimeSlot = 1;
            insertLecture(day, time, subject, teacherName, currentSection);
            printf("\nLecture added for Section %c: %s at %s on %s\n", 
                   currentSection, subject, time, day);
        }
    }
    
    printf("\nNew teacher assigned successfully!\n");
    printf("You can use the View Timetable option to check the updated schedule.\n");
}

// Function to get subject name from code
const char* getSubjectName(const char* subjectCode) {
    static char subjectName[MAX_LEN];  // Make it static to persist after function returns
    
    // First check curriculum array
    for(int i = 0; i < CURRICULUM_SIZE; i++) {
        if(strcmp(curriculum[i].code, subjectCode) == 0) {
            strcpy(subjectName, curriculum[i].name);
            return subjectName;
        }
    }
    
    // If not found in curriculum, return the code itself
    strcpy(subjectName, subjectCode);
    return subjectName;
}

// Function to save timetable to file
void saveTimetable() {
    FILE *fp;
    char filename[100];
    
    // Create filename with current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "timetable_%Y%m%d_%H%M%S.txt", t);
    
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("\nError: Could not create file %s!\n", filename);
        return;
    }
    
    // Write header
    fprintf(fp, "================================================================\n");
    fprintf(fp, "                        TIMETABLE DATA                            \n");
    fprintf(fp, "                  Saved on: %s", ctime(&now));
    fprintf(fp, "================================================================\n\n");
    
    // Write timetable data for each section
    char sections[] = {'A', 'B', 'C', 'D'};
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    
    for(int s = 0; s < 4; s++) {
        fprintf(fp, "\nSECTION %c TIMETABLE\n", sections[s]);
        fprintf(fp, "----------------------------------------------------------------\n");
        fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n", 
               "Day", "Time", "Code", "Subject Name", "Faculty");
        fprintf(fp, "----------------------------------------------------------------\n");
        
        // For each day
        for(int d = 0; d < 6; d++) {
            Lecture* temp = head;
            int hasLecture = 0;
            
            while(temp) {
                if(temp->section == sections[s] && strcmp(temp->day, days[d]) == 0) {
                    const char* subjectName = getSubjectName(temp->subject);
                    fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n",
                           temp->day,
                           temp->time,
                           temp->subject,
                           subjectName,
                           temp->faculty);
                    hasLecture = 1;
                }
                temp = temp->next;
            }
            
            if(hasLecture) {
                fprintf(fp, "----------------------------------------------------------------\n");
            }
        }
    }
    
    // Write subject legend
    fprintf(fp, "\n\nSUBJECT LEGEND:\n");
    fprintf(fp, "----------------------------------------------------------------\n");
    fprintf(fp, "%-8s | %-40s | %s\n", "Code", "Subject Name", "Type");
    fprintf(fp, "----------------------------------------------------------------\n");
    
    SubjectInfo subjects[50];
    int subjectCount = 0;
    getUniqueSubjects(subjects, &subjectCount);
    
    for(int i = 0; i < subjectCount; i++) {
        fprintf(fp, "%-8s | %-40s | %s\n",
               subjects[i].code,
               subjects[i].name,
               subjects[i].isLab ? "Lab" : "Theory");
    }
    fprintf(fp, "----------------------------------------------------------------\n");
    
    fprintf(fp, "\n================================================================\n");
    fprintf(fp, "                          END OF DATA                             \n");
    fprintf(fp, "================================================================\n");
    
    fclose(fp);
    printf("\nTimetable saved successfully to file: %s\n", filename);
}

// Function to trim whitespace from a string
void trim(char* str) {
    char* end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;  // All spaces?
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    // Write new null terminator character
    end[1] = '\0';
}

// Function to load timetable from file
void loadTimetable(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("\nError: Could not open file %s!\n", filename);
        return;
    }
    
    // Clear existing timetable
    while(head != NULL) {
        Lecture* temp = head;
        head = head->next;
        free(temp);
    }
    
    char line[256];
    char day[10], time[20], subject[50], faculty[50];
    char section = '\0';
    
    // Skip header until we find a section
    while(fgets(line, sizeof(line), fp)) {
        if(strstr(line, "SECTION") != NULL) {
            section = line[8];  // Get section letter
            
            // Skip header lines
            fgets(line, sizeof(line), fp);  // separator
            fgets(line, sizeof(line), fp);  // column headers
            fgets(line, sizeof(line), fp);  // separator
            
            // Read lectures
            while(fgets(line, sizeof(line), fp) && strlen(line) > 2) {
                // Skip separator lines
                if(strstr(line, "---") != NULL || strstr(line, "SECTION") != NULL) {
                    break;
                }
                
                // Parse line
                if(sscanf(line, "%s | %[^|] | %[^|] | %[^\n]", day, time, subject, faculty) == 4) {
                    // Trim whitespace from each field
                    trim(time);
                    trim(subject);
                    trim(faculty);
                    
                    // Insert lecture
                    insertLecture(day, time, subject, faculty, section);
                }
            }
        }
    }
    
    fclose(fp);
    printf("\nTimetable loaded successfully from file: %s\n", filename);
}

// Print full timetable for all sections
void displayFullTimetable() {
    char sections[] = {'A', 'B', 'C', 'D'};
    for (int i = 0; i < 4; i++) {
        printf(COLOR_HEADER "\n==============================\n" COLOR_RESET);
        printf(COLOR_HEADER "      SECTION %c TIMETABLE      \n" COLOR_RESET, sections[i]);
        printf(COLOR_HEADER "==============================\n" COLOR_RESET);
        printTimetable(sections[i]);
    }
}

// Save timetable for a single section
void displaySaveSectionTimetable(char section) {
    FILE *fp;
    char filename[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(filename, sizeof(filename), "section_%c_timetable_%04d%02d%02d_%02d%02d%02d.txt", section, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf(COLOR_ERROR "\nError: Could not create file %s!\n" COLOR_RESET, filename);
        return;
    }
    fprintf(fp, "==============================\n");
    fprintf(fp, "  SECTION %c TIMETABLE\n", section);
    fprintf(fp, "==============================\n");
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    for(int d = 0; d < 6; d++) {
        Lecture* temp = head;
        int hasLecture = 0;
        while(temp) {
            if(temp->section == section && strcmp(temp->day, days[d]) == 0) {
                const char* subjectName = getSubjectName(temp->subject);
                fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n",
                        temp->day, temp->time, temp->subject, subjectName, temp->faculty);
                hasLecture = 1;
            }
            temp = temp->next;
        }
        if(hasLecture) {
            fprintf(fp, "----------------------------------------------\n");
        }
    }
    fclose(fp);
    printf(COLOR_SUCCESS "\nSection %c timetable saved to file: %s\n" COLOR_RESET, section, filename);
}

// Swap classes (between or within section, with/without teacher)
void swapClasses() {
    char section1, section2;
    printf(CLI_COLOR_MENU "\nSwap Classes Menu\n" COLOR_RESET);
    printf("1. Swap between two sections\n");
    printf("2. Swap within the same section\n");
    printf("Enter choice: ");
    int swapChoice;
    scanf("%d", &swapChoice);
    if (swapChoice == 1) {
        printf("Enter first section (A/B/C/D): ");
        scanf(" %c", &section1);
        printf("Enter second section (A/B/C/D): ");
        scanf(" %c", &section2);
        if ((section1 >= 'A' && section1 <= 'D') && (section2 >= 'A' && section2 <= 'D') && section1 != section2) {
            printf("Do you want to swap teachers as well? (1-Yes, 0-No): ");
            int swapTeacher;
            scanf("%d", &swapTeacher);
            swapTeachersFlexible(section1, section2, swapTeacher);
        } else {
            printf(COLOR_ERROR "Invalid section(s)!\n" COLOR_RESET);
        }
    } else if (swapChoice == 2) {
        printf("Enter section (A/B/C/D): ");
        scanf(" %c", &section1);
        if (section1 >= 'A' && section1 <= 'D') {
            printf("Select Day:\n1. Monday\n2. Tuesday\n3. Wednesday\n4. Thursday\n5. Friday\n6. Saturday\nEnter choice (1-6): ");
            int dayChoice;
            scanf("%d", &dayChoice);
            if(dayChoice < 1 || dayChoice > 6) {
                printf(COLOR_ERROR "Invalid day choice!\n" COLOR_RESET);
                return;
            }
            const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
            const char* selectedDay = days[dayChoice-1];
            getAvailableTimeSlots(section1, selectedDay, 1);
            printf("Enter first time slot number: ");
            int slot1; scanf("%d", &slot1);
            printf("Enter second time slot number: ");
            int slot2; scanf("%d", &slot2);
            printf("Do you want to swap teachers as well? (1-Yes, 0-No): ");
            int swapTeacher; scanf("%d", &swapTeacher);
            swapWithinSection(section1, selectedDay, slot1, slot2, swapTeacher);
        } else {
            printf(COLOR_ERROR "Invalid section!\n" COLOR_RESET);
        }
    } else {
        printf(COLOR_ERROR "Invalid choice!\n" COLOR_RESET);
    }
}

// Flexible swap between sections, with/without teacher
void swapTeachersFlexible(char section1, char section2, int swapTeacher) {
    char day1[10], time1[20], subject1[MAX_LEN], teacher1[MAX_LEN];
    char day2[10], time2[20], subject2[MAX_LEN], teacher2[MAX_LEN];
    printf("\nEnter details for the first class to swap:\n");
    printf("Section (A/B/C/D): "); scanf(" %c", &section1);
    printf("Day (MON/TUE/WED/THU/FRI/SAT): "); scanf("%s", day1);
    printf("Time (e.g., 8:00-8:55): "); scanf("%s", time1);
    printf("Subject code: "); scanf("%s", subject1);
    printf("Teacher name: "); getchar(); fgets(teacher1, sizeof(teacher1), stdin); size_t len1 = strlen(teacher1); if(len1 > 0 && teacher1[len1-1] == '\n') teacher1[len1-1] = '\0';

    printf("\nEnter details for the second class to swap with:\n");
    printf("Section (A/B/C/D): "); scanf(" %c", &section2);
    printf("Day (MON/TUE/WED/THU/FRI/SAT): "); scanf("%s", day2);
    printf("Time (e.g., 8:00-8:55): "); scanf("%s", time2);
    printf("Subject code: "); scanf("%s", subject2);
    printf("Teacher name: "); getchar(); fgets(teacher2, sizeof(teacher2), stdin); size_t len2 = strlen(teacher2); if(len2 > 0 && teacher2[len2-1] == '\n') teacher2[len2-1] = '\0';

    Lecture *lec1 = NULL, *lec2 = NULL, *temp = head;
    while(temp) {
        if(temp->section == section1 && strcmp(temp->day, day1) == 0 && strcmp(temp->time, time1) == 0 && strcmp(temp->subject, subject1) == 0 && strcmp(temp->faculty, teacher1) == 0) {
            lec1 = temp;
        }
        if(temp->section == section2 && strcmp(temp->day, day2) == 0 && strcmp(temp->time, time2) == 0 && strcmp(temp->subject, subject2) == 0 && strcmp(temp->faculty, teacher2) == 0) {
            lec2 = temp;
        }
        temp = temp->next;
    }
    if(lec1 && lec2) {
        char tmpSubject[MAX_LEN], tmpFaculty[MAX_LEN];
        strcpy(tmpSubject, lec1->subject);
        if(swapTeacher) strcpy(tmpFaculty, lec1->faculty);
        strcpy(lec1->subject, lec2->subject);
        if(swapTeacher) strcpy(lec1->faculty, lec2->faculty);
        strcpy(lec2->subject, tmpSubject);
        if(swapTeacher) strcpy(lec2->faculty, tmpFaculty);
        printf(COLOR_SUCCESS "\nClasses swapped successfully!\n" COLOR_RESET);
        saveTimetable();
        char notify;
        printf("Do you want to notify the teacher about the change? (y/n): ");
        scanf(" %c", &notify);
        if(notify == 'y' || notify == 'Y') {
            char number[32];
            printf("Enter WhatsApp number (with country code, e.g., 919999999999): ");
            scanf("%s", number);
            char msg[512], encoded[1024], command[1200];
            snprintf(msg, sizeof(msg), "Class is swapped: %s (%s) <-> %s (%s)", lec1->faculty, lec1->subject, lec2->faculty, lec2->subject);
            url_encode(msg, encoded, sizeof(encoded));
            snprintf(command, sizeof(command), "start \"\" \"https://wa.me/%s?text=%s\"", number, encoded);
            printf(COLOR_SUCCESS "\nOpening WhatsApp Web to notify the teacher...\n" COLOR_RESET);
            system(command);
        }
    } else {
        printf(COLOR_ERROR "\nCould not find both classes for swapping.\n" COLOR_RESET);
    }
}

// Swap within the same section
void swapWithinSection(char section, const char* day, int slot1, int slot2, int swapTeacher) {
    // Find lectures for the two slots
    Lecture* temp = head;
    Lecture* lec1 = NULL; Lecture* lec2 = NULL;
    int count = 0;
    char time1[20] = "", time2[20] = "";
    // Get time strings for slots
    int idx = 0;
    temp = head;
    while(temp) {
        if(temp->section == section && strcmp(temp->day, day) == 0) {
            idx++;
            if(idx == slot1) strcpy(time1, temp->time);
            if(idx == slot2) strcpy(time2, temp->time);
        }
        temp = temp->next;
    }
    temp = head;
    while(temp) {
        if(temp->section == section && strcmp(temp->day, day) == 0) {
            if(strcmp(temp->time, time1) == 0) lec1 = temp;
            if(strcmp(temp->time, time2) == 0) lec2 = temp;
        }
        temp = temp->next;
    }
    if(lec1 && lec2) {
        char tmpSubject[MAX_LEN], tmpFaculty[MAX_LEN];
        strcpy(tmpSubject, lec1->subject);
        if(swapTeacher) strcpy(tmpFaculty, lec1->faculty);
        strcpy(lec1->subject, lec2->subject);
        if(swapTeacher) strcpy(lec1->faculty, lec2->faculty);
        strcpy(lec2->subject, tmpSubject);
        if(swapTeacher) strcpy(lec2->faculty, tmpFaculty);
        printf(COLOR_SUCCESS "\nClasses swapped successfully!\n" COLOR_RESET);
    } else {
        printf(COLOR_ERROR "\nCould not find lectures for the specified slots.\n" COLOR_RESET);
    }
}

// Function to URL-encode a string (for WhatsApp sharing)
void url_encode(const char* src, char* dest, int max_len) {
    int j = 0;
    for (int i = 0; src[i] != '\0' && j < max_len - 1; i++) {
        if ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z') || (src[i] >= '0' && src[i] <= '9')) {
            dest[j++] = src[i];
        } else if (src[i] == ' ') {
            dest[j++] = '%';
            dest[j++] = '2';
            dest[j++] = '0';
        } else {
            j += sprintf(dest + j, "%%%02X", (unsigned char)src[i]);
        }
        if (j >= max_len - 4) break; // leave space for null terminator
    }
    dest[j] = '\0';
}

// Share feature: open WhatsApp Web with a message
void shareViaWhatsApp() {
    int shareChoice;
    char section;
    char filename[128];
    printf(COLOR_HILITE "\nShare Timetable Options:\n" COLOR_RESET);
    printf("1. Share Full Timetable (All Sections)\n");
    printf("2. Share Section Timetable\n");
    printf(COLOR_INPUT "Enter choice: " COLOR_RESET);
    scanf("%d", &shareChoice);
    if (shareChoice == 1) {
        // Save full timetable
        saveTimetable();
        // Find the latest file generated by saveTimetable (by timestamped name)
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(filename, sizeof(filename), "timetable_%Y%m%d_%H%M%S.txt", t);
    } else if (shareChoice == 2) {
        printf(COLOR_INPUT "Enter section (A/B/C/D): " COLOR_RESET);
        scanf(" %c", &section);
        if (section >= 'A' && section <= 'D') {
            // Save section timetable
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            snprintf(filename, sizeof(filename), "section_%c_timetable_%04d%02d%02d_%02d%02d%02d.txt", section, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            displaySaveSectionTimetable(section);
        } else {
            printf(COLOR_ERROR "Invalid section!\n" COLOR_RESET);
            return;
        }
    } else {
        printf(COLOR_ERROR "Invalid share choice!\n" COLOR_RESET);
        return;
    }
    
    // Create a more professional message
    char message[1024];
    if (shareChoice == 1) {
        snprintf(message, sizeof(message), 
            "📅 *MCA Timetable - All Sections*\n\n"
            "Hello everyone! Here's the complete timetable for all sections.\n\n"
            "📁 File: %s\n\n"
            "Please download and share with your classmates! 📚✨\n\n"
            "Note: You'll need to attach the file manually in WhatsApp Web.",
            filename);
    } else {
        snprintf(message, sizeof(message), 
            "📅 *MCA Timetable - Section %c*\n\n"
            "Hello Section %c! Here's your updated timetable.\n\n"
            "📁 File: %s\n\n"
            "Please download and share with your section mates! 📚✨\n\n"
            "Note: You'll need to attach the file manually in WhatsApp Web.",
            section, section, filename);
    }
    
    char encoded[2048];
    url_encode(message, encoded, sizeof(encoded));
    char command[3200];
    snprintf(command, sizeof(command), "start \"\" \"https://wa.me/?text=%s\"", encoded);
    printf(COLOR_SUCCESS "\nOpening WhatsApp Web in your browser...\n" COLOR_RESET);
    printf(COLOR_HILITE "The message is pre-filled. You can now select the MCA group or any contact to send it to.\n" COLOR_RESET);
    printf(COLOR_HILITE "Please attach the file '%s' manually in WhatsApp Web.\n" COLOR_RESET, filename);
    system(command);
    
    // Also provide the group invite link
    printf(COLOR_HILITE "\nMCA Group Link: https://chat.whatsapp.com/KJZLZnetrNW4rDr4N8eKYi\n" COLOR_RESET);
}

// Helper: case-insensitive substring search (portable strcasestr)
char* strcasestr_portable(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++; n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

void searchTimetable() {
    int searchType;
    char query[100];
    printf("\n1. Search by Teacher\n2. Search by Subject\n3. Search by Time\n");
    printf("Enter choice: ");
    scanf("%d", &searchType);
    printf("Enter search keyword: ");
    getchar(); // clear newline after scanf
    fgets(query, sizeof(query), stdin);
    size_t len = strlen(query);
    if (len > 0 && query[len-1] == '\n') query[len-1] = '\0';
    Lecture* temp = head;
    int found = 0;
    printf("\nSearch Results:\n");
    while(temp) {
        if ((searchType == 1 && strcasestr_portable(temp->faculty, query)) ||
            (searchType == 2 && (strcasestr_portable(temp->subject, query) || strcasestr_portable(getSubjectName(temp->subject), query))) ||
            (searchType == 3 && strcasestr_portable(temp->time, query))) {
            printf("Section %c | %s | %s | %s | %s\n", temp->section, temp->day, temp->time, temp->subject, temp->faculty);
            found = 1;
        }
        temp = temp->next;
    }
    if (!found) printf("No matches found.\n");
}

// Email Timetable (open mailto: link)
void emailTimetable() {
    int emailChoice;
    char section;
    char filename[128];
    printf("\n1. Email Full Timetable\n2. Email Section Timetable\n");
    printf("Enter choice: ");
    scanf("%d", &emailChoice);
    if (emailChoice == 1) {
        saveTimetable();
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(filename, sizeof(filename), "timetable_%Y%m%d_%H%M%S.txt", t);
    } else if (emailChoice == 2) {
        printf("Enter section (A/B/C/D): ");
        scanf(" %c", &section);
        if (section >= 'A' && section <= 'D') {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            snprintf(filename, sizeof(filename), "section_%c_timetable_%04d%02d%02d_%02d%02d%02d.txt", section, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            displaySaveSectionTimetable(section);
        } else {
            printf("Invalid section!\n");
            return;
        }
    } else {
        printf("Invalid choice!\n");
        return;
    }
    char gmail_url[1024];
    snprintf(gmail_url, sizeof(gmail_url),
        "start \"\" \"https://mail.google.com/mail/?view=cm&fs=1&su=Timetable&body=Please find the timetable attached: %s. Please attach the file manually.\"",
        filename);
    printf("\nOpening Gmail compose window in your browser...\nPlease attach the file '%s' manually.\n", filename);
    system(gmail_url);
}

// Notification/Reminder (demo: immediate notification)
void setNotification() {
    char msg[128];
    int delayMin;
    printf("Enter reminder message: ");
    getchar();
    fgets(msg, sizeof(msg), stdin);
    size_t len = strlen(msg);
    if (len > 0 && msg[len-1] == '\n') msg[len-1] = '\0';
    printf("In how many minutes do you want to see this notification? ");
    scanf("%d", &delayMin);
    printf("Waiting %d minute(s)...\n", delayMin);
#ifdef _WIN32
    Sleep(delayMin * 60 * 1000); // Sleep takes milliseconds
#else
    sleep(delayMin * 60); // POSIX fallback
#endif
    char command[512];
    snprintf(command, sizeof(command),
        "powershell -Command \"Add-Type -AssemblyName PresentationFramework; [System.Windows.MessageBox]::Show('%s')\"",
        msg);
    system(command);
}

// Statistics Dashboard
void statisticsDashboard() {
    // Most loaded teacher
    TeacherLoad teachers[20];
    int teacherCount = 0;
    for(int i = 0; i < 20; i++) {
        teachers[i].lectureCount = 0;
        teachers[i].sectionCount = 0;
        memset(teachers[i].sections, 0, sizeof(teachers[i].sections));
    }
    Lecture* temp = head;
    while(temp) {
        if(strcmp(temp->faculty, "-") != 0 && !strstr(temp->faculty, "Lab") && !strstr(temp->faculty, "Sec")) {
            int found = 0;
            for(int i = 0; i < teacherCount; i++) {
                if(strcmp(teachers[i].name, temp->faculty) == 0) {
                    found = 1;
                    break;
                }
            }
            if(!found) {
                strcpy(teachers[teacherCount].name, temp->faculty);
                teacherCount++;
            }
        }
        temp = temp->next;
    }
    temp = head;
    while(temp) {
        if(strcmp(temp->faculty, "-") != 0 && !strstr(temp->faculty, "Lab") && !strstr(temp->faculty, "Sec")) {
            for(int i = 0; i < teacherCount; i++) {
                if(strcmp(teachers[i].name, temp->faculty) == 0) {
                    teachers[i].lectureCount++;
                    int sectionExists = 0;
                    for(int j = 0; j < teachers[i].sectionCount; j++) {
                        if(teachers[i].sections[j] == temp->section) {
                            sectionExists = 1;
                            break;
                        }
                    }
                    if(!sectionExists) {
                        teachers[i].sections[teachers[i].sectionCount++] = temp->section;
                    }
                    break;
                }
            }
        }
        temp = temp->next;
    }
    int maxLectures = 0, maxIdx = -1;
    for(int i = 0; i < teacherCount; i++) {
        if(teachers[i].lectureCount > maxLectures) {
            maxLectures = teachers[i].lectureCount;
            maxIdx = i;
        }
    }
    printf("\nMost loaded teacher: %s (%d lectures)\n", maxIdx >= 0 ? teachers[maxIdx].name : "N/A", maxLectures);
    // Busiest day for each section
    char sections[] = {'A', 'B', 'C', 'D'};
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    for(int s = 0; s < 4; s++) {
        int maxDay = 0, maxLect = 0;
        for(int d = 0; d < 6; d++) {
            int count = 0;
            temp = head;
            while(temp) {
                if(temp->section == sections[s] && strcmp(temp->day, days[d]) == 0) count++;
                temp = temp->next;
            }
            if(count > maxLect) { maxLect = count; maxDay = d; }
        }
        printf("Busiest day for Section %c: %s (%d lectures)\n", sections[s], days[maxDay], maxLect);
    }
    // Free periods per section
    for(int s = 0; s < 4; s++) {
        int freeCount = 0;
        temp = head;
        while(temp) {
            if(temp->section == sections[s] && strcmp(temp->faculty, "-") == 0) freeCount++;
            temp = temp->next;
        }
        printf("Free periods for Section %c: %d\n", sections[s], freeCount);
    }
}

void shareTimetableViaQR() {
    // 1. Save timetable to file as 'timetable.txt'
    char filename[32] = "timetable.txt";
    saveTimetableAs(filename);
    // 1.5. Get absolute path and check if file exists
    char absPath[260];
#ifdef _WIN32
    if (_fullpath(absPath, filename, sizeof(absPath)) != NULL) {
        printf("Timetable file absolute path: %s\n", absPath);
    }
#else
    realpath(filename, absPath);
    printf("Timetable file absolute path: %s\n", absPath);
#endif
    FILE *test = fopen(absPath, "r");
    if (!test) {
        printf(COLOR_ERROR "File %s does not exist before upload!\n" COLOR_RESET, absPath);
        return;
    }
    fclose(test);
    // 2. Upload to catbox.moe and extract the link
    char curlCmd[512];
    char linkFile[64] = "catbox_link.txt";
    snprintf(curlCmd, sizeof(curlCmd), "curl.exe -s -F \"reqtype=fileupload\" -F \"fileToUpload=@%s\" https://catbox.moe/user/api.php > %s", absPath, linkFile);
    printf("Running: %s\n", curlCmd);
    system(curlCmd);
    // 3. Read the link from the output file
    FILE *fp = fopen(linkFile, "r");
    if (!fp) {
        printf(COLOR_ERROR "Could not open catbox.moe response file.\n" COLOR_RESET);
        return;
    }
    char link[512] = "";
    if (!fgets(link, sizeof(link), fp)) {
        printf(COLOR_ERROR "Failed to read catbox.moe response.\n" COLOR_RESET);
        fclose(fp);
        return;
    }
    fclose(fp);
    // Remove trailing newline if present
    size_t linkLen = strlen(link);
    if (linkLen > 0 && link[linkLen-1] == '\n') link[linkLen-1] = '\0';
    if (link[0] == '\0') {
        printf(COLOR_ERROR "Failed to extract catbox.moe link.\n" COLOR_RESET);
        return;
    }
    
    // 4. Ask user what they want to do with the link
    printf(COLOR_SUCCESS "\nTimetable uploaded successfully!\n" COLOR_RESET);
    printf(COLOR_HILITE "Generated link: %s\n" COLOR_RESET, link);
    printf("\nWhat would you like to do?\n");
    printf("1. Open QR code in browser\n");
    printf("2. Share link to WhatsApp group\n");
    printf("3. Both\n");
    printf(COLOR_INPUT "Enter choice (1-3): " COLOR_RESET);
    
    int shareChoice;
    scanf("%d", &shareChoice);
    
    if (shareChoice == 1 || shareChoice == 3) {
        // 4a. Open QR code in browser using api.qrserver.com
        char qrUrl[1024];
        snprintf(qrUrl, sizeof(qrUrl), "start \"\" \"https://api.qrserver.com/v1/create-qr-code/?data=%s&size=200x200\"", link);
        printf(COLOR_SUCCESS "\nOpening QR code in your browser. Scan it to download the timetable.\n" COLOR_RESET);
        system(qrUrl);
    }
    
    if (shareChoice == 2 || shareChoice == 3) {
        // 4b. Share link to WhatsApp group
        shareLinkToWhatsAppGroup(link);
    }
    
    // Optionally, clean up
    remove(linkFile);
}

// Function to share link to WhatsApp group
void shareLinkToWhatsAppGroup(const char* link) {
    // WhatsApp group invite link for MCA UNOFFICIAL ALL SECTION 2024
    const char* groupInviteLink = "https://chat.whatsapp.com/KJZLZnetrNW4rDr4N8eKYi";
    
    // Create message with the timetable link
    char message[1024];
    snprintf(message, sizeof(message), 
        "📅 *MCA Timetable Update*\n\n"
        "Hello everyone! Here's the updated timetable:\n"
        "%s\n\n"
        "You can download the complete timetable file from this link. "
        "Please share this with your classmates! 📚✨",
        link);
    
    // URL encode the message
    char encodedMessage[2048];
    url_encode(message, encodedMessage, sizeof(encodedMessage));
    
    // Create WhatsApp Web URL with pre-filled message
    char whatsappUrl[3072];
    snprintf(whatsappUrl, sizeof(whatsappUrl), 
        "https://wa.me/?text=%s", encodedMessage);
    
    // Open WhatsApp Web in browser
    char command[3200];
    snprintf(command, sizeof(command), "start \"\" \"%s\"", whatsappUrl);
    
    printf(COLOR_SUCCESS "\nOpening WhatsApp Web in your browser...\n" COLOR_RESET);
    printf(COLOR_HILITE "The message is pre-filled with the timetable link.\n" COLOR_RESET);
    printf(COLOR_HILITE "You can now select the group or contact to send it to.\n" COLOR_RESET);
    
    system(command);
    
    // Also provide the group invite link as an alternative
    printf(COLOR_HILITE "\nAlternative: Join the MCA group directly:\n" COLOR_RESET);
    printf(COLOR_HILITE "Group Link: %s\n" COLOR_RESET, groupInviteLink);
    
    char groupCommand[512];
    snprintf(groupCommand, sizeof(groupCommand), "start \"\" \"%s\"", groupInviteLink);
    printf(COLOR_HILITE "Opening group invite link...\n" COLOR_RESET);
    system(groupCommand);
}

// Save timetable to a custom file
void saveTimetableAs(const char* filename) {
    FILE *fp;
    fp = fopen(filename, "w");
    if(fp == NULL) {
        printf(COLOR_ERROR "\nError: Could not create file %s!\n" COLOR_RESET, filename);
        return;
    }
    // Write header
    time_t now = time(NULL);
    fprintf(fp, "================================================================\n");
    fprintf(fp, "                        TIMETABLE DATA                            \n");
    fprintf(fp, "                  Saved on: %s", ctime(&now));
    fprintf(fp, "================================================================\n\n");
    // Write timetable data for each section
    char sections[] = {'A', 'B', 'C', 'D'};
    const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    for(int s = 0; s < 4; s++) {
        fprintf(fp, "\nSECTION %c TIMETABLE\n", sections[s]);
        fprintf(fp, "----------------------------------------------------------------\n");
        fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n", 
               "Day", "Time", "Code", "Subject Name", "Faculty");
        fprintf(fp, "----------------------------------------------------------------\n");
        // For each day
        for(int d = 0; d < 6; d++) {
            Lecture* temp = head;
            int hasLecture = 0;
            while(temp) {
                if(temp->section == sections[s] && strcmp(temp->day, days[d]) == 0) {
                    const char* subjectName = getSubjectName(temp->subject);
                    fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n",
                           temp->day,
                           temp->time,
                           temp->subject,
                           subjectName,
                           temp->faculty);
                    hasLecture = 1;
                }
                temp = temp->next;
            }
            if(hasLecture) {
                fprintf(fp, "----------------------------------------------------------------\n");
            }
        }
    }
    // Write subject legend
    fprintf(fp, "\n\nSUBJECT LEGEND:\n");
    fprintf(fp, "----------------------------------------------------------------\n");
    fprintf(fp, "%-8s | %-40s | %s\n", "Code", "Subject Name", "Type");
    fprintf(fp, "----------------------------------------------------------------\n");
    SubjectInfo subjects[50];
    int subjectCount = 0;
    getUniqueSubjects(subjects, &subjectCount);
    for(int i = 0; i < subjectCount; i++) {
        fprintf(fp, "%-8s | %-40s | %s\n",
               subjects[i].code,
               subjects[i].name,
               subjects[i].isLab ? "Lab" : "Theory");
    }
    fprintf(fp, "----------------------------------------------------------------\n");
    fprintf(fp, "\n================================================================\n");
    fprintf(fp, "                          END OF DATA                             \n");
    fprintf(fp, "================================================================\n");
    fclose(fp);
    printf(COLOR_SUCCESS "\nTimetable saved successfully to file: %s\n" COLOR_RESET, filename);
}

int main() {
    initializeTimetable();
    int choice, subChoice, viewChoice, saveChoice;
    char section, section1, section2;
    int dayChoice;
    char filename[100];
    while (1) {
        printf(COLOR_HEADER "\n====== Classroom Scheduler ======\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "1. Display Timetable\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "2. Swap Classes\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "3. Change Class Section\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "4. Teacher Load Analysis\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "5. Assign New Teacher\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "6. Save Timetable\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "7. Load Timetable\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "8. Search Timetable\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "9. Set Notification/Reminder\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "10. Statistics Dashboard\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "11. Share Timetable via QR Code\n" COLOR_RESET);
        printf(CLI_COLOR_MENU "12. Exit\n" COLOR_RESET);
        printf(COLOR_INPUT "Enter choice: " COLOR_RESET);
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf(COLOR_HILITE "\nDisplay Timetable Options:\n" COLOR_RESET);
                printf("1. Display Full Timetable (All Sections)\n");
                printf("2. Display Timetable for a Section\n");
                printf(COLOR_INPUT "Enter choice: " COLOR_RESET);
                scanf("%d", &subChoice);
                if(subChoice == 1) {
                    displayFullTimetable();
                } else if(subChoice == 2) {
                    printf(COLOR_INPUT "Enter section (A/B/C/D): " COLOR_RESET);
                    scanf(" %c", &section);
                    if (section >= 'A' && section <= 'D') {
                        printf("1. Full Week Timetable\n2. Specific Day Timetable\n");
                        printf(COLOR_INPUT "Enter choice: " COLOR_RESET);
                        scanf("%d", &viewChoice);
                        if(viewChoice == 1) {
                            printTimetable(section);
                        } else if(viewChoice == 2) {
                            printf("Select Day:\n1. Monday\n2. Tuesday\n3. Wednesday\n4. Thursday\n5. Friday\n6. Saturday\n");
                            printf(COLOR_INPUT "Enter choice (1-6): " COLOR_RESET);
                            scanf("%d", &dayChoice);
                            if(dayChoice >= 1 && dayChoice <= 6) {
                                const char* days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
                                printDayTimetable(section, days[dayChoice-1]);
                            } else {
                                printf(COLOR_ERROR "Invalid day choice!\n" COLOR_RESET);
                            }
                        } else {
                            printf(COLOR_ERROR "Invalid view choice!\n" COLOR_RESET);
                        }
                    } else {
                        printf(COLOR_ERROR "Invalid section!\n" COLOR_RESET);
                    }
                } else {
                    printf(COLOR_ERROR "Invalid display choice!\n" COLOR_RESET);
                }
                break;
            case 2:
                swapClasses();
                break;
            case 3:
                printf(COLOR_INPUT "Enter current section (A/B/C/D): " COLOR_RESET);
                scanf(" %c", &section1);
                printf(COLOR_INPUT "Enter new section (A/B/C/D): " COLOR_RESET);
                scanf(" %c", &section2);
                if ((section1 >= 'A' && section1 <= 'D') && (section2 >= 'A' && section2 <= 'D')) {
                    changeClassSection(section1, section2);
                } else {
                    printf(COLOR_ERROR "Invalid section!\n" COLOR_RESET);
                }
                break;
            case 4:
                analyzeTeacherLoad();
                break;
            case 5:
                assignNewTeacher();
                break;
            case 6:
                printf(COLOR_HILITE "\nSave Timetable Options:\n" COLOR_RESET);
                printf("1. Save Full Timetable (All Sections)\n");
                printf("2. Save Timetable for a Section\n");
                printf(COLOR_INPUT "Enter choice: " COLOR_RESET);
                scanf("%d", &saveChoice);
                if(saveChoice == 1) {
                    saveTimetable();
                } else if(saveChoice == 2) {
                    printf(COLOR_INPUT "Enter section (A/B/C/D): " COLOR_RESET);
                    scanf(" %c", &section);
                    if (section >= 'A' && section <= 'D') {
                        displaySaveSectionTimetable(section);
                    } else {
                        printf(COLOR_ERROR "Invalid section!\n" COLOR_RESET);
                    }
                } else {
                    printf(COLOR_ERROR "Invalid save choice!\n" COLOR_RESET);
                }
                break;
            case 7:
                printf(COLOR_INPUT "Enter filename to load: " COLOR_RESET);
                scanf(" %s", filename);
                loadTimetable(filename);
                break;
            case 8:
                searchTimetable();
                break;
            case 9:
                setNotification();
                break;
            case 10:
                statisticsDashboard();
                break;
            case 11:
                shareTimetableViaQR();
                break;
            case 12:
                printf(COLOR_HEADER "Thank you for using Classroom Scheduler!\n" COLOR_RESET);
                return 0;
            default:
                printf(COLOR_ERROR "Invalid choice. Please try again.\n" COLOR_RESET);
        }
    }
    return 0;
}