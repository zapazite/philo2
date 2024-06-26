/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-prin <mde-prin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 08:45:24 by mde-prin          #+#    #+#             */
/*   Updated: 2024/06/24 13:27:07 by mde-prin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo_bonus.h"

int	ft_atoui(const char *nptr)
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
		args[i] = ft_atoui(argv[i + 1]);
		if (args[i++] == -1 || args[0] == 0)
			return (1);
	}
	if (argc == 5)
		args[4] = -1;
	return (0);
}

void	ft_thread(t_philo philo)
{
	pthread_create(&philo.tid, NULL, (void *)ft_monitor, &philo);
	pthread_create(&philo.ltid, NULL, (void *)ft_listen, &philo);
	pthread_create(&philo.ptid, NULL, (void *)ft_routine, &philo);
	pthread_join(philo.tid, NULL);
	pthread_join(philo.ltid, NULL);
	pthread_join(philo.ptid, NULL);
	ft_close_sem(philo);
}

void	ft_close_sem(t_philo philo)
{
	sem_close(philo.sem_1);
	sem_close(philo.sem_2);
	sem_close(philo.sem_fork);
	sem_close(philo.sem_dead);
	sem_close(philo.sem_print);
	sem_close(philo.sem_flag);
	sem_close(philo.sem_full);
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
			return (ft_thread(philo));
	}
	i = -1;
	while (++i < NBR)
		sem_wait(philo.sem_full);
	sem_post(philo.sem_dead);
	i = -1;
	while (++i < NBR)
		waitpid(pid[i], &status, 0);
	ft_close_sem(philo);
}

void	ft_log(char *s, t_philo *philo)
{
	int			flag;
	long long	log_time;

	log_time = ft_time_ms() - philo->start_time;
	sem_wait(philo->sem_print);
	if (!ft_is_end(philo))
		printf("%lld %d %s\n", log_time, philo->id, s);
	sem_post(philo->sem_print);
}

void	ft_eat_sleep_think(t_philo *philo)
{
	sem_wait(philo->sem_1);
	sem_wait(philo->sem_fork);
	ft_log("has taken a fork", philo);
	if (philo->nbr_philo == 1)
		return ((void)usleep(philo->time_to_die * 2000));
	sem_wait(philo->sem_2);
	sem_wait(philo->sem_fork);
	ft_log("has taken a fork", philo);
	sem_post(philo->sem_2);
	sem_post(philo->sem_1);
	ft_log("is eating", philo);
	sem_wait(philo->sem_flag);
	philo->time_last_meal = ft_time_ms();
	sem_post(philo->sem_flag);
	if (!--philo->nbr_must_eat)
	{
		sem_post(philo->sem_full);
		philo->nbr_must_eat = -1;
	}
	usleep(philo->time_to_eat * 1000);
	sem_post(philo->sem_fork);
	sem_post(philo->sem_fork);
	ft_log("is sleeping", philo);
	usleep(philo->time_to_sleep * 1000);
	ft_log("is thinking", philo);
}

int	ft_is_end(t_philo *philo)
{
	int	end;

	sem_wait(philo->sem_flag);
	end = philo->end;
	sem_post(philo->sem_flag);
	return (end);
}

void	*ft_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	ft_log("is thinking", philo);
	while (!ft_is_end(philo))
		ft_eat_sleep_think(philo);
	return (NULL);
}

void	*ft_listen(void *arg)
{
	t_philo	*philo;
	int		i;

	philo = (t_philo *)arg;
	sem_wait(philo->sem_dead);
	sem_post(philo->sem_dead);
	sem_wait(philo->sem_flag);
	philo->end = 1;
	sem_post(philo->sem_flag);
	i = -1;
	while (++i < philo->nbr_philo)
		sem_post(philo->sem_full);
	return (NULL);
}
//printf("%lld %d died\n", ft_time_ms() - philo->start_time, philo->id);

int	ft_is_dead(t_philo *philo)
{
	int	dead;

	sem_wait(philo->sem_flag);
	dead = (ft_time_ms() >= philo->time_last_meal + philo->time_to_die);
	sem_post(philo->sem_flag);
	return (dead);
}

void	*ft_monitor(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	while (!ft_is_end(philo))
	{
		sem_wait(philo->sem_print);
		if (ft_is_dead(philo) && !ft_is_end(philo))
		{
			sem_wait(philo->sem_flag);
			philo->end = 1;
			sem_post(philo->sem_flag);
			sem_post(philo->sem_dead);
			printf("%lld %d died\n", ft_time_ms()
				- philo->start_time, philo->id);
			usleep(philo->time_to_die * 2000);
			sem_post(philo->sem_print);
			return (NULL);
		}
		sem_post(philo->sem_print);
		usleep(400);
	}
	return (NULL);
}

void	ft_open_sem(t_philo *philo)
{
	philo->sem_1 = sem_open("/1", O_CREAT, S_IRWXU,
			philo->nbr_philo / 2 + (philo->nbr_philo == 1));
	if (philo->sem_1 == SEM_FAILED)
		exit(1);
	philo->sem_2 = sem_open("/2", O_CREAT, S_IRWXU, 1);
	if (philo->sem_2 == SEM_FAILED)
		exit(1);
	philo->sem_fork = sem_open("/fork", O_CREAT, S_IRWXU, philo->nbr_philo);
	if (philo->sem_fork == SEM_FAILED)
		exit(1);
	philo->sem_dead = sem_open("/dead", O_CREAT, S_IRWXU, 0);
	if (philo->sem_dead == SEM_FAILED)
		exit(1);
	philo->sem_full = sem_open("/full", O_CREAT, S_IRWXU, 0);
	if (philo->sem_full == SEM_FAILED)
		exit(1);
	philo->sem_print = sem_open("/print", O_CREAT, S_IRWXU, 1);
	if (philo->sem_print == SEM_FAILED)
		exit(1);
	philo->sem_flag = sem_open("/flag", O_CREAT, S_IRWXU, 1);
	if (philo->sem_flag == SEM_FAILED)
		exit(1);
}

void	ft_start(long long args[])
{
	t_philo		philo;

	sem_unlink("/1");
	sem_unlink("/2");
	sem_unlink("/fork");
	sem_unlink("/dead");
	sem_unlink("/print");
	sem_unlink("/flag");
	sem_unlink("/full");
	philo.nbr_philo = args[0];
	philo.time_to_die = args[1];
	philo.time_to_eat = args[2];
	philo.time_to_sleep = args[3];
	philo.nbr_must_eat = args[4];
	ft_open_sem(&philo);
	philo.end = 0;
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
