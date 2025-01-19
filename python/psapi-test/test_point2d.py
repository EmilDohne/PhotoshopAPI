import unittest
import nose

from psapi.geometry import Point2D

class TestPoint2D(unittest.TestCase):
    # These tests are very basic in their nature as the main goal of these is to check
    # the bindings rather than the underlying logic as that is covered on the C++ side.

    def test_addition_by_point2d(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(2, 2)
        
        result = point_1 + point_2
        self.assertAlmostEqual(result.x, 3)
        self.assertAlmostEqual(result.y, 3)
        
    def test_addition_by_float(self):
        point_1 = Point2D(1, 1)
        result = point_1 + 2.0
        
        self.assertAlmostEqual(result.x, 3)
        self.assertAlmostEqual(result.y, 3)
        
    def test_subtraction_by_point2d(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(2, 2)
        
        result = point_1 - point_2
        self.assertAlmostEqual(result.x, -1)
        self.assertAlmostEqual(result.y, -1)
        
    def test_subtraction_by_float(self):
        point_1 = Point2D(1, 1)
        result = point_1 - 2.0
        
        self.assertAlmostEqual(result.x, -1)
        self.assertAlmostEqual(result.y, -1)
        
    def test_multiplication_by_point2d(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(2, 2)
        
        result = point_1 * point_2
        self.assertAlmostEqual(result.x, 2)
        self.assertAlmostEqual(result.y, 2)
        
    def test_multiplication_by_float(self):
        point_1 = Point2D(1, 1)
        result = point_1 * 2.0
        
        self.assertAlmostEqual(result.x, 2)
        self.assertAlmostEqual(result.y, 2)
        
    def test_division_by_point2d(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(2, 2)
        
        result = point_1 / point_2
        self.assertAlmostEqual(result.x, .5)
        self.assertAlmostEqual(result.y, .5)
        
    def test_division_by_float(self):
        point_1 = Point2D(1, 1)
        result = point_1 / 2.0
        
        self.assertAlmostEqual(result.x, .5)
        self.assertAlmostEqual(result.y, .5)
        
    def test_negation(self):
        point_1 = Point2D(1, 1)
        point_1 = -point_1
        
        self.assertAlmostEqual(point_1.x, -1)
        self.assertAlmostEqual(point_1.y, -1)

    def test_lerp(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(3, 3)
        
        result = Point2D.lerp(point_1, point_2, .5)
        self.assertAlmostEqual(result.x, 2)
        self.assertAlmostEqual(result.y, 2)
        
    def test_distance(self):
        point_1 = Point2D(1, 1)
        point_2 = Point2D(3, 1)
        
        self.assertAlmostEqual(point_1.distance(point_2), 2)
        
        
if __name__ == "__main__":
    nose.main()