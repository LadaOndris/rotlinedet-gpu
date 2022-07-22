
#include "gtest/gtest.h"
#include "linedet.hpp"
#include "coordinates.hpp"

TEST (LineEndPointsOnImage, VerticalLine) {

    float nan = std::numeric_limits<double>::quiet_NaN();
    slope_intercept_line_t line = {.slope = nan, .intercept = 500};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    EXPECT_EQ(endpoints.x1, 500);
    EXPECT_EQ(endpoints.y1, 0);

    EXPECT_EQ(endpoints.x2, 500);
    EXPECT_EQ(endpoints.y2, imageDimensions.height - 1);
}

TEST (LineEndPointsOnImage, HorizontalLine) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = 0, .intercept = 500};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    EXPECT_EQ(endpoints.x1, 0);
    EXPECT_EQ(endpoints.y1, 500);

    EXPECT_EQ(endpoints.x2, imageDimensions.width - 1);
    EXPECT_EQ(endpoints.y2, 500);
}


TEST(transformFromAccSpaceToImageSpace, AccumulatorCenter) {
    image_dimensions_t imageShape = {.width = 1920, .height = 1080};
    image_dimensions_t paddedImageShape = {.width = 2204, .height = 2204};
    point_t point = {.x = 1102, .y = 0};
    float angle = M_PI / 4;
    auto transformedPoint = transformFromAccSpaceToImageSpace(
            point, angle, imageShape, paddedImageShape);

    EXPECT_FLOAT_EQ(transformedPoint.x, 1739.23167);
    EXPECT_FLOAT_EQ(transformedPoint.y, -239.231673);
}

TEST(lineFromAngleAndCol, AccumulatorCenter) {
    image_dimensions_t imageShape = {.width = 1920, .height = 1080};
    image_dimensions_t paddedImageShape = {.width = 2204, .height = 2204};
    auto line = lineFromAngleAndCol(M_PI / 4, 1102, imageShape, paddedImageShape);


}