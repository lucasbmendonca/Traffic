//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
				
				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}

/* AULA 3 - 3/3/2021*/

/*Função para calcular negativo de uma imagem a cinzento*/

int vc_gray_negative(IVC *srcdst)
{
    if(srcdst != NULL && srcdst->channels == 1)
    {
        int i;
        int flag = srcdst->height * srcdst->bytesperline;
        for(i = 0; i < flag; i+=srcdst->channels)
		{
			if(srcdst->data[i] <= 255 && srcdst->data != NULL)
			{
				srcdst->data[i]= 255 - srcdst->data[i];
			}
			else
			{
				printf("srcdst->data[%d]\tVALUE INVALID: %d", i, srcdst->data[i]);
			}

		}
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
    }
	return 0;
}

/*Função para calcular negativo de uma imagem RGB*/

int vc_rgb_negative(IVC *srcdst)
{
	if(srcdst != NULL && srcdst->channels == 3)
    {
        int i;
        int flag = srcdst->height * srcdst->bytesperline;
        for(i = 0; i < flag; i+=srcdst->channels)
		{
			srcdst->data[i]= 255 - srcdst->data[i];
			srcdst->data[i+1]= 255 - srcdst->data[i+1];
			srcdst->data[i+2]= 255 - srcdst->data[i+2];
		}
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
    }
	return 0;
}

/* AULA 3 - 3/3/2021*/

/* Funções para representar a componente R , G e B em tons de cinzento */

IVC *vc_rgb_get_red_gray(IVC* srcdst)
{
	if(srcdst != NULL && srcdst->channels == 3)
    {
        int i, j;
		j = 0;
        int flag = srcdst->height * srcdst->bytesperline;
		IVC *redInGray = vc_image_new(srcdst->width, srcdst->height, 1, 255);
		
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			redInGray->data[j] = srcdst->data[i];
			j++;

		}
		return redInGray;
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
		exit(1);
    }
}

IVC *vc_rgb_get_green_gray(IVC* srcdst)
{
	if(srcdst != NULL && srcdst->channels == 3)
    {
        int i, j;
		j = 0;
        int flag = srcdst->height * srcdst->bytesperline;
		IVC *greenInGray = vc_image_new(srcdst->width, srcdst->height, 1, 255);
		
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			greenInGray->data[j] = srcdst->data[i+1];
			j++;

		}
		return greenInGray;
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
		exit(1);
    }
}

IVC *vc_rgb_get_blue_gray(IVC* srcdst)
{
	if(srcdst != NULL && srcdst->channels == 3)
    {
        int i, j;
		j = 0;
        int flag = srcdst->height * srcdst->bytesperline;
		IVC *blueInGray = vc_image_new(srcdst->width, srcdst->height, 1, 255);
		
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			blueInGray->data[j] = srcdst->data[i+2];
			j++;

		}
		return blueInGray;
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
		exit(1);
    }
}

/* AULA 4 - 5/3/2021*/

/* Funções para converter RGB em tons de cinzento - formula ponderada - 1 canal destino */

IVC *vc_rgb_to_gray(IVC *src)
{
	if(src != NULL && src->channels == 3)
    {
        int i, j;
        int flag = src->height * src->bytesperline;
		IVC *rgbInGray = vc_image_new(src->width, src->height, 1, src->levels);
		j = 0;
		int destPos = 0;
		
		for(i = 0; i < flag; i+=src->channels)
		{
			destPos = i * rgbInGray->channels;
			rgbInGray->data[j] = (0.299 * (float)src->data[i]) + (0.587 * (float)src->data[i+1]) + (0.114 * (float)src->data[i+2]);
			j++;

		}
		return rgbInGray;
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
		exit(1);
    }

	
}

/* Funções para converter RGB em tons de cinzento - formula aritmetica - 1 canal destino */


IVC *vc_rgb_to_gray_avg(IVC *src)
{
	if(src != NULL && src->channels == 3)
    {
        int i, j;
        int flag = src->height * src->bytesperline;
		IVC *rgbInGray = vc_image_new(src->width, src->height, 1, src->levels);
		j = 0;
		
		for(i = 0; i < flag; i+=src->channels)
		{
			rgbInGray->data[j] = (src->data[i] + src->data[i+1] + src->data[i+2])/3;
			j++;

		}
		return rgbInGray;
    }
    else
    {
        printf("Invalid Image/Channel Value\n");
		exit(1);
    }
}

