#ifndef NEURAL_NETWORK_H_
#define NEURAL_NETWORK_H_

#include "../data_file/data_file.h"

typedef struct neural_network_t_
{
    float b[DATA_LABELS];
    float W[DATA_LABELS][DATA_IMAGE_SIZE];
} neural_network_t;

typedef struct neural_network_gradient_t_
{
    float b_grad[DATA_LABELS];
    float W_grad[DATA_LABELS][DATA_IMAGE_SIZE];
} neural_network_gradient_t;

void neural_network_random_weights(neural_network_t *network);
void neural_network_hypothesis(data_image_t *image, neural_network_t *network, float activations[DATA_LABELS]);
float neural_network_gradient_update(data_image_t *image, neural_network_t *network, neural_network_gradient_t *gradient, uint8_t label);
float neural_network_training_step(data_dataset_t *dataset, neural_network_t *network, float learning_rate);

#endif