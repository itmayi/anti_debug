#include <jni.h>
#include <sys/system_properties.h>
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/inotify.h>


#define TAG "TEST"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define MAXLEN 1024

int extractDigitFromStr(char * p){
    int  value = 0;
    while (*p != NULL){
        if ((*p >= '0') && (*p <= '9')){
            value = value * 10 + *p - '0';
        }
        p++;
    }
    return value;
}


/**
 * 监测/dev/__properties__
 */
char globalCh;
void checkPropertyFile(bool loop){

    FILE *fp = fopen("/dev/__properties__","rb");

    if (fp == NULL){
        LOGE("open /dev/__properties__ failed!");
        return;
    }

    //移动文件指针
    fseek(fp,0xAB4,SEEK_SET);
    if (loop){
        while (1){
            char ch = (char)fgetc(fp);
            LOGD("value:%c",ch);
            if ('1' == ch){
                LOGE("内存中ro.debuggable属性值被修改为1,/dev/propperties跟随改变!");
                break;
            }
            fseek(fp,-1,SEEK_CUR);
            sleep(3000);
        }
    } else{
        globalCh = (char)fgetc(fp);
    }

    fclose(fp);
}

/**
 * 监测/proc/1/status
 */
int checkStatus(){
    FILE *fp = fopen("/proc/1/status","r");

    if (fp == NULL){
        LOGE("open /proc/1/status failed!");
        return -1;
    }
    char line[100];
    memset(line,0, sizeof(line));
    while (fgets(line, sizeof(line),fp)){
        if (strstr(line,"TracerPid") != NULL){
            int trace = extractDigitFromStr(line);
            LOGD("TracerPid:%d",trace);
            if (trace != 0){
                LOGE("process init is injected!");
                return trace;
            }
            return 0;
        }
    }
    fclose(fp);
}

/**
 * 监测/proc/1/stat
 */
int checkStat(){
    FILE *fp = fopen("/proc/1/stat","r");

    if (fp == NULL){
        LOGE("open /proc/1/stat failed !!!");
        return -1;
    }
//    char *p;
    char line[100];
    memset(line,0, sizeof(line));
    if (fgets(line, sizeof(line),fp)){
        if (strstr(line,"(init) t") != NULL){
            LOGD("line:%s",line);
            LOGE("process init is injected!");
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/**
 * 监测 /proc/1/wchan
 */
int checkWchan(){
    FILE *fp = fopen("/proc/1/wchan","r");
    if (fp == NULL){
        LOGE("open /proc/1/wchan failed !!!");
        return -1;
    }

    char line[100];
    if (fgets(line, sizeof(line),fp)){
        if (strstr(line,"ptrace_stop") != NULL){
            LOGD("%s",line);
            LOGE("process init is injected!");
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void check_inotify(const char *inotify_file)
{
    LOGD("check inotify!!!");

    ptrace(PTRACE_TRACEME,0,0,0);

    int ret, len, i;
    char readbuf[MAXLEN];
    int fd, wd;
    fd_set readfds;
    fd = inotify_init();

    LOGD("inotify_file:%s",inotify_file);
    wd = inotify_add_watch(fd, inotify_file, IN_ALL_EVENTS);
    if (wd >= 0) {
        while (1) {
            i = 0;
            FD_ZERO(&readfds); // 使得readfds清零
            FD_SET(fd, &readfds); // 将fd加入readfds集合
            ret = select(fd + 1, &readfds, 0, 0, 0);
            if (ret == -1) {
                break;
            }
            if (ret) {
                len = read(fd, readbuf, MAXLEN);
                while (i < len) {
                    struct inotify_event *event = (struct inotify_event *) &readbuf[i];
                    if (event->mask & IN_ALL_EVENTS) { //IN_MODIFY监控不到对进程内存数据的修改

//                        ret = kill(pid, SIGKILL);
LOGE("%s was changed,mask:%x",inotify_file,event->mask);
//LOGD("mask:%x",event->mask);
//                        LOGD("ret:%d", ret);
//                        return;
                    }
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }
    }
    inotify_rm_watch(fd, wd);
    close(fd);
}


void add_inotify(){
    char *pagemap = "/proc/1/pagemap";  //监控不到mprop
    char *mem = "/proc/1/mem";  //监控不到mprop
    char *maps = "/proc/1/maps";    //仅能监测到access/open操作
    char *property = "/dev/__properties__";     //仅能监测到open/NoWritableClose操作

    pthread_t tid_1;
    pthread_t tid_2;
    pthread_t tid_3;
    pthread_t tid_4;

    /**
     * param1:线程标识符
     * param2:线程属性
     * param3:起始函数地址
     * param4:函数参数
     */
    int ret_1 = pthread_create(&tid_1, 0, (void *(*)(void *)) check_inotify, pagemap);
    if (ret_1 >= 0){
        /**
         * 主线程与子线程分离，子线程结束后，资源自动回收
         */
        pthread_detach(tid_1);
    }

    int ret_2 = pthread_create(&tid_2, NULL, (void *(*)(void *)) check_inotify, mem);
    if (ret_2 >= 0){
        pthread_detach(tid_2);
    }

    int ret_3 = pthread_create(&tid_3, NULL, (void *(*)(void *)) check_inotify, maps);
    if (ret_3 >= 0){
        pthread_detach(tid_3);
    }

    int  ret_4 = pthread_create(&tid_4, NULL, (void *(*)(void *)) check_inotify, property);
    if (ret_4 >= 0){
        pthread_detach(tid_4);
    }
}


JNIEXPORT jstring JNICALL
Java_com_yyt_anti_1debug_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject  obj/* this */) {

    char buff[10];
    memset(buff, 0, 10);
    const prop_info *pi = __system_property_find("ro.debuggable");
    if (pi != 0){
        __system_property_read(pi,0,buff);
    }

    if (strcmp(buff,"1") == 0){
        LOGD("ro.debuggable=1");
        LOGE("系统处于可调试状态！");
    } else{
        LOGD("ro.debuggable=0");
    }

    return (*env)->NewStringUTF(env,buff);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }

//    add_inotify();

    pid_t pid = fork();

    if (pid == 0){
        while (1){
            if (checkStatus() > 0){
                break;
            }
            sleep(1);
        }
        exit(0);
    }

    // 返回jni的版本
    return JNI_VERSION_1_4;
}
