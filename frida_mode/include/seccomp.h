#ifndef _SECCOMP_H
#define _SECCOMP_H

#ifndef __APPLE__

  #include <stdint.h>
  #include <linux/filter.h>

  #include "frida-gumjs.h"

  /******************************************************************************/
  #define PR_SET_NO_NEW_PRIVS 38

  #define SECCOMP_SET_MODE_STRICT 0
  #define SECCOMP_SET_MODE_FILTER 1
  #define SECCOMP_GET_ACTION_AVAIL 2
  #define SECCOMP_GET_NOTIF_SIZES 3

  #define SECCOMP_IOC_MAGIC '!'
  #define SECCOMP_IO(nr) _IO(SECCOMP_IOC_MAGIC, nr)
  #define SECCOMP_IOR(nr, type) _IOR(SECCOMP_IOC_MAGIC, nr, type)
  #define SECCOMP_IOW(nr, type) _IOW(SECCOMP_IOC_MAGIC, nr, type)
  #define SECCOMP_IOWR(nr, type) _IOWR(SECCOMP_IOC_MAGIC, nr, type)

  /* Flags for seccomp notification fd ioctl. */
  #define SECCOMP_IOCTL_NOTIF_RECV SECCOMP_IOWR(0, struct seccomp_notif)
  #define SECCOMP_IOCTL_NOTIF_SEND SECCOMP_IOWR(1, struct seccomp_notif_resp)
  #define SECCOMP_IOCTL_NOTIF_ID_VALID SECCOMP_IOW(2, __u64)

  #define SECCOMP_FILTER_FLAG_NEW_LISTENER (1UL << 3)
  #define SECCOMP_RET_ALLOW 0x7fff0000U
  #define SECCOMP_RET_USER_NOTIF 0x7fc00000U

  #define SYS_seccomp __NR_seccomp
  #ifndef __NR_seccomp
    #if defined(__arm__)
      #define __NR_seccomp 383
    #elif defined(__aarch64__)
      #define __NR_seccomp 277
    #elif defined(__x86_64__)
      #define __NR_seccomp 317
    #elif defined(__i386__)
      #define __NR_seccomp 354
    #else
      #pragma error "Unsupported architecture"
    #endif
  #endif

  #define SECCOMP_USER_NOTIF_FLAG_CONTINUE (1UL << 0)

struct seccomp_notif_resp {

  __u64 id;
  __s64 val;
  __s32 error;
  __u32 flags;

};

struct seccomp_data {

  int   nr;
  __u32 arch;
  __u64 instruction_pointer;
  __u64 args[6];

};

struct seccomp_notif {

  __u64               id;
  __u32               pid;
  __u32               flags;
  struct seccomp_data data;

};

struct seccomp_notif_sizes {

  __u16 seccomp_notif;
  __u16 seccomp_notif_resp;
  __u16 seccomp_data;

};

  /******************************************************************************/

  #define SECCOMP_SOCKET_SEND_FD 0x1D3
  #define SECCOMP_SOCKET_RECV_FD 0x1D4

  #define SECCOMP_OUTPUT_FILE_FD 0x1D5
  #define SECCOMP_PARENT_EVENT_FD 0x1D6

enum {

