# Transfer Function

Transfer functions describe fixed points on a 13 segment curve. The values can range between 0 and 1000 where 1000 represents 100.0%. Therefore you could represent 65.3% by the number 653. If the curve segment is empty it is marked by a -1

**Total Size** : 28 bytes
| Size | Type | Variable | Description |
|---|---|---|---|
| **2** | int16_t | 0 Val | Value for the 0 Element in the curve, this value must be set
| **2** | int16_t | 5 Val | Value for the 5 Element in the curve, this value may be empty 
| **2** | int16_t | 10 Val | Value for the 10 Element in the curve, this value may be empty 
| **2** | int16_t | 20 Val | Value for the 20 Element in the curve, this value may be empty 
| **2** | int16_t | 30 Val | Value for the 30 Element in the curve, this value may be empty 
| **2** | int16_t | 40 Val | Value for the 40 Element in the curve, this value may be empty 
| **2** | int16_t | 50 Val | Value for the 50 Element in the curve, this value may be empty 
| **2** | int16_t | 60 Val | Value for the 60 Element in the curve, this value may be empty 
| **2** | int16_t | 70 Val | Value for the 70 Element in the curve, this value may be empty 
| **2** | int16_t | 80 Val | Value for the 80 Element in the curve, this value may be empty 
| **2** | int16_t | 90 Val | Value for the 90 Element in the curve, this value may be empty 
| **2** | int16_t | 95 Val | Value for the 95 Element in the curve, this value may be empty 
| **2** | int16_t | 100 Val | Value for the 100 Element in the curve, this value must be set
| **2** | bool | PrinterCurve | 0 if we let the printer supply the curve; 1 if we override the printers default curve
