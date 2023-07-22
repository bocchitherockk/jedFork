# Jpeg Encoder/Decoder
This is a jpeg encoder/decoder in c that reads JPEG images and converts them to PPM and BMP, and vise versa.  
This project was origionally created for my **olivec<!-- add the link for the olivec repo -->** library project for computer graphics and image processing.  
**The purpose of this project is to deeply understand the core and fundamentals of jpeg images, and how does jpeg compression work.**  
90% of this work is done by Daniel Hardling in c++ which I found on youtube teaching **[everything you need to know about jpeg](https://www.youtube.com/watch?v=CPT4FSkFUgs&list=PLpsTn9TA_Q8VMDyOPrDKmSJYt1DLgDZU4&index=1&ab_channel=DanielHarding)**.  
All i did was convert the logic from c++ to c, so all the credit goes to him.  
Also i am new to git and github, so i don't know if what i am doing is OK.  
*[original work](https://github.com/dannye/jed)*

## quick start:
- make a **`main.c`** file in the ***src*** directory
- run **`build.sh`** to build the project
- run the executable **`main`** under the ***bin*** directory  
**!! if you want to use names other than `main.c` feel free to change `build.sh` accordingly !!**
### a small example of a main.c file:
```c
#include <stdlib.h>
#include "jpeg.h"
int main() {
    char *inputFileName = "input.jpg";
    char *outputFileName = "output.ppm";
    JpegImage img = Jpeg_readImage(inputFileName);
    Jpeg_printImage(&img);
    Block *blocks = Jpeg_getBlocks(&img);
    Jpeg_saveImageAsPpm(&img, blocks, outputFileName);
    outputFileName = "output.bmp";
    Jpeg_saveImageAsBmp(&img, blocks, outputFileName);
    Jpeg_destroyImage(&img);
    free(blocks);
    return 0;
}
```

## Resources:
- [learn about PPM/PGM/PBM image files](http://paulbourke.net/dataformats/ppm/)
- learn about jpeg compression process:
    - [Branch Education](https://www.youtube.com/watch?v=Kv1Hiv3ox8I&t=772s&ab_channel=BranchEducation)
    - [Computerphile](https://www.youtube.com/watch?v=n_uNPbdenRs&ab_channel=Computerphile)
    - [Computerphile](https://www.youtube.com/watch?v=Q2aEzeMDHMA&t=1s&ab_channel=Computerphile)
    - [asecuritysite](https://asecuritysite.com/comms/dct2)
- [learn about JFIF format and its segments](https://mykb.cipindanci.com/archive/SuperKB/1294/JPEG%20File%20Layout%20and%20Format.htm)
- [learn about everything](https://www.youtube.com/watch?v=CPT4FSkFUgs&list=PLpsTn9TA_Q8VMDyOPrDKmSJYt1DLgDZU4&index=1&ab_channel=DanielHarding) (this guy is a GOAT really, i can't thank him enough)