
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

TEST (LineEndPointsOnImage, VerticallikeLineIntersectsBottomAndTop) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = 1, .intercept = -100};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    ASSERT_EQ(endpoints.x1, 100);
    ASSERT_EQ(endpoints.y1, 0);
    ASSERT_EQ(endpoints.x2, 1179);
    ASSERT_EQ(endpoints.y2, 1079);
}


TEST (LineEndPointsOnImage, VerticallikeLineIntersectsBottomAndRight) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = 1, .intercept = -1000};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    ASSERT_EQ(endpoints.x2, 1000);
    ASSERT_EQ(endpoints.y2, 0);
    ASSERT_EQ(endpoints.x1, 1919);
    ASSERT_EQ(endpoints.y1, 919);
}

TEST (LineEndPointsOnImage, VerticallikeLineIntersectsBottomAndLeft) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = -1, .intercept = 100};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    ASSERT_EQ(endpoints.x2, 100);
    ASSERT_EQ(endpoints.y2, 0);
    ASSERT_EQ(endpoints.x1, 0);
    ASSERT_EQ(endpoints.y1, 100);
}

TEST (LineEndPointsOnImage, VerticallikeLineIntersectsLeftAndTop) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = 1, .intercept = 100};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    ASSERT_EQ(endpoints.x1, 0);
    ASSERT_EQ(endpoints.y1, 100);
    ASSERT_EQ(endpoints.x2, 979);
    ASSERT_EQ(endpoints.y2, 1079);
}

TEST (LineEndPointsOnImage, VerticallikeLineIntersectsRightAndTop) {
    // slope = y / x = 0 / x = 0
    slope_intercept_line_t line = {.slope = -1, .intercept = 2460};
    image_dimensions_t imageDimensions = {.width = 1920, .height = 1080};
    image_endpoints_t endpoints = lineEndPointsOnImage(line, imageDimensions);

    ASSERT_EQ(endpoints.x1, 1919);
    ASSERT_EQ(endpoints.y1, 541);
    ASSERT_EQ(endpoints.x2, 1381);
    ASSERT_EQ(endpoints.y2, 1079);
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

TEST(transformFromAccSpaceToImageSpace, NoRotation) {
    image_dimensions_t imageShape = {.width = 1000, .height = 1000};
    image_dimensions_t paddedImageShape = {.width = 1414, .height = 1414};
    point_t point = {.x = 707, .y = 0};
    float angle = 0;
    auto transformedPoint = transformFromAccSpaceToImageSpace(
            point, angle, imageShape, paddedImageShape);

    EXPECT_FLOAT_EQ(transformedPoint.x, 500);
    EXPECT_FLOAT_EQ(transformedPoint.y, -207);
}

TEST(transformFromAccSpaceToImageSpace, ClockwiseRotation) {
    image_dimensions_t imageShape = {.width = 1000, .height = 1000};
    image_dimensions_t paddedImageShape = {.width = 1414, .height = 1414};
    point_t point = {.x = 707, .y = 0};
    float angle = -45.f / 180.f * M_PI;
    auto transformedPoint = transformFromAccSpaceToImageSpace(
            point, angle, imageShape, paddedImageShape);

    EXPECT_NEAR(transformedPoint.x, 0.0755057, 0.00001);
    EXPECT_NEAR(transformedPoint.y, 0.0755057, 0.00001);
}


TEST(transformFromAccSpaceToImageSpace, FullHDClockwiseHitsAlmostCorner) {
    image_dimensions_t imageShape = {.width = 1920, .height = 1080};
    image_dimensions_t paddedImageShape = {.width = 2204, .height = 2204};
    point_t point = {.x = 1078, .y = 0};
    float angle = -59.5 / 180.f * M_PI;
    auto transformedPoint = transformFromAccSpaceToImageSpace(
            point, angle, imageShape, paddedImageShape);

    EXPECT_NEAR(transformedPoint.x, -1.69625552, 0.0001);
    EXPECT_NEAR(transformedPoint.y, 1.37182387, 0.0001);
}

TEST(lineFromAngleAndCol, AccumulatorCenter) {
    image_dimensions_t imageShape = {.width = 1920, .height = 1080};
    image_dimensions_t paddedImageShape = {.width = 2204, .height = 2204};
    auto line = lineFromAngleAndCol(M_PI / 4, 1102, imageShape, paddedImageShape);


}