/* gaussiannb.c
 */
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "gaussiannb.h"

bool gaussiannb_init(gaussiannb *gnb, size_t num_classes, size_t num_features) {
	gnb->num_classes  = num_classes;
	gnb->num_features = num_features;
	gnb->num_samples  = 0;
	gnb->classes      = calloc(num_classes, sizeof(gaussiannbclass));

	if (gnb->classes == NULL) {
		return false;
	}

	// initialize class weights to 1.0
	for (size_t c = 0; c < num_classes; c++) {
		gnb->classes[c].weight = 1.0;
	}

	return true;
}

void gaussiannb_destroy(gaussiannb gnb) {
	if (gnb.num_classes > 0) {
		free(gnb.classes[0].mean);
		free(gnb.classes[0].variance);
	}

	free(gnb.classes);
}

void gaussiannb_train(gaussiannb *gnb, double **X, int *y, size_t num_samples) {
	size_t  bufsiz    = gnb->num_classes * gnb->num_features;
	double *means     = calloc(bufsiz, sizeof(double));
	double *variances = calloc(bufsiz, sizeof(double));

	if (means == NULL || variances == NULL) {
		if (means != NULL) { free(means); }
		if (variances != NULL) { free(variances); }
		return;
	}

	gnb->num_samples += num_samples; // needed for online learning

	for (size_t c = 0; c < gnb->num_classes; c++) {
		gnb->classes[c].count    = 0;
		gnb->classes[c].mean     = means + (c * gnb->num_features);
		gnb->classes[c].variance = variances + (c * gnb->num_features);
		size_t count             = 0;

		// calculate mean and variance of features
		for (size_t i = 0; i < num_samples; i++) {
			if (y[i] == c) {
				count++;
				for (size_t j = 0; j < gnb->num_features; j++) {
					gnb->classes[c].mean[j] += X[i][j];
				}
			}
		}

		for (size_t j = 0; j < gnb->num_features; j++) {
			gnb->classes[c].mean[j] /= count;
		}

		for (size_t i = 0; i < num_samples; i++) {
			if (y[i] == c) {
				for (size_t j = 0; j < gnb->num_features; j++) {
					gnb->classes[c].variance[j] += pow(X[i][j] - gnb->classes[c].mean[j], 2);
				}
			}
		}

		for (size_t j = 0; j < gnb->num_features; j++) {
			if (count == 0) {
				gnb->classes[c].variance[j] = GNB_EPSILON;
			} else {
				// regularization to avoid overfitting
				gnb->classes[c].variance[j] = (gnb->classes[c].variance[j] / count) + GNB_ALPHA;
			}
		}

		// laplace smoothing using class weight
		gnb->classes[c].prior = (double)(count + gnb->classes[c].weight) / (num_samples + gnb->num_classes);
	}
}

int gaussiannb_predict(gaussiannb *gnb, double *X) {
	double best_posterior = -INFINITY;
	int    best_class     = -1;

	for (size_t c = 0; c < gnb->num_classes; c++) {
		double log_prob = log(gnb->classes[c].prior * gnb->classes[c].weight);

		// calculate log(probabilities) of features
		for (size_t j = 0; j < gnb->num_features; j++) {
			double mean = gnb->classes[c].mean[j];
			double var  = gnb->classes[c].variance[j] + GNB_EPSILON;
			double prob = (1 / sqrt(2 * M_PI * var)) * exp(-pow(X[j] - mean, 2) / (2 * var));
			log_prob += log(prob);
		}

		if (log_prob > best_posterior) {
			best_posterior = log_prob;
			best_class = c;
		}
	}

	return best_class;
}

double gaussiannb_mahalanobis_distance(gaussiannb *gnb, double *X, size_t class_index) {
	double distance = 0.0;

	for (size_t i = 0; i < gnb->num_features; i++) {
		double diff = X[i] - gnb->classes[class_index].mean[i];
		distance += (diff * diff) / (gnb->classes[class_index].variance[i] + GNB_EPSILON);
	}

	return sqrt(distance);
}

void gaussiannb_update(gaussiannb *gnb, double *X, int y) {
	// handle invalid input
	if (X == NULL || gnb == NULL || y >= gnb->num_classes || y < 0) {
		return;
	}

	for (size_t i = 0; i < gnb->num_features; i++) {
		double old_mean = gnb->classes[y].mean[i];
		gnb->classes[y].mean[i] += (X[i] - old_mean) / (gnb->classes[y].count + 1);
		double old_variance = gnb->classes[y].variance[i];
		gnb->classes[y].variance[i] = (gnb->classes[y].count * old_variance + (X[i] - old_mean) * (X[i] - gnb->classes[y].mean[i])) / (gnb->classes[y].count + 1);
	}

	gnb->num_samples++;

	gnb->classes[y].count++;
	gnb->classes[y].prior = (double)(gnb->classes[y].count + 1) / (gnb->num_samples + gnb->num_classes);
}

void gaussiannb_adjust_weight(gaussiannb *gnb, int class_index, double weight) {
	if (class_index >= 0 && class_index < gnb->num_classes) {
		gnb->classes[class_index].weight = weight;
	}
	// class_index out of range ...
}
