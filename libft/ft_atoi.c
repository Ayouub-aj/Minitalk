#include "libft.h"

int	ft_atoi(const char *nptr)
{
	int					sign;
	unsigned long long	result;

	sign = 1;
	result = 0;
	while (*nptr == 32 || (*nptr >= 9 && *nptr <= 13))
		nptr++;
	if (*nptr == '-')
		sign = sign * -1;
	if (*nptr == '-' || *nptr == '+')
		nptr++;
	while (*nptr >= '0' && *nptr <= '9')
	{
		result = result * 10 + *nptr - '0';
		nptr++;
	}
	if (sign == -1 && result > 9223372036854775807)
		return (0);
	if (result > 9223372036854775807)
		return (-1);
	return (result * sign);
}
