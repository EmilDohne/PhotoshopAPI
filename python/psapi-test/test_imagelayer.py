import unittest
import nose
import nose.tools
import os

import numpy as np
import cv2

import psapi


class TestImageLayer(unittest.TestCase):
    path = os.path.join(os.path.dirname(__file__), "documents", "BaseFile.psb")
    bin_data_path = os.path.join(os.path.dirname(__file__), "bin_data", "monza_npy.bin")

    def test_get_image_data(self):
        file = psapi.LayeredFile.read(self.path)
        layer: psapi.ImageLayer_16bit = file["Render"]["AOV"]["Beauties"]["MonzaSP1_DawnShot_V1_v002_ED.BaseAOV"]
        
        image_data = layer.image_data

        self.assertTrue(-1 in image_data)
        self.assertTrue(0 in image_data)
        self.assertTrue(1 in image_data)
        self.assertTrue(2 in image_data)
        self.assertTrue(len(layer.channels) == 4)
        self.assertTrue(layer.num_channels == 4)

    def test_channel_setting_roundtrip(self):
        file = psapi.LayeredFile.read(self.path)
        layer: psapi.ImageLayer_16bit = file["Render"]["AOV"]["Beauties"]["MonzaSP1_DawnShot_V1_v002_ED.BaseAOV"]
        
        image_data_prev = layer.image_data
        layer.set_image_data(image_data_prev)
        image_data = layer.image_data

        for index, channel in image_data_prev.items():
            self.assertTrue(np.array_equal(channel, image_data[index]))

    def test_set_image_data_with_npy_array(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        for index, channel in layer.image_data.items():
            if index == -1:
                self.assertTrue(np.array_equal(channel, data[3]))
            else:
                self.assertTrue(np.array_equal(channel, data[index]))
        
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.red), data[0]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.green), data[1]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.blue), data[2]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.alpha), data[3]))

        self.assertTrue(np.array_equal(layer.get_channel_by_index(0), data[0]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(1), data[1]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(2), data[2]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(-1), data[3]))

    def test_set_image_data_with_index_map(self):
        npy_load = np.load(self.bin_data_path)
        data = {
            0: npy_load[0],
            1: npy_load[1],
            2: npy_load[2],
            -1: npy_load[3]
        }

        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        for index, channel in layer.image_data.items():
            self.assertTrue(np.array_equal(channel, data[index]))
        
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.red), data[0]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.green), data[1]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.blue), data[2]))
        self.assertTrue(np.array_equal(layer.get_channel_by_id(psapi.enum.ChannelID.alpha), data[-1]))

        self.assertTrue(np.array_equal(layer.get_channel_by_index(0), data[0]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(1), data[1]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(2), data[2]))
        self.assertTrue(np.array_equal(layer.get_channel_by_index(-1), data[-1]))
    
    @nose.tools.raises(ValueError)
    def test_set_incorrect_width(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 300, height = 188)

    @nose.tools.raises(ValueError)
    def test_set_incorrect_height(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 299)

    @nose.tools.raises(RuntimeError)
    def test_set_incorrect_channels(self):
        data = np.load(self.bin_data_path)
        data_dict = {
            0 : data[0],
            2 : data[1],
            3 : data[2],
            -1 : data[3]
        }
        layer = psapi.ImageLayer_16bit(data_dict, "Layer", width = 400, height = 188)

    @nose.tools.raises(ValueError)
    def test_set_image_data_flipped_dimensions(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 188, height = 400)

    @nose.tools.raises(RuntimeError)
    def test_replace_incorrect_channels(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        layer.set_channel_by_id(psapi.enum.ChannelID.cyan, data[0])

    @nose.tools.raises(RuntimeError)
    def test_set_missing_channels(self):
        data = np.load(self.bin_data_path)
        data_dict = {
            0 : data[0],
            1 : data[1],
            # 2 : data[2],
            -1 : data[3]
        }
        layer = psapi.ImageLayer_16bit(data_dict, "Layer", width = 400, height = 188)

    @nose.tools.raises(ValueError)
    def test_replace_image_data_without_rescale(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        data_rescaled = np.ndarray((4, 376, 800), dtype=np.uint16)
        for i in range(data.shape[0]):
            data_rescaled[i] = cv2.resize(data[i], (800, 376))
        layer.set_image_data(data_rescaled)

    def test_non_c_style_contiguous(self):
        data = np.load(self.bin_data_path)
        # Flip the 2nd and 3rd dimension and copy the data to enforce c-style continuous data
        data_copy = np.reshape(data, (data.shape[0], data.shape[2], data.shape[1])).copy()
        # Now flip back but without copy which means the strides are not c-style contiguous
        # and we instead force this conversion in-place
        data_copy = np.reshape(data_copy, (data_copy.shape[0], data_copy.shape[2], data_copy.shape[1]))
        layer = psapi.ImageLayer_16bit(data_copy, "Layer", width = 400, height = 188)

        self.assertTrue(np.array_equal(data[0], layer[0]))
        self.assertTrue(np.array_equal(data[1], layer[1]))
        self.assertTrue(np.array_equal(data[2], layer[2]))
        self.assertTrue(np.array_equal(data[3], layer[-1]))


    def test_set_channel_with_id(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        layer.set_channel_by_id(psapi.enum.ChannelID.red, data[0])
        self.assertTrue(np.array_equal(data[0], layer[0]))

    def test_set_channel_with_index(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        layer.set_channel_by_index(0, data[0])
        self.assertTrue(np.array_equal(data[0], layer[0]))

    def test_set_channel_with_setitem(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        layer[0] = data[0]
        self.assertTrue(np.array_equal(data[0], layer[0]))

    def test_get_channel_with_id(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        tmp = layer.get_channel_by_id(psapi.enum.ChannelID.red)
        self.assertTrue(np.array_equal(data[0], tmp))

    def test_get_channel_with_index(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        tmp = layer.get_channel_by_index(0)
        self.assertTrue(np.array_equal(data[0], tmp))

    def test_get_channel_with_setitem(self):
        data = np.load(self.bin_data_path)
        layer = psapi.ImageLayer_16bit(data, "Layer", width = 400, height = 188)
        tmp = layer[0]
        self.assertTrue(np.array_equal(data[0], tmp))

    def test_automatic_mask_promotion(self):
        data = np.load(self.bin_data_path)
        data_dict = {
            0 : data[0],
            1 : data[1],
            2 : data[2],
            -1 : data[3],
            -2 : data[3]
        }
        layer = psapi.ImageLayer_16bit(data_dict, "Layer", width = 400, height = 188)
        self.assertTrue(np.array_equal(layer.mask, data[3]))


if __name__ == "__main__":
    nose.main()