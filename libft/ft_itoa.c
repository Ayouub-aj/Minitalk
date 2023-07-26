#include "libft.h"

char	*ft_itoa(int n)
{
	int			sign;
	int			size;
	char		*conv;
	long int	nn;

	sign = n < 0;
	nn = n;
	if (sign > 0)
		nn = -nn;
	size = 1;
	while (n / 10 != 0 && size++)
		n /= 10;
	conv = malloc(sizeof(char) * (size + sign + 1));
	if (!conv)
		return (NULL);
	conv[size + sign] = '\0';
	while (size-- != 0)
	{
		conv[size + sign] = nn % 10 + '0';
		nn /= 10;
	}
	if (sign > 0)
		conv[0] = '-';
	return (conv);
}
