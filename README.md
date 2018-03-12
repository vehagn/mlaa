# MLAA
**M**orpho**L**ogical **A**nti**A**liasing for monchromatic models based on [Reshetov A. 2009. Morphological Antialiasing, in Proceedings of the Conference on High Performance Graphics 2009](http://doi.acm.org/10.1145/1572769.1572787)

## Use
A simple proof of concept for smoothing syntethic models in a hope to alleviate the "staircase effect." Note that horizontal and vertical edges are kept as-is, while only slopes are anti-aliased by "hallucinating" edges/faux supersampling.

**Before:**<br>
![alt text][raw]

**After**:<br>
![alt text][mlaa]

[raw]: https://raw.githubusercontent.com/vehagn/mlaa/master/raw.bmp "Raw model"
[mlaa]: https://raw.githubusercontent.com/vehagn/mlaa/master/mlaa.bmp "MLAA model"

## Limitations
Currently only Z-shapes are fully supported. U shapes are implemented, but some bugs are present.

### Edge classifications
1. ▏ (1) left
2. ▕ (2) right
4. ▁ (4) bottom
8. ▔ (8) top

### Shape classifications
#### Row shapes
2. └┐ (2)
4. ┌┘ (4)
8. └┘ (8)
16. ┌┐ (16)
32. ─┘ (32)
64. ─┐ (64)
128. └─ (128)
256. ┌─ (256)

#### Column shapes
3. ┐ (3)<br>
   └
5. ┌ (5) <br>
   ┘
9. ┐ (9) <br>
   ┘
17. ┌ (17) <br> 
    └
33. │ (33) <br>
    ┘
65. │ (65) <br>
    └
129. ┐ (129) <br>
     │
257. ┌ (257) <br>
     │
