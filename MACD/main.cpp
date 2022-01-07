#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#define DEBUG 0 // proste macro do debugowania - zly koncept, ale nie potrzebuje nic ponad to w tym momencie
#define fileName "wig20.csv"

typedef struct resources {
	float money;
	int stocks;
} resources;


float alpha(int N)
{
	return (2.0 / (N + 1.0));
}

void printVector(std::vector < float > &v)
{
	for (auto i = v.begin(); i != v.end(); ++i)
		std::cout << *i << ' ' << std::endl;
}

void printToFile(std::vector <float> v, std::string fileOutName)
{
	std::ofstream file;
	file.open(fileOutName, std::ios::out);
	for (auto i = v.begin(); i != v.end(); ++i)
		file << *i << std::endl;
}

void loadData(std::vector <float> &v)
{
	std::ifstream file;
	file.open(fileName, std::ios::in);
	std::string tmp;
	float tmpFloat = 0;
	int i = 0;
	char separator = ' ';  // jakim znakiem sa oddzielone dane
	std::getline(file, tmp); // pominiecie linii z opisem kolumn
	while (std::getline(file, tmp, separator))
	{
		if (i % 5 == 1)  // == 1 jesli chcemy dane z 2. kolumny, == 2 jesli z 3. itp.
		{
			tmpFloat = stof(tmp);
			v.push_back(tmpFloat);
			if(DEBUG) std::cout << tmp << std::endl;
		}
		i++;
	}
}

float EMA(std::vector <float> v, int N, int currentDay)
{
	float nominator = 0.0F, denominator = 0.0F;
	float oneMinusAlpha = 1.0 - alpha(N);
	float tmp = 0.0;
	for (int i = 0; i <= N; i++)
	{
		tmp = pow(oneMinusAlpha, i);

		if (currentDay - i > 0)
			nominator += tmp * v[currentDay - i];
		else
			nominator += tmp * v[0];

		denominator += tmp;
		if (DEBUG)
		{
			std::cout << N << ", " << tmp << ", " << nominator << ", " << denominator << " -> " << (nominator / denominator) << std::endl;
		}
	}
	return nominator / denominator;
}

void buyStock(std::vector <float> price, int day, resources *resources)
{
	if (resources->money >= price[day])
	{
		resources->stocks++;
		resources->money -= price[day];
		std::cout << "bought 1 stock for " << price[day];
		std::cout << " balance: " << resources->money << std::endl;
	}
}

void sellAllStocks(std::vector <float> price, int day, resources *resources)
{
	float tmp = 0;
	if (resources->stocks > 0)
	{
		for (int i = 0; i < resources->stocks; i++)
		{
			tmp += price[day];
		}
		resources->money += tmp;
		std::cout << "sold " << resources->stocks << " stock for " << price[day];
		resources->stocks = 0;
		std::cout << " balance: " << resources->money << std::endl;
	}

}

void sellStock(std::vector <float> price, int day, resources* resources)
{
	float tmp = 0;
	if (resources->stocks > 0)
	{
		resources->money += price[day];
		std::cout << "sold 1 stock for " << price[day];
		resources->stocks--;
		std::cout << " balance: " << resources->money << std::endl;
	}
}

void strategyA(std::vector <float> macd, std::vector <float> signal, std::vector <float> input)
{
	//Strategia naiwna - kupujemy akcje kiedy MACD przecina od dolu i sprzedajemy wszystkie akcje kiedy MACD przecina od gory
	resources resources;
	resources.money = 100000.0;
	resources.stocks = 0;
	if (DEBUG)
	{
		std::cout << resources.money << " " << resources.stocks << std::endl;
	}
	int length = macd.size();
	if (signal.size() > length) length = signal.size();
	for (int i = 1; i < length; i++)
	{
		if (macd[i - 1] < signal[i - 1] && macd[i] > signal[i])
		{
			buyStock(input, i, &resources);
		}
		else if (macd[i - 1] > signal[i - 1] && macd[i] < signal[i])
		{
			sellAllStocks(input, i, &resources);
		}
	}
}

void strategyB(std::vector <float> macd, std::vector <float> signal, std::vector <float> input)
{

	resources resources;
	resources.money = 100000.0;
	resources.stocks = 0;
	
	bool buySignal = false;
	bool sellSignal = false;
	int daySnap = 0;
	float priceSnap = 0;

	if (DEBUG)
	{
		std::cout << resources.money << " " << resources.stocks << std::endl;
	}

	int length = macd.size();
	if (signal.size() > length) length = signal.size();

	for (int i = 1; i < length; i++)
	{
		if (macd[i - 1] < signal[i - 1] && macd[i] > signal[i])
		{
			sellSignal = true;
			buySignal = false;
			daySnap = i;
			priceSnap = input[i];
		}
		else if (macd[i - 1] > signal[i - 1] && macd[i] < signal[i])
		{
			buySignal = true;
			sellSignal = false;
			daySnap = i;
			priceSnap = input[i];
		}

		if (sellSignal)
		{
			if (i >= daySnap + 3 && input[i] > priceSnap )
			{
				sellStock(input, i, &resources);
			}
		}
		else if (buySignal)
		{
			if (i >= daySnap + 3 && input[i] < priceSnap)
			{
				buyStock(input, i, &resources);
			}
		}
	}
	sellAllStocks(input, length-1, &resources);
}

void strategyC(std::vector <float> macd, std::vector <float> signal, std::vector <float> input)
{

}

int main()
{
	std::vector < float > input;
	std::vector < float > macd;
	std::vector < float > signal;

	loadData(input);

	if (DEBUG) printVector(input);

	for (int i = 0; i < input.size(); i++)
	{
		macd.push_back(EMA(input, 12, i) - EMA(input, 26, i));
		signal.push_back(EMA(macd, 9, i));
	}
	if(DEBUG)
	{
		std::cout << macd.size();
		std::cout << signal.size();
	}
	printToFile(macd, "macd.txt");
	printToFile(signal, "signal.txt");
	printToFile(input, "input.txt");

	strategyA(macd, signal, input);
	std::cout << std::endl << std::endl;
	strategyB(macd, signal, input);
	std::cout << std::endl << std::endl;
	strategyC(macd, signal, input);

}