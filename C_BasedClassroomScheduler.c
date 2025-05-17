#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_LEN 100

typedef struct Lecture
{
    char day[MAX_LEN];
    char time[MAX_LEN];
    char subject[MAX_LEN];
    char faculty[MAX_LEN];
    char section;
    struct Lecture *next;
} Lecture;

Lecture *head = NULL;

// Stack for undo
Lecture *stack[100];
int top = -1;

// Queue for swap
Lecture *queue[100];
int front = -1, rear = -1;

// Structure to store teacher workload
typedef struct
{
    char name[MAX_LEN];
    int lectureCount;
    char sections[4]; // To store which sections they teach
    int sectionCount;
} TeacherLoad;

// Structure to store subject information
typedef struct
{
    char code[MAX_LEN];
    char name[MAX_LEN];
    int isLab;
} SubjectInfo;

// Structure to store time slot information
typedef struct
{
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
    {"PBL", "PROJECT BASED LEARNING", 0}};
const int CURRICULUM_SIZE = sizeof(curriculum) / sizeof(curriculum[0]);

// Function declarations
void insertLecture(char *day, char *time, char *subject, char *faculty, char section);
const char *getSubjectName(const char *subjectCode);
void printTimetable(char section);
void initializeTimetable(void);

// Insert a new lecture
void insertLecture(char *day, char *time, char *subject, char *faculty, char section)
{
    Lecture *newLecture = (Lecture *)malloc(sizeof(Lecture));
    if (newLecture == NULL)
    {
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
void push(Lecture *lec)
{
    stack[++top] = lec;
}

// Pop from undo stack
Lecture *pop()
{
    if (top == -1)
        return NULL;
    return stack[top--];
}

// Add to queue
void enqueue(Lecture *lec)
{
    if (front == -1)
        front = 0;
    queue[++rear] = lec;
}

// Remove from queue
Lecture *dequeue()
{
    if (front == -1 || front > rear)
        return NULL;
    return queue[front++];
}

// Print timetable for a section
void printTimetable(char section)
{
    Lecture *temp = head;
    printf("\n================================================================\n");
    printf("                    TIMETABLE FOR SECTION %c                        \n", section);
    printf("================================================================\n\n");

    const char *days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char *fullDays[] = {"MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};

    // For each day
    for (int i = 0; i < 6; i++)
    {
        int hasLecture = 0;
        temp = head;
        while (temp)
        {
            if (temp->section == section && strcmp(temp->day, days[i]) == 0)
            {
                hasLecture = 1;
                break;
            }
            temp = temp->next;
        }

        if (hasLecture)
        {
            printf("\n%s:\n", fullDays[i]);
            printf("----------------------------------------------------------------\n");
            printf("%-13s | %-7s | %-35s | %-15s |\n",
                   "Time", "Code", "Subject", "Faculty");
            printf("----------------------------------------------------------------\n");

            // Sort lectures by time for this day
            Lecture *sortedLectures[20];
            int lectureCount = 0;

            temp = head;
            while (temp)
            {
                if (temp->section == section && strcmp(temp->day, days[i]) == 0)
                {
                    sortedLectures[lectureCount++] = temp;
                }
                temp = temp->next;
            }

            // Simple bubble sort by time
            for (int j = 0; j < lectureCount - 1; j++)
            {
                for (int k = 0; k < lectureCount - j - 1; k++)
                {
                    if (strcmp(sortedLectures[k]->time, sortedLectures[k + 1]->time) > 0)
                    {
                        Lecture *temp = sortedLectures[k];
                        sortedLectures[k] = sortedLectures[k + 1];
                        sortedLectures[k + 1] = temp;
                    }
                }
            }

            // Print sorted lectures
            for (int j = 0; j < lectureCount; j++)
            {
                const char *subjectName = getSubjectName(sortedLectures[j]->subject);
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
void handleUnavailable(char *faculty, char section)
{
    Lecture *temp = head;
    while (temp)
    {
        if (strcmp(temp->faculty, faculty) == 0 && temp->section == section)
        {
            push(temp); // Save for undo
            printf("\nTeacher %s unavailable for %s (%s).\n", faculty, temp->subject, temp->time);

            // Try to swap with a lecture not using same teacher
            Lecture *trySwap = head;
            while (trySwap)
            {
                if (trySwap->section == section &&
                    strcmp(trySwap->faculty, faculty) != 0 &&
                    strcmp(trySwap->time, temp->time) != 0)
                {

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
void undo()
{
    Lecture *lec = pop();
    if (lec == NULL)
    {
        printf("Nothing to undo.\n");
        return;
    }
    printf("Undo: Restored %s (%s).\n", lec->subject, lec->faculty);
}

// Initial timetable data
void initializeTimetable()
{
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
void printDayTimetable(char section, const char *day)
{
    Lecture *temp = head;
    int hasLecture = 0;

    while (temp)
    {
        if (temp->section == section && strcmp(temp->day, day) == 0)
        {
            hasLecture = 1;
            break;
        }
        temp = temp->next;
    }

    if (!hasLecture)
    {
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
    Lecture *sortedLectures[20];
    int lectureCount = 0;

    temp = head;
    while (temp)
    {
        if (temp->section == section && strcmp(temp->day, day) == 0)
        {
            sortedLectures[lectureCount++] = temp;
        }
        temp = temp->next;
    }

    // Simple bubble sort by time
    for (int i = 0; i < lectureCount - 1; i++)
    {
        for (int j = 0; j < lectureCount - i - 1; j++)
        {
            if (strcmp(sortedLectures[j]->time, sortedLectures[j + 1]->time) > 0)
            {
                Lecture *temp = sortedLectures[j];
                sortedLectures[j] = sortedLectures[j + 1];
                sortedLectures[j + 1] = temp;
            }
        }
    }

    // Print sorted lectures
    for (int i = 0; i < lectureCount; i++)
    {
        const char *subjectName = getSubjectName(sortedLectures[i]->subject);
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
void getAvailableTimeSlots(char section, const char *day, int showAll)
{
    Lecture *temp = head;
    int found = 0;

    printf("\nAvailable time slots for Section %c on %s:\n", section, day);
    printf("----------------------------------------\n");

    // First pass: collect all time slots
    char times[20][20];
    int timeCount = 0;
    while (temp)
    {
        if (temp->section == section && strcmp(temp->day, day) == 0)
        {
            strcpy(times[timeCount], temp->time);
            timeCount++;
        }
        temp = temp->next;
    }

    // Sort time slots
    for (int i = 0; i < timeCount - 1; i++)
    {
        for (int j = 0; j < timeCount - i - 1; j++)
        {
            if (strcmp(times[j], times[j + 1]) > 0)
            {
                char temp[20];
                strcpy(temp, times[j]);
                strcpy(times[j], times[j + 1]);
                strcpy(times[j + 1], temp);
            }
        }
    }

    // Display time slots
    for (int i = 0; i < timeCount; i++)
    {
        temp = head;
        while (temp)
        {
            if (temp->section == section && strcmp(temp->day, day) == 0 && strcmp(temp->time, times[i]) == 0)
            {
                if (showAll || strcmp(temp->faculty, "-") == 0)
                {
                    printf("%d. %s - %s - %s\n",
                           i + 1,
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
void swapTeachers(char section1, char section2)
{
    if (section1 == section2)
    {
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

    if (dayChoice < 1 || dayChoice > 6)
    {
        printf("Invalid day choice!\n");
        return;
    }

    const char *days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char *fullDays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char *selectedDay = days[dayChoice - 1];

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
    Lecture *temp = head;
    Lecture *lec1 = NULL;
    Lecture *lec2 = NULL;
    char time1[20] = "", time2[20] = "";

    // Get time slots
    int count = 0;
    while (temp)
    {
        if (temp->section == section1 && strcmp(temp->day, selectedDay) == 0)
        {
            count++;
            if (count == timeChoice1)
            {
                strcpy(time1, temp->time);
            }
        }
        temp = temp->next;
    }

    count = 0;
    temp = head;
    while (temp)
    {
        if (temp->section == section2 && strcmp(temp->day, selectedDay) == 0)
        {
            count++;
            if (count == timeChoice2)
            {
                strcpy(time2, temp->time);
            }
        }
        temp = temp->next;
    }

    // Find lectures for swapping
    temp = head;
    while (temp)
    {
        if (strcmp(temp->day, selectedDay) == 0)
        {
            if (temp->section == section1 && strcmp(temp->time, time1) == 0)
            {
                lec1 = temp;
            }
            if (temp->section == section2 && strcmp(temp->time, time2) == 0)
            {
                lec2 = temp;
            }
        }
        temp = temp->next;
    }

    if (lec1 && lec2)
    {
        // Swap faculty members
        char tempFaculty[MAX_LEN];
        strcpy(tempFaculty, lec1->faculty);
        strcpy(lec1->faculty, lec2->faculty);
        strcpy(lec2->faculty, tempFaculty);

        printf("\nTeachers swapped successfully!\n");
        printf("Section %c (%s): %s - %s\n", section1, time1, lec1->subject, lec1->faculty);
        printf("Section %c (%s): %s - %s\n", section2, time2, lec2->subject, lec2->faculty);
    }
    else
    {
        printf("\nCould not find lectures for both sections at the specified times.\n");
    }
}

// Function to change class of a section
void changeClassSection(char oldSection, char newSection)
{
    Lecture *temp = head;
    int found = 0;

    printf("\nEnter the day (MON/TUE/WED/THU/FRI/SAT): ");
    char day[10];
    scanf("%s", day);

    printf("Enter the time slot (e.g., 8:00-8:55): ");
    char time[20];
    scanf("%s", time);

    // Find the lecture to change
    while (temp)
    {
        if (temp->section == oldSection && strcmp(temp->day, day) == 0 && strcmp(temp->time, time) == 0)
        {
            temp->section = newSection;
            found = 1;
            printf("\nClass changed successfully!\n");
            printf("Changed from Section %c to Section %c\n", oldSection, newSection);
            printf("Details: %s - %s - %s\n", temp->subject, temp->faculty, temp->time);
            break;
        }
        temp = temp->next;
    }

    if (!found)
    {
        printf("\nNo lecture found for the specified details.\n");
    }
}

// Function to check if a string contains "Lab" or "Sec"
int isLabOrSection(const char *str)
{
    return (strstr(str, "Lab") != NULL || strstr(str, "Sec") != NULL);
}

// Function to normalize teacher names
void normalizeTeacherName(char *name)
{
    // Convert "Mr. Neeraj" and "Mr. Neeraj Panwar" to "Mr. Neeraj Panwar"
    if (strcmp(name, "Mr. Neeraj") == 0)
    {
        strcpy(name, "Mr. Neeraj Panwar");
    }
}

// Function to analyze teacher workload
void analyzeTeacherLoad()
{
    TeacherLoad teachers[20]; // Assuming max 20 teachers
    int teacherCount = 0;
    const int MAX_LECTURES = 15; // Threshold for overloaded teachers

    // Initialize teacher array
    for (int i = 0; i < 20; i++)
    {
        teachers[i].lectureCount = 0;
        teachers[i].sectionCount = 0;
        memset(teachers[i].sections, 0, sizeof(teachers[i].sections));
    }

    // First pass: collect all unique teachers (excluding labs)
    Lecture *temp = head;
    while (temp)
    {
        if (strcmp(temp->faculty, "-") != 0 && !isLabOrSection(temp->faculty))
        { // Skip free periods and labs
            char normalizedName[MAX_LEN];
            strcpy(normalizedName, temp->faculty);
            normalizeTeacherName(normalizedName);

            int found = 0;
            for (int i = 0; i < teacherCount; i++)
            {
                if (strcmp(teachers[i].name, normalizedName) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                strcpy(teachers[teacherCount].name, normalizedName);
                teacherCount++;
            }
        }
        temp = temp->next;
    }

    // Second pass: count lectures and collect sections (excluding labs)
    temp = head;
    while (temp)
    {
        if (strcmp(temp->faculty, "-") != 0 && !isLabOrSection(temp->faculty))
        { // Skip free periods and labs
            char normalizedName[MAX_LEN];
            strcpy(normalizedName, temp->faculty);
            normalizeTeacherName(normalizedName);

            for (int i = 0; i < teacherCount; i++)
            {
                if (strcmp(teachers[i].name, normalizedName) == 0)
                {
                    teachers[i].lectureCount++;

                    // Add section if not already present
                    int sectionExists = 0;
                    for (int j = 0; j < teachers[i].sectionCount; j++)
                    {
                        if (teachers[i].sections[j] == temp->section)
                        {
                            sectionExists = 1;
                            break;
                        }
                    }
                    if (!sectionExists)
                    {
                        teachers[i].sections[teachers[i].sectionCount++] = temp->section;
                    }
                    break;
                }
            }
        }
        temp = temp->next;
    }

    // Sort teachers by lecture count (descending)
    for (int i = 0; i < teacherCount - 1; i++)
    {
        for (int j = 0; j < teacherCount - i - 1; j++)
        {
            if (teachers[j].lectureCount < teachers[j + 1].lectureCount)
            {
                TeacherLoad temp = teachers[j];
                teachers[j] = teachers[j + 1];
                teachers[j + 1] = temp;
            }
        }
    }

    // Display results
    printf("\n================================================================\n");
    printf("                    TEACHER WORKLOAD ANALYSIS                      \n");
    printf("================================================================\n\n");
    printf("%-25s | %-15s | %-15s | %s\n", "Teacher Name", "Lectures/Week", "Sections", "Status");
    printf("----------------------------------------------------------------\n");

    for (int i = 0; i < teacherCount; i++)
    {
        char sections[10] = "";
        // Sort sections alphabetically
        for (int j = 0; j < teachers[i].sectionCount - 1; j++)
        {
            for (int k = 0; k < teachers[i].sectionCount - j - 1; k++)
            {
                if (teachers[i].sections[k] > teachers[i].sections[k + 1])
                {
                    char temp = teachers[i].sections[k];
                    teachers[i].sections[k] = teachers[i].sections[k + 1];
                    teachers[i].sections[k + 1] = temp;
                }
            }
        }

        for (int j = 0; j < teachers[i].sectionCount; j++)
        {
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
int isTimeSlotAvailable(const char *day, const char *time, char section)
{
    Lecture *temp = head;
    while (temp)
    {
        if (temp->section == section &&
            strcmp(temp->day, day) == 0 &&
            strcmp(temp->time, time) == 0)
        {
            return 0; // Time slot is occupied
        }
        temp = temp->next;
    }
    return 1; // Time slot is available
}

// Function to get time string from choice
void getTimeString(int choice, char *time)
{
    switch (choice)
    {
    case 1:
        strcpy(time, "8:00-8:55");
        break;
    case 2:
        strcpy(time, "8:55-9:50");
        break;
    case 3:
        strcpy(time, "10:10-11:05");
        break;
    case 4:
        strcpy(time, "11:05-12:00");
        break;
    case 5:
        strcpy(time, "12:00-12:55");
        break;
    case 6:
        strcpy(time, "2:10-3:05");
        break;
    case 7:
        strcpy(time, "3:05-4:00");
        break;
    case 8:
        strcpy(time, "4:00-4:55");
        break;
    case 9:
        strcpy(time, "4:55-5:50");
        break;
    }
}

// Function to get unique subjects from timetable
void getUniqueSubjects(SubjectInfo subjects[], int *count)
{
    *count = 0;

    // Copy from curriculum
    for (int i = 0; i < CURRICULUM_SIZE; i++)
    {
        strcpy(subjects[*count].code, curriculum[i].code);
        strcpy(subjects[*count].name, curriculum[i].name);
        subjects[*count].isLab = curriculum[i].isLab;
        (*count)++;
    }
}

// Function to find existing time slots for a subject
void findExistingTimeSlots(const char *subject, TimeSlot slots[], int *count)
{
    *count = 0;
    Lecture *temp = head;

    while (temp)
    {
        if (strcmp(temp->subject, subject) == 0)
        {
            strcpy(slots[*count].day, temp->day);
            strcpy(slots[*count].time, temp->time);
            slots[*count].section = temp->section;
            (*count)++;
        }
        temp = temp->next;
    }
}

// Function to assign new teacher
void assignNewTeacher()
{
    char teacherName[MAX_LEN];
    char subject[MAX_LEN];
    char time[MAX_LEN];
    char day[4];
    int dayChoice;
    int timeChoice;
    int subjectChoice;
    const char *days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};

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

    if (subjectChoice == 1)
    {
        if (subjectCount == 0)
        {
            printf("\nNo existing subjects found. Please create a new subject.\n");
            printf("Enter new subject code: ");
            scanf(" %[^\n]s", subject);
        }
        else
        {
            printf("\nExisting subjects:\n");
            printf("%-8s | %-40s | %s\n", "Code", "Subject Name", "Type");
            printf("----------------------------------------------------------\n");
            for (int i = 0; i < subjectCount; i++)
            {
                printf("%d. %-6s | %-40s | %s\n",
                       i + 1,
                       subjects[i].code,
                       subjects[i].name,
                       subjects[i].isLab ? "Lab" : "Theory");
            }

            int choice;
            do
            {
                printf("\nEnter subject number (1-%d): ", subjectCount);
                scanf("%d", &choice);
                if (choice < 1 || choice > subjectCount)
                {
                    printf("Invalid choice. Please try again.\n");
                }
            } while (choice < 1 || choice > subjectCount);

            strcpy(subject, subjects[choice - 1].code);
            printf("\nSelected: %s - %s\n", subjects[choice - 1].code, subjects[choice - 1].name);

            // Find existing time slots for this subject
            TimeSlot existingSlots[20];
            int slotCount = 0;

            findExistingTimeSlots(subject, existingSlots, &slotCount);

            if (slotCount > 0)
            {
                printf("\nFound existing time slots for %s:\n", subject);
                for (int i = 0; i < slotCount; i++)
                {
                    printf("Section %c: %s %s\n",
                           existingSlots[i].section,
                           existingSlots[i].day,
                           existingSlots[i].time);
                }

                printf("\nAssigning %s to these time slots...\n", teacherName);

                // Update all existing slots with the new teacher
                Lecture *temp = head;
                while (temp)
                {
                    if (strcmp(temp->subject, subject) == 0)
                    {
                        strcpy(temp->faculty, teacherName);
                    }
                    temp = temp->next;
                }

                printf("\nTeacher assigned successfully!\n");
                return;
            }
            else
            {
                printf("\nNo existing time slots found for this subject.\n");
                printf("Proceeding with manual time slot assignment...\n");
            }
        }
    }
    else if (subjectChoice == 2)
    {
        char subjectName[MAX_LEN];
        printf("Enter new subject code (max 8 chars): ");
        scanf(" %[^\n]s", subject);
        printf("Enter subject name (max 40 chars): ");
        scanf(" %[^\n]s", subjectName);
        printf("Is this a lab subject? (1 for Yes, 0 for No): ");
        int isLab;
        scanf("%d", &isLab);
    }
    else
    {
        printf("Invalid choice. Operation cancelled.\n");
        return;
    }

    // Only proceed with manual time slot assignment for new subjects
    // or existing subjects with no time slots

    // For each section
    char sections[] = {'A', 'B', 'C', 'D'};
    for (int i = 0; i < 4; i++)
    {
        char currentSection = sections[i];
        int validTimeSlot = 0;

        while (!validTimeSlot)
        {
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

            if (dayChoice < 1 || dayChoice > 6)
            {
                printf("Invalid day choice! Please try again.\n");
                continue;
            }

            strcpy(day, days[dayChoice - 1]);

            // Show available time slots and mark occupied ones
            printf("\nAvailable Time Slots for Section %c on %s:\n", currentSection, days[dayChoice - 1]);
            for (int j = 1; j <= 9; j++)
            {
                char tempTime[MAX_LEN];
                getTimeString(j, tempTime);
                printf("%d. %s %s\n", j, tempTime,
                       isTimeSlotAvailable(day, tempTime, currentSection) ? "(Available)" : "(Occupied)");
            }

            printf("\nEnter time slot number (1-9): ");
            scanf("%d", &timeChoice);

            if (timeChoice < 1 || timeChoice > 9)
            {
                printf("Invalid time choice! Please try again.\n");
                continue;
            }

            getTimeString(timeChoice, time);

            // Check if time slot is available
            if (!isTimeSlotAvailable(day, time, currentSection))
            {
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
const char *getSubjectName(const char *subjectCode)
{
    static char subjectName[MAX_LEN]; // Make it static to persist after function returns

    // First check curriculum array
    for (int i = 0; i < CURRICULUM_SIZE; i++)
    {
        if (strcmp(curriculum[i].code, subjectCode) == 0)
        {
            strcpy(subjectName, curriculum[i].name);
            return subjectName;
        }
    }

    // If not found in curriculum, return the code itself
    strcpy(subjectName, subjectCode);
    return subjectName;
}

// Function to save timetable to file
void saveTimetable()
{
    FILE *fp;
    char filename[100];

    // Create filename with current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "timetable_%Y%m%d_%H%M%S.txt", t);

    fp = fopen(filename, "w");
    if (fp == NULL)
    {
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
    const char *days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};

    for (int s = 0; s < 4; s++)
    {
        fprintf(fp, "\nSECTION %c TIMETABLE\n", sections[s]);
        fprintf(fp, "----------------------------------------------------------------\n");
        fprintf(fp, "%-5s | %-13s | %-8s | %-40s | %s\n",
                "Day", "Time", "Code", "Subject Name", "Faculty");
        fprintf(fp, "----------------------------------------------------------------\n");

        // For each day
        for (int d = 0; d < 6; d++)
        {
            Lecture *temp = head;
            int hasLecture = 0;

            while (temp)
            {
                if (temp->section == sections[s] && strcmp(temp->day, days[d]) == 0)
                {
                    const char *subjectName = getSubjectName(temp->subject);
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

            if (hasLecture)
            {
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

    for (int i = 0; i < subjectCount; i++)
    {
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
void trim(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return; // All spaces?

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';
}

// Function to load timetable from file
void loadTimetable(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("\nError: Could not open file %s!\n", filename);
        return;
    }

    // Clear existing timetable
    while (head != NULL)
    {
        Lecture *temp = head;
        head = head->next;
        free(temp);
    }

    char line[256];
    char day[10], time[20], subject[50], faculty[50];
    char section = '\0';

    // Skip header until we find a section
    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, "SECTION") != NULL)
        {
            section = line[8]; // Get section letter

            // Skip header lines
            fgets(line, sizeof(line), fp); // separator
            fgets(line, sizeof(line), fp); // column headers
            fgets(line, sizeof(line), fp); // separator

            // Read lectures
            while (fgets(line, sizeof(line), fp) && strlen(line) > 2)
            {
                // Skip separator lines
                if (strstr(line, "---") != NULL || strstr(line, "SECTION") != NULL)
                {
                    break;
                }

                // Parse line
                if (sscanf(line, "%s | %[^|] | %[^|] | %[^\n]", day, time, subject, faculty) == 4)
                {
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

int main()
{
    initializeTimetable();

    int choice, viewChoice;
    char section, section1, section2;
    int dayChoice;
    char filename[100];

    while (1)
    {
        printf("\n====== Classroom Scheduler ======\n");
        printf("1. View Timetable\n");
        printf("2. Swap Teachers\n");
        printf("3. Change Class Section\n");
        printf("4. Teacher Load Analysis\n");
        printf("5. Assign New Teacher\n");
        printf("6. Save Timetable\n");
        printf("7. Load Timetable\n");
        printf("8. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printf("\nEnter section (A/B/C/D): ");
            scanf(" %c", &section);
            if (section >= 'A' && section <= 'D')
            {
                printf("\nView Options:\n");
                printf("1. Full Week Timetable\n");
                printf("2. Specific Day Timetable\n");
                printf("Enter choice: ");
                scanf("%d", &viewChoice);

                switch (viewChoice)
                {
                case 1:
                    printTimetable(section);
                    break;
                case 2:
                    printf("\nSelect Day:\n");
                    printf("1. Monday\n");
                    printf("2. Tuesday\n");
                    printf("3. Wednesday\n");
                    printf("4. Thursday\n");
                    printf("5. Friday\n");
                    printf("6. Saturday\n");
                    printf("Enter choice (1-6): ");
                    scanf("%d", &dayChoice);

                    if (dayChoice >= 1 && dayChoice <= 6)
                    {
                        const char *days[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT"};
                        printDayTimetable(section, days[dayChoice - 1]);
                    }
                    else
                    {
                        printf("Invalid day choice!\n");
                    }
                    break;
                default:
                    printf("Invalid view choice!\n");
                }
            }
            else
            {
                printf("Invalid section! Please enter A, B, C, or D.\n");
            }
            break;
        case 2:
            printf("\nEnter first section (A/B/C/D): ");
            scanf(" %c", &section1);
            printf("Enter second section (A/B/C/D): ");
            scanf(" %c", &section2);
            if ((section1 >= 'A' && section1 <= 'D') && (section2 >= 'A' && section2 <= 'D'))
            {
                swapTeachers(section1, section2);
            }
            else
            {
                printf("Invalid section! Please enter A, B, C, or D.\n");
            }
            break;
        case 3:
            printf("\nEnter current section (A/B/C/D): ");
            scanf(" %c", &section1);
            printf("Enter new section (A/B/C/D): ");
            scanf(" %c", &section2);
            if ((section1 >= 'A' && section1 <= 'D') && (section2 >= 'A' && section2 <= 'D'))
            {
                changeClassSection(section1, section2);
            }
            else
            {
                printf("Invalid section! Please enter A, B, C, or D.\n");
            }
            break;
        case 4:
            analyzeTeacherLoad();
            break;
        case 5:
            assignNewTeacher();
            break;
        case 6:
            saveTimetable();
            break;
        case 7:
            printf("\nEnter filename to load: ");
            scanf(" %s", filename);
            loadTimetable(filename);
            break;
        case 8:
            printf("Thank you for using Classroom Scheduler!\n");
            return 0;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}