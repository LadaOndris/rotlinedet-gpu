
#include "coordinates.hpp"

#include <cmath>
#include <limits>
#include <vector>

using namespace std;

bool areClose(double val1, double val2) {
    return fabs(val1 - val2) < 0.00001;
}

bool isLineHorizontal(slope_intercept_line_t line) {
    // Check that the slope was computed as y/x where y=0
    return areClose(line.slope, 0);
}

bool isLineVertical(slope_intercept_line_t line) {
    // Check that the slope was computed as y/x where x=0
    return std::isnan(line.slope);
}

float solve4y(float x, slope_intercept_line_t line) {
    if (isLineVertical(line)) {
        return line.intercept;
    }
    return line.slope * x + line.intercept;
}

float solve4x(float y, slope_intercept_line_t line) {
    if (isLineHorizontal(line)) {
        return 0;
    }
    if (isLineVertical(line)) {
        return line.intercept;
    }
    return (y - line.intercept) / line.slope;
}

void append_if_within_image(float x, float y, vector<int> &coords, image_dimensions_t image_dimensions) {
    bool isWithinImage = 0 <= y && y < image_dimensions.height &&
                         0 <= x && x < image_dimensions.width;
    if (isWithinImage) {
        coords.push_back(x);
        coords.push_back(y);
    }
}

image_endpoints_t lineEndPointsOnImage(slope_intercept_line_t line, image_dimensions_t image_dimensions) {
    vector<int> coords;
    coords.reserve(4);

    float x, y;
    if (!isLineVertical(line)) {
        x = 0;
        y = solve4y(x, line);
        append_if_within_image(x, y, coords, image_dimensions);

        x = image_dimensions.width - 1;
        y = solve4y(x, line);
        append_if_within_image(x, y, coords, image_dimensions);
    }
    if (!isLineHorizontal(line)) {
        y = 0;
        x = solve4x(y, line);
        append_if_within_image(x, y, coords, image_dimensions);

        y = image_dimensions.height - 1;
        x = solve4x(y, line);
        append_if_within_image(x, y, coords, image_dimensions);
    }

    image_endpoints_t endpoints = {
            .x1 = coords[0], .y1 = coords[1],
            .x2 = coords[2], .y2 = coords[3]
    };
    return endpoints;
}

slope_intercept_line_t slopeInterceptLineFromTwoPoints(
        point_t point1, point_t point2) {
    float dx = point2.x - point1.x;
    float dy = point2.y - point1.y;
    float nan = std::numeric_limits<double>::quiet_NaN();
    float slope = dx == 0 ? nan : dy / dx;
    float intercept = dx == 0 ? point1.x : point1.y - slope * point1.x;

    slope_intercept_line_t line;
    line.slope = slope;
    line.intercept = intercept;
    return line;
}

point_t transformFromAccSpaceToImageSpace(point_t point, float angle,
                                          image_dimensions_t imageShape,
                                          image_dimensions_t paddedImageShape) {
    float iWidth = imageShape.width / 2;
    float iHeight = imageShape.height / 2;
    float p = paddedImageShape.width / 2;

    float s = std::sin(angle);
    float c = std::cos(angle);
    point_t transformedPoint;
    transformedPoint.x = c * point.x - s * point.y + p * (s - c) + iWidth;
    transformedPoint.y = s * point.x + c * point.y - p * (s + c) + iHeight;
    return transformedPoint;
}

slope_intercept_line_t lineFromAngleAndCol(float angle, int col,
                                           image_dimensions_t imageShape,
                                           image_dimensions_t paddedImageShape) {
    auto maxY = static_cast<float>(paddedImageShape.height - 1);
    point_t point1 = {.x = static_cast<float>(col), .y = 0};
    point_t point2 = {.x = static_cast<float>(col), .y =maxY};
    point_t transformedPoint1 = transformFromAccSpaceToImageSpace(point1, angle, imageShape, paddedImageShape);
    point_t transformedPoint2 = transformFromAccSpaceToImageSpace(point2, angle, imageShape, paddedImageShape);

    auto line = slopeInterceptLineFromTwoPoints(transformedPoint1, transformedPoint2);
    return line;
}