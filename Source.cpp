#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2/core/types_c.h>

extern "C" {
#include "vc.h"
}

int main(void) {
	// V�deo
	char videofile[20] = "video.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("VC", cv::WINDOW_AUTOSIZE);

	cv::namedWindow("VC2", cv::WINDOW_AUTOSIZE);

	cv::Mat frame, frame2;

	OVC* blobs, * blobs2, * blobs3;
	int nlabels, nlabels2, nlabels3;
	while (key != 'q') {
		/* Leitura de uma frame do v�deo */
		capture.read(frame);
		capture.read(frame2);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* N�mero da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		/* Exemplo de inser��o texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);


		// Fa�a o seu c�digo aqui...
		//
		// Cria uma nova imagem IVC
		IVC *image = vc_image_new(video.width, video.height, 3, 255);
		IVC* image2; // vc_image_new(video.width, video.height, 1, 255);
		IVC* image3 = vc_image_new(video.width, video.height, 1, 255);
		IVC* image4 = vc_image_new(video.width, video.height, 1, 255);
		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		// Executa uma fun��o da nossa biblioteca vc
		//vc_bgr_to_rgb(image,image2);
		image2 = vc_rgb_to_gray(image);
		vc_gray_negative(image2);
		vc_gray_to_binary(image2, 150);
		vc_binary_erode(image2, image3, 11);
		blobs = vc_binary_blob_labelling(image3, image4, &nlabels);
		vc_binary_blob_info(image4, blobs, nlabels);
		//vc_bounding_box_binary(image, blobs, nlabels);
		vc_bounding_box_rgb(image, blobs, nlabels);
		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame2.data, image3->data, video.width * video.height * 1);
		memcpy(frame.data, image->data, video.width* video.height * 3);
		// Liberta a mem�ria da imagem IVC que havia sido criada
		vc_image_free(image);
		vc_image_free(image2);
		vc_image_free(image3);
		
		// +++++++++++++++++++++++++

		/* Exibe a frame */
		cv::imshow("VC", frame);

		cv::imshow("VC2", frame2);

		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	/* Fecha a janela */
	cv::destroyWindow("VC");
	/* Fecha a janela */
	cv::destroyWindow("VC2");

	/* Fecha o ficheiro de v�deo */
	capture.release();

	return 0;
}