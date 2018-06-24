
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

#include "jimi/http_all.h"

namespace timax {
namespace rpc {

struct endpoint
{
    std::string address;
    std::string port;

    endpoint() {}
    endpoint(const endpoint & src) {
        address = src.address;
        port = src.port;
    }

    void from_string(const std::string & str) {
        // TODO: from string
    }

    int parse(const std::string & info) {
        from_string(info);
        return 0;
    }

    void set_endpoint(const endpoint & _endpoint) {
        address = _endpoint.address;
        port = _endpoint.port;
    }

    void set_endpoint(const std::string & str) {
        from_string(str);
    }
};

class endpoint_list
{
private:
    std::vector<endpoint> list_;

public:
    void clear() {}

    void from_string(const std::string & str) {
        // TODO: from string
    }

    int add(const endpoint & end_point) {
        list_.push_back(end_point);
        return 0;
    }

    int parse(const std::string & info) {
        from_string(info);
        return 0;
    }

    int parse_list(const std::string & info_list) {
        from_string(info_list);
        return 0;
    }
};

static rpc::endpoint get_tcp_endpoint(const std::string & endpoint_info) {
    rpc::endpoint endpoint;
    endpoint.parse(endpoint_info);
    return endpoint;
}

static rpc::endpoint_list get_tcp_endpoints(const std::string & endpoint_list) {
    rpc::endpoint_list endpoints;
    endpoints.parse_list(endpoint_list);
    return endpoints;
}

} // namespace rpc

struct error_code
{
    //
};

struct status
{
    enum status_t {
        unknown,
        succeed,
        failed,
        timeout,
        queue_disconnect,
        consumer_disconnect,
        exists
    };

    status_t code;

    status() : code(status_t::unknown) {}
    status(status_t status_code) : code(status_code) {}
    status(int status_code) : code(static_cast<status_t>(status_code)) {}
    ~status() {}

    bool operator == (const status & _status) {
        return (code == _status.code);
    }

    bool operator == (status_t status_code) {
        return (code == status_code);
    }

    bool operator == (int status_code) {
        return (code == static_cast<status_t>(status_code));
    }

    bool operator != (const status & _status) {
        return (code != _status.code);
    }

    bool operator != (status_t status_code) {
        return (code != status_code);
    }

    bool operator != (int status_code) {
        return (code != static_cast<status_t>(status_code));
    }
};

bool operator == (const status & a, const status & b) {
    return (a.code == b.code);
}

bool operator != (const status & a, const status & b) {
    return (a.code != b.code);
}

template <typename T>
struct message {
    std::string content_;

    message() {}
    message(const char * content) : content_(content) {}
    message(const std::string & content) : content_(content) {}

    std::string content() const {
        return content_;
    }

    std::string get() const {
        return content_;
    }

    void set(const std::string & text) {
        content_ = text;
    }

    const char * to_string() const {
        return content_.c_str();
    }
};

class config
{
protected:
    rpc::endpoint_list host_;
    rpc::endpoint_list remote_;

public:
    config() {}
    ~config() {}

    void set_endpoint(const std::string & endpoint) {
        host_.from_string(endpoint);
    }    

    void set_endpoint(const rpc::endpoint & endpoint) {
        host_.add(endpoint);
    }

    void set_remote_endpoint(const rpc::endpoint & endpoint) {
        remote_.clear();
        remote_.add(endpoint);
    }

    void set_remote_endpoints(const rpc::endpoint_list & endpoint_list) {
        remote_ = endpoint_list;
    }
};

class producer_config : public config
{
public:
    producer_config() {}
    ~producer_config() {}
};

class producer
{
private:
    producer_config config_;

public:
    producer() {}
    ~producer() {}

    void set_config(const producer_config & config) {
        config_ = config;
    }

    template <typename U>
    timax::status send(const message<U> & msg) {
        timax::status result;
        return result;
    }

    template <typename U>
    timax::status push(const message<U> & msg) {
        timax::status result;
        return result;
    }
};

class consumer_config : public config
{
public:
    consumer_config() {}
    ~consumer_config() {}
};

template <typename T>
class consumer
{
public:
    typedef message<T> message_type;
    typedef std::function<void(const timax::consumer<T> &, const timax::status &)> accept_callback_type;
    typedef std::function<void(const timax::consumer<T> &, const message_type &, const timax::status &)> receive_callback_type;

private:
    consumer_config config_;
    accept_callback_type on_accept_;
    receive_callback_type on_receive_;

public:
    consumer() {}
    ~consumer() {}
    
