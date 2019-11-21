#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

#define ROOM_CAPACITY 4
#define ROOM_NUMBER 1
#define STUDENT_NUMBER 12

enum roomStatus{
    entryFree,
    idle,
    full
}



void *roomKeeper(void *num);
void *student(void *num);

// Defining semaphores
struct studyRoom{
    sem_t lockRoom;             // studyRoom locks the room when it's full.
    sem_t studyRoomChairs;      // studyRoomChairs limits the number of students allowed in the room at the same time.
    sem_t usingBroom;           // usingBroom shows the roomKeeper is cleaning or not.
    int status;                 // declares status of room.
    int studentInRoom;          // keeps number of students in the room;
} typedef studyRoom;

studyRoom studyRooms[ROOM_NUMBER];
// studyDone is flag to emptying a room after studying.
int studyDone = 0;

int main(int argc, char *argv[]){
    // RoomKeepers
    pthread_t rk_tid[ROOM_NUMBER];
    // Students
    pthread_t std_tid[STUDENT_NUMBER];

    //Initialize rooms/semaphores
    for (int i = 0; i < ROOM_NUMBER; i++)
    {
        sem_init(&(studyRooms[i].lockRoom),0,1);
        sem_init(&(studyRooms[i].studyRoomChairs),0,ROOM_CAPACITY);
        sem_inti(&(studyRooms[i].usingBroom),0,0);
        studyRooms[i].status = entryFree;
        studyRooms[i].studentInRoom = 0;
    }
    

    // Creating roomKeepers
    for (int i = 0; i < ROOM_NUMBER; i++)
    {
        pthread_create(&rk_tid[i],NULL roomKeeper,(void *)&i);
    }
    for (int i = 0; i < STUDENT_NUMBER; i++)
    {
        pthread_create(&st_tid[i],NULL student,(void *)&i);
    }
    

}

int anyFreeStudyRoom(){
    for(int i = 0;i < ROOM_NUMBER;i++){
        if(sem_trywait(studyRooms[i].lockRoom) =0 0)
            return i;
    }
    return -1;
}

void * student(void *num){
    int number = *(int *)num;
    int targetRoom = -1;
    printf("Student arrives library.");
    while(true){
        targetRoom = anyFreeStudyRoom();
        if(targetRoom != -1)
            break;
    }
    

}