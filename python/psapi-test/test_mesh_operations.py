import unittest

import numpy

import photoshopapi as psapi

class TestMeshOperations(unittest.TestCase):
    

    def test_create_normalized_quad(self):
        quad = psapi.geometry.create_normalized_quad()
        
        self.assertTrue(len(quad) == 4)
        
        self.assertAlmostEqual(quad[0].x, 0.0)
        self.assertAlmostEqual(quad[0].y, 0.0)
        
        self.assertAlmostEqual(quad[1].x, 1.0)
        self.assertAlmostEqual(quad[1].y, 0.0)
        
        self.assertAlmostEqual(quad[2].x, 0.0)
        self.assertAlmostEqual(quad[2].y, 1.0)
        
        self.assertAlmostEqual(quad[3].x, 1.0)
        self.assertAlmostEqual(quad[3].y, 1.0)
        
    def test_create_quad(self):
        quad = psapi.geometry.create_quad(50, 25)
        
        self.assertTrue(len(quad), 4)
        
        self.assertAlmostEqual(quad[0].x, 0.0)
        self.assertAlmostEqual(quad[0].y, 0.0)
        
        self.assertAlmostEqual(quad[1].x, 50.0)
        self.assertAlmostEqual(quad[1].y, 0.0)
        
        self.assertAlmostEqual(quad[2].x, 0.0)
        self.assertAlmostEqual(quad[2].y, 25.0)
        
        self.assertAlmostEqual(quad[3].x, 50.0)
        self.assertAlmostEqual(quad[3].y, 25.0)
        
    def test_create_homography(self):
        quad_a = psapi.geometry.create_quad(50, 25)
        quad_b = psapi.geometry.create_quad(50, 25)
        
        quad_b[0] = psapi.geometry.Point2D(-5, -10)
        
        homography = psapi.geometry.create_homography(quad_a, quad_b)
        
        self.assertTrue(homography.shape == (3, 3))
        self.assertTrue(homography.dtype == numpy.double)
        