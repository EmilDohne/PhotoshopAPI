import unittest
import os

from pathlib import Path
from typing import Tuple

import numpy as np

import photoshopapi as psapi


class TestIccProfile(unittest.TestCase):

    def _get_icc_file_path(self, file_name: str) -> os.PathLike:
        """
        Get a file from the PhotoshopTest/documents/ICCProfiles directory by providing only the name of the file itself.

        :param file_name: Name of the file to get. Example of this could be 'AdobeRGB1998.icc'
        """
        base_path = os.path.dirname(os.path.abspath(__file__))
        return os.path.join(base_path, "../../PhotoshopTest/documents/ICCProfiles", file_name)

    def _get_image_file_path(self, file_name: str) -> os.PathLike:
        """
        Get a file from the psapi-test/image_data directory by providing only the name of the file itself.
        
        :param file_name: Name of the file to get. Example of this could be 'uv_grid.jpg'
        """
        base_path = os.path.dirname(os.path.abspath(__file__))
        return os.path.join(base_path, "image_data", file_name)
    
    def _get_original(self) -> Tuple[psapi.LayeredFile_8bit, np.ndarray]:
        psd_file = psapi.LayeredFile_8bit.read(self._get_icc_file_path("AdobeRGB1998.psb"))

        self.assertTrue(
            np.array_equal(np.fromfile(self._get_icc_file_path("AdobeRGB1998.icc"), dtype=np.uint8), psd_file.icc),
            "The profile in AdobeRGB1998.psb should match the AdobeRGB1998.icc file."
        )

        self.assertIsNotNone(psd_file.icc)
        self.assertEqual(psd_file.icc.shape, (560,))
        adobe_icc = psd_file.icc.copy()
        
        return psd_file, adobe_icc
    
    def test_modify_icc_profile_path(self):
        psd_file, adobe_icc = self._get_original()

        psd_file.icc = self._get_icc_file_path("AppleRGB.icc")

        self.assertFalse(
            np.array_equal(adobe_icc, psd_file.icc),
            "The ICC profile should have changed after modification."
        )
        
        psd_file.write(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified_path.psb"))
        
        modified_psd_file = psapi.LayeredFile_8bit.read(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified_path.psb"))
        self.assertIsNotNone(modified_psd_file.icc)
        
        self.assertFalse(
            np.array_equal(adobe_icc, modified_psd_file.icc),
            "The ICC profile should have changed after writing the modified file."
        )

    def test_modify_icc_profile_numpy(self):
        psd_file, adobe_icc = self._get_original()

        psd_file.icc = np.fromfile(self._get_icc_file_path("AppleRGB.icc"), dtype=np.uint8)

        self.assertFalse(
            np.array_equal(adobe_icc, psd_file.icc),
            "The ICC profile should have changed after modification."
        )
        
        psd_file.write(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified_numpy.psb"))
        
        modified_psd_file = psapi.LayeredFile_8bit.read(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified_numpy.psb"))
        self.assertIsNotNone(modified_psd_file.icc)
        
        self.assertFalse(
            np.array_equal(adobe_icc, modified_psd_file.icc),
            "The ICC profile should have changed after writing the modified file."
        )

    def test_raise_on_invalid_type(self):
        psd_file, _ = self._get_original()
        
        with self.assertRaises(TypeError):
            psd_file.icc = object()  # type: ignore

        with self.assertRaises(TypeError):
            psd_file.icc = None  # type: ignore

        # bytes is treated as filepath
        with self.assertRaises(RuntimeError) as context:
            psd_file.icc = bytes([10, 20, 30])  # type: ignore
        self.assertIn("Must pass a valid .icc file into the ctor.", str(context.exception))

    def test_accept_other_types(self):
        psd_file, _ = self._get_original()
        
        # These types pass, they are implicitly converted - maybe we want to check them more strictly
        psd_file.icc = 123  # type: ignore
        self.assertTrue(np.array_equal(psd_file.icc, np.array([123])))

        psd_file.icc = [1, 2, 3, 4]  # type: ignore
        self.assertTrue(np.array_equal(psd_file.icc, np.array([1, 2, 3, 4])))

        psd_file.icc = bytearray([10, 20, 30])  # type: ignore
        self.assertTrue(np.array_equal(psd_file.icc, np.array([10, 20, 30])))

        psd_file.icc = memoryview(bytes([10, 20, 30]))  # type: ignore
        self.assertTrue(np.array_equal(psd_file.icc, np.array([10, 20, 30])))
