#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#pragma warning(disable: 4146 4244 4800)
#include <mpirxx.h>
#pragma warning(default: 4146 4244 4800)

/*
<Knio>   p = 1
<Knio>   for n in ns:
<Knio>     p *= n
<Knio>   for n in ns:
<Knio>     g = gcd(p // n, n)
<Knio>     if g != 1:
<Knio>       print('%d %d' % (g, n))
*/

int main(int argc, char *argv[])
{
	if (argc < 2) {
		cout << "Usage: " << argv[0] << " <filename>" << endl;
		return 1;
	}

	ifstream in;
	in.open(argv[1]);
	if (in.fail()) {
		cout << "Error opening the file." << endl;
		return 1;
	}

	vector<mpz_class> allkeys;
	mpz_class n;
	mpz_class p = 1;
	mpz_class g;
	string key;

	cout << "Reading keys from file. . ." << endl;

	while (in.good()) {
		getline(in, key);
		if (key != "") {
			allkeys.emplace(allkeys.end(), key, 16);
		}
		if (allkeys.back() == 0) {
			cout << "Opps looks like a key == 0 failing horribly now" << endl;
			return 1;
		}
	}

	cout << "Loaded " << allkeys.size() << " keys" << endl;
	cout << "Multiplying all keys together. . ." << endl;

	int c = 0;
	for each (n in allkeys)
	{
		cout << "\r";
		p *= n;
		c++;
		if ((c % 1000) == 0) {
			cout << c << " keys multiplied";
		}
	}
	cout << endl;
	cout << "Doing gcd thing. . ." << endl;

	mpz_class t;
	c = 0;
	for each (n in allkeys)
	{
		cout << "\r";
		t = p / n;
		mpz_gcd(g.get_mpz_t(), t.get_mpz_t(), n.get_mpz_t());
		if (g != 1) {
			cout << "G: " << g.get_str(16) << " N: " << n.get_str(16) << endl;
		}
		c++;
		if ((c % 1000) == 0) {
			cout << c << " keys gcdeded";
		}
	}
	cout << endl << "Goodbye." << endl;
	return 0;
}