#include <mutex>
#include <queue>
#include <list>

// Simple mutex-guarded queue
template<typename T> class ThreadSafeQueue
{
private:
	std::mutex mutex;
	std::queue<T> queue;
public:
	void push(T value)
	{
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(value);
	};

	T pop()
	{
		std::unique_lock<std::mutex> lock(mutex);
		T value;
		std::swap(value, queue.front());
		queue.pop();
		return value;
	};

	bool empty() {
		std::unique_lock<std::mutex> lock(mutex);
		return queue.empty();
	}
};
