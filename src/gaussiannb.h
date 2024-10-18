/* gaussiannb.h
 */
#ifndef GAUSSIANNB_H
#define GAUSSIANNB_H

#include <stdbool.h>

/* structures
 */
typedef struct {
	double *mean;
	double *variance;
	double  prior;
} gaussiannbclass;

typedef struct {
	size_t num_classes;
	size_t num_features;
	gaussiannbclass *classes;
} gaussiannb;

/* function definitions
 */
bool gaussiannb_init(gaussiannb *, size_t, size_t);
void gaussiannb_destroy(gaussiannb);
void gaussiannb_train(gaussiannb *, double **, int *, size_t);
int  gaussiannb_predict(gaussiannb *, double *);
double gaussiannb_mahalanobis_distance(gaussiannb *, double *, size_t);
#endif /* GAUSSIANNB_H */
