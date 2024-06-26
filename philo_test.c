/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_test.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-prin <mde-prin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 08:45:24 by mde-prin          #+#    #+#             */
/*   Updated: 2024/06/24 09:03:53 by mde-prin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

int	ft_atopi(const char *nptr)
{
	long	n;

	n = 0;
	if ((*nptr < '0' || *nptr > '9') && *nptr != '+')
		return (-1);
	if (*nptr == '+')
		(void)*nptr++;
	if (*nptr < '0' || *nptr > '9')
		return (-1);
	while (*nptr >= '0' && *nptr <= '9')
		n = n * 10 + (*nptr++ - '0');
	if (n > INT_MAX)
		return (-1);
	return ((int)n);
}

unsigned long	ft_strlen(const char *s)
{
	unsigned long	l;
	if (!s)
		return (0);
	l = 0;
	while (s[l])
		l++;
	return (l);
}

void	ft_putnbr(int n)
{
	long	nb;
	char	c;

	nb = (long)n;
	if (nb < 0)
	{
		nb = -nb;
		c = '-';
		write(1, &c, 1);
	}
	if (nb > 9)
		ft_putnbr(nb / 10);
	c ='0' + nb % 10;
	write(1, &c, 1);
}

long long	ft_time_ms(void)
{
	struct timeval	tv;
	long long		sec;
	long long		usec;

	gettimeofday(&tv, NULL);
	sec = (long long)tv.tv_sec;
	usec = (long long)tv.tv_usec;
	return ((long long)(sec * 1000 + usec / 1000));
}

int	ft_check(int argc, char *argv[], long long *args)
{
	int	i;

	if (argc < 5 || argc > 6)
		return (1);
	i = 0;
	while (argv[i + 1])
	{
		args[i] = ft_atopi(argv[i + 1]);
		if (args[i++] == -1 || args[0] == 0)
			return (1);
	}
	if (argc == 5)
		args[4] = -1;
	return (0);
}

void	ft_philo_proc(t_philo philo, const int NBR)
{
	int			i;
	int			status;
	int			pid[NBR];

	i = -1;
	while (++i < NBR)
	{
		pid[i] = fork();
		philo.id = i + 1;
		if (pid[i] < 0)
			exit(1);
		if (pid[i] == 0)
		{
			pthread_create(&philo.tid, NULL, (void *)ft_monitor, &philo);
			pthread_create(&philo.ltid, NULL, (void *)ft_listen, &philo);
			ft_routine(&philo);
			pthread_join(philo.tid, NULL);
			pthread_join(philo.ltid, NULL);
			return ;
		}
	}
	i = -1;
	while (++i < NBR)
	{
		waitpid(pid[i], &status, 0);
	}
	sem_close(philo.sem_1);
	sem_close(philo.sem_2);
	sem_close(philo.sem_fork);
	sem_close(philo.sem_dead);
	sem_close(philo.sem_print);
}

void	ft_log(char *s, t_philo *philo)
{
	long long	log_time;

	log_time = ft_time_ms() - philo->start_time;
	sem_wait(philo->sem_print);
	printf("%lld %d %s\n", log_time, philo->id, s);
	sem_post(philo->sem_print);
}

void	ft_routine(t_philo *philo)
{
	ft_log("is thinking", philo);
	while (philo->nbr_must_eat)
	{
		sem_wait(philo->sem_1);
		sem_wait(philo->sem_fork);
		ft_log("has taken a fork", philo);
		sem_wait(philo->sem_2);
		sem_wait(philo->sem_fork);
		ft_log("has taken a fork", philo);
		sem_post(philo->sem_2);
		sem_post(philo->sem_1);
		ft_log("is eating", philo);
		philo->time_last_meal = ft_time_ms();
		philo->nbr_must_eat -= (philo->nbr_must_eat > 0);
		usleep(philo->time_to_eat * 1000);
		sem_post(philo->sem_fork);
		sem_post(philo->sem_fork);
		ft_log("is sleeping", philo);
		usleep(philo->time_to_sleep * 1000);
		ft_log("is thinking", philo);
	}
	sem_close(philo->sem_1);
	sem_close(philo->sem_2);
	sem_close(philo->sem_fork);
	sem_close(philo->sem_dead);
	sem_close(philo->sem_print);
	exit(0);
}

void	*ft_listen(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	sem_wait(philo->sem_dead);
	sem_post(philo->sem_dead);
	exit(0);
}

void	*ft_monitor(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	while (1)
	{
		if (ft_time_ms() >= philo->time_last_meal + philo->time_to_die)
		{
			sem_wait(philo->sem_print);
			printf("%lld %d died\n", ft_time_ms() - philo->start_time, philo->id);
			sem_post(philo->sem_dead);
			return (NULL);
		}
		usleep(100);
	}
}

void	ft_start(long long args[])
{
	t_philo		philo;
	sem_t		*sem_1;
	sem_t		*sem_2;
	sem_t		*sem_full;
	sem_t		*sem_fork;
	sem_t		*sem_dead;
	sem_t		*sem_print;

	sem_unlink("/fork");
	sem_unlink("/dead");
	sem_unlink("/print");
	sem_unlink("/1");
	sem_unlink("/2");
	sem_1 = sem_open("/1", O_CREAT, S_IRWXU, args[0] / 2 + (args[0] == 1));
	if (sem_1 == SEM_FAILED)
		exit(1);
	sem_2 = sem_open("/2", O_CREAT, S_IRWXU, 1);
	if (sem_2 == SEM_FAILED)
		exit(1);
	sem_fork = sem_open("/fork", O_CREAT, S_IRWXU, args[0]);
	if (sem_fork == SEM_FAILED)
		exit(1);
	sem_dead = sem_open("/dead", O_CREAT, S_IRWXU, 0);
	if (sem_dead == SEM_FAILED)
		exit(1);
	sem_print = sem_open("/print", O_CREAT, S_IRWXU, 1);
	if (sem_print == SEM_FAILED)
		exit(1);
	philo.nbr_philo = args[0];
	philo.time_to_die = args[1];
	philo.time_to_eat = args[2];
	philo.time_to_sleep = args[3];
	philo.nbr_must_eat = args[4];
	philo.sem_1 = sem_1;
	philo.sem_2 = sem_2;
	philo.sem_fork = sem_fork;
	philo.sem_dead = sem_dead;
	philo.sem_print = sem_print;
	philo.start_time = ft_time_ms();
	philo.time_last_meal = philo.start_time;
	ft_philo_proc(philo, (const int)args[0]);
}

int	main(int argc, char *argv[])
{
	static long long	args[5];

	if (ft_check(argc, argv, args))
		return (1);
	if (!args[4])
		return (0);
	ft_start(args);
	return (0);
}
