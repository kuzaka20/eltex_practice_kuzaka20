#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_EVENTS 10
#define FIFO_DIR "/tmp"
#define CMD_BUF 128

typedef enum { AVAILABLE, BUSY } driver_status_t;

void driver_loop(pid_t pid) {
    char fifo_in[64], fifo_out[64];
    snprintf(fifo_in, sizeof(fifo_in), "%s/driver_%d_in.fifo", FIFO_DIR, pid);
    snprintf(fifo_out, sizeof(fifo_out), "%s/driver_%d_out.fifo", FIFO_DIR, pid);

    if (mkfifo(fifo_in, 0666) < 0 && errno != EEXIST) { perror("mkfifo in"); exit(1); }
    if (mkfifo(fifo_out, 0666) < 0 && errno != EEXIST) { perror("mkfifo out"); exit(1); }

    int fd_in = open(fifo_in, O_RDWR);
    int fd_out = open(fifo_out, O_RDWR);
    if (fd_in < 0 || fd_out < 0) { perror("open fifo"); exit(1); }

    int epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create1"); exit(1); }

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = fd_in;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd_in, &ev) < 0) perror("epoll_ctl");

    int timer_fd = -1;
    driver_status_t status = AVAILABLE;
    int busy_time = 0;

    while(1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) { perror("epoll_wait"); continue; }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == fd_in) {
                char buf[CMD_BUF];
                ssize_t r = read(fd_in, buf, sizeof(buf)-1);
                if (r <= 0) continue;
                buf[r] = '\0';

                if (strncmp(buf, "TASK ", 5) == 0) {
                    int t = atoi(buf + 5);
                    char msg[64];
                    if (status == BUSY) {
                        snprintf(msg, sizeof(msg), "Busy %d\n", busy_time);
                        if (write(fd_out, msg, strlen(msg)) < 0) perror("write out");
                    } else {
                        status = BUSY;
                        busy_time = t;

                        timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
                        if (timer_fd < 0) { perror("timerfd_create"); continue; }

                        struct itimerspec ts;
                        ts.it_value.tv_sec = t;
                        ts.it_value.tv_nsec = 0;
                        ts.it_interval.tv_sec = 0;
                        ts.it_interval.tv_nsec = 0;
                        if (timerfd_settime(timer_fd, 0, &ts, NULL) < 0) perror("timerfd_settime");

                        ev.events = EPOLLIN;
                        ev.data.fd = timer_fd;
                        if (epoll_ctl(epfd, EPOLL_CTL_ADD, timer_fd, &ev) < 0) perror("epoll_ctl");

                        snprintf(msg, sizeof(msg), "Task accepted for %d seconds\n", t);
                        if (write(fd_out, msg, strlen(msg)) < 0) perror("write out");
                    }
                } else if (strncmp(buf, "STATUS", 6) == 0) {
                    char msg[64];
                    if (status == BUSY)
                        snprintf(msg, sizeof(msg), "Busy %d\n", busy_time);
                    else
                        snprintf(msg, sizeof(msg), "Available\n");
                    if (write(fd_out, msg, strlen(msg)) < 0) perror("write out");
                }
            } else if (events[i].data.fd == timer_fd) {
                uint64_t expirations;
                if (read(timer_fd, &expirations, sizeof(expirations)) < 0) perror("read timer_fd");
                status = AVAILABLE;
                busy_time = 0;
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, timer_fd, NULL) < 0) perror("epoll_ctl");
                close(timer_fd);
                timer_fd = -1;
            }
        }
    }
}

void create_driver() {
    pid_t pid = fork();
    if (pid == 0) {
        driver_loop(getpid());
        exit(0);
    } else if (pid > 0) {
        printf("Driver created with PID %d\n", pid);
        char fifo_in[64], fifo_out[64];
        snprintf(fifo_in, sizeof(fifo_in), "%s/driver_%d_in.fifo", FIFO_DIR, pid);
        snprintf(fifo_out, sizeof(fifo_out), "%s/driver_%d_out.fifo", FIFO_DIR, pid);
        mkfifo(fifo_in, 0666);
        mkfifo(fifo_out, 0666);
    } else {
        perror("fork");
    }
}

void send_task(pid_t pid, int t) {
    char fifo_in[64], fifo_out[64], buf[128];
    snprintf(fifo_in, sizeof(fifo_in), "%s/driver_%d_in.fifo", FIFO_DIR, pid);
    snprintf(fifo_out, sizeof(fifo_out), "%s/driver_%d_out.fifo", FIFO_DIR, pid);

    int fd_in = open(fifo_in, O_WRONLY);
    int fd_out = open(fifo_out, O_RDONLY);
    if (fd_in < 0 || fd_out < 0) { perror("open fifo"); return; }

    snprintf(buf, sizeof(buf), "TASK %d", t);
    if (write(fd_in, buf, strlen(buf)) < 0) perror("write in");

    ssize_t r = read(fd_out, buf, sizeof(buf)-1);
    if (r > 0) {
        buf[r] = '\0';
        printf("%s", buf);
    }

    close(fd_in);
    close(fd_out);
}

void get_status(pid_t pid) {
    char fifo_in[64], fifo_out[64], buf[128];
    snprintf(fifo_in, sizeof(fifo_in), "%s/driver_%d_in.fifo", FIFO_DIR, pid);
    snprintf(fifo_out, sizeof(fifo_out), "%s/driver_%d_out.fifo", FIFO_DIR, pid);

    int fd_in = open(fifo_in, O_WRONLY);
    int fd_out = open(fifo_out, O_RDONLY);
    if (fd_in < 0 || fd_out < 0) { perror("open fifo"); return; }

    if (write(fd_in, "STATUS", 6) < 0) perror("write in");

    ssize_t r = read(fd_out, buf, sizeof(buf)-1);
    if (r > 0) {
        buf[r] = '\0';
        printf("Driver %d: %s", pid, buf);
    }

    close(fd_in);
    close(fd_out);
}

int main() {
    char cmd[128];
    while (1) {
        printf("CLI> ");
        fflush(stdout);
        if (!fgets(cmd, sizeof(cmd), stdin)) break;

        if (strncmp(cmd, "create_driver", 13) == 0) {
            create_driver();
        } else if (strncmp(cmd, "send_task", 9) == 0) {
            pid_t pid;
            int t;
            sscanf(cmd+10, "%d %d", &pid, &t);
            send_task(pid, t);
        } else if (strncmp(cmd, "get_status", 10) == 0) {
            pid_t pid;
            sscanf(cmd+11, "%d", &pid);
            get_status(pid);
        } else if (strncmp(cmd, "exit", 4) == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }
    return 0;
}