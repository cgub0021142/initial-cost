// initial cost.cpp: 定義主控台應用程式的進入點。
//
#include"fftw3.h"
#include"waveFunc.h"
#include<fstream>
#include <ctime>
#include <cmath>
#include <iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

#define right 1
//#define left 1
//venus scale=8 range=18  VR.png VL.png
//sawtooth scale=8 range=18,19(保險)  SR.ppm  SL.ppm
//tsukuba scale=16 range=14  TR.png   TL.png
//map scale=8 range=30  MR.pgm   ML.pgm
//cone teddy scale=4 range=60
 int range = 60;
int scale = 4;
double t = (double)clock() / CLOCKS_PER_SEC;
Point interestPoint(-1,-1);
void onMouse(int Event, int x, int y, int flags, void* param) {
	if (Event == CV_EVENT_LBUTTONDOWN) {
		interestPoint.x = x;
		interestPoint.y = y;
	}
	
}
int main()
{

	
	//ofstream ofile("C:/Users/USER/Desktop/時間記錄.txt", ios::out);
	string CL = "CL.png", CR = "CR.png", SR = "SR.ppm", SL = "SL.ppm", MR = "MR.pgm", ML = "ML.pgm", TR = "TR.png", TL = "TL.png", teddyR = "teddyR.png", teddyL = "teddyL.png", VL = "VL.png", VR = "VR.png";
	string teddyRs = "teddyR縮小.png", teddyLs = "teddyL縮小.png";
	Mat L = imread(CL, 1);//convert image to the single channel grayscale image.
	Mat R = imread(CR, 1);


	//Mat im = imread("img.png", 0);
	//cout << "img ch="<<im.channels() << endl;
	//imshow("L", L);
	//imshow("R", R);
	//waitKey(10);
	//for (int i = 0; i < 10; i++) {
	//	Mat Lroi = L(Rect(250, i*10, 150, 100));
	//	imshow(to_string(i), Lroi);
	//}
	//waitKey(0);
	float H = L.rows;
	float W = L.cols;

	//14 36 33  自己玩設40   論文:窗大小=33 rc=7,rp=36,  顏色,距離 ，縮小圖的窗用17
	float rc = 14;   //顏色權重
	int windowSize = 17;
	cout << "windowsize= " << windowSize << endl;
	//int rg = windowSize-1;  //距離權重
	float rg = 36;
	//設一個空的三維矩陣，高寬dRange，不考慮downsize
	/////////////////////////////////////////////////
	//int size[3] = { H,W,range + 1 };
	//cout << L.size();
	//pyrDown(L, L);
	//cout << L.size();
	////////////////////////////////////////////////////////
	//////////////////initial cost=wavelet 要除以2
	cout << "紀錄" << endl;
	cout << "原本圖片高寬，如果要用到小波轉換碰到長度為奇數，無條件補1" << L.size() << endl << "down sample後再放大=" ;
	cout << ceil(H / 2) * 2 <<"，就是奇數補1補成偶數"<< endl;

	//for wavelet downsample
	H = ceil(H / 2); W = ceil(W / 2); range /= 2; scale *= 2;
	cout <<"H downsample= "<< H << endl <<"W downsample= "<< W << endl;
	int size[3] = { H ,W ,range  + 1 }; 
	//上面的意思就是如果是奇數就補一行，for wavelet downsample H&W
	Mat initial(3, size, CV_32F, Scalar(0));
	//@三維矩陣
	int decrX, decrY, incrX, incrY;
	

#ifdef left

	// 計算initial
	for (int d = 0; d <= range; d++)
		for (int y = 0; y < H; y++)
			for (int x = d; x < W; x++) {
				initial.at<float>(y, x, d) = abs((L.at<Vec3b>(y, x)[0] - R.at<Vec3b>(y, x - d)[0]))
					+ abs((L.at<Vec3b>(y, x)[1] - R.at<Vec3b>(y, x - d)[1]))
					+ abs((L.at<Vec3b>(y, x)[2] - R.at<Vec3b>(y, x - d)[2]));

			}

	std::cout << "initial end at:" << endl;
	std::cout << (double)clock() / CLOCKS_PER_SEC << " 秒" << endl;
	//@ 計算initial



	L.convertTo(L, CV_32F, 1.0 / 255);
	R.convertTo(R, CV_32F, 1.0 / 255);


	cvtColor(R, R, CV_BGR2Lab);//L:0-100 a:-128~128    b:-128~128   用浮點數表示
	cvtColor(L, L, CV_BGR2Lab);





	std::cout << "計算權重Lab:" << (double)clock() / CLOCKS_PER_SEC << " 秒" << endl;









	//for channel sum use
	Mat m1(1, 3, CV_8U);
	m1.at<uchar>(0, 0) = 1;
	m1.at<uchar>(0, 1) = 1;
	m1.at<uchar>(0, 2) = 1;



	//////////////////////////////////////////////////////////////////////





	//輸出矩陣
	Mat out(H, W, CV_8U, Scalar(0));


	//aggregate
	Mat aggregateMat(3, size, CV_32F, Scalar(0));

	int windowR = (windowSize - 1) / 2;
	int aggregateXend = W - windowR;
	int aggregateYend = H - windowR;
	int doutVal;
	int doutCost = 255;













	//距離權重，如果左右一起算，rg要除2
	float rgd = rg / 2.0;//rg for 去除*2用
	Mat disW(windowSize, windowSize, CV_32F);
	for (int i = -windowR; i <= windowR; i++)
		for (int j = -windowR; j <= windowR; j++)
			disW.at<float>(j + windowR, i + windowR) = exp(-sqrt(pow(i, 2) + pow(j, 2)) / rgd);
	//cout<<"disw"<<disW<<endl;

	int stop = 0;

	//左上  中上  右上
	//左中  正常  右中
	//左下  中下  右下



	//for (rc = 2000; rc <= 2000; rc++) {
	//每一個點(不正常區)中上
	for (int x = windowR + range; x < aggregateXend; x++) {
		for (int y = 0; y < windowR; y++) {

			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowSize, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowR + 1 + y, windowSize, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0],R.at<Vec3f>(y, x - d)[1] ,R.at<Vec3f>(y, x - d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
					}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
	}
	cout << "中上" << (double)clock() / CLOCKS_PER_SEC << " 秒";

	//每一個點(不正常區)中下
	decrY = 1;
	for (int y = H - windowR; y < H; y++) {
		for (int x = windowR + range; x < aggregateXend; x++) {

			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowSize, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowSize - decrY, windowSize, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0],R.at<Vec3f>(y, x - d)[1] ,R.at<Vec3f>(y, x - d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
					}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
		++decrY;
	}
	cout << "中下" << (double)clock() / CLOCKS_PER_SEC << " 秒";

	//每一個點(正常區)
	for (int x = windowR + range; x < aggregateXend; x++) {
		for (int y = windowR; y < aggregateYend; y++) {

			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//
			//先偷乘距離權重，左右都有算
			//weightWindowRef = weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowSize, windowSize, CV_32F);
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0],R.at<Vec3f>(y, x - d)[1] ,R.at<Vec3f>(y, x - d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = 0; wx < windowSize; wx++)
					for (int wy = 0; wy < windowSize; wy++) {//中心點  +   window移動量    +window 左上真實移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx - windowR, d)* weightWindow.at<float>(wy, wx);
					}


				//for (int wx = -windowR; wx <= windowR; wx++)
				//	for (int wy = -windowR; wy <= windowR; wy++) {//中心點， 其他位移點
				//		aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy , x + wx , d)* weightWindow.at<float>(wy+windowR, wx+windowR);
				//	}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
	}
	std::cout << "正常" << (double)clock() / CLOCKS_PER_SEC << " 秒";





	decrX = 1;

	//每一個點(不正常區)右上

	for (int x = W - windowR; x < W; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0],L.at<Vec3f>(y, x)[1],L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowR + y + 1, windowSize - decrX, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR - decrX; wx++)
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;




			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowR + y + 1, windowSize - decrX, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;//下面這3行是不要用右圖權重時使用
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------


				//右window權重
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0],R.at<Vec3f>(y, x - d)[1],R.at<Vec3f>(y, x - d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR - decrX; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((R.at<Vec3f>(y + wy, x + wx - d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------







									//initial cost*權重
				for (int wx = -windowR; wx <= windowR - decrX; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}

		++decrX;
	}

	std::cout << "右上" << (double)clock() / CLOCKS_PER_SEC << " 秒";

	decrX = 1;
	//每一個點(不正常區)右中
	for (int x = W - windowR; x < W; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0],L.at<Vec3f>(y, x)[1],L.at<Vec3f>(y, x)[2] };
			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize - decrX, CV_32F, Scalar(0));
			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR - decrX; wx++)
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;




			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowSize, windowSize - decrX, CV_32F);
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;//下面這3行是不要用右圖權重時使用
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------


				//右window權重
				Vec3f twm;//target window mid point
				twm[0] = R.at<Vec3f>(y, x - d)[0];
				twm[1] = R.at<Vec3f>(y, x - d)[1];
				twm[2] = R.at<Vec3f>(y, x - d)[2];
				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR - decrX; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((R.at<Vec3f>(y + wy, x + wx - d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------







									//initial cost*權重
				for (int wx = 0; wx < windowSize - decrX; wx++)
					for (int wy = 0; wy < windowSize; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx - windowR, d)* weightWindow.at<float>(wy, wx);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}

		++decrX;
	}

	std::cout << "右中" << (double)clock() / CLOCKS_PER_SEC << " 秒";




	//每一個點(不正常區)右下
	decrY = 1;

	for (int y = H - windowR; y < H; y++) {
		decrX = 1;
		for (int x = W - windowR; x < W; x++) {

			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowSize - decrX, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR - decrX; wx++)
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到

				Mat weightWindow(windowSize - decrY, windowSize - decrX, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0],R.at<Vec3f>(y, x - d)[1] ,R.at<Vec3f>(y, x - d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR - decrX; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -windowR; wx <= windowR - decrX; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
					}

				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;

			++decrX;
		}
		++decrY;
	}
	cout << "右下" << (double)clock() / CLOCKS_PER_SEC << " 秒";



	//每一個點(不正常區)左中左   ok??
	for (int x = 1; x < windowR; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowR + 1 + x, CV_32F, Scalar(0));///////////////////////////





																				//開始計算 左window權重             和中心點的相對座標
			for (int wx = -x; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + x) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//size不一樣不能用mul
			//先偷乘距離權重，左右都有算
			//weightWindowRef=weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			Mat weightWindow;
			for (int d = 0; d <= x; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				Rect rect(d, 0, windowR + 1 + x - d, windowSize);
				weightWindowRef(rect).copyTo(weightWindow);


				//Mat weightWindow = weightWindowRef(Rect(d, 0, windowR + 1 + x - d, windowSize));/////////////////////////  
				//cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0], R.at<Vec3f>(y, x - d)[1], R.at<Vec3f>(y, x - d)[2] };


				float wws = 0;//window weight sum
							  //相對中心點座標
				for (int wx = -x + d; wx <= windowR; wx++)////////////////////////////////////////////////////////
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + windowR, wx + x - d) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x + wx - d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + x - d) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + x - d);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------





									// windowR +1+ x -d

									//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
				for (int wx = -x + d; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + x - d);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}

	std::cout << "左中左" << (double)clock() / CLOCKS_PER_SEC << " 秒";




	//每一個點(不正常區)左中右    //一開始(d=0)，window不會框到圖片外   ok
	for (int x = windowR; x < windowR + range; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點，refWinMid
			Vec3f rwm = { L.at<Vec3f>(y, x)[0],L.at<Vec3f>(y, x)[1],L.at<Vec3f>(y, x)[2] };


			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize, CV_32F, Scalar(0));///////////////////////////




																		   //開始計算 左window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ windowR, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				int pixalcut = (x - windowR - d)*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > 16)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowSize);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//右window權重
					//target window mid point
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------
					if (x == 29 && y == 75)
						cout << "d=" << d << ",權重加總=" << wws << endl;

					//initial cost*權重
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//右window權重
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = 0; wx < windowSize; wx++)
						for (int wy = 0; wy < windowSize; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx - windowR, d)* weightWindow.at<float>(wy, wx);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}
				if (x == 29 && y == 75)
					cout << "d=" << d << ", cost=" << aggregateMat.at<float>(75, 29, d) << endl;

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}
	std::cout << "左中右" << (double)clock() / CLOCKS_PER_SEC << " 秒";





	//每一個點(不正常區)左上右    //一開始(d=0)，window不會框到圖片外   ok
	for (int x = windowR; x < windowR + range; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點，refWinMid
			Vec3f rwm = { L.at<Vec3f>(y, x)[0],L.at<Vec3f>(y, x)[1],L.at<Vec3f>(y, x)[2] };


			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowSize, CV_32F, Scalar(0));///////////////////////////




																				//開始計算 左window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ y, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				int pixalcut = (x - windowR - d)*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > 16)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowR + 1 + y);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//右window權重
					//target window mid point
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + y, wx + windowR - pixalcut) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + y, wx + windowR - pixalcut) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + y, wx + windowR - pixalcut);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------
					if (x == 29 && y == 75)
						cout << "d=" << d << ",權重加總=" << wws << endl;

					//initial cost*權重
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR - pixalcut);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//右window權重
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + y, wx + windowR);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}


			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}
	std::cout << "左上右" << (double)clock() / CLOCKS_PER_SEC << " 秒";







	decrY = 1;
	//每一個點(不正常區)左下右    //一開始(d=0)，window不會框到圖片外   ok
	for (int y = H - windowR; y < H; y++) {

		for (int x = windowR; x < windowR + range; x++) {


			//中心點，refWinMid
			Vec3f rwm = { L.at<Vec3f>(y, x)[0],L.at<Vec3f>(y, x)[1],L.at<Vec3f>(y, x)[2] };


			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowSize, CV_32F, Scalar(0));///////////////////////////




																				   //開始計算 左window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ windowR, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				int pixalcut = (x - windowR - d)*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > 16)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowSize - decrY);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//右window權重
					//target window mid point
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------
					if (x == 29 && y == 75)
						cout << "d=" << d << ",權重加總=" << wws << endl;

					//initial cost*權重
					for (int wx = -windowR + pixalcut; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR - pixalcut);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//右window權重
					Vec3f twm = { R.at<Vec3f>(y, x - d)[0] , R.at<Vec3f>(y, x - d)[1] , R.at<Vec3f>(y, x - d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((R.at<Vec3f>(y + wy, x - d + wx)[0] - twm[0]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[1] - twm[1]), 2)
								+ pow((R.at<Vec3f>(y + wy, x - d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}


			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
		++decrY;

	}
	std::cout << "左下右" << (double)clock() / CLOCKS_PER_SEC << " 秒";























	//每一個點(不正常區)左上左   
	for (int x = 1; x < windowR; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowR + 1 + x, CV_32F, Scalar(0));///////////////////////////





																					 //開始計算 左window權重             和中心點的相對座標
			for (int wx = -x; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + x) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//size不一樣不能用mul
			//先偷乘距離權重，左右都有算
			//weightWindowRef=weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			Mat weightWindow;
			for (int d = 0; d <= x; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				Rect rect(d, 0, windowR + 1 + x - d, windowR + 1 + y);
				weightWindowRef(rect).copyTo(weightWindow);


				//Mat weightWindow = weightWindowRef(Rect(d, 0, windowR + 1 + x - d, windowSize));/////////////////////////  
				//cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0], R.at<Vec3f>(y, x - d)[1], R.at<Vec3f>(y, x - d)[2] };


				float wws = 0;//window weight sum
							  //相對中心點座標
				for (int wx = -x + d; wx <= windowR; wx++)////////////////////////////////////////////////////////
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + y, wx + x - d) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x + wx - d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + x - d) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + x - d);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------





									// windowR +1+ x -d

									//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
				for (int wx = -x + d; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + x - d);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}

	std::cout << "左上左" << (double)clock() / CLOCKS_PER_SEC << " 秒";


	decrY = 1;
	//每一個點(不正常區)左下左 
	for (int y = H - windowR; y < H; y++) {

		for (int x = 1; x < windowR; x++) {


			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//左window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowR + 1 + x, CV_32F, Scalar(0));///////////////////////////





																						//開始計算 左window權重             和中心點的相對座標
			for (int wx = -x; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + x) = exp(-sqrt(
						pow((L.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((L.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//size不一樣不能用mul
			//先偷乘距離權重，左右都有算
			//weightWindowRef=weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			Mat weightWindow;
			for (int d = 0; d <= x; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				Rect rect(d, 0, windowR + 1 + x - d, windowSize - decrY);
				weightWindowRef(rect).copyTo(weightWindow);


				//Mat weightWindow = weightWindowRef(Rect(d, 0, windowR + 1 + x - d, windowSize));/////////////////////////  
				//cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { R.at<Vec3f>(y, x - d)[0], R.at<Vec3f>(y, x - d)[1], R.at<Vec3f>(y, x - d)[2] };


				float wws = 0;//window weight sum
							  //相對中心點座標
				for (int wx = -x + d; wx <= windowR; wx++)////////////////////////////////////////////////////////
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						//顏色
						weightWindow.at<float>(wy + windowR, wx + x - d) *= exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x + wx - d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx - d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + x - d) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + x - d);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------





									// windowR +1+ x -d

									//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
				for (int wx = -x + d; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + x - d);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}

		++decrY;
	}

	std::cout << "左下左" << (double)clock() / CLOCKS_PER_SEC << " 秒";

	//@aggregate









	imshow("out", out);
	waitKey(0);


	L = imread(SL, 1);//convert image to the single channel grayscale image.
	R = imread(SR, 1);
	cout << endl << windowR << "  < x < " << windowR + range << endl;
	cout << "y <" << H << endl;
	int testX = 1, testY = 1;
	while (testX != 0 && testY != 0) {
		cout << endl << "x=";
		cin >> testX;
		cout << "y=";
		cin >> testY;
		//cout << endl << "("+to_string(testX)")" << endl;
		for (int d = 0; d <= range; d++)
			cout << "d=" << d << ", cost=" << aggregateMat.at<float>(testY, testX, d) << endl;
		int d = out.at<uchar>(testY, testX) / scale;
		int pixalcut = (testX - d - windowR)*-1;
		//Mat partL,partR;
		//if (pixalcut > 0) {
		//	Mat partR = R(Rect(testX - windowR + pixalcut-d, testY - windowR, windowSize - pixalcut-d, windowSize));
		//	Mat partL = L(Rect(testX - windowR + pixalcut, testY - windowR, windowSize - pixalcut - d, windowSize));
		//	//imshow("partL", partL);
		//	//imshow("partR", partR);
		//	//waitKey(0);
		//}
		//if (pixalcut < 0) {
		//	Mat partR = R(Rect(testX - windowR, testY - windowR, windowSize, windowSize));
		//	Mat partL = L(Rect(testX - windowR, testY - windowR, windowSize, windowSize));
		//	//imshow("partL", partL);
		//	//imshow("partR", partR);
		//	//waitKey(0);
		//}
	}







	for (int y = 0; y < H; y++)
		out.at<uchar>(y, windowR) = 255;
	imshow("out", out);
	imwrite("D.png", out);

	//}

	std::cout << (double)clock() / CLOCKS_PER_SEC << " 秒";
	//waitKey(0);
#endif
#ifdef right

#pragma region sobel
	//Mat biL, biR;
	//Mat Lgx, Lgy, Rgx, Rgy, biLgx, biLgy, biRgx, biRgy;
	//Sobel(L, Lgx, CV_16S, 1, 0);
	//Sobel(L, Lgy, CV_16S, 0, 1);
	//Sobel(R, Rgx, CV_16S, 1, 0);
	//Sobel(R, Rgy, CV_16S, 0, 1);
	//bilateralFilter(L, biL, 11, 50, 50);
	//bilateralFilter(R, biR, 11, 50, 50);
	//Sobel(biL, biLgx, CV_16S, 1, 0);
	//Sobel(biL, biLgy, CV_16S, 0, 1);
	//Sobel(biR, biRgx, CV_16S, 1, 0);
	//Sobel(biR, biRgy, CV_16S, 0, 1);
	///*imshow("Rgx", Rgx);
	//imshow("Lgx", Lgx);
	//imshow("biRgx", biRgx);
	//imshow("biLgx", biLgx);
	//waitKey(0);*/
	//Mat Igx(3, size, CV_32F, Scalar(0));
	////用x梯度+smooth(x)的梯度
	//for (int d = 0; d <= range; d++)
	//	for (int y = 0; y < H; y++)
	//		for (int x = 0; x < W - d; x++)
	//		{
	//			Igx.at<float>(y, x, d) =
	//				  2*abs((Rgx.at<Vec3s>(y, x)[0] - Lgx.at<Vec3s>(y, x + d)[0]))
	//				+ 2*abs((Rgx.at<Vec3s>(y, x)[1] - Lgx.at<Vec3s>(y, x + d)[1]))
	//				+ 2*abs((Rgx.at<Vec3s>(y, x)[2] - Lgx.at<Vec3s>(y, x + d)[2]))
	//				+  abs((biRgx.at<Vec3s>(y, x)[0] - biLgx.at<Vec3s>(y, x + d)[0]))
	//				+  abs((biRgx.at<Vec3s>(y, x)[1] - biLgx.at<Vec3s>(y, x + d)[1]))
	//				+  abs((biRgx.at<Vec3s>(y, x)[2] - biLgx.at<Vec3s>(y, x + d)[2]));
	//				
	//		}
	//Mat Igy(3, size, CV_32F, Scalar(0));
	//for (int d = 0; d <= range; d++)
	//	for (int y = 0; y < H; y++)
	//		for (int x = 0; x < W - d; x++)
	//		{
	//			Igy.at<float>(y, x, d) =
	//				2 * abs((Rgy.at<Vec3s>(y, x)[0] - Lgy.at<Vec3s>(y, x + d)[0]))
	//				+ 2 * abs((Rgy.at<Vec3s>(y, x)[1] - Lgy.at<Vec3s>(y, x + d)[1]))
	//				+ 2 * abs((Rgy.at<Vec3s>(y, x)[2] - Lgy.at<Vec3s>(y, x + d)[2]));
	//				/*+ abs((biRgy.at<Vec3s>(y, x)[0] - biLgy.at<Vec3s>(y, x + d)[0]))
	//				+ abs((biRgy.at<Vec3s>(y, x)[1] - biLgy.at<Vec3s>(y, x + d)[1]))
	//				+ abs((biRgy.at<Vec3s>(y, x)[2] - biLgy.at<Vec3s>(y, x + d)[2]));*/

	//		}
	//Igx /= 18.0;
	//Igy /= 2.0;
	//initial=Igy;
#pragma endregion
#pragma region initial use wavelet
	//normalize 的mask不能用""multi channel""的
	//OpenCV Error: Assertion failed，assert裡面false就中斷並印出
	//((cn == 1 && (_mask.empty() || _mask.type() == 0)) || 
	//(cn > 1 && _mask.empty() && !minIdx && !maxIdx)) ，因為這裡會中斷不能用mask
	//in cv::minMaxIdx,

	//Mat waveletTrans(Mat img)，return 32f
	Mat waveL = waveletTrans(L);
	cout << "Lsize: " << L.size() << endl;
	cout << "waveLsize: " << waveL.size() << endl;
 	Mat waveR = waveletTrans(R);
	imshow("waveL圖片直接輸出", waveL);
	waitKey(0);
	
	//圖片用成LL normalize，取出
	Mat LLzero,mask1(L.rows,L.cols,CV_8U,Scalar(2));
	//waveL.copyTo(LLzero);
	//waveL.copyTo(mask);
	//mask.setTo(255);
	//Rect r1(0, 0, W, H);
	//L.convertTo(L, CV_32F, 1.0 / 255);
	Mat L1;
	//mask1(r1).setTo(0);
	L.copyTo(L1, mask1);
	imshow("L1", L1);
	waitKey(0);


	//waveL(r1).setTo(0);
	imshow("waveL", waveL);
	waitKey(0);
	//cout << mask.channels() << endl;
	//cout << mask1.type() << endl;

	//Mat mask(H, W, CV_32F, Scalar(1));
	normalize(waveL, LLzero, 0.0, 1.0, NORM_MINMAX,-1);
	imshow("waveL標準0-1", LLzero);
	waitKey(0);
	//imwrite("waveL.png", waveL);
	//cout << waveL << endl;
	//Mat waveimg = waveletTrans(im);
	Mat al, bl, cl, dl;
	Mat ar, br, cr, dr;
	cout << W << "here" << H << endl;
	waveL(Rect(0, 0, W, H)).copyTo(al);
	waveL(Rect(W, 0, W, H)).copyTo(bl);
	waveL(Rect(0, H, W, H)).copyTo(cl);
	waveL(Rect(W, H, W, H)).copyTo(dl);
	waveR(Rect(0, 0, W, H)).copyTo(ar);
	waveR(Rect(W, 0, W, H)).copyTo(br);
	waveR(Rect(0, H, W, H)).copyTo(cr);
	waveR(Rect(W, H, W, H)).copyTo(dr);
	

	cout << "alLsize" << al.size() << endl;

 	for (int d = 0; d <= range; d++)
		for (int y = 0; y < H; y++)
			for (int x = 0; x < W - d; x++) {
				////LL
				//initial.at<float>(y, x, d) = abs((ar.at<Vec3f>(y, x)[0] - al.at<Vec3f>(y, x + d)[0]))
				//	+ abs((ar.at<Vec3f>(y, x)[1] - al.at<Vec3f>(y, x + d)[1]))
				//	+ abs((ar.at<Vec3f>(y, x)[2] - al.at<Vec3f>(y, x + d)[2]));
				////LH
				//initial.at<float>(y, x, d) = (abs((br.at<Vec3f>(y, x)[0] - bl.at<Vec3f>(y, x + d)[0]))
				//	+ abs((br.at<Vec3f>(y, x)[1] - bl.at<Vec3f>(y, x + d)[1]))
				//	+ abs((br.at<Vec3f>(y, x)[2] - bl.at<Vec3f>(y, x + d)[2])));
				////HL
				//initial.at<float>(y, x, d) = (abs((cr.at<Vec3f>(y, x)[0] - cl.at<Vec3f>(y, x + d)[0]))
				//	+ abs((cr.at<Vec3f>(y, x)[1] - cl.at<Vec3f>(y, x + d)[1]))
				//	+ abs((cr.at<Vec3f>(y, x)[2] - cl.at<Vec3f>(y, x + d)[2])));
				//HH
				initial.at<float>(y, x, d) = (abs((dr.at<Vec3f>(y, x)[0] - dl.at<Vec3f>(y, x + d)[0]))
					+ abs((dr.at<Vec3f>(y, x)[1] - dl.at<Vec3f>(y, x + d)[1]))
					+ abs((dr.at<Vec3f>(y, x)[2] - dl.at<Vec3f>(y, x + d)[2])));
				////all
				//initial.at<float>(y, x, d) = abs((ar.at<Vec3f>(y, x)[0] - al.at<Vec3f>(y, x + d)[0]))
				//	+ abs((ar.at<Vec3f>(y, x)[1] - al.at<Vec3f>(y, x + d)[1]))
				//	+ abs((ar.at<Vec3f>(y, x)[2] - al.at<Vec3f>(y, x + d)[2]))
				//	+ (abs((br.at<Vec3f>(y, x)[0] - bl.at<Vec3f>(y, x + d)[0]))
				//		+ abs((br.at<Vec3f>(y, x)[1] - bl.at<Vec3f>(y, x + d)[1]))
				//		+ abs((br.at<Vec3f>(y, x)[2] - bl.at<Vec3f>(y, x + d)[2])))
				//+ (abs((cr.at<Vec3f>(y, x)[0] - cl.at<Vec3f>(y, x + d)[0]))
				//	+ abs((cr.at<Vec3f>(y, x)[1] - cl.at<Vec3f>(y, x + d)[1]))
				//	+ abs((cr.at<Vec3f>(y, x)[2] - cl.at<Vec3f>(y, x + d)[2])))
				//+ (abs((dr.at<Vec3f>(y, x)[0] - dl.at<Vec3f>(y, x + d)[0]))
				//	+ abs((dr.at<Vec3f>(y, x)[1] - dl.at<Vec3f>(y, x + d)[1]))
				//	+ abs((dr.at<Vec3f>(y, x)[2] - dl.at<Vec3f>(y, x + d)[2])));

				
			}
	////1
	//Mat a1Norm, arNorm;
	//normalize(al, a1Norm, 1.0, 0.0, NORM_MINMAX);
	//normalize(ar, arNorm, 1.0, 0.0, NORM_MINMAX);
	//a1Norm.copyTo(L);
	//arNorm.copyTo(R);
	////2
	//Mat b1Norm, brNorm;
	//normalize(bl, b1Norm, 1.0, 0.0, NORM_MINMAX);
	//normalize(br, brNorm, 1.0, 0.0, NORM_MINMAX);
	//b1Norm.copyTo(L);
	//brNorm.copyTo(R);
	////3
	//Mat c1Norm, crNorm;
	//normalize(cl, c1Norm, 1.0, 0.0, NORM_MINMAX);
	//normalize(cr, crNorm, 1.0, 0.0, NORM_MINMAX);
	//c1Norm.copyTo(L);
	//crNorm.copyTo(R);
	//4
	Mat d1Norm, drNorm;
	normalize(dl, d1Norm, 1.0, 0.0, NORM_MINMAX);
	normalize(dr, drNorm, 1.0, 0.0, NORM_MINMAX);
	d1Norm.copyTo(L);
	drNorm.copyTo(R);

	//L = waveL(Rect(0, 0, W, H ));//這邊轉換會有問題，cvtColor(R, R, CV_BGR2Lab);//L:0-100 a:-128~128    b:-128~128   用浮點數表示
	//R = waveR(Rect(0, 0, W, H ));
	////所以下面寫出轉換成三通道的code
	//vector<Mat> v1,v2;
	//for (int i = 0; i<3; i++)
	//{
	//	v1.push_back(L);
	//	v2.push_back(R);
	//}
	//merge(v1, L);
	//merge(v2, R);
	//
	//
#pragma endregion
#pragma region pydown compare with wavelet
	
	//pyrDown(R, R);
	//pyrDown(L, L);
	//for (int d = 0; d <= range; d++)
	//	for (int y = 0; y < H ; y++)
	//		for (int x = 0; x < W  - d; x++) {

	//			initial.at<float>(y, x, d) = abs((R.at<Vec3b>(y, x)[0] - L.at<Vec3b>(y, x + d)[0]))
	//							+ abs((R.at<Vec3b>(y, x)[1] - L.at<Vec3b>(y, x + d)[1]))
	//							+ abs((R.at<Vec3b>(y, x)[2] - L.at<Vec3b>(y, x + d)[2]));

	//		}
	//


#pragma endregion
#pragma region color cost
	//// 計算initial
	//for (int d = 0; d <= range; d++)
	//	for (int y = 0; y < H; y++)
	//		for (int x = 0; x < W - d; x++) {
	//			
	//			initial.at<float>(y, x, d) = abs((R.at<Vec3b>(y, x)[0] - L.at<Vec3b>(y, x + d)[0]))
	//				+ abs((R.at<Vec3b>(y, x)[1] - L.at<Vec3b>(y, x + d)[1]))
	//				+ abs((R.at<Vec3b>(y, x)[2] - L.at<Vec3b>(y, x + d)[2]));
	//	
	//		}
	//std::cout << "initial end at:" << endl;
	//std::cout << (double)clock() / CLOCKS_PER_SEC << " 秒" << endl;
	////@ 計算initial
#pragma endregion
	//L.convertTo(L, CV_32F, 1.0 / 255);
	//R.convertTo(R, CV_32F, 1.0 / 255);
	//normalize(L, L, 1.0, 0.0, NORM_MINMAX);
	//normalize(R, R, 1.0, 0.0, NORM_MINMAX);
	cv::cvtColor(R, R, CV_BGR2Lab);					//L:0-100 a:-128~128    b:-128~128   用浮點數表示
	cv::cvtColor(L, L, CV_BGR2Lab);

	//輸出矩陣
	Mat out(H, W, CV_8U, Scalar(0));


	//aggregate
	Mat aggregateMat(3, size, CV_32F, Scalar(0));

	int windowR = (windowSize - 1) / 2;
	int aggregateXend = W - windowR;
	int aggregateYend = H - windowR;
	int doutVal;
	int doutCost = 255;



	std::cout << "計算權重Lab:" << (double)clock() / CLOCKS_PER_SEC << " 秒" << endl;









	//距離權重，如果左右一起算，rg要除2
	float rgd = rg / 2.0;//rg for 去除*2用
	Mat disW(windowSize, windowSize, CV_32F);
	for (int i = -windowR; i <= windowR; i++)
		for (int j = -windowR; j <= windowR; j++)
			disW.at<float>(j + windowR, i + windowR) = exp(-sqrt(pow(i, 2) + pow(j, 2)) / rgd);
	//cout<<"disw"<<disW<<endl;

	int stop = 0;

	//左上  中上  右上
	//左中  正常  右中
	//左下  中下  右下



	//for (rc = 2000; rc <= 2000; rc++) {

	//每一個點(不正常區)中上
	for (int x = windowR; x < aggregateXend; x++) {
		for (int y = 0; y < windowR; y++) {

			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowSize, CV_32F, Scalar(0));

			//開始計算 右window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowR + 1 + y, windowSize, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//左window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0],L.at<Vec3f>(y, x + d)[1] ,L.at<Vec3f>(y, x + d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
							pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + windowR);

					}
				//@左window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
					}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
	}
	cout << "中上" << (double)clock() / CLOCKS_PER_SEC << " 秒";

	//每一個點(不正常區)中下
	decrY = 1;
	for (int y = H - windowR; y < H; y++) {
		for (int x = windowR; x < aggregateXend; x++) {

			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowSize, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowSize - decrY, windowSize, CV_32F, Scalar(0));
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//左window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0],L.at<Vec3f>(y, x + d)[1] ,L.at<Vec3f>(y, x + d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
							pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@左window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
					}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
		++decrY;
	}
	cout << "中下" << (double)clock() / CLOCKS_PER_SEC << " 秒";
	cout << range << endl;
	cout << "2X:" << windowR << "~" << aggregateXend - range << endl;
	cout << "3X:" << W - windowR - range << "~" << W - windowR << endl;
	cout << "4X:" << W - windowR << "~" << W << endl;

	//每一個點(正常區)
	for (int x = windowR; x < aggregateXend - range; x++) {
		for (int y = windowR; y < aggregateYend; y++) {

			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize, CV_32F, Scalar(0));

			//開始計算 右window權重
			for (int wx = -windowR; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//
			//先偷乘距離權重，左右都有算
			//weightWindowRef = weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow(windowSize, windowSize, CV_32F);
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//右window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0],L.at<Vec3f>(y, x + d)[1] ,L.at<Vec3f>(y, x + d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -windowR; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = 0; wx < windowSize; wx++)
					for (int wy = 0; wy < windowSize; wy++) {//中心點  +   window移動量    +window 左上真實移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx - windowR, d)* weightWindow.at<float>(wy, wx);
					}


				//for (int wx = -windowR; wx <= windowR; wx++)
				//	for (int wy = -windowR; wy <= windowR; wy++) {//中心點， 其他位移點
				//		aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy , x + wx , d)* weightWindow.at<float>(wy+windowR, wx+windowR);
				//	}
				//@initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
	}
	std::cout << "正常" << (double)clock() / CLOCKS_PER_SEC << " 秒";







	//每一個點(不正常區)左上

	for (int x = 0; x < windowR; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0],R.at<Vec3f>(y, x)[1],R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowR + y + 1, windowR + 1 + x, CV_32F, Scalar(0));

			//開始計算 左window權重
			for (int wx = -x; wx <= windowR; wx++)
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + x) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ left window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;




			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow;
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;//下面這3行是不要用右圖權重時使用
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------


				//左window權重
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0],L.at<Vec3f>(y, x + d)[1],L.at<Vec3f>(y, x + d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -x; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + y, wx + x) *= exp(-sqrt(pow((R.at<Vec3f>(y + wy, x + wx + d)[0] - twm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx + d)[1] - twm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx + d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + x) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + x);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------







									//initial cost*權重
				for (int wx = -x; wx <= windowR; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + x);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
	}

	std::cout << "左上" << (double)clock() / CLOCKS_PER_SEC << " 秒";




	//每一個點(不正常區)左中
	for (int x = 0; x < windowR; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0],R.at<Vec3f>(y, x)[1],R.at<Vec3f>(y, x)[2] };
			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowR + 1 + x, CV_32F, Scalar(0));
			//開始計算 右window權重
			for (int wx = -x; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + x) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;




			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到


				Mat weightWindow;
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;//下面這3行是不要用右圖權重時使用
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------


				//左window權重
				Vec3f twm;//target window mid point
				twm[0] = L.at<Vec3f>(y, x + d)[0];
				twm[1] = L.at<Vec3f>(y, x + d)[1];
				twm[2] = L.at<Vec3f>(y, x + d)[2];
				float wws = 0;//window weight sum
				for (int wx = -x; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow((L.at<Vec3f>(y + wy, x + wx + d)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + x);

					}
				//@左window權重
				weightWindow /= wws;//正規化-----------------------------







									//initial cost*權重
				for (int wx = -x; wx < windowR; wx++)
					for (int wy = 0; wy < windowSize; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx, d)* weightWindow.at<float>(wy, wx + x);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}

	std::cout << "左中" << (double)clock() / CLOCKS_PER_SEC << " 秒";




	//每一個點(不正常區)左下
	decrY = 1;

	for (int y = H - windowR; y < H; y++) {

		for (int x = 0; x < windowR; x++) {

			//中心點
			Vec3f rwm = { L.at<Vec3f>(y, x)[0], L.at<Vec3f>(y, x)[1], L.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize - decrY, windowR + 1 + x, CV_32F, Scalar(0));

			//開始計算 右window權重
			for (int wx = -x; wx <= windowR; wx++)
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + x) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到

				Mat weightWindow;
				weightWindowRef.copyTo(weightWindow);
				//float tempsum = 0;
				//tempsum = sum(weightWindow)[0];
				//weightWindow /= tempsum;//正規化-----------------------------
				////cout << tempsum << endl;

				//左window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0],L.at<Vec3f>(y, x + d)[1] ,L.at<Vec3f>(y, x + d)[2] };

				float wws = 0;//window weight sum
				for (int wx = -x; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {
						//顏色/////////
						weightWindow.at<float>(wy + windowR, wx + x) *= exp(-sqrt(
							pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx + x) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + x);

					}
				//@右window權重
				weightWindow /= wws;//正規化-----------------------------

									////左右權重相乘
									////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
									////weightWindow=weightWindow.mul(disW);





									//initial cost*權重
				for (int wx = -x; wx <= windowR; wx++)
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {//中心點  +   window移動量                -window 左上移動座標
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + x);
					}

				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
		++decrY;
	}
	cout << "左下" << (double)clock() / CLOCKS_PER_SEC << " 秒";


	decrX = 1;
	//每一個點(不正常區)右中右   ok??
	for (int x = W - windowR; x < W; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize - decrX, CV_32F, Scalar(0));///////////////////////////





																				   //開始計算 左window權重             和中心點的相對座標
			for (int wx = -windowR; wx <= windowR - decrX; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//size不一樣不能用mul
			//先偷乘距離權重，左右都有算
			//weightWindowRef=weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			Mat weightWindow;
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				if (x + d > W - 1)
					break;
				Rect rect(d, 0, windowSize - decrX - d, windowSize);
				weightWindowRef(rect).copyTo(weightWindow);


				//Mat weightWindow = weightWindowRef(Rect(d, 0, windowR + 1 + x - d, windowSize));/////////////////////////  
				//cout << tempsum << endl;

				//左window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0], L.at<Vec3f>(y, x + d)[1], L.at<Vec3f>(y, x + d)[2] };


				float wws = 0;//window weight sum
							  //相對中心點座標
				for (int wx = -windowR; wx <= windowR - decrX - d; wx++)////////////////////////////////////////////////////////
					for (int wy = -windowR; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
							pow((L.at<Vec3f>(y + wy, x + wx + d)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + windowR, wx  + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

						wws += weightWindow.at<float>(wy + windowR, wx + windowR);

					}
				//@左window權重
				weightWindow /= wws;//正規化-----------------------------





									// windowR +1+ x -d

									//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
				for (int wx = -windowR; wx <= windowR - decrX - d; wx++)
					for (int wy = -windowR; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}

		++decrX;
	}

	std::cout << "右中右" << (double)clock() / CLOCKS_PER_SEC << " 秒";











	//每一個點(不正常區)右中左    //一開始(d=0)，window不會框到圖片外   ok
	for (int x = W - windowR - range; x < W - windowR; x++) {

		for (int y = windowR; y < aggregateYend; y++) {


			//中心點，refWinMid
			Vec3f rwm = { R.at<Vec3f>(y, x)[0],R.at<Vec3f>(y, x)[1],R.at<Vec3f>(y, x)[2] };


			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize, CV_32F, Scalar(0));///////////////////////////




																		   //開始計算 右window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ windowR, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
											  /*if (x + d > W - 1)
											  break;*/
				int pixalcut = (W - 1 - (x + windowR + d))*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > windowR)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowSize);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//左window權重
					//target window mid point
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR ) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@左window權重
					weightWindow /= wws;//正規化-----------------------------

										//initial cost*權重
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//左window權重
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@左window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = 0; wx < windowSize; wx++)
						for (int wy = 0; wy < windowSize; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy - windowR, x + wx - windowR, d)* weightWindow.at<float>(wy, wx);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}


			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}
	std::cout << "右中左" << (double)clock() / CLOCKS_PER_SEC << " 秒";





	//每一個點(不正常區)右上左    //一開始(d=0)，window不會框到圖片外   ok
	for (int x = W - windowR - range; x < W - windowR; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點，refWinMid
			Vec3f rwm = { R.at<Vec3f>(y, x)[0],R.at<Vec3f>(y, x)[1],R.at<Vec3f>(y, x)[2] };


			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowSize, CV_32F, Scalar(0));///////////////////////////




																				//開始計算 左window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ y, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				int pixalcut = (W - 1 - (x + windowR + d))*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > windowR)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowR + 1 + y);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//左window權重
					//target window mid point
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + y, wx + windowR ) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + y, wx + windowR);

						}
					//@左window權重
					weightWindow /= wws;//正規化-----------------------------

										//initial cost*權重
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//左window權重
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + y, wx + windowR);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -y; wy <= windowR; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}


			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}


	}
	std::cout << "右上左" << (double)clock() / CLOCKS_PER_SEC << " 秒";







	decrY = 1;
	//每一個點(不正常區)右下左    //一開始(d=0)，window不會框到圖片外   ok
	for (int y = H - windowR; y < H; y++) {

		for (int x = W - windowR - range; x < W - windowR; x++) {

			//中心點，refWinMid
			Vec3f rwm = { R.at<Vec3f>(y, x)[0],R.at<Vec3f>(y, x)[1],R.at<Vec3f>(y, x)[2] };


			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowSize, windowSize, CV_32F, Scalar(0));///////////////////////////



																		   //開始計算 左window權重    相對中心點座標
			for (int wx = -windowR; wx <= windowR; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -windowR; wy <= windowR - decrY; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy+ windowR, wx+ windowR) *= exp(   -sqrt(   pow(wx,2) + pow(wy,2) ) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;


			//
			//算左邊右邊權重相乘
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				int pixalcut = (W - 1 - (x + windowR + d))*(-1);//幾個框到圖片外
				Mat weightWindow;


				if (pixalcut > 0) {
					if (pixalcut > windowR)
						break;
					//weightWindow = weightWindowRef(Rect(pixalcut, 0, windowSize - pixalcut, windowSize));//這裡寫錯
					Rect rect(pixalcut, 0, windowSize - pixalcut, windowSize - decrY);//正確複製
					weightWindowRef(rect).copyTo(weightWindow);


					//左window權重
					//target window mid point
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR ) *= exp(-sqrt(pow((wx), 2) + pow((wy), 2)) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@右window權重
					weightWindow /= wws;//正規化-----------------------------


										//initial cost*權重
					for (int wx = -windowR; wx <= windowR - pixalcut; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
						}
					//@initial cost*權重

				}
				else {

					weightWindowRef.copyTo(weightWindow);

					//左window權重
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0] , L.at<Vec3f>(y, x + d)[1] , L.at<Vec3f>(y, x + d)[2] };

					float wws = 0;//window weight sum
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							//顏色/////////
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + d + wx)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + d + wx)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow((wx ),2) + pow((wy ),2) ) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@左window權重
					weightWindow /= wws;//正規化-----------------------------


										////左右權重相乘
										////weightWindow = weightWindow.mul(weightWindowRef);//左右window 一個一個相乘(pixel by pixel)  輸出一個矩陣，不是內積
										////weightWindow=weightWindow.mul(disW);





										//initial cost*權重
					for (int wx = -windowR; wx <= windowR; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
						}
					//@initial cost*權重

				}

				//cout << aggregateMat.at<float>(y, x, d) << " ";
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}


			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}
		++decrY;

	}
	std::cout << "右下左" << (double)clock() / CLOCKS_PER_SEC << " 秒";







	decrX = 1;
	//每一個點(不正常區)右上右   
	for (int x = W - windowR; x < W; x++) {

		for (int y = 0; y < windowR; y++) {


			//中心點
			Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

			//右window權重 ------ this is reference image
			Mat weightWindowRef(windowR + 1 + y, windowSize - decrX, CV_32F, Scalar(0));///////////////////////////





																						//開始計算 左window權重             和中心點的相對座標
			for (int wx = -windowR; wx <= windowR - decrX; wx++)////////////////////////////////////////////////////////////////
				for (int wy = -y; wy <= windowR; wy++) {


					//顏色
					weightWindowRef.at<float>(wy + y, wx + windowR) = exp(-sqrt(
						pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
						+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
					////距離
					//weightWindowRef.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

				}

			//@ right window weight 

			///////////////////////////////////////////////////////////////////////
			doutVal = 0;
			doutCost = 3000;

			//size不一樣不能用mul
			//先偷乘距離權重，左右都有算
			//weightWindowRef=weightWindowRef.mul(disW);








			//
			//算左邊右邊權重相乘
			Mat weightWindow;
			for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
				if (x + d > W - 1)
					break;
				Rect rect(d, 0, windowSize - decrX - d, windowR + 1 + y);
				weightWindowRef(rect).copyTo(weightWindow);


				//Mat weightWindow = weightWindowRef(Rect(d, 0, windowR + 1 + x - d, windowSize));/////////////////////////  
				//cout << tempsum << endl;

				//左window權重
				//target window mid point
				Vec3f twm = { L.at<Vec3f>(y, x + d)[0], L.at<Vec3f>(y, x + d)[1], L.at<Vec3f>(y, x + d)[2] };


				float wws = 0;//window weight sum
							  //相對中心點座標
				for (int wx = -windowR; wx <= windowR - decrX - d; wx++)////////////////////////////////////////////////////////
					for (int wy = -y; wy <= windowR; wy++) {
						//顏色
						weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(
							pow((L.at<Vec3f>(y + wy, x + wx + d)[0] - twm[0]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[1] - twm[1]), 2)
							+ pow((L.at<Vec3f>(y + wy, x + wx + d)[2] - twm[2]), 2)) / rc);
						////距離
						//weightWindow.at<float>(wy + y, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

						wws += weightWindow.at<float>(wy + y, wx + windowR);

					}
				//@左window權重
				weightWindow /= wws;//正規化-----------------------------





									// windowR +1+ x -d

									//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
				for (int wx = -windowR; wx <= windowR - decrX - d; wx++)
					for (int wy = -y; wy <= windowR; wy++) {
						aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + y, wx + windowR);
					}
				//initial cost*權重
				/*if(aggregateMat.at<float>(y, x, d)>400)
				cout << aggregateMat.at<float>(y, x, d) << endl;*/
				/////////////////////////////////////////////////////////////////////
				//簡化 最小out
				if (doutCost > aggregateMat.at<float>(y, x, d)) {
					doutCost = aggregateMat.at<float>(y, x, d);
					doutVal = d;
				}

			}
			//@end 
			out.at<uchar>(y, x) = doutVal * scale;


		}

		++decrX;
	}

	std::cout << "右上右" << (double)clock() / CLOCKS_PER_SEC << " 秒";


	decrY = 1;
	//每一個點(不正常區)右下右 
	for (int y = H - windowR; y < H; y++) {
		decrX = 1;
		for (int x = W - windowR; x < W; x++) {
			{
				//中心點
				Vec3f rwm = { R.at<Vec3f>(y, x)[0], R.at<Vec3f>(y, x)[1], R.at<Vec3f>(y, x)[2] };

				//右window權重 ------ this is reference image
				Mat weightWindowRef(windowSize - decrY, windowSize - decrX, CV_32F, Scalar(0));///////////////////////////

																							   //開始計算 左window權重             和中心點的相對座標
				for (int wx = -windowR; wx <= windowR - decrX; wx++)////////////////////////////////////////////////////////////////
					for (int wy = -windowR; wy <= windowR - decrY; wy++) {


						//顏色
						weightWindowRef.at<float>(wy + windowR, wx + windowR) = exp(-sqrt(
							pow((R.at<Vec3f>(y + wy, x + wx)[0] - rwm[0]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx)[1] - rwm[1]), 2)
							+ pow((R.at<Vec3f>(y + wy, x + wx)[2] - rwm[2]), 2)) / rc);
						////距離
						//weightWindowRef.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

					}

				//@ right window weight 

				///////////////////////////////////////////////////////////////////////
				doutVal = 0;
				doutCost = 3000;


				//
				//算左邊右邊權重相乘
				Mat weightWindow;
				for (int d = 0; d <= range; d++) {//d當最內層迴圈，因為左邊彩圖weight重複用到//////////////////////
					if (x + d > W - 1)
						break;
					Rect rect(d, 0, windowSize - decrX - d, windowSize - decrY);
					weightWindowRef(rect).copyTo(weightWindow);

					//左window權重
					//target window mid point
					Vec3f twm = { L.at<Vec3f>(y, x + d)[0], L.at<Vec3f>(y, x + d)[1], L.at<Vec3f>(y, x + d)[2] };


					float wws = 0;//window weight sum
								  //相對中心點座標
					for (int wx = -windowR; wx <= windowR - decrX - d; wx++)////////////////////////////////////////////////////////
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							//顏色
							weightWindow.at<float>(wy + windowR, wx + windowR) *= exp(-sqrt(
								pow((L.at<Vec3f>(y + wy, x + wx + d)[0] - twm[0]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + wx + d)[1] - twm[1]), 2)
								+ pow((L.at<Vec3f>(y + wy, x + wx + d)[2] - twm[2]), 2)) / rc);
							////距離
							//weightWindow.at<float>(wy + windowR, wx + x - d) *= exp(-sqrt(pow(wx, 2) + pow(wy, 2)) / rg);

							wws += weightWindow.at<float>(wy + windowR, wx + windowR);

						}
					//@左window權重
					weightWindow /= wws;//正規化-----------------------------





										// windowR +1+ x -d

										//initial cost*權重               wx,wy從0開始是因為要讓weight window可以方便存取
					for (int wx = -windowR; wx <= windowR - decrX - d; wx++)
						for (int wy = -windowR; wy <= windowR - decrY; wy++) {
							aggregateMat.at<float>(y, x, d) += initial.at<float>(y + wy, x + wx, d)* weightWindow.at<float>(wy + windowR, wx + windowR);
						}
					//initial cost*權重
					/*if(aggregateMat.at<float>(y, x, d)>400)
					cout << aggregateMat.at<float>(y, x, d) << endl;*/
					/////////////////////////////////////////////////////////////////////
					//簡化 最小out
					if (doutCost > aggregateMat.at<float>(y, x, d)) {
						doutCost = aggregateMat.at<float>(y, x, d);
						doutVal = d;
					}

				}
				//@end 
				out.at<uchar>(y, x) = doutVal * scale;


			}
			++decrX;

		}
		++decrY;
	}
	std::cout << "左下左" << (double)clock() / CLOCKS_PER_SEC << " 秒";
	////@aggregate







	//L = imread(SL, 1);//convert image to the single channel grayscale image.
	//R = imread(SR, 1);
	//cout << endl << windowR << "  < x < " << windowR + range << endl;
	//cout << "y <" << H << endl;
	//int testX = 1, testY = 1;
	//while (testX != 0 && testY != 0) {
	//	cout << endl << "x=";
	//	cin >> testX;
	//	cout << "y=";
	//	cin >> testY;
	//	//cout << endl << "("+to_string(testX)")" << endl;
	//	for (int d = 0; d <= range; d++)
	//		cout << "d=" << d << ", cost=" << aggregateMat.at<float>(testY, testX, d) << endl;
	//	int d = out.at<uchar>(testY, testX) / scale;
	//	int pixalcut = (testX - d - windowR)*-1;
	//	//Mat partL,partR;
	//	//if (pixalcut > 0) {
	//	//	Mat partR = R(Rect(testX - windowR + pixalcut-d, testY - windowR, windowSize - pixalcut-d, windowSize));
	//	//	Mat partL = L(Rect(testX - windowR + pixalcut, testY - windowR, windowSize - pixalcut - d, windowSize));
	//	//	//imshow("partL", partL);
	//	//	//imshow("partR", partR);
	//	//	//waitKey(0);
	//	//}
	//	//if (pixalcut < 0) {
	//	//	Mat partR = R(Rect(testX - windowR, testY - windowR, windowSize, windowSize));
	//	//	Mat partL = L(Rect(testX - windowR, testY - windowR, windowSize, windowSize));
	//	//	//imshow("partL", partL);
	//	//	//imshow("partR", partR);
	//	//	//waitKey(0);
	//	//}
	//}







	//for (int y = 0; y < H; y++)
	//	out.at<uchar>(y, W-windowR) = 255;
	//for (int y = 0; y < H; y++)
	//	out.at<uchar>(y, W - windowR-range) = 255;
	cout << out.size() << endl;
	//imshow("right", out);

	imwrite("右深度.png", out);


	//}

	std::cout << (double)clock() / CLOCKS_PER_SEC << " 秒";

	//比較輸出值
	namedWindow("right", 0);
	setMouseCallback("right", onMouse, NULL);
	while (true) {
		if (interestPoint.y == -1 && interestPoint.x == -1) {
			imshow("right", out);
		}
		if (interestPoint.y != -1 && interestPoint.x != -1) {
			for (int i = 0; i <= range; i++) {
				cout << "range=" << i << " cost=" << aggregateMat.at<float>(interestPoint.y, interestPoint.x, i) << endl;
			}
			cout << (int)(out.at<uchar>(interestPoint.y, interestPoint.x))/scale << endl;
			interestPoint.x = -1;
			interestPoint.y = -1;
			imshow("right", out);
		}
		if (cvWaitKey(33) == 27) {//等33msec 按esc 跳出
			break;
		}
	}



#endif
	return 0;
}

