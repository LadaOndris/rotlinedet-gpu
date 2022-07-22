//
// Created by lada on 7/22/22.
//

#ifndef ROTLINEDET_COORDINATES_HPP
#define ROTLINEDET_COORDINATES_HPP

typedef struct {
    float x;
    float y;
} point_t;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} image_endpoints_t;

typedef struct {
    float slope;
    float intercept;
} slope_intercept_line_t;

typedef struct {
    int width;
    int height;
} image_dimensions_t;

image_endpoints_t lineEndPointsOnImage(slope_intercept_line_t line, image_dimensions_t image_dimensions);

slope_intercept_line_t lineFromAngleAndCol(float angle, int col, image_dimensions_t imageShape,
                                           image_dimensions_t paddedImageShape);

point_t transformFromAccSpaceToImageSpace(point_t point, float angle,
                                          image_dimensions_t imageShape,
                                          image_dimensions_t paddedImageShape);

#endif //ROTLINEDET_COORDINATES_HPP
