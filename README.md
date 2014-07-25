The AnimeRAster project aims to create a lossless image format with at least comparable compression ratios to the best lossless image methods, but optimized for drawings instead of photographs.

The following scenarios and features are considered:

## Screen-caps of video sources

- YUV support with chroma-subsampling. Early tests suggest a 25-50% size reduction compared to converting it to RGB.
- High bit depths, for saving Hi10p without any loss at all
- Subtitles as a separate layer, both as a rendered overlay, and storing the ASS data as meta-data.
- More meta-data, such as file source and frame-number

## Visual Novel CGs

Continuation of the cgCompress project.

- Multi-frame format to store all related images in one group.
- Exploit correlation between images. cgCompress already is 20-25% better than the next best alternative.
- Do it in the filtering phrase, and use the concepts from normal image compression to further improve compression.
- Consider how memory usage can be reduced when there is a lot of frames.


## Status

Format far from finalized

- Basic single-frame RGB compression. Currently about 5 % worse than webp, and a bit more compared to BCIF.
- Basic single-frame YUV compression. Needs more experimentation with color decorrelation.