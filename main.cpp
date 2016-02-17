#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

using namespace std;

#include <mpirxx.h>

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

condition_variable cv;
mutex m;
bool flaggy = false;

void progress(unsigned int &count, unsigned int total) {
	do {
		{
			unique_lock<mutex> lk(m);
			cv.wait(lk, []{return flaggy;});
			flaggy = false;
		}
		cout << "\r";
		cout << (count * 100) / total << "% (" << count << "/" << total << ")";
		cout.flush();
	} while (count != total);
	cout << endl;
}

void incCount(unsigned int &c, unsigned int amount)
{
	lock_guard<mutex> lk(m);
	c += amount;
	flaggy = true;
	cv.notify_all();
}

void multiply_worker(vector<mpz_class>& q, size_t offset, size_t stride, mpz_class& result, unsigned int &c) {
	mpz_class p = 1;
	const auto len = q.size();
	unsigned int lc = 0;
	while (offset < len) {
		p *= q[offset];
		offset += stride;
		lc++;
		if(lc == 10) {
			incCount(c, lc);
			lc = 0;
		}
	}

	incCount(c, lc);
	result = p;
}

void gcd_worker(vector<mpz_class>& q, mpz_class& p, size_t offset, size_t stride, unsigned int &c) {
	mpz_class g;
	const auto len = q.size();
	unsigned int lc = 0;
	while (offset < len) {
		mpz_class n = q[offset];
		mpz_class t = p / n;
		mpz_gcd(g.get_mpz_t(), t.get_mpz_t(), n.get_mpz_t());
		if (g != 1) {
			cout << endl << "G: " << g.get_str(16) << " N: " << n.get_str(16) << endl;
		}
		offset += stride;
		lc++;
		if(lc == 10) {
			incCount(c, lc);
			lc = 0;
		}
	}
	incCount(c, lc);
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
	unsigned int count(0);
	unsigned int total(allkeys.size());

	thread progress_thread(progress, ref(count), total);

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

	unsigned int count2(0);
	progress_thread = thread(progress, ref(count2), total);

	threads.clear();

	for (int i = 0; i < N; ++i) {
		threads.push_back(thread(gcd_worker, ref(allkeys), ref(p), i, N, ref(count2)));
	}

	for (th : threads) th.join();

	progress_thread.join();

	cout << endl << "Goodbye." << endl;
	return 0;
}

