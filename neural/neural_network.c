#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../data_file/data_file.h"
#include "neural_network.h"

#define PIXEL_SCALE(x) (((float)(x)) / 255.0f)

#define RAND_FLOAT() (((float)rand()) / ((float)RAND_MAX))

void neural_network_random_weights(neural_network_t *network)
{
    int i, j;

    for (i = 0; i < DATA_LABELS; i++)
    {
        network->b[i] = RAND_FLOAT();

        for (j = 0; j < DATA_IMAGE_SIZE; j++)
        {
            network->W[i][j] = RAND_FLOAT();
        }
    }
}

void neural_network_softmax(float *activations, int length)
{
    int i;
    float sum, max;

    for (i = 1, max = activations[0]; i < length; i++)
    {
        if (activations[i] > max)
        {
            max = activations[i];
        }
    }

    for (i = 0, sum = 0; i < length; i++)
    {
        activations[i] = exp(activations[i] - max);
        sum += activations[i];
    }

    for (i = 0; i < length; i++)
    {
        activations[i] /= sum;
    }
}

void neural_network_hypothesis(data_image_t *image, neural_network_t *network, float activations[DATA_LABELS])
{
    int i, j;

    for (i = 0; i < DATA_LABELS; i++)
    {
        activations[i] = network->b[i];

        for (j = 0; j < DATA_IMAGE_SIZE; j++)
        {
            activations[i] += network->W[i][j] * PIXEL_SCALE(image->pixels[j]);
        }
    }

    neural_network_softmax(activations, DATA_LABELS);
}

float neural_network_gradient_update(data_image_t *image, neural_network_t *network, neural_network_gradient_t *gradient, uint8_t label)
{
    float activations[DATA_LABELS];
    float b_grad, W_grad;
    int i, j;

    // First forward propagate through the network to calculate activations
    neural_network_hypothesis(image, network, activations);

    for (i = 0; i < DATA_LABELS; i++)
    {
        // This is the gradient for a softmax bias input
        b_grad = (i == label) ? activations[i] - 1 : activations[i];

        for (j = 0; j < DATA_IMAGE_SIZE; j++)
        {
            // The gradient for the neuron weight is the bias multiplied by the input weight
            W_grad = b_grad * PIXEL_SCALE(image->pixels[j]);

            // Update the weight gradient
            gradient->W_grad[i][j] += W_grad;
        }

        // Update the bias gradient
        gradient->b_grad[i] += b_grad;
    }

    // Cross entropy loss
    return 0.0f - log(activations[label]);
}

float neural_network_training_step(data_dataset_t *dataset, neural_network_t *network, float learning_rate)
{
    neural_network_gradient_t gradient;
    float total_loss;
    int i, j;

    // Zero initialise gradient for weights and bias vector
    memset(&gradient, 0, sizeof(neural_network_gradient_t));

    // Calculate the gradient and the loss by looping through the training set
    for (i = 0, total_loss = 0; i < dataset->size; i++)
    {
        total_loss += neural_network_gradient_update(&dataset->images[i], network, &gradient, dataset->labels[i]);
    }

    // Apply gradient descent to the network
    for (i = 0; i < DATA_LABELS; i++)
    {
        network->b[i] -= learning_rate * gradient.b_grad[i] / ((float)dataset->size);

        for (j = 0; j < DATA_IMAGE_SIZE; j++)
        {
            network->W[i][j] -= learning_rate * gradient.W_grad[i][j] / ((float)dataset->size);
        }
    }

    return total_loss;
}