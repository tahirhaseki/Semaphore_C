#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 
#include <time.h>

#define ROOM_CAPACITY 4
#define ROOM_NUMBER 10
#define STUDENT_NUMBER 100

// Room status declarecation.
enum roomStatus{
    entryFree,
    idle,
    full
};

// Thread function declaring.
void *roomKeeper(void *num);
void *student(void *num);
void *console(void *);

// Defining semaphores
struct studyRoom{
    sem_t studyRoomChairs;      // studyRoomChairs limits the number of students allowed in the room at the same time.
    sem_t usingBroom;           // usingBroom shows the roomKeeper is cleaning or not.
    sem_t closeRoom;            // Send keeper to closing info.
    int status;                 // declares status of room.
    int studentInRoom;          // keeps number of students in the room;
    int completed;              // keeps number of students who complete studying with team.
    int useCount;               // keeps using count of room.
    int studentIDs[4]; 
} typedef studyRoom;

sem_t screen;                   // Refresh console screen.

// Sleep thread in milisecond. Source: https://stackoverflow.com/a/1157217
int msleep(long msec){
    struct timespec ts;
    int res;

    if(msec < 0){
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts,&ts);
    } while (res);
    return res;
}
void updateScreen();

studyRoom studyRooms[ROOM_NUMBER];

int studyDone = 0;              // studyDone is flag to emptying a room after studying.
int allDone = 1;                // allDone is flag to finishing program.
int maxUsage = 1;               // Keep usage of rooms under control for equal using.

int main(int argc, char *argv[]){
    
    pthread_t rk_tid[ROOM_NUMBER];          // RoomKeepers
    pthread_t std_tid[STUDENT_NUMBER];      // Students
    pthread_t console_tid;                  // Console Simulation

    //Initialize rooms/semaphores
    sem_init(&screen,0,0);
    for (int i = 0; i < ROOM_NUMBER; i++)
    {
        sem_init(&(studyRooms[i].studyRoomChairs),0,ROOM_CAPACITY);
        sem_init(&(studyRooms[i].usingBroom),0,0);
        sem_init(&(studyRooms[i].closeRoom),0,0);
        studyRooms[i].status = entryFree;
        studyRooms[i].studentInRoom = 0;
        studyRooms[i].completed = 0;
        studyRooms[i].useCount = 0;
        studyRooms[i].studentIDs[0] = 0;
        studyRooms[i].studentIDs[1] = 0;
        studyRooms[i].studentIDs[2] = 0;
        studyRooms[i].studentIDs[3] = 0;

    }
    
    // Student and Room Keeper Id's Declare
    int std_numbers[STUDENT_NUMBER];
    int rk_numbers[ROOM_NUMBER];
    for(int i = 0;i < ROOM_NUMBER;i++){
        rk_numbers[i] = i;
    }
    for(int i = 0;i < STUDENT_NUMBER;i++){
        std_numbers[i] = i;
    }

    // First console running.
    pthread_create(&console_tid,NULL,console,NULL);
    msleep(500);

    // Creating roomKeepers
    for (int i = 0; i < ROOM_NUMBER; i++)
    {
        pthread_create(&rk_tid[i],NULL,roomKeeper,(void *)(rk_numbers+i));
    }
    // Creating students.
    for (int i = 0; i < STUDENT_NUMBER; i++)
    {
        msleep(rand() % (700-150 +1) + 150);
        pthread_create(&std_tid[i],NULL,student,(void *)(std_numbers+i));
    }

    // Joining all thread to be ready for ending program.
    for (int i = 0;i < STUDENT_NUMBER ;i++)
    {
        pthread_join(std_tid[i],NULL);
    }
    allDone =  0;
    for (int i = 0;i < ROOM_NUMBER ;i++)
    {
        pthread_join(rk_tid[i],NULL);
    }
    updateScreen();
    printf("Everyone has studied.Library is closed.\n");


}

// Checks rooms for availability for students.
int anyFreeStudyRoom(){
    int changeMaxUsage = 1;
    for(int i = 0;i < ROOM_NUMBER;i++){
        if(studyRooms[i].useCount < maxUsage){
            changeMaxUsage = 0;
            break;
        }
        if(i == ROOM_NUMBER - 1 && changeMaxUsage){
            maxUsage = studyRooms[i].useCount + 1;
        }
    }
    for(int j = 0;j < 2;j++){
        for(int i = 0;i < ROOM_NUMBER;i++){
            if(studyRooms[i].useCount < maxUsage && studyRooms[i].status == idle && sem_trywait(&studyRooms[i].studyRoomChairs)== 0){
                return i;
            }
        }
    }
    // Checks not full rooms and returns room's id.
    for(int i = 0;i < ROOM_NUMBER;i++){
        if(studyRooms[i].useCount < maxUsage && studyRooms[i].status != full && sem_trywait(&studyRooms[i].studyRoomChairs)== 0){
            return i;
        }
    }
    return -1;
}

