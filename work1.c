#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <stdio.h>

#include <leptonica/allheaders.h>
#include <tesseract/capi.h>

#define BLACK 20.0
#define YELLOW 70.0
 
#define min_f(a, b, c)  (fminf(a, fminf(b, c)))
#define max_f(a, b, c) (fmaxf(a, fmaxf(b, c)))


int xflag = 0;
int last_index =0;
int skip_frames = 12;

int t_flag = 0;

//Global Offset for Stitching
int offset = 5;

PIX *pix_temp;
PIX *pixa, *pix1;

l_int32      w, d;



int stitch_images(PIX* im1, PIX* im2, int index){
  l_int32  type, comptype, d1, d2, same, first, last;
  l_float32    fract, diff, rmsdiff;
  NUMA *na1;
  PIX *pixd, *pix1;
       char write_path[100];

  d1 = pixGetDepth(im1);
  d2 = pixGetDepth(im2);

  if (d1 == 1 && d2 == 1) {
   pixEqual(im1, im2, &same);
        if (same) {
            fprintf(stderr, "Images are identical\n");
            pixd = pixCreateTemplate(im1);  /* write empty pix for diff */
        }
        else {
                comptype = L_COMPARE_XOR;
                pixCompareBinary(im1, im2, comptype, &fract, &pixd);
        }
  }

  else {
           //comptype = L_COMPARE_ABS_DIFF;
           comptype = L_COMPARE_SUBTRACT;
           pixCompareGrayOrRGB(im1, im2, comptype, GPLOT_PNG, &same, &diff, &rmsdiff, &pixd);
            if (same){
                fprintf(stderr, "Images are identical\n");
            }

            else {
                fprintf(stderr, "Images differ: <diff> = %10.6f\n", diff);
                fprintf(stderr, "               <rmsdiff> = %10.6f\n", rmsdiff);

                 if (diff >= 30){
                    pixSaveTiled(im1, pixa, 1.0, 0, 1,1);
                    pixSaveTiled(pixd, pixa, 1.0, 0, 1,1);
                    pix1 = pixaDisplay(pixa, w*2, 0.80*d);
                    sprintf(write_path,"im%04d.jpg",index);
                    pixWrite(write_path,pix1,IFF_JFIF_JPEG);
                    pixa = pixaCreate(5);
                }
            }

       if (d1 != 16 && !same) {

            na1 = pixCompareRankDifference(im1, im2, 1);
            if (na1) {

              if (na1->array[250] < 10){
               
              printf("%20.10f\n", na1->array[250]);
              }

            }
       }
  }

return 1;

}
void rgb2lab(float R, float G, float B,float *L, float *a, float *b){
	//Conversion to the CIE-LAB color space to get the Luminance
	float X, Y, Z, fX, fY, fZ;

	X = 0.412453*R + 0.357580*G + 0.180423*B;
	Y = 0.212671*R + 0.715160*G + 0.072169*B;
	Z = 0.019334*R + 0.119193*G + 0.950227*B;

	X /= (255 * 0.950456);
	Y /=  255;
	Z /= (255 * 1.088754);

	if (Y > 0.008856)
	{
		fY = pow(Y, 1.0/3.0);
		*L = 116.0*fY - 16.0;
	}
	else
	{
		fY = 7.787*Y + 16.0/116.0;
		*L = 903.3*Y;
	}

	if (X > 0.008856)
		fX = pow(X, 1.0/3.0);
	else
		fX = 7.787*X + 16.0/116.0;

	if (Z > 0.008856)
		fZ = pow(Z, 1.0/3.0);
	else
		fZ = 7.787*Z + 16.0/116.0;

	*a = 500.0*(fX - fY);
	*b = 200.0*(fY - fZ);

	if (*L < BLACK) {
	*a *= exp((*L - BLACK) / (BLACK / 4));
	*b *= exp((*L - BLACK) / (BLACK / 4));
	*L = BLACK;
	}
	if (*b > YELLOW)
	*b = YELLOW;
}

int detect_stopper_color(int r, int g, int b){

//For Detecting Blue Color
  if (r>=20 && r<=35){
            if(g >=75 && g<=135){
              if(b>=130 && b<=180){
                  return 1;
       }
}
}
   else return 0;
}