    void set_config(const consumer_config & config) {
        config_ = config;
    }

     void set_on_accept(const accept_callback_type & on_accept) {
         on_accept_ = on_accept;
     }

    void set_on_receive(const receive_callback_type & on_receive) {
        on_receive_ = on_receive;
    }

    timax::status bind(const std::string & endpoint) {
        config_.set_endpoint(endpoint);
        return timax::status::succeed;
    }

    timax::status bind(const rpc::endpoint & endpoint) {
        config_.set_endpoint(endpoint);
        return timax::status::succeed;
    }

    timax::status listen() {
        return timax::status::succeed;
    }

    template <typename U>
    timax::status receive(timax::message<U> & msg) {
        timax::status result;
        return result;
    }

    template <typename U>
    timax::status pull(timax::message<U> & msg) {
        timax::status result;
        return result;
    }
};

class topic
{
private:
    std::string name_;

public:
    topic() {}
    topic(const char * name) : name_(name) {}
    topic(const std::string & name) : name_(name) {}
    topic(const topic & src) : name_(src.name_) {}
    ~topic() {}

    void set_name(const std::string & name) {
        name_ = name;
    }

    const char * to_string() const {
        return name_.c_str();
    }
};

class publisher_config : public config
{
public:
    publisher_config() {}
    ~publisher_config() {}
};

class publisher
{
private:
    publisher_config config_;
    timax::topic topic_;

public:
    publisher() {}
    ~publisher() {}

    void set_config(const publisher_config & config) {
        config_ = config;
    }

    timax::status register_topic(const timax::topic & topic) {
        timax::status result;
        topic_ = topic;
        return result;
    }

    template <typename U>
    timax::status publish(const timax::topic & topic, const timax::message<U> & msg) {
        timax::status result;
        return result;
    }
};

class subscriber_config : public config
{
public:
    subscriber_config() {}
    ~subscriber_config() {}
};

template <typename T>
class subscriber
{
public:
    typedef message<T> message_type;
    typedef std::function<void(const timax::subscriber<T> &, const timax::topic &,
                          const timax::status &)> accept_callback_type;
    typedef std::function<void(const timax::subscriber<T> &, const timax::topic &,
                          const message_type &, const timax::status &)> receive_callback_type;

private:
    subscriber_config config_;
    accept_callback_type on_accept_;
    receive_callback_type on_receive_;

public:
    subscriber() {}
    ~subscriber() {}

    void set_config(const subscriber_config & config) {
        config_ = config;
    }

     void set_on_accept(const accept_callback_type & on_accept) {
         on_accept_ = on_accept;
     }

    void set_on_receive(const receive_callback_type & on_receive) {
        on_receive_ = on_receive;
    }

    timax::status bind(const std::string & endpoint) {
        config_.set_endpoint(endpoint);
        return timax::status::succeed;
    }

    timax::status bind(const rpc::endpoint & endpoint) {
        config_.set_endpoint(endpoint);
        return timax::status::succeed;
    }

    timax::status listen() {
        return timax::status::succeed;
    }

    timax::status subscribe(const timax::topic & topic) {
        timax::status result;
        return result;
    }
};

class message_queue
{
private:
    rpc::endpoint endpoint_;

public:
    message_queue() {}
    ~message_queue() {}

    status bind(const std::string & endpoint) {
        endpoint_.set_endpoint(endpoint);
        return status::succeed;
    }

    status bind(const rpc::endpoint & endpoint) {
        endpoint_ = endpoint;
        return status::succeed;
    }

    status listen() {
        return status::succeed;
    }
};

class message_queue_cluster
{
private:
    rpc::endpoint endpoint_;
    std::vector<message_queue> nodes_;

public:
    message_queue_cluster() {}
    ~message_queue_cluster() {}

    void clear() {}

    std::size_t set_node(const std::string & info) {
        clear();
        return add_node(info);
    }

    std::size_t set_node(const message_queue & queue) {
        clear();
        return add_node(queue);
    }

    std::size_t set_node_list(const std::vector<message_queue> & queue_list) {
        clear();
        return add_node_list(queue_list);
    }
    
    std::size_t add_node(const std::string & info) {
        //
        return nodes_.size();
    }

    std::size_t add_node(const message_queue & queue) {
        nodes_.push_back(queue);
        return nodes_.size();
    }

    std::size_t add_node_list(const std::string & info) {
        //
        return nodes_.size();
    }

    std::size_t add_node_list(const std::vector<message_queue> & queue_list) {
        for (auto queue : queue_list) {
            nodes_.push_back(queue);
        }
        return nodes_.size();
    }

