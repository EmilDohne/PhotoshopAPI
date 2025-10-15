import unittest
import os

from pathlib import Path

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
    
    def test_modify_icc_profile(self):
        psd_file = psapi.LayeredFile_8bit.read(self._get_icc_file_path("AdobeRGB1998.psb"))

        self.assertTrue(
            np.array_equal(np.fromfile(self._get_icc_file_path("AdobeRGB1998.icc"), dtype=np.uint8), psd_file.icc),
            "The profile in AdobeRGB1998.psb should match the AdobeRGB1998.icc file."
        )

        self.assertIsNotNone(psd_file.icc)
        self.assertEqual(psd_file.icc.shape, (560,))
        adobe_icc = psd_file.icc.copy()

        psd_file.icc = self._get_icc_file_path("AppleRGB.icc")

        self.assertFalse(
            np.array_equal(adobe_icc, psd_file.icc),
            "The ICC profile should have changed after modification."
        )
        
        psd_file.write(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified.psb"))
        
        modified_psd_file = psapi.LayeredFile_8bit.read(self._get_image_file_path("AdobeRGB1998_to_AppleRGB_modified.psb"))
        self.assertIsNotNone(modified_psd_file.icc)
        
        self.assertFalse(
            np.array_equal(adobe_icc, modified_psd_file.icc),
            "The ICC profile should have changed after writing the modified file."
        )
