#include "gtest/gtest.h"
#include <shared_queues/processor.h>
#include <ctime>
#include <cstdlib>
#include <chrono>

namespace clients
{

class client
{
public:
    ~client()
    {
        std::cout << __func__ << '\n';
    }

    void notify(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::cout << key << ' ' << value << '\n';
        m_data.emplace_back(key, value); 
    }

    std::mutex m_mutex;
    std::vector<std::pair<std::string, std::string>> m_data;

}; // class Client_1

} // namespace clients

void worker_thread(std::string key, std::vector<std::string> values, std::shared_ptr<shared_queues::processor<std::string, std::string>> processor)
{
    for (const auto& value : values)
    {
        processor->push(key, value);
    }
}

void gen(std::vector<std::string>& data)
{
    for (int i = 0; i < 10000; i++)
    {
        data.emplace_back(std::to_string(std::rand()));
    }
}

TEST (shared_queues_test, base) { 
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    auto processor = std::make_shared<shared_queues::processor<std::string, std::string>>();
    auto client_1 = std::make_shared<clients::client>();
    auto client_2 = std::make_shared<clients::client>();
    processor->subscribe(std::string("red"), client_1, &clients::client::notify);
    processor->subscribe(std::string("black"), client_2, &clients::client::notify);
    std::vector<std::string> red_values;
    gen(red_values);
    std::vector<std::string> black_values;
    gen(black_values);
    std::thread thread_1(worker_thread, "red", red_values, processor);
    std::thread thread_2(worker_thread, "black", black_values, processor);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    if (thread_2.joinable())
        thread_2.join();
    if (thread_1.joinable())
        thread_1.join();
    std::sort(red_values.begin(), red_values.end());
    std::sort(black_values.begin(), black_values.end());
    std::sort(client_1->m_data.begin(), client_1->m_data.end());
    std::sort(client_2->m_data.begin(), client_2->m_data.end());
    auto comp = [](const std::string& left, const std::pair<std::string, std::string>& right) { return left.compare(right.second) == 0; };
    EXPECT_TRUE(std::equal(red_values.begin(), red_values.end(), client_1->m_data.begin(), comp));
    EXPECT_TRUE(std::equal(black_values.begin(), black_values.end(), client_2->m_data.begin(), comp));
}
