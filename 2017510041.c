#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

#define ROOM_CAPACITY 4
#define ROOM_NUMBER 2
#define STUDENT_NUMBER 8

enum roomStatus{
    entryFree,
    idle,
    full
};



void *roomKeeper(void *num);
void *student(void *num);
void *studentTest(void *num);

// Defining semaphores
struct studyRoom{
    sem_t lockRoom;             // studyRoom locks the room when it's full.
    sem_t studyRoomChairs;      // studyRoomChairs limits the number of students allowed in the room at the same time.
    sem_t usingBroom;           // usingBroom shows the roomKeeper is cleaning or not.
    int status;                 // declares status of room.
    int studentInRoom;          // keeps number of students in the room;
    int completed;
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
        sem_init(&(studyRooms[i].usingBroom),0,0);
        studyRooms[i].status = entryFree;
        studyRooms[i].studentInRoom = 0;
        studyRooms[i].completed = 0;
    }
    
    int std_numbers[STUDENT_NUMBER];
    int rk_numbers[ROOM_NUMBER];
    for(int i = 0;i < ROOM_NUMBER;i++){
        rk_numbers[i] = i;
    }
    for(int i = 0;i < STUDENT_NUMBER;i++){
        std_numbers[i] = i;
    }


    // Creating roomKeepers
    for (int i = 0; i < ROOM_NUMBER; i++)
    {
        pthread_create(&rk_tid[i],NULL,roomKeeper,(void *)(rk_numbers+i));
    }
    for (int i = 0; i < STUDENT_NUMBER; i++)
    {
        pthread_create(&std_tid[i],NULL,student,(void *)(std_numbers+i));
    }
    for (int i = 0;i < STUDENT_NUMBER ;i++)
    {
        pthread_join(std_tid[i],NULL);
    }
        for (int i = 0;i < ROOM_NUMBER ;i++)
    {
        pthread_join(rk_tid[i],NULL);
    }
    

}

int anyFreeStudyRoom(){
    for(int i = 0;i < ROOM_NUMBER;i++){
        if(studyRooms[i].status != full &&sem_trywait(&studyRooms[i].studyRoomChairs)== 0)
            return i;
    }
    return -1;
}
void * studentTest(void *num){
    int number = *(int *)num;
    int targetRoom = -1;
    printf("Student %d arrives library.\n",number);
}
void * student(void *num){
    int number = *(int *)num;
    int targetRoom = -1;
    printf("Student %d arrives library.\n",number);
    while(1){
        //printf("std %d looking for empty room",number);
        targetRoom = anyFreeStudyRoom();
        if(targetRoom != -1)
            break;
    }
    printf("Student %d found room.\n",number);
    if(studyRooms[targetRoom].studentInRoom++ == 0){
        sem_post(&studyRooms[targetRoom].usingBroom);
    }
    printf("Student %d studying.\n",number);
    while(1){
        if(studyRooms[targetRoom].studentInRoom == ROOM_CAPACITY){
            printf("Student %d studying with all group.\n",number);  
            studyRooms[targetRoom].completed++;          
            break;
        }
    }
    while(1){
        if(studyRooms[targetRoom].completed == ROOM_CAPACITY){
            printf("Student %d leaving room.\n",number);
            break;
        }
    }
}
void *roomKeeper(void *num){
    while(1){
        int number = *(int *)num;
        int temp = 0;
        printf("RoomKeeper cleaning room %d.\n",number);
        sem_wait(&studyRooms[number].usingBroom);
        printf("RoomKeeper is working now.\n");
        studyRooms[number].status = idle;
        printf("Room %d is Idle.\n",number);
        while(1){
            if(studyRooms[number].studentInRoom != temp){
                printf("In room %d, last %d students, let's go up!\n",number,4-studyRooms[number].studentInRoom);
                temp = studyRooms[number].studentInRoom;
            }
            if(studyRooms[number].status != full && studyRooms[number].studentInRoom == 4){
                studyRooms[number].status = full;
                printf("Room %d is full.\n",number);
            }
            if(studyRooms[number].completed == 4){
                sleep(1);
                studyRooms[number].studentInRoom = 0;
                temp = 0;
                for(int i = 0;i < ROOM_CAPACITY;i++)
                    sem_post(&studyRooms[number].studyRoomChairs);
                printf("Room %d is entryFree.\n",number);
                //sem_post(&studyRooms[number].usingBroom);
                studyRooms[number].completed = 0;
                studyRooms[number].status = entryFree;
                break;
            }
        }
    }
}