  SYS_READ = 0,
  SYS_WRITE = 1,
  SYS_OPEN = 2,
  SYS_CLOSE = 3,
  SYS_STAT = 4,
  SYS_FSTAT = 5,
  SYS_LSTAT = 6,
  SYS_POLL = 7,
  SYS_LSEEK = 8,
  SYS_MMAP = 9,
  SYS_MPROTECT = 10,
  SYS_MUNMAP = 11,
  SYS_BRK = 12,
  SYS_RT_SIGACTION = 13,
  SYS_RT_SIGPROCMASK = 14,
  SYS_RT_SIGRETURN = 15,
  SYS_IOCTL = 16,
  SYS_PREAD64 = 17,
  SYS_PWRITE64 = 18,
  SYS_READV = 19,
  SYS_WRITEV = 20,
  SYS_ACCESS = 21,
  SYS_PIPE = 22,
  SYS_SELECT = 23,
  SYS_SCHED_YIELD = 24,
  SYS_MREMAP = 25,
  SYS_MSYNC = 26,
  SYS_MINCORE = 27,
  SYS_MADVISE = 28,
  SYS_SHMGET = 29,
  SYS_SHMAT = 30,
  SYS_SHMCTL = 31,
  SYS_DUP = 32,
  SYS_DUP2 = 33,
  SYS_PAUSE = 34,
  SYS_NANOSLEEP = 35,
  SYS_GETITIMER = 36,
  SYS_ALARM = 37,
  SYS_SETITIMER = 38,
  SYS_GETPID = 39,
  SYS_SENDFILE = 40,
  SYS_SOCKET = 41,
  SYS_CONNECT = 42,
  SYS_ACCEPT = 43,
  SYS_SENDTO = 44,
  SYS_RECVFROM = 45,
  SYS_SENDMSG = 46,
  SYS_RECVMSG = 47,
  SYS_SHUTDOWN = 48,
  SYS_BIND = 49,
  SYS_LISTEN = 50,
  SYS_GETSOCKNAME = 51,
  SYS_GETPEERNAME = 52,
  SYS_SOCKETPAIR = 53,
  SYS_SETSOCKOPT = 54,
  SYS_GETSOCKOPT = 55,
  SYS_CLONE = 56,
  SYS_FORK = 57,
  SYS_VFORK = 58,
  SYS_EXECVE = 59,
  SYS_EXIT = 60,
  SYS_WAIT4 = 61,
  SYS_KILL = 62,
  SYS_UNAME = 63,
  SYS_SEMGET = 64,
  SYS_SEMOP = 65,
  SYS_SEMCTL = 66,
  SYS_SHMDT = 67,
  SYS_MSGGET = 68,
  SYS_MSGSND = 69,
  SYS_MSGRCV = 70,
  SYS_MSGCTL = 71,
  SYS_FCNTL = 72,
  SYS_FLOCK = 73,
  SYS_FSYNC = 74,
  SYS_FDATASYNC = 75,
  SYS_TRUNCATE = 76,
  SYS_FTRUNCATE = 77,
  SYS_GETDENTS = 78,
  SYS_GETCWD = 79,
  SYS_CHDIR = 80,
  SYS_FCHDIR = 81,
  SYS_RENAME = 82,
  SYS_MKDIR = 83,
  SYS_RMDIR = 84,
  SYS_CREAT = 85,
  SYS_LINK = 86,
  SYS_UNLINK = 87,
  SYS_SYMLINK = 88,
  SYS_READLINK = 89,
  SYS_CHMOD = 90,
  SYS_FCHMOD = 91,
  SYS_CHOWN = 92,
  SYS_FCHOWN = 93,
  SYS_LCHOWN = 94,
  SYS_UMASK = 95,
  SYS_GETTIMEOFDAY = 96,
  SYS_GETRLIMIT = 97,
  SYS_GETRUSAGE = 98,
  SYS_SYSINFO = 99,
  SYS_TIMES = 100,
  SYS_PTRACE = 101,
  SYS_GETUID = 102,
  SYS_SYSLOG = 103,
  SYS_GETGID = 104,
  SYS_SETUID = 105,
  SYS_SETGID = 106,
  SYS_GETEUID = 107,
  SYS_GETEGID = 108,
  SYS_SETPGID = 109,
  SYS_GETPPID = 110,
  SYS_GETPGRP = 111,
  SYS_SETSID = 112,
  SYS_SETREUID = 113,
  SYS_SETREGID = 114,
  SYS_GETGROUPS = 115,
  SYS_SETGROUPS = 116,
  SYS_SETRESUID = 117,
  SYS_GETRESUID = 118,
  SYS_SETRESGID = 119,
  SYS_GETRESGID = 120,
  SYS_GETPGID = 121,
  SYS_SETFSUID = 122,
  SYS_SETFSGID = 123,
  SYS_GETSID = 124,
  SYS_CAPGET = 125,
  SYS_CAPSET = 126,
  SYS_RT_SIGPENDING = 127,
  SYS_RT_SIGTIMEDWAIT = 128,
  SYS_RT_SIGQUEUEINFO = 129,
  SYS_RT_SIGSUSPEND = 130,
  SYS_SIGALTSTACK = 131,
  SYS_UTIME = 132,
  SYS_MKNOD = 133,
  SYS_USELIB = 134,
  SYS_PERSONALITY = 135,
  SYS_USTAT = 136,
  SYS_STATFS = 137,
  SYS_FSTATFS = 138,
  SYS_SYSFS = 139,
  SYS_GETPRIORITY = 140,
  SYS_SETPRIORITY = 141,
  SYS_SCHED_SETPARAM = 142,
  SYS_SCHED_GETPARAM = 143,
  SYS_SCHED_SETSCHEDULER = 144,
  SYS_SCHED_GETSCHEDULER = 145,
  SYS_SCHED_GET_PRIORITY_MAX = 146,
  SYS_SCHED_GET_PRIORITY_MIN = 147,
  SYS_SCHED_RR_GET_INTERVAL = 148,
  SYS_MLOCK = 149,
  SYS_MUNLOCK = 150,
  SYS_MLOCKALL = 151,
  SYS_MUNLOCKALL = 152,
  SYS_VHANGUP = 153,
  SYS_MODIFY_LDT = 154,
  SYS_PIVOT_ROOT = 155,
  SYS__SYSCTL = 156,
  SYS_PRCTL = 157,
  SYS_ARCH_PRCTL = 158,
  SYS_ADJTIMEX = 159,
  SYS_SETRLIMIT = 160,
  SYS_CHROOT = 161,
  SYS_SYNC = 162,
  SYS_ACCT = 163,
  SYS_SETTIMEOFDAY = 164,
  SYS_MOUNT = 165,
  SYS_UMOUNT2 = 166,
  SYS_SWAPON = 167,
  SYS_SWAPOFF = 168,
  SYS_REBOOT = 169,
  SYS_SETHOSTNAME = 170,
  SYS_SETDOMAINNAME = 171,
  SYS_IOPL = 172,
  SYS_IOPERM = 173,
  SYS_CREATE_MODULE = 174,
  SYS_INIT_MODULE = 175,
  SYS_DELETE_MODULE = 176,
  SYS_GET_KERNEL_SYMS = 177,
  SYS_QUERY_MODULE = 178,
  SYS_QUOTACTL = 179,
  SYS_NFSSERVCTL = 180,
  SYS_GETPMSG = 181,
  SYS_PUTPMSG = 182,
  SYS_AFS_SYSCALL = 183,
  SYS_TUXCALL = 184,
  SYS_SECURITY = 185,
  SYS_GETTID = 186,
  SYS_READAHEAD = 187,
  SYS_SETXATTR = 188,
  SYS_LSETXATTR = 189,
  SYS_FSETXATTR = 190,
  SYS_GETXATTR = 191,
  SYS_LGETXATTR = 192,
  SYS_FGETXATTR = 193,
  SYS_LISTXATTR = 194,
  SYS_LLISTXATTR = 195,
  SYS_FLISTXATTR = 196,
  SYS_REMOVEXATTR = 197,
  SYS_LREMOVEXATTR = 198,
  SYS_FREMOVEXATTR = 199,
  SYS_TKILL = 200,
  SYS_TIME = 201,
  SYS_FUTEX = 202,
  SYS_SCHED_SETAFFINITY = 203,
  SYS_SCHED_GETAFFINITY = 204,
  SYS_SET_THREAD_AREA = 205,
  SYS_IO_SETUP = 206,
  SYS_IO_DESTROY = 207,
  SYS_IO_GETEVENTS = 208,
  SYS_IO_SUBMIT = 209,
  SYS_IO_CANCEL = 210,
  SYS_GET_THREAD_AREA = 211,
  SYS_LOOKUP_DCOOKIE = 212,
  SYS_EPOLL_CREATE = 213,
  SYS_EPOLL_CTL_OLD = 214,
  SYS_EPOLL_WAIT_OLD = 215,
  SYS_REMAP_FILE_PAGES = 216,
  SYS_GETDENTS64 = 217,
  SYS_SET_TID_ADDRESS = 218,
  SYS_RESTART_SYSCALL = 219,
  SYS_SEMTIMEDOP = 220,
  SYS_FADVISE64 = 221,
  SYS_TIMER_CREATE = 222,
  SYS_TIMER_SETTIME = 223,
  SYS_TIMER_GETTIME = 224,
  SYS_TIMER_GETOVERRUN = 225,
  SYS_TIMER_DELETE = 226,
  SYS_CLOCK_SETTIME = 227,
  SYS_CLOCK_GETTIME = 228,
  SYS_CLOCK_GETRES = 229,
  SYS_CLOCK_NANOSLEEP = 230,
  SYS_EXIT_GROUP = 231,
  SYS_EPOLL_WAIT = 232,
  SYS_EPOLL_CTL = 233,
  SYS_TGKILL = 234,
  SYS_UTIMES = 235,
  SYS_VSERVER = 236,
  SYS_MBIND = 237,
  SYS_SET_MEMPOLICY = 238,
  SYS_GET_MEMPOLICY = 239,
  SYS_MQ_OPEN = 240,
  SYS_MQ_UNLINK = 241,
  SYS_MQ_TIMEDSEND = 242,
  SYS_MQ_TIMEDRECEIVE = 243,
  SYS_MQ_NOTIFY = 244,
  SYS_MQ_GETSETATTR = 245,
  SYS_KEXEC_LOAD = 246,
  SYS_WAITID = 247,
  SYS_ADD_KEY = 248,
  SYS_REQUEST_KEY = 249,
  SYS_KEYCTL = 250,
  SYS_IOPRIO_SET = 251,
  SYS_IOPRIO_GET = 252,
  SYS_INOTIFY_INIT = 253,
  SYS_INOTIFY_ADD_WATCH = 254,
  SYS_INOTIFY_RM_WATCH = 255,
  SYS_MIGRATE_PAGES = 256,
  SYS_OPENAT = 257,
  SYS_MKDIRAT = 258,
  SYS_MKNODAT = 259,
  SYS_FCHOWNAT = 260,
  SYS_FUTIMESAT = 261,
  SYS_NEWFSTATAT = 262,
  SYS_UNLINKAT = 263,
  SYS_RENAMEAT = 264,
  SYS_LINKAT = 265,
  SYS_SYMLINKAT = 266,
  SYS_READLINKAT = 267,
  SYS_FCHMODAT = 268,
  SYS_FACCESSAT = 269,
  SYS_PSELECT6 = 270,
  SYS_PPOLL = 271,
  SYS_UNSHARE = 272,
  SYS_SET_ROBUST_LIST = 273,
  SYS_GET_ROBUST_LIST = 274,
  SYS_SPLICE = 275,
  SYS_TEE = 276,
  SYS_SYNC_FILE_RANGE = 277,
  SYS_VMSPLICE = 278,
  SYS_MOVE_PAGES = 279,
  SYS_UTIMENSAT = 280,
  SYS_EPOLL_PWAIT = 281,
  SYS_SIGNALFD = 282,
  SYS_TIMERFD_CREATE = 283,
  SYS_EVENTFD = 284,
  SYS_FALLOCATE = 285,
  SYS_TIMERFD_SETTIME = 286,
  SYS_TIMERFD_GETTIME = 287,
  SYS_ACCEPT4 = 288,
  SYS_SIGNALFD4 = 289,
  SYS_EVENTFD2 = 290,
  SYS_EPOLL_CREATE1 = 291,
  SYS_DUP3 = 292,
  SYS_PIPE2 = 293,
  SYS_INOTIFY_INIT1 = 294,
  SYS_PREADV = 295,
  SYS_PWRITEV = 296,
  SYS_RT_TGSIGQUEUEINFO = 297,
  SYS_PERF_EVENT_OPEN = 298,
  SYS_RECVMMSG = 299,
  SYS_FANOTIFY_INIT = 300,
  SYS_FANOTIFY_MARK = 301,
  SYS_PRLIMIT64 = 302

};

