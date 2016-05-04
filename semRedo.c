//@author:Xingfan Xia

//global
int sem_count;
pthread_mutex_t lock;
pthread_cond_t cond;    

//init
void sem_init() {
	sem_count = 0;
}

void up() {
	pthread_mutex_lock(&lock);
	sem_count++;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

void down() {
	pthread_mutex_lock(&lock);
	while(sem_count == 0) {
		pthread_cond_wait(&cond, &lock);
	}
	sem_count--;
	pthread_mutex_unlock(&lock);
}