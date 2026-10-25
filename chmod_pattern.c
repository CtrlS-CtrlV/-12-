/* chmod_pattern.c — зміна прав файлів за розширенням */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

static int ends_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suf_len = strlen(suffix);
    if (suf_len > str_len) return 0;
    return strcmp(str + str_len - suf_len, suffix) == 0;
}

static void format_perms(mode_t mode, char *buf)
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
    if (argc < 3) {
        fprintf(stderr, "Використання: %s <каталог> <розширення>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dirpath = argv[1];
    const char *ext = argv[2];
    int count = 0;

    DIR *dir = opendir(dirpath);
    if (!dir) { perror("opendir"); return EXIT_FAILURE; }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!ends_with(entry->d_name, ext))
            continue;

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == -1) {
            perror(fullpath);
            continue;
        }

        if (!S_ISREG(st.st_mode))
            continue;

        char old_perms[10], new_perms[10];
        format_perms(st.st_mode, old_perms);

        mode_t new_mode = st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;
        if (chmod(fullpath, new_mode) == -1) {
            perror("chmod");
            continue;
        }

        format_perms(new_mode, new_perms);
        printf("  %s: %s -> %s\n", entry->d_name, old_perms, new_perms);
        count++;
    }

    closedir(dir);
    printf("\nЗмінено прав: %d файл(ів) з розширенням '%s'\n", count, ext);

    return EXIT_SUCCESS;
}