// Student thread function.
void * student(void *num){
    int number = *(int *)num + 1;       // Student ID init.
    int targetRoom = -1;            // Target room init.
    /* 
    ** Student Arrives Library and checks for available study room.
    ** printf("Student %d arrives library.\n",number);
    **/

    while(1){
        targetRoom = anyFreeStudyRoom();
        if(targetRoom != -1)
        {
            studyRooms[targetRoom].studentIDs[studyRooms[targetRoom].studentInRoom] = number;
            break;
        }
    }

    /* 
    ** Student found room.If room is empty then wakes up room keeper.
    ** printf("Student %d found room.\n",number);
    **/
   
    if(studyRooms[targetRoom].studentInRoom++ == 0){
        sem_post(&studyRooms[targetRoom].usingBroom);
    }

    /*
    ** Student working in the room and waits other students.
    ** printf("Student %d studying.\n",number);
    ** When room is full. All students in the room works as a group.
    ** After studying together send close information to room keeper for each student.
    */
    while(1){
        if(studyRooms[targetRoom].studentInRoom == ROOM_CAPACITY){
            //printf("Student %d studying with all group.\n",number);          
            sem_post(&studyRooms[targetRoom].closeRoom);
            break;
        }
    }

    // After Room keeper's approve for all students in the room. Students can leave.
    while(1){
        if(studyRooms[targetRoom].completed == ROOM_CAPACITY){
            //printf("Student %d leaving room.\n",number);
            break;
        }
    }
}


void *roomKeeper(void *num){
    int shutDown = 0;
    while(allDone){
        int number = *(int *)num;       // Room Keeper ID init.
        int temp = 0;                   // Temp value for check differences of students number in room.

        /*
        ** In the initialize of thread, room keeper is cleaning room. 
        ** printf("RoomKeeper cleaning room %d.\n",number);
        ** Waits semaphore to unlock.
        ** printf("Room %d created.\n",number+1);
        ** printf("RoomKeeper is cleaning room %d\n",number+1);
        */

       while(1){
           if(sem_trywait(&studyRooms[number].usingBroom) == 0){
               break;
           }
           else if(!allDone){
               shutDown = 1;
               break;
           }
       }
        if(shutDown){
            sem_post(&screen);
            break;
        }

        /*
        ** Room keeper starts working.
        ** printf("RoomKeeper is working now.\n");
        ** Then changes room status.
        ** printf("Room %d is Idle.\n",number);
        */

        studyRooms[number].status = idle;

        while(1){
            // Room keeper announces left spaces in the room for every change.
            if(studyRooms[number].studentInRoom != temp){
                //printf("In room %d, last %d students, let's go up!\n",number,ROOM_CAPACITY-studyRooms[number].studentInRoom);
                temp = studyRooms[number].studentInRoom;
                sem_post(&screen);
            }
            // Check numbers of students in room and changes room's status.
            if(studyRooms[number].status != full && studyRooms[number].studentInRoom == ROOM_CAPACITY){
                studyRooms[number].useCount++;
                studyRooms[number].status = full;
                //printf("Room %d is full.\n",number);
            }

            // Checks students before leaving. 
            if(sem_trywait(&studyRooms[number].closeRoom) == 0){
                studyRooms[number].completed++;
            }

            // Checks every student is approved. Then sets room's attributes properly.
            if(studyRooms[number].completed == ROOM_CAPACITY){
                msleep(5000);
                studyRooms[number].studentInRoom = 0;
                temp = 0;
                studyRooms[number].completed = 0;
                studyRooms[number].status = entryFree;
                //printf("Room %d is entryFree.\n",number);
                for(int i = 0;i < ROOM_CAPACITY;i++)
                    sem_post(&studyRooms[number].studyRoomChairs);
                sem_post(&screen);
                //printf("Final state of room %d: \n inRoom = %d\n status = %d",number,studyRooms[number].studentInRoom,studyRooms[number].status);
                break;
            }
        }
    }
}
void updateScreen(){
    system("clear");
    for(int i = 0;i < ROOM_NUMBER;i++){
        int inRoom = studyRooms[i].studentInRoom;
        int status = studyRooms[i].status;
        printf("Room %d = ",i+1);
        for(int j = 0;j < inRoom;j++){
            printf("%d ",studyRooms[i].studentIDs[j]);
        }
        for(int j = 0;j < ROOM_CAPACITY-inRoom;j++){
            printf("O ");
        }
        if(status == entryFree){
            printf("Status = EntryFree - Cleaning - useCount: %d\n",studyRooms[i].useCount );
        }
        else if(status == idle){
            printf("Status = Idle - Not Cleaning - useCount: %d\n",studyRooms[i].useCount);
        }
        else{
            printf("Status = Full - Not Cleaning - useCount: %d\n",studyRooms[i].useCount);
        }
    }
}

// Thread function for console.
void *console(void* p){
    int temp_seats[ROOM_NUMBER];
    for(int i = 0; i< ROOM_NUMBER;i++){
        temp_seats[i] = 0;
        printf("Room %d = O O O O Status = EntryFree - Cleaning - useCount: %d\n",i+1,studyRooms[i].useCount);
    }
    // Updates screen for every semaphore post. 
    // And it is updating for every 2 seconds no matter what.
    while(allDone){
        struct timespec ts;
        if(clock_gettime(CLOCK_REALTIME, &ts) == 0){
            ts.tv_sec += 3;
        }
        if(sem_timedwait(&screen,&ts) == 0)
            updateScreen();
        else{
            updateScreen();
        }
    }
}
