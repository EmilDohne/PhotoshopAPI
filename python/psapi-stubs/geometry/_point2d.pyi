from typing import Tuple

class Point2D:
    
    @property
    def x(self: Point2D) -> float:
        ...
    
    @x.setter
    def x(self: Point2D, value: float) -> None:
        ...
        
    @property
    def y(self: Point2D) -> float:
        ...
    
    @x.setter
    def y(self: Point2D, value: float) -> None:
        ...
        
    def __init__(self: Point2D, x: float, y: float) -> None:
        ...
        
    def __eq__(self: Point2D, other: Point2D) -> bool:
        ...
        
    def __ne__(self: Point2D, other: Point2D) -> bool:
        ...
        
    def __hash__(self: Point2D) -> str:
        ...
        
    def __len__(self: Point2D) -> int:
        ...
        
    def __add__(self: Point2D, other: Tuple[Point2D, float]) -> Point2D:
        ...
        
    def __sub__(self: Point2D, other: Tuple[Point2D, float]) -> Point2D:
        ...
        
    def __neg__(self: Point2D) -> Point2D:
        ...

    def __mul__(self: Point2D, other: Tuple[Point2D, float]) -> Point2D:
        ...
        
    def __truediv__(self: Point2D, other: Tuple[Point2D, float]) -> Point2D:
        ...
        
    def distance(self: Point2D, other: Point2D) -> float:
        ...
        
    @staticmethod
    def lerp(a: Point2D, b: Point2D, t: float) -> Point2D:
        ...