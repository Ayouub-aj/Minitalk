#ifndef MINITALK_H
# define MINITALK_H

# include <unistd.h>
# include <signal.h>
# include "libft/libft.h"

void	signalhandle(int sig);
void	kill_sig(int sig, const int pid);
void	string_format(char word, const int pid);

# define ANSI_COLOR_RED		"\x1b[31m"

#endif