    status bind(const std::string & endpoint) {
        endpoint_.set_endpoint(endpoint);
        return status::succeed;
    }

    status bind(const rpc::endpoint & endpoint) {
        endpoint_ = endpoint;
        return status::succeed;
    }

    status listen() {
        return status::succeed;
    }
};

} // namespace timax

using namespace timax;

// 点对点模式－－1:1请求应答模式：消息队列示例代码
void p2p_mode_request_response_queue_demo()
{
    timax::message_queue_cluster queue_cluster;

    auto queue_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");

    // 绑定IP和端口
    timax::status result = queue_cluster.bind(queue_endpoint);
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // 开始监听
    result = queue_cluster.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// 点对点模式－－1:1请求应答模式：生产者示例代码
void p2p_mode_request_response_producer_demo()
{
    timax::producer_config config;
    auto endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");
    config.set_remote_endpoint(endpoint);

    timax::producer producer;
    producer.set_config(config);

    timax::message<std::string> msg("This is a test message.");

    timax::status result = producer.send(msg);
    if (result == timax::status::succeed) {
        // 发送成功
        std::cout << "producer send succeed." << std::endl;
    }
    else {
        // 发送失败
        std::cout << "producer send failed." << std::endl;
    }
}

// 消费者接收消息的回调函数
void on_request_response_consumer_receive(const timax::consumer<std::string> & consumer,
                                          const timax::message<std::string> & msg,
                                          const timax::status & result)
{
    if (result == timax::status::succeed) {
        // 接收成功
        std::cout << "consumer receive succeed." << std::endl;
        std::cout << "message: " << msg.to_string() << std::endl;
    }
    else {
        // 接收失败
        std::cout << "consumer receive failed." << std::endl;
        std::cout << "message: " << msg.to_string() << std::endl;
    }
}

// 点对点模式－－1:1请求应答模式：消费者示例代码
void p2p_mode_request_response_consumer_demo()
{
    timax::consumer_config config;
    // 有些时候可能需要主动联系消息队列服务器, 所以要指定一下服务器的IP和端口.
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");
    config.set_remote_endpoint(remote_endpoint);

    timax::consumer<std::string> consumer;
    consumer.set_config(config);

    using namespace std::placeholders;
    timax::consumer<std::string>::receive_callback_type on_receive =
        std::bind(&on_request_response_consumer_receive, consumer, _2, _3);
    consumer.set_on_receive(on_receive);

    // 为 consumer 绑定IP和端口
    timax::status result = consumer.bind("192.168.3.200:7000");
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "consumer bind failed." << std::endl;
        return;
    }

    // consumer 开始监听
    result = consumer.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "consumer listen failed." << std::endl;
        return;
    }

    // 监听成功后, 接收到消息会调用上面的回调函数.
}

// 点对点模式－－1:1管道push-pull模式：消息队列示例代码
void p2p_mode_push_pull_queue_demo()
{
    timax::message_queue_cluster queue_cluster;

    auto queue_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");

    // 绑定IP和端口
    timax::status result = queue_cluster.bind(queue_endpoint);
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // 开始监听
    result = queue_cluster.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// 点对点模式－－1:1管道push-pull模式：生产者示例代码
void p2p_mode_push_pull_producer_demo()
{
    timax::producer_config config;
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");
    config.set_remote_endpoint(remote_endpoint);

    timax::producer producer;
    producer.set_config(config);

    timax::message<std::string> msg("This is a test message.");

    timax::status result = producer.push(msg);
    if (result == timax::status::succeed) {
        // push成功, 也可使用异步push的方式, 这里使用的是同步的方式.
        std::cout << "producer push succeed." << std::endl;
    }
    else {
        // push失败
        std::cout << "producer push failed." << std::endl;
    }
}

// 点对点模式－－1:1管道push-pull模式：消费者示例代码
void p2p_mode_push_pull_consumer_demo()
{
    timax::consumer_config config;
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");
    config.set_remote_endpoint(remote_endpoint);

    timax::consumer<std::string> consumer;
    consumer.set_config(config);

    timax::message<std::string> msg;
    timax::status result = consumer.pull(msg);
    if (result == timax::status::succeed) {
        // pull成功, 成功后的应答是consumer自动完成的, 无须手动回应.
        std::cout << "consumer pull message: " << msg.to_string() << std::endl;
    }
    else {
        // pull失败
        std::cout << "consumer pull failed." << std::endl;
    }
}

// 发布－订阅模式－－标准模式：消息队列集群示例代码
void pub_sub_mode_standard_queue_demo()
{
    timax::message_queue queue;

    // 绑定queue的IP和端口
    timax::status result = queue.bind("192.168.3.180:6000");
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "message queue bind failed." << std::endl;
        return;
    }

    // 开始监听queue
    result = queue.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "message queue listen failed." << std::endl;
        return;
    }

    timax::message_queue_cluster cluster;

    // 把 queue 添加到 cluster 里, 作为其一个子节点
    cluster.add_node_list("192.168.3.180:6000|192.168.3.181:6000|192.168.3.182:6000");

    // 绑定cluster的IP和端口
    result = cluster.bind("192.168.3.180:5000");
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // 开始监听cluster
    result = cluster.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// 发布－订阅模式－－标准模式：发布者示例代码
