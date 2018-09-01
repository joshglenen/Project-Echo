#include "stdafx.h"
#include <iostream>
#include <math.h>
#include <deque>
#include <complex>
#include <valarray>
#define N 16

//decided to abandon kiss_fft in favor of the more supported but expensive fftw libraries//

extern "C" 
{
#include "kiss_fft.h"
#include "kiss_fftr.h"
}

using namespace std;

class signalStatisticsForDeque
{
public:double static myMean(deque<double> args[])
{
	double mean = 0;
	for (int i = 0; i < args->size(); i++)  mean += args->at(i);
	return mean / args->size();
}

public:double static variance(deque<double> args[])
{
	double var = 0;
	double mean = myMean(args);
	for (int i = 0; i < args->size(); i++)
	{
		var += (args->at(i) - mean)*(args->at(i) - mean);
	}
	return var / (args->size() - 1);
}

public:double static stdDeviation(deque<double> args[])
{
	double sd = variance(args);
	return sqrt(sd);
}

public:double static avgMagnitude(deque<double> args[])
{
	double avg = 0;
	for (int i = 0; i < args->size(); i++) avg += abs(args->at(i));
	return avg / args->size();
}

public:double static avgPower(deque<double> args[])
{
	double avg = 0;
	for (int i = 0; i < args->size(); i++) avg += args->at(i)*args->at(i);
	return avg / args->size();
}

public:int static zeroCrossings(deque<double> args[])
{
	int buffer = 0;
	for (int i = 1; i < args->size(); i++)
	{
		if ((args->at(i - 1) == 0) && (i>2)) if (((args->at(i - 2) >= 0) && (args->at(i)<0)) || (args->at(i - 2) <= 0) && (args->at(i) > 0)) buffer++;
		else if (((args->at(i - 1) >= 0) && (args->at(i)<0)) || (args->at(i - 1) <= 0) && (args->at(i) > 0)) buffer++;
	}

	return buffer;
}
};

class signalGenerator
{
	public: double static sinusoid(int  time, int period = 1000, double amplitude = 1)
	{
		double buffer = amplitude*sin(2*3.14159265*time / period);
		if (buffer < 0.0001) return 0; //removes epsilon for cleaner data
		return buffer;
	}
};

class audioBuffer : signalGenerator
{
	private: int bufferSize = 40;
	public: deque<double> leftBuffer;
	public: deque<double> rightBuffer;

	public: bool bufferFull(char L_or_R)
	{
		if ('L')
		{
			if (leftBuffer.size() == bufferSize) return true;
		}
		else if ('R')
		{
			if (rightBuffer.size() == bufferSize) return true;
		}
		return false;
	}

	public:audioBuffer()
	{
		cout << "!audio buffer created!" << endl;
	}

	public:~audioBuffer()
	{
		cout << "!audio buffer deleted!" << endl;
	}

	public:void DisplayLeftContents()
	{
		for (unsigned int i = 0; i < leftBuffer.size(); i++)
		{
			std::cout << "Element[" << i << "] = " << leftBuffer.at(i) << std::endl;
		}
		std::cout << std::endl;
	}

	public:void DisplayRightContents()
	{
		for (unsigned int i = 0; i < rightBuffer.size(); i++)
		{
			std::cout << "Element[" << i << "] = " << rightBuffer.at(i) << std::endl;
		}
		std::cout << std::endl;
	}

	public:double leftInput(int  t)
	{
		leftBuffer.push_back(sinusoid(t));
		if (leftBuffer.size() > bufferSize)
		{
			double ret = leftBuffer.front();
			leftBuffer.pop_front();
			return ret;
		}
		return 0;
	}

	public:double rightInput(int  t)
	{
		rightBuffer.push_back(sinusoid(t, 12));
		if (rightBuffer.size() > bufferSize)
		{
			double ret = rightBuffer.front();
			rightBuffer.pop_front();
			return ret;
		}
		return 0;
	}

};

class EarSensors : public audioBuffer, signalStatisticsForDeque
{
	public: EarSensors() {}
	public: ~EarSensors() {}

