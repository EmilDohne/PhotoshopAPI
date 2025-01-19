import unittest
import nose
import nose.tools
import os
import tempfile

from typing import Tuple, Optional

import numpy as np

import psapi


class TestSmartObjectLayer(unittest.TestCase):
    
    def _get_image_file_path(self, file_name: str) -> os.PathLike:
        """
        Get a file from the psapi-test/image_data directory by providing only the name of the file itself.
        
        :param file_name: Name of the file to get. Example of this could be 'uv_grid.jpg'
        """
        base_path = os.path.dirname(os.path.abspath(__file__))
        return os.path.join(base_path, "image_data", file_name)

    def _construct_layer_and_file(
            self, 
            image_name: str, 
            linkage: psapi.enum.LinkedLayerType,
            warp: Optional[psapi.SmartObjectWarp] = None,
            ) -> Tuple[psapi.LayeredFile_8bit, psapi.SmartObjectLayer_8bit]:
        """
        Utility function for constructing a file and a smartobjectlayer for use in the test cases.
        """
        file = psapi.LayeredFile_8bit(psapi.enum.ColorMode.rgb, 1024, 1024)
        layer = psapi.SmartObjectLayer_8bit(
            file,
            self._get_image_file_path(image_name),
            "SmartObjectLayer",
            linkage,
            warp,
            )
        
        file.add_layer(layer)
        
        return file, layer

    def test_construction_without_warp(self):
        """
        SmartObjectLayer: Test constructing layer without warp, will default generate warp
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)

    def test_construction_with_warp(self):
        """
        SmartObjectLayer: Test constructing layer with warp
        """
        warp = psapi.SmartObjectWarp.generate_default(200, 108, 4, 4)
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data, warp)

    def test_modify_warp(self):
        """
        SmartObjectLayer: Test modifying the warp and setting it back on the image data
        """
        warp = psapi.SmartObjectWarp.generate_default(200, 108, 4, 4)
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data, warp)
        
        # previous_image_data = layer.image_data

        # self.assertIn(0, previous_image_data)
        # self.assertIn(1, previous_image_data)
        # self.assertIn(2, previous_image_data)
        
        # self.assertTrue(previous_image_data[0].shape == (108, 200))
        # self.assertTrue(previous_image_data[1].shape == (108, 200))
        # self.assertTrue(previous_image_data[2].shape == (108, 200))

        # warp = layer.warp
        # points = warp.points
        # points[0] = points[0] + 25.0
        # warp.points = points
        # layer.warp = warp

        # current_image_data = layer.image_data
        
        # self.assertIn(0, current_image_data)
        # self.assertIn(1, current_image_data)
        # self.assertIn(2, current_image_data)
        
        # self.assertTrue(current_image_data[0].shape == (108, 200))
        # self.assertTrue(current_image_data[1].shape == (108, 200))
        # self.assertTrue(current_image_data[2].shape == (108, 200))
        
        # self.assertTrue(len(previous_image_data) == len(previous_image_data))
        # for key in previous_image_data:
        #     self.assertTrue(np.any(np.not_equal(previous_image_data[key], current_image_data[key])))
            
    @nose.tools.raises(RuntimeError)
    def test_set_image_data(self):
        """
        SmartObjectLayer: (ExpectedFailure) Test setting image data
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        img_data: dict = {0: np.ndarray((108, 200), np.uint8)}
        layer.set_image_data(img_data)
        
    @nose.tools.raises(AttributeError)
    def test_set_channel(self):
        """
        SmartObjectLayer: (ExpectedFailure) Test setting channel
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        layer.set_channel(0, np.ndarray((108, 200), np.uint8))
        
    def test_get_original_image_data(self):
        """
        SmartObjectLayer: Test getting the original image data
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        
        original_data = layer.original_image_data()
        self.assertIn(0, original_data)
        self.assertIn(1, original_data)
        self.assertIn(2, original_data)
        
        self.assertEqual(layer.original_width(), 200)
        self.assertEqual(layer.original_height(), 108)

        print(layer.warp.points)

        self.assertEqual(layer.width, 200)
        self.assertEqual(layer.height, 108)

        self.assertTrue(original_data[0].shape == (108, 200))
        self.assertTrue(original_data[1].shape == (108, 200))
        self.assertTrue(original_data[2].shape == (108, 200))
        
    def test_get_hash(self):
        """
        SmartObjectLayer: Test getting the hash of a layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        self.assertTrue(isinstance(layer.hash(), str))
        
    def test_get_filename(self):
        """
        SmartObjectLayer: Test getting the filename of a layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        self.assertEqual(layer.filename(), "ImageStackerImage_lowres.png")
        
    def test_get_filepath(self):
        """
        SmartObjectLayer: Test getting the filepath of a layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.external)
        
        self.assertEqual(str(layer.filepath()), self._get_image_file_path("ImageStackerImage_lowres.png"))
        
    def test_get_linkage(self):
        """
        SmartObjectLayer: Test getting and setting the `linkage` property
        """
        file_1, layer_1 = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.external)
        self.assertEqual(layer_1.linkage, psapi.enum.LinkedLayerType.external)
        
        layer_1.linkage = psapi.enum.LinkedLayerType.data
        self.assertEqual(layer_1.linkage, psapi.enum.LinkedLayerType.data)  
        
        file_2, layer_2 = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        self.assertEqual(layer_2.linkage, psapi.enum.LinkedLayerType.data)  
        
    def test_replace(self):
        """
        SmartObjectLayer: Test replacing of image data
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        

        print(layer.width)
        print(layer.height)

        raise ValueError("")

        image_data = layer.image_data
        previous_hash = layer.hash()
        
        layer.replace(self._get_image_file_path("uv_grid.jpg"))
        
        replaced_image_data = layer.image_data
        current_hash = layer.hash()
        
        self.assertNotEqual(previous_hash, current_hash)
        
        self.assertIn(0, image_data)
        self.assertIn(1, image_data)
        self.assertIn(2, image_data)
        self.assertIn(0, replaced_image_data)
        self.assertIn(1, replaced_image_data)
        self.assertIn(2, replaced_image_data)
        
        # Despite having replaced the image data, since the warp is identical the shape should not have changed.
        # The image is just stretched now.
        self.assertTrue(image_data[0].shape == replaced_image_data[0].shape)
        self.assertTrue(image_data[1].shape == replaced_image_data[1].shape)
        self.assertTrue(image_data[2].shape == replaced_image_data[2].shape)
        
    def test_roundtrip(self):
        """
        SmartObjectLayer: Test roundtripping of both `data` and `external` linked layers asserting the result is the same
        """
        file, layer_1 = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        
        # layer_1_image_data = layer_1.image_data
        # layer_2 = layer = psapi.SmartObjectLayer_8bit(
        #     file,
        #     self._get_image_file_path("uv_grid.jpg"),
        #     "SmartObjectLayer_external",
        #     psapi.enum.LinkedLayerType.external,
        #     )
        # file.add_layer(layer_2)
        # layer_2_image_data = layer_2.image_data
        
        # tmp_file = tempfile.NamedTemporaryFile(suffix=".psb", )
        # file.write(tmp_file.name)
        
        # read_file = psapi.LayeredFile.read(tmp_file.name)

        # read_layer_1 = read_file["SmartObjectLayer"]
        # read_layer_1_image_data = read_layer_1.image_data
        # read_layer_2 = read_file["SmartObjectLayer_external"]
        # read_layer_2_image_data = read_layer_2.image_data
        
        # for key in layer_1_image_data:
        #      self.assertTrue(
        #         np.array_equal(layer_1_image_data[key], read_layer_1_image_data[key]),
        #         f"Arrays for key {key} are not equal: {layer_1_image_data[key]} != {read_layer_1_image_data[key]}"
        #     )
        # for key in layer_2_image_data:
        #      self.assertTrue(
        #         np.array_equal(layer_2_image_data[key], read_layer_2_image_data[key]),
        #         f"Arrays for key {key} are not equal: {layer_2_image_data[key]} != {read_layer_2_image_data[key]}"
        #     )
        
    def test_move(self):
        """
        SmartObjectLayer: Test moving (translating) the layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        layer.move(50, 50)
        
    def test_rotate(self):
        """
        SmartObjectLayer: Test rotating the layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        layer.rotate(45, layer.center_x, layer.center_y)
        
    def test_scale(self):
        """
        SmartObjectLayer: Test scaling the layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        layer.scale(2.0, 2.0, layer.center_x, layer.center_y)
        
    def test_transform(self):
        """
        SmartObjectLayer: Test transforming the layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        mat = np.identity(3, np.double)
        layer.transform(mat)
        
        # Since we transform by an identity matrix this shouldn't actually do anything.
        self.assertTrue(layer.width == 200)
        self.assertTrue(layer.height == 108)
        
    def test_reset_transform(self):
        """
        SmartObjectLayer: Test resetting the transform on a layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        
        layer.rotate(45, layer.center_x, layer.center_y)
        self.assertNotEqual(layer.width, 200)
        self.assertNotEqual(layer.height, 108)
        
        layer.reset_transform()
        self.assertEqual(layer.width, 200)
        self.assertEqual(layer.height, 108)

    def test_reset_warp(self):
        """
        SmartObjectLayer: Test resetting the warp on a layer
        """
        file, layer = self._construct_layer_and_file("ImageStackerImage_lowres.png", psapi.enum.LinkedLayerType.data)
        
        warp = layer.warp
        points = warp.points
        points[0] = points[0] - 50.0
        warp.points = points
        layer.warp = warp

        print(layer.warp.points)
        print(layer.width, layer.height)
        print(layer.center_x, layer.center_y)

        self.assertNotEqual(layer.width, 200)
        self.assertNotEqual(layer.height, 108)
        
        layer.reset_warp()
        self.assertEqual(layer.width, 200)
        self.assertEqual(layer.height, 108)
        
        
if __name__ == "__main__":
    nose.main()