//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);


/* AULA 3 - 3/3/2021*/

/*Função para calcular negativo de uma imagem*/

int vc_gray_negative(IVC *srcdst);
/*Função para calcular negativo de uma imagem RGB*/

int vc_rgb_negative(IVC *srcdst);

/* AULA 3 - 3/3/2021*/

/* Funções para representar a componente R , G e B em tons de cinzento */

IVC *vc_rgb_get_red_gray(IVC* srcdst);
IVC *vc_rgb_get_green_gray(IVC* srcdst);
IVC *vc_rgb_get_blue_gray(IVC* srcdst);

/* AULA 4 - 5/3/2021*/

/* Funções para converter RGB em tons de cinzento - formula ponderada - 1 canal destino */
IVC *vc_rgb_to_gray(IVC *src);


/* Funções para converter RGB em tons de cinzento - formula aritmética - 1 canal destino */
IVC *vc_rgb_to_gray_avg(IVC *src);


/* AULA 5 - 10/3/2021*/

// Conversão de RGB para HSV
int vc_rgb_to_hsv(IVC *srcdst);
int vc_hsv_segmentation(IVC *srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

/* AULA 7 - 10/3/2021*/

// Binarização por thresholding manual
int vc_gray_to_binary(IVC *srcdst, int threshold);
int vc_gray_to_binary_2_thresholds(IVC *srcdst, int minThreshold, int maxThreshold);

//Binarização, por thresholding através da média global
int vc_gray_to_binary_global_mean(IVC *srcdst);

//binarização, por thresholding adaptativo midpoint, de uma imagem em tons de cinzento

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel,  int Cmin);

int vc_gray_to_binary_niblack(IVC *src, IVC *dst, int kernel, double alfa);

int vc_subtract_image(IVC *src, IVC* subtractor);


//------------------------------------------------------------
int vc_binary_dilate(IVC *src, IVC* dst, int kernel);

int vc_binary_erode(IVC *src, IVC* dst, int kernel);

int vc_binary_open(IVC *src, IVC *dst, int kernel);

int vc_binary_close(IVC *src, IVC *dst, int kernel);

int histograma(IVC *src, IVC *dst);
int vc_histograma_gray(IVC *src, IVC *dst);


int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Per�metro
	int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
int vc_bounding_box_binary(IVC *src,OVC* blobs,int numeroBlobs);
int vc_blob_stats_to_txt(OVC *blobs, int numberOfBlobs);
int vc_bounding_box_rgb(IVC *src,OVC* blobs,int numeroBlobs);
int vc_bgr_to_rgb(IVC* imagemEntrada, IVC* imagemSaida);
IVC* vc_remove_background_rgb(IVC* backgroundFrame, IVC* realtimeFrame, int T);