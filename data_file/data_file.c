#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "data_file.h"

uint32_t map_uint32(uint32_t in)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (
        ((in & 0xFF000000) >> 24) |
        ((in & 0x00FF0000) >> 8) |
        ((in & 0x0000FF00) << 8) |
        ((in & 0x000000FF) << 24));
#else
    return in;
#endif
}

uint8_t *get_labels(const char *path, uint32_t *number_of_labels)
{
    FILE *stream;
    data_label_file_header_t header;
    uint8_t *labels;

    stream = fopen(path, "rb");

    if (NULL == stream)
    {
        fprintf(stderr, "Could not open file: %s\n", path);
        return NULL;
    }

    if (1 != fread(&header, sizeof(data_label_file_header_t), 1, stream))
    {
        fprintf(stderr, "Could not read label file header from: %s\n", path);
        fclose(stream);
        return NULL;
    }

    header.magic_number = map_uint32(header.magic_number);
    header.number_of_labels = map_uint32(header.number_of_labels);

    if (DATA_LABEL_MAGIC != header.magic_number)
    {
        fprintf(stderr, "Invalid header read from label file: %s (%08X not %08X)\n", path, header.magic_number, DATA_LABEL_MAGIC);
        fclose(stream);
        return NULL;
    }

    *number_of_labels = header.number_of_labels;

    labels = (uint8_t*)malloc(*number_of_labels * sizeof(uint8_t));

    if (labels == NULL)
    {
        fprintf(stderr, "Could not allocated memory for %d labels\n", *number_of_labels);
        fclose(stream);
        return NULL;
    }

    if (*number_of_labels != fread(labels, 1, *number_of_labels, stream))
    {
        fprintf(stderr, "Could not read %d labels from: %s\n", *number_of_labels, path);
        free(labels);
        fclose(stream);
        return NULL;
    }

    fclose(stream);

    return labels;
}

data_image_t *get_images(const char *path, uint32_t *number_of_images)
{
    FILE *stream;
    data_image_file_header_t header;
    data_image_t *images;

    stream = fopen(path, "rb");

    if (NULL == stream)
    {
        fprintf(stderr, "Could not open file: %s\n", path);
        return NULL;
    }

    if (1 != fread(&header, sizeof(data_image_file_header_t), 1, stream))
    {
        fprintf(stderr, "Could not read image file header from: %s\n", path);
        fclose(stream);
        return NULL;
    }

    header.magic_number = map_uint32(header.magic_number);
    header.number_of_images = map_uint32(header.number_of_images);
    header.number_of_rows = map_uint32(header.number_of_rows);
    header.number_of_columns = map_uint32(header.number_of_columns);

    if (DATA_IMAGE_MAGIC != header.magic_number)
    {
        fprintf(stderr, "Invalid header read from image file: %s (%08X not %08X)\n", path, header.magic_number, DATA_IMAGE_MAGIC);
        fclose(stream);
        return NULL;
    }

    if (DATA_IMAGE_WIDTH != header.number_of_rows)
    {
        fprintf(stderr, "Invalid number of image rows in image file %s (%d not %d)\n", path, header.number_of_rows, DATA_IMAGE_WIDTH);
    }

    if (DATA_IMAGE_HEIGHT != header.number_of_columns)
    {
        fprintf(stderr, "Invalid number of image columns in image file %s (%d not %d)\n", path, header.number_of_columns, DATA_IMAGE_HEIGHT);
    }

    *number_of_images = header.number_of_images;
    images = (data_image_t*)malloc(*number_of_images * sizeof(data_image_t));

    if (images == NULL)
    {
        fprintf(stderr, "Could not allocated memory for %d images\n", *number_of_images);
        fclose(stream);
        return NULL;
    }

    if (*number_of_images != fread(images, sizeof(data_image_t), *number_of_images, stream))
    {
        fprintf(stderr, "Could not read %d images from: %s\n", *number_of_images, path);
        free(images);
        fclose(stream);
        return NULL;
    }

    fclose(stream);

    return images;
}

data_dataset_t *data_get_dataset(const char *image_path, const char *label_path)
{
    data_dataset_t *dataset;
    uint32_t number_of_images, number_of_labels;

    dataset = (data_dataset_t*)calloc(1, sizeof(data_dataset_t));

    if (NULL == dataset)
    {
        return NULL;
    }

    dataset->images = get_images(image_path, &number_of_images);

    if (NULL == dataset->images)
    {
        data_free_dataset(dataset);
        return NULL;
    }

    dataset->labels = get_labels(label_path, &number_of_labels);

    if (NULL == dataset->labels)
    {
        data_free_dataset(dataset);
        return NULL;
    }

    if (number_of_images != number_of_labels)
    {
        fprintf(stderr, "Number of images does not match number of labels (%d != %d)\n", number_of_images, number_of_labels);
        data_free_dataset(dataset);
        return NULL;
    }

    dataset->size = number_of_images;

    return dataset;
}

void data_free_dataset(data_dataset_t *dataset)
{
    free(dataset->images);
    free(dataset->labels);
    free(dataset);
}

int data_batch(data_dataset_t *dataset, data_dataset_t *batch, int size, int number)
{
    int start_offset;

    start_offset = size * number;

    if (start_offset >= dataset->size)
    {
        return 0;
    }

    batch->images = &dataset->images[start_offset];
    batch->labels = &dataset->labels[start_offset];
    batch->size = size;

    if (start_offset + batch->size > dataset->size)
    {
        batch->size = dataset->size - start_offset;
    }

    return 1;
}