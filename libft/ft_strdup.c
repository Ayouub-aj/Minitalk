#include "libft.h"

char	*ft_strdup(const char *s)
{
	char	*clone;
	char	*pclone;

	clone = malloc(ft_strlen(s) + 1);
	if (!clone)
		return (NULL);
	pclone = clone;
	while (*s)
	{
		*clone++ = *s++;
	}
	*clone = '\0';
	return (pclone);
}
