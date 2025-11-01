import unittest
import os
import shutil
import photoshopapi as psapi


class TestLayeredFile(unittest.TestCase):
    base_path = os.path.join(os.path.dirname(__file__), "documents", "BaseFile.psb")
    copy_path = os.path.join(os.path.dirname(__file__), "documents", "你好.psd")
    out_path = os.path.join(os.path.dirname(__file__), "documents", "你好_2.psd")

    def test_unicode_characters(self):
        # Copy and rename the file
        shutil.copy2(self.base_path, self.copy_path)
        self.assertTrue(os.path.exists(self.copy_path), "Failed to copy file to Unicode path")

        # Attempt to open the new file
        doc = psapi.LayeredFile.read(self.copy_path)
        self.assertIsNotNone(doc, "Failed to open copied document")


    def test_write_unicode_chars(self):
        # Copy and rename the file
        shutil.copy2(self.base_path, self.copy_path)
        self.assertTrue(os.path.exists(self.copy_path), "Failed to copy file to Unicode path")

        # Attempt to open the new file
        doc = psapi.LayeredFile.read(self.copy_path)
        self.assertIsNotNone(doc, "Failed to open copied document")

        doc.write(self.out_path)
        self.assertTrue(os.path.exists(self.out_path), "Failed to save file to Unicode path")
