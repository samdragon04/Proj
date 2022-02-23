#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <bitset>
#include <iomanip>

int main()
{
	using namespace std;
	
	//----------cru,cmの値を取得----------
	vector<float> truth_cm;
	vector<float> truth_analog;
	vector<string> cru;
	string filename = "data/O2simResult.txt";
	ifstream ifs(filename);
	if(!ifs)
	{
		cout << "Can't open " << filename << endl;
		return 1;
	}
			

	string cru1 = "now";
	string cru2 = "prev";
	string time1 = "now";
	string time2 = "prev";
	string str;		//不必要なデータ用
	float t_cm;		//truth CM
	float t_analog;

	getline(ifs,cru1);
	
	//timeあるいはcruが変わる→次の1600個のデータ
	while(ifs >> time1 >> cru1)
	{
		ifs >> str >> str >> t_cm >> str >> t_analog >> str;
		truth_analog.push_back(t_analog);
		truth_cm.push_back(t_cm);
		cru.push_back(cru1);
		if(time2 != time1)
		{
			//cout << time1 << " "  << t_cm << endl;
			//truth_cm.push_back(t_cm);
			//cru.push_back(cru1);
		}
		else if(cru2 != cru1)
		{
			//cout << time1 << " " cru1 << " " << t_cm << endl;
			//truth_cm.push_back(t_cm);
			//cru.push_back(cru1);
	
		}
		time2 = time1;
		cru2 = cru1;
	}

	ifs.close();


	//---------fittingの計算----------
	filename = "data/ModelsimInputData_pedestal.txt";
	ifs.open(filename);
	if(!ifs)
	{
		cout << "Can't open " << filename << endl;
		return 1;
	}
	
	int input;
	vector<int> input_data;
	vector<int> input_data_pe;
	vector<float> calc_cm;		//CM by fitting
	vector<float> o2_adc;
	int cru_pos = 0;


	while(ifs >> hex >> input)
	{
	cru_pos += 1;
		if ((stoi(cru[cru_pos-1])) % 10 < 2 )
		{

		input_data.push_back(input);
		input_data_pe.push_back(input);
		if(input_data.size() < 1200)
		{
			continue;
		}

		int cnt = 1200, i = 0;
		vector<int> hist;
		hist.push_back(cnt);
		while(cnt > 600)
		{
			cnt -= count(input_data.begin(), input_data.end(), i++);
			hist.push_back(cnt);
		}
		
		float a, b, med_cm, med;
		a = hist[i] - hist[i -1];
		b = hist[i - 1] - (hist[i] - hist[i - 1]) * (i - 1);
		med_cm = (600 - b) / a;

		for (int j = 1; j < 500; j++)
		{
			if (med_cm == 0)
			{
				med = 0;
			}

			else if (med_cm > ((j-1) * 0.25) && med_cm <= (j * 0.25))
			{
				med = (j * 0.25);
			}
		}

		float o2_adcs;

		for (int k = 0; k < 1200; k++)
		{
			o2_adcs = input_data[k] - med;
			o2_adc.push_back(o2_adcs);
		}
		
		calc_cm.push_back(med);
		input_data.clear();
		}


		else if ((stoi(cru[cru_pos-1])) % 10 > 1 && (stoi(cru[cru_pos-1])) % 10 < 6 )
		{
		input_data.push_back(input);
		input_data_pe.push_back(input);
		if(input_data.size() < 1440)
		{
			continue;
		}

		int cnt = 1440, i = 0;
		vector<int> hist;
		hist.push_back(cnt);
		while(cnt > 720)
		{
			cnt -= count(input_data.begin(), input_data.end(), i++);
			hist.push_back(cnt);
		}
		
		float a, b, med_cm, med;
		a = hist[i] - hist[i -1];
		b = hist[i - 1] - (hist[i] - hist[i - 1]) * (i - 1);
		med_cm = (720 - b) / a;

		for (int j = 1; j < 500; j++)
		{
			if (med_cm == 0)
			{
				med = 0;
			}

			else if (med_cm > ((j-1) * 0.25) && med_cm <= (j * 0.25))
			{
				med = (j * 0.25);
			}
			
		}

		float o2_adcs;

		for (int k = 0; k < 1440; k++)
		{
			o2_adcs = input_data[k] - med;
			o2_adc.push_back(o2_adcs);
		}
		
		calc_cm.push_back(med);
		input_data.clear();
		}


		else if ((stoi(cru[cru_pos-1])) % 10 > 5)
		{
		input_data.push_back(input);
		input_data_pe.push_back(input);
		if(input_data.size() < 1600)
		{
			continue;
		}

		int cnt = 1600, i = 0;
		vector<int> hist;
		hist.push_back(cnt);
		while(cnt > 800)
		{
			cnt -= count(input_data.begin(), input_data.end(), i++);
			hist.push_back(cnt);
		}
		
		float a, b, med_cm, med;
		a = hist[i] - hist[i -1];
		b = hist[i - 1] - (hist[i] - hist[i - 1]) * (i - 1);
		med_cm = (800 - b) / a;

		for (int j = 1; j < 500; j++)
		{
			if (med_cm == 0)
			{
				med = 0;
			}

			else if (med_cm > ((j-1) * 0.25) && med_cm <= (j * 0.25))
			{
				med = (j * 0.25);
			}
			
		}

		float o2_adcs;

		for (int k = 0; k < 1600; k++)
		{
			o2_adcs = input_data[k] - med;
			o2_adc.push_back(o2_adcs);
		}
		
		calc_cm.push_back(med);
		input_data.clear();
		}

		
	}
	ifs.close();

	//---------結果をファイルに出力----------
	filename = "data/adc_error.txt";
	ofstream ofs(filename);
	if(!ofs)
	{
		cout << "Can't open " << filename << endl;
		return 1;
	}

	float analog_data, total_error;
	if(o2_adc.size() == truth_analog.size())
	{
		for(auto i = 0; i < truth_analog.size(); ++i)
		{
			analog_data = truth_analog[i] - truth_cm[i];
			total_error = o2_adc[i] - analog_data;

			ofs << analog_data << " "  << o2_adc[i] << " " << total_error << endl;			
		}
	}
	else
	{
		cout << "A number of common mode is not equal." << endl;
		cout << "truth_analog: " << truth_analog.size() << " o2_adc: "  << o2_adc.size() << endl;
		return 1;
	}
		
	return 0;
}
