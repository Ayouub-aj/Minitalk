#include "minitalk.h"

void	signalhandle(int sig)
{
	static char	receive = 0;
	static int	idx = 0;

	receive = receive << 1;
	if (sig == SIGUSR1)
		receive = receive + 0;
	else if (sig == SIGUSR2)
		receive = receive + 1;
	idx++;
	if (idx == 8)
	{
		ft_putchar_fd(receive, 1);
		receive = 0;
		idx = 0;
	}
	return ;
}

int	main(void)
{
	write(1, "PID: ", 5);
	ft_putnbr_fd(getpid(), 1);
	ft_putstr_fd("\n", 1);
	signal(SIGUSR1, signalhandle);
	signal(SIGUSR2, signalhandle);
	while (1)
		pause ();
	return (0);
}