/* AULA 5 - 10/3/2021*/

// Conversão de RGB para HSV
int vc_rgb_to_hsv(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i<size; i = i + channels)
	{
		r = (float)data[i];
		g = (float)data[i + 1];
		b = (float)data[i + 2];

		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0,255]
		data[i] = (unsigned char) (hue / 360.0f * 255.0f);
		data[i + 1] = (unsigned char) (saturation);
		data[i + 2] = (unsigned char) (value);
	}

	return 1;
}


// hmin,hmax = [0, 360]; smin,smax = [0, 100]; vmin,vmax = [0, 100]
int vc_hsv_segmentation(IVC *srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int hue, saturation, value;
	int h, s, v; // h=[0, 360] s=[0, 100] v=[0, 100]
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		h = ((float)data[i]) / 255.0f * 360.0f;
		s = ((float)data[i + 1]) / 255.0f * 100.0f;
		v = ((float)data[i + 2]) / 255.0f * 100.0f;

		if ((h > hmin) && (h <= hmax) && (s >= smin) && (s <= smax) && (v >= vmin) && (v <= vmax))
		{
			data[i] = 255;
			data[i + 1] = 255;
			data[i + 2] = 255;
		}
		else
		{
			data[i] = 0;
			data[i + 1] = 0;
			data[i + 2] = 0;
		}
	}

	return 1;
}


/* AULA 7 - 10/3/2021*/

// Binarização por thresholding manual
int vc_gray_to_binary(IVC *srcdst, int threshold)
{
	if(srcdst != NULL && srcdst->channels == 1 && threshold >= 0 && threshold <= 255)
    {
        int i;
        int flag = srcdst->height * srcdst->bytesperline;
		
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			if(srcdst->data[i]<=threshold)
			{
				srcdst->data[i] = 0;
			}
			else
			{
				srcdst->data[i] = 255;
			}

		}
		return 0;
    }
	else if((threshold < 0) || (threshold > 255))
	{
		printf("ERROR: Invalid threshold value\n");
	}
    else
    {
        printf("ERROR: Invalid Image/Channel Value\n");
    }
	return(-1);
}

int vc_gray_to_binary_2_thresholds(IVC *srcdst, int minThreshold, int maxThreshold)
{
	long int pos;
	int x,y;

	for(y = 0; y< srcdst->height; y++)
	{
		for(x = 0; x< srcdst->width; x++)
		{
			pos = (y * srcdst->bytesperline) + (x * srcdst->channels);
			
			if(srcdst->data[pos] <= maxThreshold && srcdst->data[pos] >= minThreshold )
			{
				srcdst->data[pos] = 255;
			}
			else
			{
				srcdst->data[pos] = 0;
			}
		}
	}
	
	return(0);
}

//Binarização, por thresholding através da média global

int vc_gray_to_binary_global_mean(IVC *srcdst)
{
	if(srcdst != NULL && srcdst->channels == 1)
    {
        int i, sum;
		double avgThreshold;
        int flag = srcdst->height * srcdst->bytesperline;
		sum = 0;
		avgThreshold = 0;
		
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			sum += srcdst->data[i];
		}

		avgThreshold = (double)sum/flag;
		printf("Average Threshold = %lf\n", avgThreshold);
		for(i = 0; i < flag; i+=srcdst->channels)
		{
			if((double)srcdst->data[i]<=avgThreshold)
			{
				srcdst->data[i] = 0;
			}
			
			else
			{
				srcdst->data[i] = 255;
			}
		}

		return 0;
    }
    else
    {
        printf("ERROR: Invalid Image/Channel Value\n");
    }
	return(-1);

}



//binarização, por thresholding adaptativo midpoint, de uma imagem em tons de cinzento

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel)
{
	int x, y, kx, ky;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int max, min;
	long int pos, posk;
	double threshold;


	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return -1;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return -1;
	if (src->channels != 1 || dst->channels != 1) return 0;

	for (y = 0; y<src->height; y++)
	{
		for (x = 0; x<src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;

			max = 0;
			min = 255;

			// NxM Vizinhos
			for (ky = -offset; ky <= offset; ky++)
			{
				for (kx = -offset; kx <= offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky < src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;

						if (src->data[posk] > max) max = src->data[posk];
						if (src->data[posk] < min) min = src->data[posk];
					}
				}
			}

			threshold = ((double)(max + min) / (double)2);

			if ((double)src->data[pos] > threshold) dst->data[pos] = 255;
			else dst->data[pos] = 0;
		}
	}

	return 0;
}

