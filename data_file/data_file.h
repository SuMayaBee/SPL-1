
#ifndef DATA_FILE_H_
#define DATA_FILE_H_

#include <stdint.h>

#define DATA_LABEL_MAGIC 0x00000801
#define DATA_IMAGE_MAGIC 0x00000803
#define DATA_IMAGE_WIDTH 28
#define DATA_IMAGE_HEIGHT 28
#define DATA_IMAGE_SIZE (DATA_IMAGE_WIDTH * DATA_IMAGE_HEIGHT)
#define DATA_LABELS 10

typedef struct data_label_file_header_t
{
    uint32_t magic_number;
    uint32_t number_of_labels;
} __attribute__((packed)) data_label_file_header_t;

typedef struct data_image_file_header_t
{
    uint32_t magic_number;
    uint32_t number_of_images;
    uint32_t number_of_rows;
    uint32_t number_of_columns;
} __attribute__((packed)) data_image_file_header_t;

typedef struct data_image_t
{
    uint8_t pixels[DATA_IMAGE_SIZE];
} __attribute__((packed)) data_image_t;

typedef struct data_dataset_t
{
    data_image_t *images;
    uint8_t *labels;
    uint32_t size;
} data_dataset_t;

data_dataset_t *data_get_dataset(const char *image_path, const char *label_path);
void data_free_dataset(data_dataset_t *dataset);
int data_batch(data_dataset_t *dataset_t, data_dataset_t *batch, int batch_size, int batch_number);

#endif