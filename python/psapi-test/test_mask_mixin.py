import unittest
import os

import numpy as np

import psapi
import psapi.geometry


class TestMaskMixin(unittest.TestCase):
    bin_data_path = os.path.join(os.path.dirname(__file__), "bin_data", "monza_npy.bin")

    def test_add_mask(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")

        layer.mask = npy_load[0]

        self.assertEqual(layer.mask_width(), npy_load[0].shape[1])
        self.assertEqual(layer.mask_height(), npy_load[0].shape[0])
        self.assertEqual(layer.mask_position, psapi.geometry.Point2D(layer.mask_width() / 2, layer.mask_height() / 2))

    def test_has_mask(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")

        layer.mask = npy_load[0]
        self.assertTrue(layer.has_mask())

    def test_mask_disabled(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_disabled = True
        self.assertEqual(layer.mask_disabled, True)
        layer.mask_disabled = False
        self.assertEqual(layer.mask_disabled, False)

    def test_mask_relative_to_layer(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_relative_to_layer = True
        self.assertEqual(layer.mask_relative_to_layer, True)
        layer.mask_relative_to_layer = False
        self.assertEqual(layer.mask_relative_to_layer, False)

    def test_mask_relative_to_layer(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_relative_to_layer = True
        self.assertEqual(layer.mask_relative_to_layer, True)
        layer.mask_relative_to_layer = False
        self.assertEqual(layer.mask_relative_to_layer, False)

    def test_mask_default_color(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_default_color = 255
        self.assertEqual(layer.mask_default_color, 255)
        layer.mask_relative_to_layer = 0
        self.assertEqual(layer.mask_relative_to_layer, 0)

        with self.assertRaises(TypeError):
            layer.mask_default_color = 256

        with self.assertRaises(TypeError):
            layer.mask_default_color = -1

    def test_mask_density(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_density = 255
        self.assertEqual(layer.mask_density, 255)
        layer.mask_density = 0
        self.assertEqual(layer.mask_density, 0)

        with self.assertRaises(TypeError):
            layer.mask_density = 256

        with self.assertRaises(TypeError):
            layer.mask_density = -1

    def test_mask_feather(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_feather = 2.0
        self.assertEqual(layer.mask_feather, 2.0)
        layer.mask_feather = 55.2
        self.assertEqual(layer.mask_feather, 55.2)

    def test_mask_position(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_8bit("layer")
        layer.mask = npy_load[0]

        layer.mask_position = psapi.geometry.Point2D(10.0, 20.0)
        self.assertEqual(layer.mask_position.x, 10.0)
        self.assertEqual(layer.mask_position.y, 20.0)