void pub_sub_mode_standard_publisher_demo()
{
    timax::publisher_config config;
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:5000");
    config.set_remote_endpoint(remote_endpoint);

    timax::publisher publisher;
    publisher.set_config(config);

    timax::topic topic("Test Topic");
    timax::message<std::string> msg("This is a test message.");

    timax::status result = publisher.register_topic(topic);
    if (result != timax::status::succeed && result != timax::status::exists) {
        // 注册topic失败, 退出.
        std::cout << "publisher regiuster topic failed." << std::endl;
        return;
    }

    result = publisher.publish(topic, msg);
    if (result == timax::status::succeed) {
        // publish成功, 也可使用异步publish的方式, 这里使用的是同步的方式.
        std::cout << "publisher publish succeed." << std::endl;
    }
    else {
        // publish失败
        std::cout << "publisher publish failed." << std::endl;
    }
}

// 订阅者接收订阅消息的回调函数
void on_standard_subsciber_receive(const timax::subscriber<std::string> & subscriber,
                                   const timax::topic & topic,
                                   const timax::message<std::string> & msg,
                                   const timax::status & result) {
    if (result == timax::status::succeed) {
        // 接收成功
        std::cout << "subsciber receive succeed." << std::endl;
    }
    else {
        // 接收失败
        std::cout << "subsciber receive failed." << std::endl;
    }

    std::cout << "topic: " << topic.to_string() << std::endl;
    std::cout << "message: " << msg.to_string() << std::endl;
}

// 发布－订阅模式－－标准模式：订阅者示例代码
void pub_sub_mode_standard_subsciber_demo()
{
    timax::subscriber_config config;
    // 有些时候可能需要主动联系消息队列集群, 所以要指定一下集群服务器的IP和端口.
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:5000");
    config.set_remote_endpoint(remote_endpoint);

    timax::subscriber<std::string> subscriber;
    subscriber.set_config(config);

    timax::topic topic("Test Topic");

    using namespace std::placeholders;
    timax::subscriber<std::string>::receive_callback_type on_receive =
        std::bind(&on_standard_subsciber_receive, subscriber, _2, _3, _4);
    subscriber.set_on_receive(on_receive);

    // 为 subscriber 绑定IP和端口
    timax::status result = subscriber.bind("192.168.3.200:7000");
    if (result != timax::status::succeed) {
        // 绑定失败
        std::cout << "subscriber bind failed." << std::endl;
        return;
    }

    // subscriber 开始监听
    result = subscriber.listen();
    if (result != timax::status::succeed) {
        // 监听失败
        std::cout << "subscriber listen failed." << std::endl;
        return;
    }

    // 订阅 Topic
    result = subscriber.subscribe(topic);
    if (result != timax::status::succeed) {
        // 订阅失败
        std::cout << "subscriber subscribe failed. topic = " << topic.to_string() << std::endl;
        return;
    }

    // 订阅成功后, 接收到订阅消息会调用上面的回调函数.
}

// 发布－订阅模式－－消费者分组模式：消息队列集群示例代码
void pub_sub_mode_consumer_group_queue_demo()
{
    //
}

int main(int argn, char * argv[])
{
    p2p_mode_request_response_queue_demo();
    p2p_mode_request_response_producer_demo();
    p2p_mode_request_response_consumer_demo();

    p2p_mode_push_pull_queue_demo();
    p2p_mode_push_pull_producer_demo();
    p2p_mode_push_pull_consumer_demo();

    pub_sub_mode_standard_queue_demo();
    pub_sub_mode_standard_publisher_demo();
    pub_sub_mode_standard_subsciber_demo();

    pub_sub_mode_consumer_group_queue_demo();

    printf("http_parser_unittest\n\n");
    return 0;
}
