# To test:
#
# make clean dev
# python test.py

import unittest

import dfutil
import dfconvert
import shutil


class MyTest(unittest.TestCase):
    def test(self):
        
        self.assertTrue(dfutil.normalize("other.html"))

        self.assertTrue(dfconvert.get("input.docx", "output.html"))
        
        shutil.copyfile("input.docx", "dummy.docx");
        
        self.assertTrue(dfconvert.put("dummy.docx", "output.html"))
        
        self.assertTrue(dfconvert.create("output.docx", "output.html"))
        
        
if __name__ == '__main__':
    unittest.main()        

