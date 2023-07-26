#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	int		i;
	int		j;
	char	*fresh;

	i = 0;
	if (!s1 || !s2)
		return (NULL);
	j = ft_strlen(s1) + ft_strlen(s2);
	fresh = (char *)malloc(sizeof(char) * j + 1);
	if (fresh == NULL)
		return (0);
	while (s1[i])
	{
		fresh[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
	{
		fresh[i] = s2[j];
		j++;
		i++;
	}
	fresh[i] = '\0';
	return (fresh);
}
