# MLAA
**M**orpho**L**ogical **A**nti**A**liasing for monchromatic models based on [Reshetov A. 2009. Morphological Antialiasing, in Proceedings of the Conference on High Performance Graphics 2009](http://doi.acm.org/10.1145/1572769.1572787)

## Limitations
Currently only Z-shapes are fully supported. U shapes are implemented, but some bugs are present.

## Implementation
A simple proof of concept for smoothing syntethic models.

### Edge classifications
1. ▏ left
2. ▕ right
4. ▁ bottom
8. ▔

### Shape classifications
#### Row shapes
2. └┐
4. ┌┘
8. └┘
16. ┌┐
32. ─┘
64. ─┐
128. └─
256. ┌─

#### Column shapes
3. ┐
   └
5. ┌
   ┘
9. ┐
   ┘
17. ┌
    └
33. │
    ┘
65. │
    └
129. ┐
     │
257. ┌
     │
