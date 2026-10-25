/* my_find.c — рекурсивний обхід дерева каталогів */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static long long traverse(const char *dirpath, int depth)
{
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror(dirpath);
        return 0;
    }

    long long total_size = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        /* Пропустити "." та ".." */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror(fullpath);
            continue;
        }

        for (int i = 0; i < depth; i++) printf("  ");

        if (S_ISDIR(st.st_mode)) {
            printf("[DIR]  %s\n", entry->d_name);
            long long sub_size = traverse(fullpath, depth + 1);
            total_size += sub_size;
        } else if (S_ISREG(st.st_mode)) {
            printf("[FILE] %s (%lld байт)\n", entry->d_name, (long long)st.st_size);
            total_size += st.st_size;
        } else if (S_ISLNK(st.st_mode)) {
            char target[4096];
            ssize_t len = readlink(fullpath, target, sizeof(target) - 1);
            if (len != -1) {
                target[len] = '\0';
                printf("[LINK] %s -> %s\n", entry->d_name, target);
            } else {
                printf("[LINK] %s -> [помилка readlink]\n", entry->d_name);
            }
        }
    }

    closedir(dir);
    return total_size;
}

int main(int argc, char *argv[])
{
    const char *start = (argc > 1) ? argv[1] : ".";

    printf("Обхід дерева каталогів: %s\n\n", start);
    long long total = traverse(start, 0);
    printf("\nЗагальний розмір: %lld байт (%.2f МБ)\n",
           total, total / (1024.0 * 1024.0));

    return EXIT_SUCCESS;
}
