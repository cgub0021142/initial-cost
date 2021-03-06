
//bior2.2 bior4.4 就是5/3 9/7
// dwt2_sym(name,sig,cA,cH,cV,cD);////important
//===========================================================
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <complex>
#include <cmath>
#include <algorithm>
#include "wavelet2d.h"
#include <opencv2\opencv.hpp>
using namespace std;
using namespace cv;
void* maxval(vector<vector<double> > &arr, double &max) {
	max = 0;
	//vector size----->Returns the number of elements in the vector.
	for (unsigned int i = 0; i < arr.size(); i++) {
		for (unsigned int j = 0; j < arr[0].size(); j++) {
			if (max <= arr[i][j]) {
				max = arr[i][j];
			}
		}
	}
	return 0;
}
void drawHistImg(const Mat &src, Mat &dst) {
	int histSize = 256;
	float histMaxValue = 0;
	for (int i = 0; i<histSize; i++) {
		float tempValue = src.at<float>(i);
		if (histMaxValue < tempValue) {
			histMaxValue = tempValue;
		}
	}

	float scale = (0.9 * 256) / histMaxValue;
	for (int i = 0; i<histSize; i++) {
		int intensity = static_cast<int>(src.at<float>(i)*scale);
		line(dst, Point(i, 255), Point(i, 255 - intensity), Scalar(0));
	}
}
Mat forMerge[3],forSee[3];
Mat waveletTrans(Mat img) {
	//分成三個通道，for跑三次，再用merge合併

	
	int height, width;
	height = img.rows;
	width = img.cols;
	

	CvSize size;
	size.width = width;
	size.height = height;
	
	//imshow("Original Image", img);
	int rows = (int)height;
	int cols = (int)width;

	//把圖片轉成vector<vector<double> > 型別
	Mat matimg(img);//同一個 mat ，指到的位置是一樣的，這邊要轉成vector因為要dwt
	//覺得怪怪的這邊的col是直接假設單通道嗎   row=y長
	vector<vector<double> > vecinput(rows, vector<double>(cols));
	string nm = "haar";
    //string nm = "bior2.2";
	//string nm = "bior4.4";
	cout << "小波基底" << nm << endl;

	if (img.channels() == 3) {
		//vector<Vec3d > vecinput(rows, Vec3d(cols));
		//分成三個
		for (int k = 0; k < 3; k++) {

			for (int i = 0; i < rows; i++) {
				for (int j = 0; j < cols; j++) {
					unsigned char temp;
					temp = ((uchar*)matimg.data + i * matimg.step1(0))[j * matimg.elemSize() + k];//先指到矩陣該列的第一個，在往後指，k=1(green?)

					vecinput[i][j] = (double)temp;
				}
			}
			//驗證
			//cout << "step" << matimg.step << " " << matimg.step[0] << " " << matimg.step[1] << " " << matimg.elemSize() << endl;
			//elementsize=1因為8U
			//step每一個維度的元素個數(byte)------>要跳下一個要多少byte，step[0] step[1] step[2]


			vector<int> length;
			vector<double> output, flag;
			int J = 1;//////////////////階數


			//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓code start下方為計算

			dwt_2d_sym(vecinput, J, nm, output, flag, length);//因為輸出*****(output)********是一維矩陣(convolution) 
														  //所以要記錄y x長度，用來重新排列成2d矩陣

			double max;
			vector<int> length2;



			//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓code end，下方為顯示用	1d-vector-->2d-vector轉成mat
			//                                                           output->dwtdisp->
			//length轉到length2並用成二的倍數+外推
			dwt_output_dim_sym(length, length2, J);
			/*void* dwt_output_dim_sym(vector<int> &length, vector<int> &length2, int J) {
				unsigned int sz = length.size();
				int rows = length[sz - 2];
				int cols = length[sz - 1];
				for (int i = 0; i < J; i++) {
					rows = (int)ceil((double)rows / 2.0);
					cols = (int)ceil((double)cols / 2.0);
				}
				for (int i = 0; i < J + 1; i++) {
					length2.push_back(rows);
					length2.push_back(cols);
					rows = rows * 2;
					cols = cols * 2;
				}
				return 0;
			}*/
			/*for (int i = 0; i < length.size(); i++)
				cout << length[i] << " " << length2[i] << endl;*/

				//把原圖的yx數值找出來，轉成2d
			int siz = length2.size();
			int rows_n = length2[siz - 2];
			int cols_n = length2[siz - 1];
			vector<vector< double> > dwtdisp(rows_n, vector<double>(cols_n));




			//1d vector->2d vector
			dispDWT(output, dwtdisp, length, length2, J);
			vector<vector<double> > dwt_output = dwtdisp;



	
			//2d vec ->2d mat
			Mat forsee(rows_n, cols_n, CV_32F), foruse(rows_n, cols_n, CV_32F);
			double minVal, maxVal;

			for (int i = 0; i < rows_n; i++) {
				for (int j = 0; j < cols_n; j++) {
					foruse.at<float>(i, j) = (dwt_output[i][j]);
				}
			}
			forMerge[k] = foruse;

			//minMaxLoc(foruse, &minVal, &maxVal);
			//cout << k<<"的max=" << maxVal << " " << "min=" << minVal << endl;
			//normalize(foruse, forsee, 1.0, 0.0, NORM_MINMAX);
			////foruse.convertTo(forsee, CV_8U, 255 / (maxVal - minVal), -minVal* 255 / (maxVal - minVal));
			//minMaxLoc(forsee, &minVal, &maxVal);
			//cout << k << "的max=" << maxVal << " " << "min=" << minVal << endl;
			//imshow("forsee，LL沒有設成0", forsee);
			//imshow("foruse", foruse);
			//waitKey(0);
			//cout <<"y的長"<< rows_n << endl << cols_n << endl << rows_n / 2 << endl << cols_n / 2 << endl;
		}

		Mat mergeOut;
		merge(forMerge, 3, mergeOut);
		//imshow("merge", mergeOut);
		//waitKey(0);
		return mergeOut;
	}
	else {//單通道
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				unsigned char temp;
				temp = ((uchar*)matimg.data +     i * matimg.step)[j * matimg.elemSize() ];
				vecinput[i][j] = (double)temp;
			}
		}		

		vector<int> length;
		vector<double> vecOutput, flag;
		int J = 1;//////////////////階數





		//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓code start下方為計算小波轉換
		dwt_2d_sym(vecinput, J, nm, vecOutput, flag, length);//因為輸出*****(vecOutput)********是一維矩陣(convolution) 
													  //所以要記錄y x長度(length)，用來重新排列成2d矩陣
		vector<int> length2;





		//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓code end，下方為顯示(使用)用	1d-vector-->2d-vector轉成mat
		//                                                           vecOutput->dwtdisp->
		dwt_output_dim_sym(length, length2, J);//把原高寬抓出來，downsample成偶數
		/*void* dwt_output_dim_sym(vector<int> &length, vector<int> &length2, int J) {
			unsigned int sz = length.size();
			int rows = length[sz - 2];
			int cols = length[sz - 1];
			for (int i = 0; i < J; i++) {
				rows = (int)ceil((double)rows / 2.0);
				cols = (int)ceil((double)cols / 2.0);
			}
			for (int i = 0; i < J + 1; i++) {
				length2.push_back(rows);
				length2.push_back(cols);
				rows = rows * 2;
				cols = cols * 2;
			}
			return 0;
		}*/
		for (int i = 0; i < length.size(); i++)
			cout <<i<<"="<< length[i] << " " << length2[i] << endl;





		//把原圖的yx數值找出來，轉成2d，為什麼用新的length2????
		//length有考慮到filter長度，length2只管downsample
		int siz = length2.size();
		int rows_n = length2[siz - 2];
		int cols_n = length2[siz - 1];

		vector<vector< double> > dwtdisp(rows_n, vector<double>(cols_n));
		///////////////////////////////////////////////////////////////////1d vector->2d vector，這裡轉的時候就有切邊界
		dispDWT(vecOutput, dwtdisp, length, length2, J);
		vector<vector<double> > dwt_outputForMat = dwtdisp;


		//2d vec ->2d mat
		Mat forsee(rows_n, cols_n, CV_32F), foruse(rows_n, cols_n, CV_32F);
		double minVal, maxVal;
		for (int i = 0; i < rows_n; i++) {
			for (int j = 0; j < cols_n; j++) {
				foruse.at<float>(i, j) = (dwt_outputForMat[i][j]);
			}
		}	
		return foruse;	
	}
}