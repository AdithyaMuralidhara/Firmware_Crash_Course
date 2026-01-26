#include<stdint.h>
#include<stdio.h>
#define fifo_size 16
struct buffer{
    uint8_t data[fifo_size];
    uint8_t head;
    uint8_t tail;
};5
int fifo_push(struct buffer *fifo){
    int num;
    printf("Enter number of data to push:");
    scanf("%d",&num);
    for(int i=0;i<num;i++){
        if(((fifo->head+1)%fifo_size)==fifo->tail){ //Next position is Tail then the buffer is full
            printf("Buffer is full\n");
        }
        else{

            scanf("%d",&fifo->data[fifo->head]);
            fifo->head=(fifo->head+1)%fifo_size; //Circular buffer
        }
    }
    return 0;
}
int fifo_pop(struct buffer *fifo){
    if((fifo->head)==fifo->tail){ //Head and Tail are same then buffer is empty
        printf("Buffer is empty\n");
    }
    else{
        while(fifo->tail!=fifo->head){
            printf("%d ",fifo->data[fifo->tail]);
            fifo->tail=(fifo->tail+1)%fifo_size;
        }
    }
    return 0;
}

int main(){
    struct buffer fifo;
    fifo.head=0;
    fifo.tail=0;
    fifo_push(&fifo);
    fifo_pop(&fifo); 
    return 0;
}