int vc_binary_dilate(IVC *src, IVC* dst, int kernel) {

	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int x, y, yy, xx;
	long int pos_src, posk;

	// Verifica��o de Erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	int offset = kernel / 2;
	int verifica;

	for (y = offset; y <= height - offset; y++) {

		for (x = offset; x <= width - offset; x++) {

			pos_src = y * bytesperline_src + x * channels_src;

			verifica = 0;

			for (yy = y-offset; yy <= y + offset; yy++) {

				for (xx = x-offset; xx <= x + offset; xx++) {

					posk = yy * bytesperline_src + xx * channels_src;

					if (datasrc[posk] == 255) { verifica = 255; }

				}
			}

            datadst[pos_src] = verifica;
		}
	}

	return 1;

}

int vc_binary_erode(IVC *src, IVC *dst, int kernel) {
    unsigned char *datasrc = (unsigned char*)src->data;
    int bytesperline_src = src->width*src->channels;
    int channels_src = src->channels;
    unsigned char *datadst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline_dst = dst->width*dst->channels;
    int channels_dst = dst->channels;
    int x, y, x2, y2;
    long int pos_src, pos_dst;
    int verifica;
    kernel /= 2;
    
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
    if ((src->width != dst->width) || (src->height != dst->height))return 0;
    if ((src->channels != 1) || (dst->channels != 1))return 0;
    
    
    for (y = kernel; y < height - kernel; y++)
    {
        for (x = kernel; x < width - kernel; x++)
        {
            pos_dst = y*bytesperline_dst + x*channels_dst;
            
            verifica = 255;
            
            for (y2 = y - kernel; y2 <= y + kernel; y2++)
            {
                for (x2 = x - kernel; x2 <= x + kernel; x2++)
                {
                    pos_src = y2*bytesperline_src + x2*channels_src;
                    if (datasrc[pos_src] == 0) { verifica = 0; }
                }
            }
            
            datadst[pos_dst] = verifica;
            
        }
    }
    
    
    return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel) {

	IVC *dstTemp = vc_image_new(src->width, src->height, 1, src->levels);

	int ret = 1;

	ret &= vc_binary_erode(src, dstTemp, kernel);

	ret &= vc_binary_dilate(dstTemp, dst, kernel); // ret para verificar erro de 0 ou 1

	vc_image_free(dstTemp);

	return ret;

}

int vc_binary_close(IVC *src, IVC *dst, int kernel) {

	IVC *dstTemp = vc_image_new(src->width, src->height, 1, src->levels);

	int ret = 1;

	ret &= vc_binary_dilate(src, dstTemp, kernel); // ret para verificar erro de 0 ou 1

	ret &= vc_binary_erode(dstTemp, dst, kernel);

	vc_image_free(dstTemp);

	return ret;
}


int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, double alfa)
{
	int x, y, avgSum, T;
	long int pos, kernelX, kernelY, posKernel;
	double varianceSum, avg, variance, stdDeviation;
	int filterRange = (kernel - 1) / 2;

	//error check
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0; //confirma que a imagem existe
	if ((dst->width != src->width) || (dst->height != dst->height)) return 0;
	if (src->channels != 1 || dst->channels != 1) return 0; //confirma que a imagem da src é cinza e do dst é RGB

	

	for (y = filterRange; y < src->height - filterRange; y++) 
	{
		for (x = filterRange; x < src->width - filterRange; x++) 
		{
			
			pos = (y * src->bytesperline) + (x * src->channels);
			avgSum = 0;
			//calculo da média da vizinhança
			for (kernelY = - filterRange; kernelY <=filterRange; kernelY++)
			{
				for(kernelX = - filterRange; kernelX <=filterRange; kernelX++)
				{
					posKernel = ((y - kernelY)*src->bytesperline) + ((x-kernelX) * src->channels);
					avgSum += src->data[posKernel];
				}
			}

			avg = (avgSum / (double)(kernel*kernel));

			//calculo do desvio padrão da vizinhança

			varianceSum = 0;
			for (kernelY = - filterRange; kernelY <=filterRange; kernelY++)
			{
				for(kernelX = - filterRange; kernelX <=filterRange; kernelX++)
				{
					posKernel = ((y - kernelY)*src->bytesperline) + ((x-kernelX) * src->channels);
					varianceSum += (src->data[posKernel] - avg) * (src->data[posKernel] - avg);
				}
			}

			variance = (varianceSum/((kernel*kernel)-1));

			stdDeviation = sqrt(variance);

			T = (int)(avg + (alfa * stdDeviation));

			if (src->data[pos] > T)
				dst->data[pos] = 255;
			else
				dst->data[pos] = 0;
		}
	}

	return 1;
}

