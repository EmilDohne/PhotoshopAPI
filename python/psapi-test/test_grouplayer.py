import unittest
import nose
import nose.tools
import os

import numpy as np

import psapi


class TestGroupLayer(unittest.TestCase):
    bin_data_path = os.path.join(os.path.dirname(__file__), "bin_data", "monza_npy.bin")

    @nose.tools.raises(ValueError)
    def test_create_group_with_mask_no_size(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_16bit("group", npy_load[0], 0, 0)

    @nose.tools.raises(RuntimeError)
    def test_create_group_with_size_no_mask(self):
        layer = psapi.GroupLayer_16bit("group", width=400, height=188)

    def test_create_group_with_mask(self):
        npy_load = np.load(self.bin_data_path)
        layer = psapi.GroupLayer_16bit("group", layer_mask=npy_load[0], width=npy_load.shape[2], height=npy_load.shape[1])