#include "libft.h"

char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	unsigned int	slen;
	unsigned int	i;
	char			*result;

	if ((f == NULL) || (s == NULL))
		return (0);
	slen = ft_strlen(s);
	result = (char *)malloc(sizeof(char) * (slen + 1));
	if (result == NULL)
		return (0);
	i = 0;
	while (s[i])
	{
		result[i] = f(i, s[i]);
		i++;
	}
	result[i] = '\0';
	return (result);
}
