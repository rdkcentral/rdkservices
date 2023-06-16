/* vi: set expandtab sw=4 sts=4: */
/* opkg_solver_libsolv.h - handle package dependency solving with
   calls to libsolv.

   Copyright (C) 2015 National Instruments Corp.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#ifndef OPKG_SOLVER_LIBSOLV_H
#define OPKG_SOLVER_LIBSOLV_H

#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/queue.h>
#include <solv/solver.h>

#ifdef __cplusplus
extern "C" {
#endif

struct libsolv_solver {
    Solver *solver;
    Queue solver_jobs;
    Pool *pool;
    Repo *repo_installed;
    Repo *repo_available;
    Repo *repo_preferred;
    Repo *repo_to_install;
};
typedef struct libsolv_solver libsolv_solver_t;

struct arch_data {
    char *arch;
    int priority;
};
typedef struct arch_data arch_data_t;

enum job_action {
    JOB_NOOP,
    JOB_INSTALL,
    JOB_REMOVE,
    JOB_UPGRADE,
    JOB_DISTUPGRADE
};
typedef enum job_action job_action_t;

libsolv_solver_t *libsolv_solver_new(void);
void libsolv_solver_free(libsolv_solver_t *libsolv_solver);

void libsolv_solver_add_job(libsolv_solver_t *libsolv_solver,
                            job_action_t action, char *pkg_name);
int libsolv_solver_solve(libsolv_solver_t *libsolv_solver);
int libsolv_solver_execute_transaction(libsolv_solver_t *libsolv_solver);

int opkg_solver_libsolv_perform_action(job_action_t action, int num_pkgs,
                                       char **pkg_names);

#ifdef __cplusplus
}
#endif
#endif    /* OPKG_SOLVER_LIBSOLV_H */
