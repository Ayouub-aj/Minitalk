#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	i;
	size_t	len;
	size_t	src_size;
	size_t	dst_size;

	i = 0;
	src_size = ft_strlen(src);
	dst_size = ft_strlen(dst);
	len = dst_size;
	if (size == 0 || size < dst_size)
		return (src_size + size);
	else
	{
		while (src[i] && dst_size + i < size - 1)
		{
			dst[len] = src[i];
			i++;
			len++;
		}
		dst[len] = '\0';
	}
	return (src_size + dst_size);
}