typedef void (*seccomp_child_func_t)(int event_fd, void *ctx);

typedef void (*seccomp_filter_callback_t)(struct seccomp_notif *     req,
                                          struct seccomp_notif_resp *resp,
                                          GumReturnAddressArray *    frames);

void seccomp_atomic_set(volatile bool *ptr, bool val);
bool seccomp_atomic_try_set(volatile bool *ptr, bool val);
void seccomp_atomic_wait(volatile bool *ptr, bool val);

void seccomp_callback_parent(void);
void seccomp_callback_initialize(void);

void seccomp_child_run(seccomp_child_func_t child_func, void *ctx, pid_t *child,
                       int *event_fd);
void seccomp_child_wait(int event_fd);

int  seccomp_event_create(void);
void seccomp_event_signal(int fd);
void seccomp_event_wait(int fd);
void seccomp_event_destroy(int fd);

int  seccomp_filter_install(pid_t child);
void seccomp_filter_child_install(void);
void seccomp_filter_run(int fd, seccomp_filter_callback_t callback);

void seccomp_print(char *format, ...);

void seccomp_socket_create(int *sock);
void seccomp_socket_send(int sockfd, int fd);
int  seccomp_socket_recv(int sockfd);

char *seccomp_syscall_lookup(int id);

#endif
extern char *seccomp_filename;

void seccomp_config(void);
void seccomp_init(void);
void seccomp_on_fork(void);

#endif

