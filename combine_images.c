#include <leptonica/allheaders.h>

int main(){

l_int32      w, d;
l_int32      tw, td;
    PIX *pixa, *pixs, *pix2,*pix1;
    pixa = pixaCreate(5);
    pixs = pixRead("im2070.jpg");
    pixGetDimensions(pixs, &w, NULL, &d);
    pixSaveTiled(pixs, pixa, 1.0, 0, 1,1);
    pixDisplay(pixs,100,0);
    pix2 = pixRead("binar6.png");
    pixDisplay(pix2,100,0);
    pixSaveTiled(pix2, pixa, 1.0, 0, 1,1);

    pix1 = pixaDisplay(pixa, w*2, 0.80*d);
    pixWrite("binar6.png", pix1, IFF_PNG);


   td =  pixEqual(pixs,pix2,&tw);
   
   if (tw == 1){
     printf("%s", "Muna");
   }
   
   printf("%d", td);
   if (td == 1){
     printf("%s", "Tuna");
   }


}
