#include "libft.h"

void	*ft_calloc(size_t count, size_t size)
{
	void	*ar;

	ar = malloc (count * size);
	if (!ar)
		return (NULL);
	ft_bzero (ar, (count * size));
	return (ar);
}
