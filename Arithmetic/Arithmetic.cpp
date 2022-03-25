#include <iostream>
#include <vector>															
#include <list>																
#include <map>																
#include <fstream>


using namespace std;

class Section {
public:
	int a;
	char s;
	int Left;
	int Right;
};

bool cmp (Section a, Section b){
	return a.a > b.a;
}

class Arithmetic{	
private:
	map< char, int> table_freq; //таблица с частотами
	map< char, int>::iterator iter;
	list<Section> sect;
	map<char, int> count_freq_en(ifstream& file);
	map<char, int> count_freq_de(ifstream& file);
	list<Section> creat_table();
	void wtf(std::ofstream& file, map <char, int> freq, int n);	
	void wbtf(int st, int& n, std::ofstream& file);

	
public:
	Arithmetic();
	int h, h1, h2, h3, bit; 
	char s;
	void encode(ifstream& file, ofstream& result);
	void decode(ifstream& file, ofstream& result);

};

Arithmetic::Arithmetic() {
	h = 65535;
	h1 = (h + 1) / 4;
	h2 = h1 * 2;
	h3 = h2 * 3;
	bit = 0;
	s = 0;
};

void Arithmetic::encode(ifstream& file, ofstream& result) {
	count_freq_en(file);											//подсчитываем частоты символов
	creat_table();
	wtf(result, table_freq, table_freq.size());
	int div = sect.back().Right;									//делитель
	int	low = 0;													//нижняя граница отрезка
	 
	file.clear(); 
	file.seekg(0);

	int count = 0;
	auto iter_beg = sect.begin();  

	while (!file.eof()) {
		char c = file.get();

		// пересчитываем границы
		int r = (h - low) + 1;
		h = low + (r * iter_beg->Right) / div - 1;
		low = low + (r * iter_beg->Left) / div;
		
		//побитово записываем в результат
		for (;;) {
			if (h < h2) {											
				wbtf(0, count, result);
			}
			else if (low >= h2) {
				wbtf(1, count, result);
				low -= h2;
				h -= h2;
			}
			else if ((low >= h1) && (h < h3)) {
				bit++;
				low -= h3;
				h -= h3;
			}
			else break;
			//сдвиг
			low = 2 * low;
			h = 2 * h + 1;
		}
	}
}

void Arithmetic::decode(ifstream& file, ofstream& result) {
	count_freq_de(file);											//подсчитываем частоты символов
	creat_table();
	auto it = sect.begin();

	int n = 0;
	int low = 0; 
	int div = sect.back().Right;
	int v = (file.get() << 8) | file.get();
	char c = file.get();

	while (!file.eof()) {
		int r = h - low + 1;											//текущий масштаб
		int cum = ((v - low + 1) * div - 1) / r;						//масштабирование
		for (it = sect.begin(); it->Right <= cum; it++);				//поиск 

		// пересчет границ
		h = low + (it->Right) * r / div - 1;
		low = low + (it->Left) * r / div;

		for (;;) {  
			if (h < h2);   
			else if (low >= h2) {
				low -= h2;
				h -= h2;
				v -= h2;
			}
			else if ((low >= h1) && (h < h3)) {
				low -= h1;
				h -= h1;
				v -= h1;
			}
			else break;
			low = 2 * low;
			h = 2 * h + 1;
			v = 2 * v + (((short)c >> (7 - n)) & 1);
			n++;
			if (n == 8) {
				c = file.get();
				n = 0;
			}
		}
		result << it->s;
	}
}

map<char, int> Arithmetic::count_freq_en(ifstream& file) {
	table_freq.clear();

	char s;
	while ((s = file.get()) && !file.eof()) {
		table_freq[s]++;
	}
	return table_freq;
};

map<char, int> Arithmetic::count_freq_de(ifstream& file){
	table_freq.clear();
	int count, sym;
	char s;
	file.read((char*)&count, sizeof(count));
	while (count > 0) {
		file.read((char*)&s, sizeof(s));
		file.read((char*)&sym, sizeof(sym));
		--count;
		table_freq[s] = sym;
	}
	return table_freq;
}

list<Section> Arithmetic::creat_table() {
	sect.clear();
	for (iter = table_freq.begin(); iter != table_freq.end(); iter++) {
		Section t;
		t.s = iter->first;
		t.a = iter->second;
		sect.push_back(t);
	}
	sect.sort(cmp);
	sect.begin()->Right = sect.begin()->a;
	sect.begin()->Left = 0;

	list<Section>::iterator iter1 = sect.begin();
	iter1++;

	list<Section>::iterator iter2 = sect.begin();
	
	for (; iter1 != sect.end(); iter1++, iter2++) {
		iter1->Left = iter2->Right;
		iter1->Right = iter1->Left + iter1->a;
	}

return sect;
}

void Arithmetic:: wtf(std::ofstream& file, map <char, int> freq, int n){
	file.write((char*)(&n), sizeof(n));

	for (int i = 0; i < 256; i++) {
		if (freq[char(i)] > 0) {
			char s = char(i);
			file.write((char*)(&s), sizeof(s));
			file.write((char*)(&freq[char(i)]), sizeof(freq[char(i)]));
		}
	}
}

void Arithmetic:: wbtf(int st, int& n, std::ofstream& file) {
	if (st == 0) {
		n++;
		if (n == 8) {
			n = 0;
			file << s;
			s = 0;
		}
		while (bit > 0) {
			s = s | (1 << (7 - n));
			n++;
			if (n == 8) {
				n = 0;
				file << s;
				s = 0;
			}
			bit--;
		}
	}
	else if (st == 1) {
		s = s | (1 << (7 - n));
		n++;
		if (n == 8) {
			n = 0;
			file << s;
			s = 0;;
		}
		while (bit > 0) {
			n++;
			if (n == 8) {
				n = 0;
				file << s;
				s = 0;
			}
			bit--;
		}
	}
}

int main() {											

	ifstream file("input.txt", ios::binary);
	if (!file) return 1;

	ofstream result("output.txt", ios::out | ios::binary);
	if (!result) return 1;

	Arithmetic ar;
	ar.encode(file, result);

	file.close();
	result.close();

	ifstream file2("output.txt", ios::out | ios::binary);
	if (!file2) return 1;

	ofstream result2("result.txt");
	if (!result2) return 1;

	ar.decode(file2, result2);

	file.close();
	result.close();

	ifstream fin1("input.txt", ios::binary);
	ifstream fin2("result.txt", ios::binary);
	if (fin1 && fin2) {
		char ch1, ch2;
		bool result = true;
		while (fin1.get(ch1) && fin2.get(ch2)) {
			if (ch1 != ch2) {
				result = false;
				break;
			}
		}
		if (result) cout << "Equal" << endl;
		else cout << "Unequal" << endl;
	}
	else cout << "Error opening file!" << endl;

	return 0;
}
