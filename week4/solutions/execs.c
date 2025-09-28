/**
 * https://iafisher.com/cs644/fall2025/week4
 */
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cs644.h"

// TODO: not sure why this isn't declared in unistd.h
int execveat(int dirfd, const char* pathname, char* argv[], char* envp[], int flags);
extern char** environ;

/** Look for `pathname` on the PATH if it does not contain a slash. */
int my_execp(const char* pathname, char* argv[], char* envp[]) {
  char* PATH = getenv("PATH");
  if (strchr(pathname, '/') != NULL || PATH == NULL) {
    return execve(pathname, argv, envp);
  }

  struct cs644_str_vec v = cs644_str_vec_split(PATH, ':');
  for (size_t i = 0; i < cs644_str_vec_len(v); i++) {
    char* pathdir = cs644_str_vec_get(v, i);
    int dirfd = open(pathdir, O_RDONLY | O_DIRECTORY);
    if (dirfd < 0) {
      continue;
    }

    int r = execveat(dirfd, pathname, argv, envp, 0);
    if (r < 0) {
      // for some errors, try the next entry instead of bailing
      // this list is probably not complete
      if (errno == EACCES || errno == EISDIR || errno == ENOENT || errno == ENOEXEC) {
        continue;
      } else {
        return r;
      }
    }
  }

  cs644_str_vec_free(v);
  return ENOENT;
}

char** va_list_to_argv(va_list args) {
  struct cs644_str_vec v = cs644_str_vec_new();
  while (1) {
    char* arg = va_arg(args, char*);
    cs644_str_vec_append(&v, arg);
    if (arg == NULL) {
      break;
    }
  }
  return v.data;
}

int my_execl(const char* pathname, ...) {
  va_list args;
  va_start(args, pathname);
  char** argv = va_list_to_argv(args);
  va_end(args);
  return execve(pathname, argv, environ);
}

int my_execlp(const char* pathname, ...) {
  va_list args;
  va_start(args, pathname);
  char** argv = va_list_to_argv(args);
  va_end(args);
  return my_execp(pathname, argv, environ);
}

int my_execle(const char* pathname, ...) {
  va_list args;
  va_start(args, pathname);
  char** argv = va_list_to_argv(args);
  char** envp = va_arg(args, char**);
  va_end(args);
  return execve(pathname, argv, envp);
}

int my_execv(const char* pathname, char* argv[]) {
  return execve(pathname, argv, environ);
}

int my_execvp(const char* pathname, char* argv[]) {
  return my_execp(pathname, argv, environ);
}

int my_execvpe(const char* pathname, char* argv[], char* envp[]) {
  return my_execp(pathname, argv, environ);
}

#define ARGS0 "/usr/share/cs644/code/README.md", "/usr/share/cs644/code/LICENSE", NULL

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1][0] == '-') {
    return 1;
  }

  char* bat_path = "/usr/bin/bat";
  char* bat_args[] = {"bat", ARGS0};

  char* s = argv[1];
  if (strcmp(s, "l") == 0) {
    return my_execl(bat_path, bat_path, ARGS0);
  } else if (strcmp(s, "lp") == 0) {
    return my_execlp("bat", "bat", ARGS0);
  } else if (strcmp(s, "le") == 0) {
    setenv("BAT_STYLE", "plain", 1);
    return my_execle(bat_path, bat_path, ARGS0, environ);
  } else if (strcmp(s, "v") == 0) {
    return my_execv(bat_path, bat_args);
  } else if (strcmp(s, "vp") == 0) {
    return my_execvp("bat", bat_args);
  } else if (strcmp(s, "vpe") == 0) {
    setenv("BAT_STYLE", "plain", 1);
    return my_execvpe("bat", bat_args, environ);
  } else {
    return 0;
  }
}
