/* link_tool.c — утиліта для роботи з посиланнями */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

static void print_file_info(const char *label, const char *path, int use_lstat)
{
    struct stat st;
    int rc = use_lstat ? lstat(path, &st) : stat(path, &st);

    if (rc == -1) {
        fprintf(stderr, "  %s: %s — %s\n", label, path, strerror(errno));
        return;
    }

    printf("  %s: %s\n", label, path);
    printf("    inode:        %lu\n", (unsigned long)st.st_ino);
    printf("    тип:          %s\n",
           S_ISREG(st.st_mode)  ? "звичайний файл" :
           S_ISDIR(st.st_mode)  ? "каталог" :
           S_ISLNK(st.st_mode)  ? "символічне посилання" : "інше");
    printf("    розмір:       %ld\n", (long)st.st_size);
    printf("    hard links:   %lu\n", (unsigned long)st.st_nlink);
}

static void demo_hardlink(const char *original)
{
    printf("\n=== Жорсткі посилання ===\n");
    const char *hardlink = "/tmp/sp_hardlink_demo";

    unlink(hardlink); // видалити попереднє, якщо залишилось
    if (link(original, hardlink) == -1) {
        perror("link");
        return;
    }

    print_file_info("original", original, 0);
    print_file_info("hardlink", hardlink, 0);

    unlink(hardlink);
}

static void demo_symlink(const char *original)
{
    printf("\n=== Символічні посилання ===\n");
    const char *sym = "/tmp/sp_symlink_demo";

    unlink(sym);
    if (symlink(original, sym) == -1) {
        perror("symlink");
        return;
    }

    printf("\n  Порівняння stat() vs lstat():\n");
    print_file_info("stat()", sym, 0);
    print_file_info("lstat()", sym, 1);

    char target[4096];
    ssize_t len = readlink(sym, target, sizeof(target) - 1);
    if (len != -1) {
        target[len] = '\0';
        printf("  readlink() ціль: %s\n", target);
    }

    unlink(sym);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Використання: %s <файл>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *original = argv[1];

    printf("Початковий стан файлу:\n");
    print_file_info("original", original, 0);

    demo_hardlink(original);
    demo_symlink(original);

    printf("\n=== Підсумок ===\n");
    printf("Жорстке посилання: той самий inode, st_nlink збільшується.\n");
    printf("Символічне посилання: окремий inode, містить шлях до цілі.\n");
    printf("stat() розіменовує посилання, lstat() — ні.\n");

    return EXIT_SUCCESS;
}
