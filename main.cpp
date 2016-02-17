#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

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
/*
<Knio> i7 4790K
<Knio> FOUR GIGLEHURTZ
*/

void progress(atomic<unsigned int>& count, atomic<unsigned int>& total) {
	cout << endl;
	do {
		cout << "\r";
		cout << (count * 100) / total << "% (" << count << "/" << total << ")";
	} while (count != total);
	cout << endl;
}

void multiply_worker(vector<mpz_class>& q, size_t offset, size_t stride, mpz_class& result, atomic<unsigned int>& c) {
	mpz_class p = 1;
	const auto len = q.size();
	while (offset < len) {
		p *= q[offset];
		offset += stride;
		c++;
	}

	result = p;
}

void gcd_worker(vector<mpz_class>& q, mpz_class& p, size_t offset, size_t stride, atomic<unsigned int>& c) {
	mpz_class g;
	const auto len = q.size();
	while (offset < len) {
		mpz_class n = q[offset];
		mpz_class t = p / n;
		mpz_gcd(g.get_mpz_t(), t.get_mpz_t(), n.get_mpz_t());
		if (g != 1) {
			cout << "G: " << g.get_str(16) << " N: " << n.get_str(16) << endl;
		}
		offset += stride;
		c++;
	}
}

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

	const int N = 8;
	vector<mpz_class> allkeys;
	vector<thread> threads;
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
			cout << "Oops looks like a key == 0 failing horribly now" << endl;
			return 1;
		}
	}

	cout << "Loaded " << allkeys.size() << " keys" << endl;
	cout << "Multiplying all keys together. . ." << endl;


	mpz_class results[N];
	atomic<unsigned int> count(1);
	atomic<unsigned int> total(allkeys.size());

	thread progress_thread(progress, ref(count), ref(total));

	for (int i = 0; i < N; ++i) {
		threads.push_back(thread(multiply_worker, ref(allkeys), i, N, ref(results[i]), ref(count)));
	}

	for (th : threads) th.join();

	for (r : results) {
		p *= r;
	}

	progress_thread.join();

	cout << endl;
	cout << "Doing gcd thing. . ." << endl;

	atomic<unsigned int> count2(1);
	progress_thread = thread(progress, ref(count2), ref(total));

	threads.clear();

	for (int i = 0; i < N; ++i) {
		threads.push_back(thread(gcd_worker, ref(allkeys), ref(p), i, N, ref(count2)));
	}

	for (th : threads) th.join();

	progress_thread.join();

	cout << endl << "Goodbye." << endl;
	return 0;
}

