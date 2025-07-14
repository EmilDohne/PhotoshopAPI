from typing import List

import numpy

from ._point2d import Point2D


def create_normalized_quad() -> List[Point2D]:
    ...
    

def create_quad(width: float, height: float) -> List[Point2D]:
    ...
    

def create_homography(source_points: List[Point2D], destination_points: List[Point2D]) -> numpy.ndarray:
    ...