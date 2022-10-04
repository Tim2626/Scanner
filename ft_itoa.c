#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void	ft_strrev(char *str)
{
	size_t	i;
	size_t	taille;
	char	c;

	i = 0;
	taille = strlen(str) - 1;
	while (i < taille / 2 + 1)
	{
		c = str[i];
		str[i] = str[taille - i];
		str[taille - i] = c;
		i = i + 1;
	}
}

int	creer(char *str, int n, int i)
{
	while (n > 0)
	{
		str[i] = n % 10 + 48;
		i = i + 1;
		n = n / 10;
	}
	return (i);
}

char	*ft_itoa(int n)
{
	char	*str;
	int		i;

	if (n == 0)
		return (strdup("0"));
	if (n == -2147483648)
		return (strdup("-2147483648"));
	str = malloc(12 * sizeof(char));
	if (str == NULL)
		return (NULL);
	i = 0;
	if (n < 0)
	{
		str[i] = '-';
		n = n * -1;
		i = i + 1;
	}
	i = creer(str, n, i);
	str[i] = '\0';
	if (str[0] == '-')
		ft_strrev(&str[1]);
	else
		ft_strrev(str);
	return (str);
}