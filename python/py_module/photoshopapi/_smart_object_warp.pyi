from re import L
from typing import List

from .geometry._point2d import Point2D


class SmartObjectWarp:
    
    @property
    def points(self: SmartObjectWarp) -> List[Point2D]:
        ...
        
    @points.setter
    def points(self: SmartObjectWarp, _points: List[Point2D]) -> None:
        ...
       
    @property
    def u_dims(self: SmartObjectWarp) -> int:
        ...
        
    @property
    def v_dims(self: SmartObjectWarp) -> int:
        ...
        
    @property
    def affine_transform(self: SmartObjectWarp) -> List[Point2D]:
        ...
       
    @affine_transform.setter
    def affine_transform(self: SmartObjectWarp, _array: List[Point2D]) -> None:
        ...
        
    @property
    def non_affine_transform(self: SmartObjectWarp) -> List[Point2D]:
        ...
       
    @non_affine_transform.setter
    def non_affine_transform(self: SmartObjectWarp, _array: List[Point2D]) -> None:
        ...
        
    def no_op(self: SmartObjectWarp) -> bool:
        ...
        
    def valid(self: SmartObjectWarp) -> bool:
        ...
        
    @staticmethod
    def generate_default(width: int, height: int, u_dims: int, v_dims: int) -> SmartObjectWarp:
        ...