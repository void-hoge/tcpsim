#include <iostream>
#include <random>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <deque>

std::default_random_engine engine;
std::uniform_real_distribution<double> rnd(0.0, 1.0);

bool is_disappear(const int w_size, const int w_limit) {
	return rnd(engine) < ((double)w_size/w_limit)*0.5;
}

std::mutex mtx;

void mutexout(std::string str) {
	mtx.lock();
	std::cout << str << std::endl;
	mtx.unlock();
}

template<class T>
class mutdeque{
private:
	std::deque<T> data;
public:
	std::mutex mtx;
	mutdeque (){};
	void push_back(const T val) {
		// std::lock_guard<std::mutex> lock(mtx);
		data.push_back(val);
	}
	void pop_back() {
		// std::lock_guard<std::mutex> lock(mtx);
		data.pop_back();
	}
	void pop_front() {
		data.pop_front();
	}
	size_t size() const{
		// std::lock_guard<std::mutex> lock(mtx);
		return data.size();
	}
	T& back() {
		// std::lock_guard<std::mutex> lock(mtx);
		return data.back();
	}
	T& front() {
		// std::lock_guard<std::mutex> lock(mtx);
		return data.front();
	}
	void clear() {
		// std::lock_guard<std::mutex> lock(mtx);
		data.clear();
	}
	bool empty() {
		// std::lock_guard<std::mutex> lock(mtx);
		return data.empty();
	}
};

void sender(mutdeque<int>& netstack, mutdeque<int>& ack, const int max) {
	int lastsent = -1;
	int w_size = 1;
	int w_limit = 100;
	int count = 0;
	bool flag = true;
	int lastack = -1;
	while (true) {
		// std::cout << "begin of sender loop" << '\n';
		ack.mtx.lock();
		// mtx.lock();
		int w_size_remain = w_size;
		while (!ack.empty()) {
			flag = true;
			int rcvd = ack.front();
			if (lastsent > max) {
				return;
			}
			ack.pop_front();
			// std::cout << "sender: ack" << rcvd << " recieved" << '\n';
			mutexout("sender: ack"+std::to_string(rcvd)+" recieved");
			if (rcvd == lastack) {
				count++;
			}else {
				count = 0;
			}
			lastack = rcvd;
			if (count == 3) {
				netstack.push_back(rcvd+1);
				// std::cout << "sender: pkt" << rcvd+1 << " resent" << '\n';
				mutexout("sender: pkt"+std::to_string(rcvd+1)+" resent");
				w_size = w_size*3/5;
				mutexout("sender: window size decreased:"+std::to_string(w_size));
				w_size_remain = w_size-1;
				ack.clear();
			}
		}
		if (flag) {
			for (int i = 0; i < w_size_remain; i++) {
				// std::cout << "sender: pkt" << lastsent << " sent" << '\n';
				lastsent++;
				if (!is_disappear(w_size, w_limit)) {
					netstack.push_back(lastsent);
				}else {
					mutexout("pkt"+std::to_string(lastsent)+" disappeared");
				}
				mutexout("sender: pkt"+std::to_string(lastsent)+" sent");
			}
			if (w_size == w_size_remain) {
				w_size += 1;
				mutexout("sender: window size increased:"+std::to_string(w_size));
			}
			flag = false;
		}
		ack.mtx.unlock();
		// mtx.unlock();
	}
	// std::cout << "sender end" << '\n';
}

void reciever(mutdeque<int>& netstack, mutdeque<int>& ack, const int max) {
	int lastrecieved = -1;
	mutdeque<int> rcvd;
	while (true) {
		netstack.mtx.lock();
		// mtx.lock();
		// std::cout << "begin of reciever loop" << '\n';
		while (!netstack.empty()) {
			int tmp = netstack.front();
			netstack.pop_front();
			// std::cout << "reciever: pkt" << tmp << " recieved" << '\n';
			mutexout("reciever: pkt"+std::to_string(tmp)+" recieved");
			if (tmp == lastrecieved+1) {
				lastrecieved = tmp;
				ack.push_back(lastrecieved);
				// std::cout << "reciever: ack" << tmp << " sent" << '\n';
				mutexout("reciever: ack"+std::to_string(lastrecieved)+" sent");
				while (!rcvd.empty()) {
					if (rcvd.front() == lastrecieved+1) {
						rcvd.pop_front();
						lastrecieved++;
						mutexout("reciever: ack"+std::to_string(lastrecieved)+" sent");
					}else {
						break;
					}
				}
			}else {
				rcvd.push_back(tmp);
				ack.push_back(lastrecieved);
				// std::cout << "reciever: ack" << tmp << " sent" << '\n';
				mutexout("reciever: ack"+std::to_string(lastrecieved)+" sent");
			}
			if (lastrecieved > max) {
				return;
			}
		}
		netstack.mtx.unlock();
		// mtx.unlock();
		// std::cout << "end of reciever loop" << '\n';
	}
	std::cout << "reciever end" << '\n';
}

int main() {
	const int max = 1000; // 0-1000までの整数をパケットとする
	mutdeque<int> netstack;
	mutdeque<int> ack;
	std::thread send([&]{sender(netstack, ack, max);});
	std::thread recieve([&]{reciever(netstack, ack, max);});
	send.join();
	recieve.join();
	return 0;
}