	public: double left = 0;
	public: double right = 0;

	public: void WhichEar(int time)
	{
		left = leftInput(time);  right = rightInput(time);
		if (left>right) cout << "Sound is coming from the left side" << endl;
		else if (left<right) cout << "Sound is coming from the right side" << endl;
	}

	public: void GetStatistics()
	{
		cout << endl << "LEFT EAR: " << endl << "Mean: " << signalStatisticsForDeque::myMean(&leftBuffer) << endl;
		cout <<  "Average Magnitude: " << signalStatisticsForDeque::avgMagnitude(&leftBuffer) << endl;
		cout <<  "Average Power: " << signalStatisticsForDeque::avgPower(&leftBuffer) << endl;
		cout <<  "Zero Crossings: " << signalStatisticsForDeque::zeroCrossings(&leftBuffer) << endl;
		cout <<  "Variance: " << signalStatisticsForDeque::variance(&leftBuffer) << endl;
		cout <<  "Std Deviation: " << signalStatisticsForDeque::stdDeviation(&leftBuffer) << endl;
		cout << endl << endl << "Right EAR: " << endl << "Mean: " << signalStatisticsForDeque::myMean(&rightBuffer) << endl;
		cout << "Average Magnitude: " << signalStatisticsForDeque::avgMagnitude(&rightBuffer) << endl;
		cout << "Average Power: " << signalStatisticsForDeque::avgPower(&rightBuffer) << endl;
		cout << "Zero Crossings: " << signalStatisticsForDeque::zeroCrossings(&rightBuffer) << endl;
		cout << "Variance: " << signalStatisticsForDeque::variance(&rightBuffer) << endl;
		cout << "Std Deviation: " << signalStatisticsForDeque::stdDeviation(&rightBuffer) << endl;
	}

};

void TestFft(const char* title, const kiss_fft_cpx in[N], kiss_fft_cpx out[N])
{
	kiss_fft_cfg cfg;

	printf("%s\n", title);

	if ((cfg = kiss_fft_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
	{
		size_t i;

		kiss_fft(cfg, in, out);
		free(cfg);

		for (i = 0; i < N; i++)
			printf(" in[%2zu] = %+f , %+f    "
				"out[%2zu] = %+f , %+f\n",
				i, in[i].r, in[i].i,
				i, out[i].r, out[i].i);
	}
	else
	{
		printf("not enough memory?\n");
		exit(-1);
	}
}

void TestFftReal(const char* title, const kiss_fft_scalar in[N], kiss_fft_cpx out[N / 2 + 1])
{
	kiss_fftr_cfg cfg;

	printf("%s\n", title);

	if ((cfg = kiss_fftr_alloc(N, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
	{
		size_t i;

		kiss_fftr(cfg, in, out);
		free(cfg);

		for (i = 0; i < N; i++)
		{
			printf(" in[%2zu] = %+f    ",
				i, in[i]);
			if (i < N / 2 + 1)
				printf("out[%2zu] = %+f , %+f",
					i, out[i].r, out[i].i);
			printf("\n");
		}
	}
	else
	{
		printf("not enough memory?\n");
		exit(-1);
	}
}

void main()
{
	int kkk = 0;
	char buff = 0;
	EarSensors myEars = EarSensors();

	/////////////////


	kiss_fft_scalar in[N];
	kiss_fft_cpx out[N / 2 + 1];
	size_t i;

	for (i = 0; i < N; i++)
		in[i] = 0;
	//TestFftReal("Zeroes (real)", in, out);

	for (i = 0; i < N; i++)
		in[i] = 1;
	//TestFftReal("Ones (real)", in, out);

	for (i = 0; i < N; i++)
		in[i] = sin(2 * 3.14159 * 4 * i / N);
	//TestFftReal("SineWave (real)", in, out);


	/////////////////

	while (1) 
	{
		kkk++;
		if (myEars.bufferFull('L'))
		{
			//myEars.GetStatistics();
			//myEars.WhichEar(i);
			cin >> buff;
		}
		else
		{
			myEars.leftInput(kkk);  myEars.rightInput(kkk);
		}
	}
}
