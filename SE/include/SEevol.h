#ifndef EVOL_T
#define EVOL_T

#define NSP (7)

typedef struct {
    double mass;
    double radius;
    double rho;
    double temperature;
    double pressure;
    double velocity;
    double dcoeff;
    double yps[NSP];
} evol_t;

void SEread_evol(int ifile_id, int cycle, evol_t **shells, int *nshells);

void SEwrite_evol(int ifile_id, int cycle, evol_t *shell, int nshells);

#endif