void die(const char *errstr) {
	fputs(errstr, stderr);
	exit(1);
}


void russian_ocr(PIX* img, int index){
	TessBaseAPI *handle;
	char *text;
        FILE * fp; 
        char write_path[100];

	handle = TessBaseAPICreate();

	if(TessBaseAPIInit3(handle, NULL, "rus") != 0)
		die("Russian Language Not Set for Tesseract\n");

	TessBaseAPISetImage2(handle, img);

	if(TessBaseAPIRecognize(handle, NULL) != 0)
		die("Error in Tesseract recognition\n");

	if((text = TessBaseAPIGetUTF8Text(handle)) == NULL)
		die("Error getting text\n");

        //Writing to a File
        sprintf(write_path,"txt%04d.txt",index);
        fp = fopen(write_path, "w");
        fprintf(fp, "%s", text);
        fclose(fp);

        //Displaying it
	fputs(text, stdout);

	TessDeleteText(text);
	TessBaseAPIEnd(handle);
	TessBaseAPIDelete(handle);

}

void _process_frame_tickertext(AVFrame *frame, int width, int height, int index){
      printf("index%d\n", index);    
  if (index >1600){
	PIX *im;
	PIX *edge_im;
	PIX *lum_im;
	PIX *feat_im;
	char *subtitle_text=NULL;
	im = pixCreate(width,height,32);
	lum_im = pixCreate(width,height,32);
	feat_im = pixCreate(width,height,32);
	int i,j;
        




	for(i=(92*height)/100;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			int p=j*3+i*frame->linesize[0];
			int r=frame->data[0][p];
			int g=frame->data[0][p+1];
			int b=frame->data[0][p+2];
			pixSetRGBPixel(im,j,i,r,g,b);
			float L,A,B;
			rgb2lab((float)r,(float)g,(float)b,&L,&A,&B);
			if(L > 85)
				pixSetRGBPixel(lum_im,j,i,255,255,255);
			else
				pixSetRGBPixel(lum_im,j,i,0,0,0);
		}
	}



	//Handle the edge image
	edge_im = pixCreate(width,height,8);
	edge_im = pixConvertRGBToGray(im,0.0,0.0,0.0);
	edge_im = pixSobelEdgeFilter(edge_im, L_VERTICAL_EDGES);
	edge_im = pixDilateGray(edge_im, 21, 11);
	edge_im = pixThresholdToBinary(edge_im,45);

	for(i=92*(height/100);i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			unsigned int p1,p2,p3;
			pixGetPixel(edge_im,j,i,&p1);
			pixGetPixel(lum_im,j,i,&p3);
			if(p1==0&&p3>0)
				pixSetRGBPixel(feat_im,j,i,255,255,255);
			else
				pixSetRGBPixel(feat_im,j,i,0,0,0);
		}
	}


       int num_of_occr = 0;
       int stop = 0;
       for(i=(92*height)/100;i<height && !stop;i++){
            for(j=0; j<width  && !stop; j++){
 
                 int p=j*3+i*frame->linesize[0];
			int r=frame->data[0][p];
			int g=frame->data[0][p+1];
			int b=frame->data[0][p+2];

             //printf("r %d\n", r);
             //printf("g %d\n", g);
            // printf("b %d\n", b);

         //Detecting Blue Stoppers
         //Skipping Thirty Frames and Detecting Blue Color
        if (detect_stopper_color(r,g,b) == 1 && index % 230 == 0 && t_flag == 0){
         //printf("%s\n", "Stopper Detected");  
         xflag = 1;
         t_flag = 1;
         //Hardcoded Bouding Box Locations
         BOX* box = boxCreate(100, 525, 615, 25);
         PIX* pixd= pixClipRectangle(feat_im, box, NULL);
         last_index = index;
         pix_temp = pixCopy(NULL, pixd);
         russian_ocr(pixd, index);
         pixGetDimensions(pixd, &w, NULL, &d);




         //sprintf(write_path,"im%04d.jpg",index);
         //pixWrite(write_path,pixd,IFF_JFIF_JPEG);
         boxDestroy(&box);
	 pixDestroy(&pixd);
         stop = 1;
         }

         else if (detect_stopper_color(r,g,b) == 1 && index % (230 + offset) == 0 && t_flag == 1){
         //printf("%s\n", "Stopper Detected");  
         xflag = 1; 
         t_flag = 0;
         //Hardcoded Bouding Box Locations
         BOX* box = boxCreate(100, 525, 615, 25);
         PIX* pixd= pixClipRectangle(feat_im, box, NULL);
         last_index = index;
         int temp_varx = stitch_images(pix_temp, pixd, index);
         russian_ocr(pixd, index);


         

         boxDestroy(&box);
	 pixDestroy(&pixd);
	 //pixDestroy(&pixa);
         stop = temp_varx;

         }
         //Checking of text after blue stopper
         else if(detect_stopper_color(r,g,b) == 0 && index % (230 + offset) == 0 && xflag == 1){
         BOX* box = boxCreate(100, 525, 615, 25);
         PIX* pixd= pixClipRectangle(feat_im, box, NULL);
         russian_ocr(pixd, index);

         pixSaveTiled(pixd, pixa, 1.0, 0, 1,1);
         //sprintf(write_path,"im%04d.jpg",index);
         //pixWrite(write_path,pixd,IFF_JFIF_JPEG);
         boxDestroy(&box);
	 pixDestroy(&pixd);
         xflag = 0;
         stop = 1;
        }
     }

}

	pixDestroy(&lum_im);
	pixDestroy(&im);
	pixDestroy(&edge_im);
	pixDestroy(&feat_im);

}
}

