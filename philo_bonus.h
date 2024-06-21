/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_bonus.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-prin <mde-prin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 09:09:28 by mde-prin          #+#    #+#             */
/*   Updated: 2024/06/21 12:37:00 by mde-prin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct s_philo
{
	int				id;
	int				dead;
	int				print;
	int				nbr_philo;
	int				nbr_must_eat;
	sem_t			*sem_1;
	sem_t			*sem_2;
	sem_t			*sem_fork;
	sem_t			*sem_dead;
	sem_t			*sem_print;
	sem_t			*sem_flag;
	pthread_t		tid;
	pthread_t		ltid;
	pthread_t		ptid;
	long long		start_time;
	long long		time_to_die;
	long long		time_to_eat;
	long long		time_to_sleep;
	long long		time_last_meal;
}	t_philo;

int		ft_atopi(const char *nptr);
int		ft_check(int argc, char *argv[], long long *args);
void	ft_philo_proc(t_philo philo, const int NBR);
void	ft_log(char *s, t_philo *philo);
void	*ft_routine(void *arg);
void	*ft_listen(void *arg);
void	*ft_monitor(void *arg);
void	ft_start(long long *args);