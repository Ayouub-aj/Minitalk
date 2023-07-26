#include "libft.h"

char	*ft_substr(char const *s, unsigned int start, size_t len)
{
	char	*ptr;
	size_t	idx;
	size_t	s_size;

	if (s == NULL)
		return (NULL);
	s_size = ft_strlen(s);
	if (len == 0 || s_size <= start || s_size == 0)
		return ((char *)ft_calloc(1, sizeof(char)));
	if (s_size < len + start)
		len = s_size - start;
	ptr = (char *)ft_calloc(sizeof(char), len + 1);
	if (ptr == NULL)
		return (NULL);
	idx = 0;
	while (idx < len)
	{
		ptr[idx] = s[start + idx];
		idx++;
	}
	ptr[idx] = '\0';
	return (ptr);
}