int main(int argc, char * argv[]) {
	AVFormatContext *pFormatCtx = NULL;
	int             i, frame_count, videoStreamIdx;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;
	AVFrame         *pFrame = NULL;
	AVFrame         *pFrameRGB = NULL;
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer = NULL;

        pixa = pixaCreate(5);

    
	AVDictionary *optionsDict = NULL;
	struct SwsContext *sws_ctx = NULL;
	
	if(argc < 2) {
		printf("Please provide a video file\n");
		return -1;
	}
	
	// Register all formats and codecs
	av_register_all();
	
	// Open video file
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
		return -1; // Couldn't open file
		
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information
	
	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);
	
	// Find the first video stream
	videoStreamIdx = -1;
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStreamIdx = i;
			break;
		}
	}
	
	if(videoStreamIdx==-1)
		return -1; // Didn't find a video stream
	
	// Get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStreamIdx]->codec;
	
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}
	
	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
		return -1; // Could not open codec
		
	// Allocate video frame
	pFrame = av_frame_alloc();
	
	// Allocate an AVFrame structure
	pFrameRGB = av_frame_alloc();
	if(pFrameRGB==NULL)
		return -1;
	
	// Determine required buffer size and allocate buffer
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	
	sws_ctx = sws_getContext (
		pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24,
		SWS_BILINEAR, NULL, NULL, NULL
	);
	
	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	
	// Read frames and count the number of frames
	frame_count = 0;
	while(av_read_frame(pFormatCtx, &packet)>=0) {
		//Is this a packet from a video stream?
		if(packet.stream_index==videoStreamIdx) {
			// Decode the video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			
			//Did we get a video frame?
			if(frameFinished) {
				// Increment frame count
				frame_count++;
				
				// Convert the image from its native format to RGB
				sws_scale(
					sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0,
					pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize
				);

         _process_frame_tickertext(pFrameRGB,pCodecCtx->width,pCodecCtx->height,frame_count);
//_display_frame(pFrameRGB,pCodecCtx->width,pCodecCtx->height,frame_count);
				
				//Now you have the frame and can do whatever you wish to
				
				
			}
		}
		
		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);	
	}
	
	// Free the RBG image
	av_free(buffer);
	av_free(pFrameRGB);
	
	//Free the YUV frame
	av_free(pFrame);
	
	//Close the codec
	avcodec_close(pCodecCtx);
	
	//Close the video file
	avformat_close_input(&pFormatCtx);
	
	return 0;
}
