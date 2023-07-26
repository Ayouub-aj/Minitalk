#include "minitalk.h"

void	string_format(char word, pid_t pid)
{
	unsigned char	num;
	int				idx;

	idx = 0;
	num = 0x80;
	while (idx < 8)
	{
		kill_sig(word & num, pid);
		num = num >> 1;
		idx++;
	}
	return ;
}

void	kill_sig(int sig, pid_t pid)
{
	if (sig)
		kill(pid, SIGUSR2);
	else
		kill(pid, SIGUSR1);
	usleep(400);
	return ;
}

int	main(int argc, char *argv[])
{
	pid_t	pid;
	int		len;
	int		i;

	if (argc != 3)
	{
		ft_putstr_fd(ANSI_COLOR_RED"wrong number of arguments !! \n", 2);
		return (0);
	}
	pid = ft_atoi(argv[1]);
	if (!(pid > 100 && pid < 99998))
	{
		ft_putstr_fd(ANSI_COLOR_RED"Process id error !!\n", 2);
		return (0);
	}
	i = 0;
	len = ft_strlen(argv[2]);
	while (i < len)
	{
		string_format(argv[2][i], pid);
		i++;
	}
	return (0);
}