int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel,  int Cmin)
{
	int x, y, kernelMax, kernelMin, T;
	long int pos, posKernel;
	int filterRange = (kernel - 1) / 2;

	//error check
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0; //confirma que a imagem existe
	if ((dst->width != src->width) || (dst->height != dst->height)) return 0;
	if (src->channels != 1 || dst->channels != 1) return 0; //confirma que a imagem da src é cinza e do dst é RGB

	

	for (y = filterRange; y < src->height - filterRange; y++) 
	{
		for (x = filterRange; x < src->width - filterRange; x++) 
		{
			
			pos = (y * src->bytesperline) + (x * src->channels);

			//calculo do maximo e minimo

			kernelMax = 0;
			kernelMin = 255;
			for (posKernel = - filterRange; posKernel<=filterRange; posKernel++)
			{
				if(src->data[pos + posKernel] > kernelMax)
				{
					kernelMax == src->data[pos + posKernel];
				}
				if(src->data[pos + posKernel] < kernelMin)
				{
					kernelMin == src->data[pos + posKernel];
				}
			}

			if(Cmin > (kernelMax - kernelMin))
			{
				T = (unsigned char)src->levels/(float)2;
			}
			else
			{
				T = (unsigned char)((float)(kernelMax + kernelMin) / (float)2);
			}
 



			if (src->data[pos] > T)
				dst->data[pos] = 255;
			else
				dst->data[pos] = 0;
		}
	}

	return 1;
}



// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (src->channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(dst->data, src->data, (src->bytesperline * src->height));

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 255 labels
	for (i = 0, size = src->bytesperline * src->height; i<size; i++)
	{
		if (dst->data[i] != 0) dst->data[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<src->height; y++)
	{
		dst->data[y * src->bytesperline + 0 * src->channels] = 0;
		dst->data[y * src->bytesperline + (src->width - 1) * src->channels] = 0;
	}
	for (x = 0; x<src->width; x++)
	{
		dst->data[0 * src->bytesperline + x * src->channels] = 0;
		dst->data[(src->height - 1) * src->bytesperline + (x * src->channels)] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<src->height - 1; y++)
	{
		for (x = 1; x<src->width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * src->bytesperline + (x - 1) * src->channels; // A
			posB = (y - 1) * src->bytesperline + x * src->channels; // B
			posC = (y - 1) * src->bytesperline + (x + 1) * src->channels; // C
			posD = y * src->bytesperline + (x - 1) * src->channels; // D
			posX = y * src->bytesperline + x * src->channels; // X

													// Se o pixel foi marcado
			if (dst->data[posX] != 0)
			{
				if ((dst->data[posA] == 0) && (dst->data[posB] == 0) && (dst->data[posC] == 0) && (dst->data[posD] == 0))
				{
					dst->data[posX] = (unsigned char)label;
					labeltable[label] = (unsigned char)label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (dst->data[posA] != 0) num = labeltable[dst->data[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((dst->data[posB] != 0) && (labeltable[dst->data[posB]] < num)) num = labeltable[dst->data[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((dst->data[posC] != 0) && (labeltable[dst->data[posC]] < num)) num = labeltable[dst->data[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((dst->data[posD] != 0) && (labeltable[dst->data[posD]] < num)) num = labeltable[dst->data[posD]];

					// Atribui a etiqueta ao pixel
					dst->data[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (dst->data[posA] != 0)
					{
						if (labeltable[dst->data[posA]] != num)
						{
							for (tmplabel = labeltable[dst->data[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (dst->data[posB] != 0)
					{
						if (labeltable[dst->data[posB]] != num)
						{
							for (tmplabel = labeltable[dst->data[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (dst->data[posC] != 0)
					{
						if (labeltable[dst->data[posC]] != num)
						{
							for (tmplabel = labeltable[dst->data[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (dst->data[posD] != 0)
					{
						if (labeltable[dst->data[posD]] != num)
						{
							for (tmplabel = labeltable[dst->data[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}


	// Volta a etiquetar a imagem
	for (y = 1; y<src->height - 1; y++)
	{
		for (x = 1; x<src->width - 1; x++)
		{
			posX = (y * src->bytesperline) + (x * src->channels); // X

			if (dst->data[posX] != 0)
			{
				dst->data[posX] = labeltable[dst->data[posX]];
			}

		}	
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1 ; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b])
			{
				labeltable[b] = 0;
			}
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	int histmax, histthreshold;
	int sumx, sumy;
	int hist[256] = { 0 };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
		}
	}

	// Compute a grey level histogram
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			hist[datadst[y * bytesperline + x * channels]]++;
		}
	}

	// Threshold at the middle of the occupied levels
	histmax = 0;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i;

	// Apply the threshold
	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			posX = y * bytesperline + x * channels;

			if (datadst[posX] >= histthreshold) datadst[posX] = 255;
			else datadst[posX] = 0;
		}
	}

	return 1;
}

int histograma(IVC *src, IVC *dst) {
	int x, y,i;
	long int pos;
	int histogram[256] = { 0 };
	double pdf[256];
	float temp;
	double max = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (src->channels != 1) return 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width;x++)
		{
			pos = y * src->bytesperline + x * src->channels;
			
			histogram[src->data[pos]]++;
		}
	}

	for (i = 0; i < src->levels; i++)
	{
		pdf[i] = (histogram[i] / (double)(src->width*src->height));
		if (pdf[i] > max)
			max = pdf[i];
	}

	
	/*for(i = 0; i<256; i++)
	
	{	
		printf("Value: %d\t Frequency: %f\t\t", i, pdf[i]);
		if(i%2 == 0)
		{
			printf("\n");
		}
	}*/
	
	for (x = 0; x < dst->width;x++)
	{
		temp = ((pdf[x] * 255) / max);
		for (y = 255; y >= 0; y--)
		{
			pos = y * dst->bytesperline + x * dst->channels;

			if (temp >= (float)((float)(255) - (float)y))
				dst->data[pos] = (unsigned char)255;
			else
				dst->data[pos] = (unsigned char)0;
		}
	}

	return 1;
}


int vc_histograma_gray(IVC *src, IVC *dst)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int byteperline = src->width*src->channels;
	int channels = src->channels;
	int x, y;
	long int pos;
	int contarpixeis[256] = { 0 };
	float pdf[256];
	float conta = 0;
	float max = 0;
	double temp;
	float cdf[256] = { 0 };
	float equalizacao[256] = { 0 };



	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	//numero pixeis repetidos
	for (y = 0;y<height;y++)
	{
		for (x = 0;x<width;x++)
		{
			pos = y*byteperline + x*channels;
			contarpixeis[(int)data[pos]]++;
		}
	}

	//clacula pdf
	for (y = 0;y<256;y++)
	{
		pdf[y] = (float)contarpixeis[y] / (float)(width*height);
		conta += pdf[y];

		if (max<pdf[y])
			max = pdf[y];
	}

	//calcula grafico cdf
	for (x = 0; x < 256; x++)
	{
		for (y = x;y >= 0;y--)
		{
			cdf[x] += pdf[y];
		}
	}


	for (y = 0;y<height;y++)
	{
		for (x = 0;x<width;x++)
		{
			pos = y*byteperline + x*channels;
			dst->data[pos] = cdf[data[pos]] * 255;
		}
	}
	return 1;
}

int vc_subtract_image(IVC *src, IVC* subtractor)
{
	int x, y;
	long int pos;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != subtractor->width) || (src->height != subtractor->height) || (src->channels != subtractor->channels)) return 0;
	if (src->channels != 1) return 0;
	
	for(y = 0; y<src->height; y++)
	{
		for (x = 0; x< src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;

			if(subtractor->data[pos] == 0)
			{
				src->data[pos] = 0;
			}
		}
	}

	return 1;
}

int vc_bounding_box_rgb(IVC *src,OVC* blobs,int numeroBlobs)
{
	int i, yy,xx;
	long int pos;

	// Verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->channels != 3)) return 0;

	//iteração pela altura e largura da caixa
	for (i = 0; i < numeroBlobs; i++) {
		//iterações na altura da caixa
		for (yy = blobs[i].y; yy <= blobs[i].y + blobs[i].height;yy++) {
			//iterações a largura da caixa
			for (xx = blobs[i].x; xx <= blobs[i].x + blobs[i].width;xx++) {

				pos = yy*src->bytesperline + xx *src->channels;
				//condiçao para colocar a 255 apenas os pixeis do limite da caixa
				if (yy == blobs[i].y || yy == blobs[i].y + blobs[i].height || xx == blobs[i].x || xx == blobs[i].x + blobs[i].width) {
							src->data[pos] = 0;
							src->data[pos+1] = 255;
							src->data[pos+2] = 0;
				}
			}
		}
	}

	return 1;

}

int vc_bounding_box_binary(IVC *src,OVC* blobs,int numeroBlobs)
{
	int i, yy,xx;
	long int posk;

	// Verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->channels != 3)) return 0;

	//percorre os blobs da imagem e para cada blob vai percorrer a altura e o comprimento da sua bounding box
	for (i = 0; i < numeroBlobs; i++) {
		//percorre a altura da box
		for (yy = blobs[i].y; yy <= blobs[i].y + blobs[i].height;yy++) {
			//percorre a largura da box
			for (xx = blobs[i].x; xx <= blobs[i].x + blobs[i].width;xx++) {

				posk = yy*src->bytesperline + xx *src->channels;
				//condiçao para colocar a caixa a branco
				if (yy == blobs[i].y || yy == blobs[i].y + blobs[i].height || xx == blobs[i].x || xx == blobs[i].x + blobs[i].width) {
							src->data[posk] = 0;
				}
			}
		}
	}

	return 1;

}

int vc_blob_stats_to_txt(OVC *blobs, int numberOfBlobs)
{
	int i;
	FILE *fp;
	fp = fopen("stats.txt", "w+");
	for (i= 0; i < numberOfBlobs; i++)
	{
		fprintf(fp, "Blob: %d\tArea: %d pixels\tPerimeter: %d pixels\t Mass Center: x-%d y-%d\n", i, blobs[i].area, blobs[i].perimeter, blobs[i].xc, blobs[i].yc);
	}
	fclose(fp);
	return 0;
}

int vc_bgr_to_rgb(IVC* imagemEntrada, IVC* imagemSaida)
{
	int x, y, pos;

	if ((imagemSaida->width <= 0) || (imagemSaida->height <= 0)) {
		return 0;
	}

	if (imagemSaida->channels != 3) {
		return 0;
	}

	for (y = 0; y < imagemSaida->height; y++)
	{
		for (x = 0; x < imagemSaida->width; x++)
		{
			pos = y * imagemSaida->bytesperline + x * imagemSaida->channels;

			imagemSaida->data[pos] = imagemEntrada->data[pos + 2];
			imagemSaida->data[pos + 1] = imagemEntrada->data[pos + 1];
			imagemSaida->data[pos + 2] = imagemEntrada->data[pos];
		}
	}

	return 0;
}

IVC* vc_remove_background_rgb(IVC* backgroundFrame, IVC* realtimeFrame, int T)
{
	IVC* difference = vc_image_new(backgroundFrame->width, backgroundFrame->height, backgroundFrame->channels, backgroundFrame->levels);

	int x, y;
	long int pos;

	for (y = 0; y < difference->height; y++)
	{
		for (x = 0; x < difference->width; x++)
		{
			pos = y * (backgroundFrame->bytesperline) + (x * backgroundFrame->channels);

			if (abs(backgroundFrame->data[pos] - realtimeFrame->data[pos]) < T || abs(backgroundFrame->data[pos + 1] - realtimeFrame->data[pos + 1]) < T || abs(backgroundFrame->data[pos + 2] - realtimeFrame->data[pos + 2]) < T)
			{
				difference->data[pos] = 0;
				difference->data[pos + 1] = 0;
				difference->data[pos + 2] = 0;
			}

			else
			{
				difference->data[pos] = realtimeFrame->data[pos];
				difference->data[pos + 1] = realtimeFrame->data[pos + 1];
				difference->data[pos + 2] = realtimeFrame->data[pos + 2];
			}
		}
	}

	return difference;
}