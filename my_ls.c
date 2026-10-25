/* my_ls.c — спрощений аналог ls -la */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

static char type_char(mode_t mode)
{
    if (S_ISDIR(mode))  return 'd';
    if (S_ISLNK(mode))  return 'l';
    if (S_ISCHR(mode))  return 'c';
    if (S_ISBLK(mode))  return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '-';
}

static void mode_string(mode_t mode, char *buf)
{
    buf[0] = (mode & S_IRUSR) ? 'r' : '-';
    buf[1] = (mode & S_IWUSR) ? 'w' : '-';
    buf[2] = (mode & S_IXUSR) ? 'x' : '-';
    buf[3] = (mode & S_IRGRP) ? 'r' : '-';
    buf[4] = (mode & S_IWGRP) ? 'w' : '-';
    buf[5] = (mode & S_IXGRP) ? 'x' : '-';
    buf[6] = (mode & S_IROTH) ? 'r' : '-';
    buf[7] = (mode & S_IWOTH) ? 'w' : '-';
    buf[8] = (mode & S_IXOTH) ? 'x' : '-';
    buf[9] = '\0';
}

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : ".";

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror("lstat");
            continue;
        }

        char t = type_char(st.st_mode);
        char perms[10];
        mode_string(st.st_mode, perms);

        char time_buf[64];
        struct tm *tm_info = localtime(&st.st_mtim.tv_sec);
        strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", tm_info);

        printf("%c%s  %5ld  %s  %s\n", 
               t, perms, (long)st.st_size, time_buf, entry->d_name);
    }

    closedir(dir);
    return EXIT_SUCCESS;
}
