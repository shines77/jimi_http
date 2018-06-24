
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

// ��Ե�ģʽ����1:1����Ӧ��ģʽ����Ϣ����ʾ������
void p2p_mode_request_response_queue_demo()
{
    timax::message_queue_cluster queue_cluster;

    auto queue_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");

    // ��IP�Ͷ˿�
    timax::status result = queue_cluster.bind(queue_endpoint);
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // ��ʼ����
    result = queue_cluster.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// ��Ե�ģʽ����1:1����Ӧ��ģʽ��������ʾ������
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
        // ���ͳɹ�
        std::cout << "producer send succeed." << std::endl;
    }
    else {
        // ����ʧ��
        std::cout << "producer send failed." << std::endl;
    }
}

// �����߽�����Ϣ�Ļص�����
void on_request_response_consumer_receive(const timax::consumer<std::string> & consumer,
                                          const timax::message<std::string> & msg,
                                          const timax::status & result)
{
    if (result == timax::status::succeed) {
        // ���ճɹ�
        std::cout << "consumer receive succeed." << std::endl;
        std::cout << "message: " << msg.to_string() << std::endl;
    }
    else {
        // ����ʧ��
        std::cout << "consumer receive failed." << std::endl;
        std::cout << "message: " << msg.to_string() << std::endl;
    }
}

// ��Ե�ģʽ����1:1����Ӧ��ģʽ��������ʾ������
void p2p_mode_request_response_consumer_demo()
{
    timax::consumer_config config;
    // ��Щʱ�������Ҫ������ϵ��Ϣ���з�����, ����Ҫָ��һ�·�������IP�Ͷ˿�.
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");
    config.set_remote_endpoint(remote_endpoint);

    timax::consumer<std::string> consumer;
    consumer.set_config(config);

    using namespace std::placeholders;
    timax::consumer<std::string>::receive_callback_type on_receive =
        std::bind(&on_request_response_consumer_receive, consumer, _2, _3);
    consumer.set_on_receive(on_receive);

    // Ϊ consumer ��IP�Ͷ˿�
    timax::status result = consumer.bind("192.168.3.200:7000");
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "consumer bind failed." << std::endl;
        return;
    }

    // consumer ��ʼ����
    result = consumer.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "consumer listen failed." << std::endl;
        return;
    }

    // �����ɹ���, ���յ���Ϣ���������Ļص�����.
}

// ��Ե�ģʽ����1:1�ܵ�push-pullģʽ����Ϣ����ʾ������
void p2p_mode_push_pull_queue_demo()
{
    timax::message_queue_cluster queue_cluster;

    auto queue_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:6000");

    // ��IP�Ͷ˿�
    timax::status result = queue_cluster.bind(queue_endpoint);
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // ��ʼ����
    result = queue_cluster.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// ��Ե�ģʽ����1:1�ܵ�push-pullģʽ��������ʾ������
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
        // push�ɹ�, Ҳ��ʹ���첽push�ķ�ʽ, ����ʹ�õ���ͬ���ķ�ʽ.
        std::cout << "producer push succeed." << std::endl;
    }
    else {
        // pushʧ��
        std::cout << "producer push failed." << std::endl;
    }
}

// ��Ե�ģʽ����1:1�ܵ�push-pullģʽ��������ʾ������
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
        // pull�ɹ�, �ɹ����Ӧ����consumer�Զ���ɵ�, �����ֶ���Ӧ.
        std::cout << "consumer pull message: " << msg.to_string() << std::endl;
    }
    else {
        // pullʧ��
        std::cout << "consumer pull failed." << std::endl;
    }
}

// ����������ģʽ������׼ģʽ����Ϣ���м�Ⱥʾ������
void pub_sub_mode_standard_queue_demo()
{
    timax::message_queue queue;

    // ��queue��IP�Ͷ˿�
    timax::status result = queue.bind("192.168.3.180:6000");
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "message queue bind failed." << std::endl;
        return;
    }

    // ��ʼ����queue
    result = queue.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "message queue listen failed." << std::endl;
        return;
    }

    timax::message_queue_cluster cluster;

    // �� queue ��ӵ� cluster ��, ��Ϊ��һ���ӽڵ�
    cluster.add_node_list("192.168.3.180:6000|192.168.3.181:6000|192.168.3.182:6000");

    // ��cluster��IP�Ͷ˿�
    result = cluster.bind("192.168.3.180:5000");
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "message queue cluster bind failed." << std::endl;
        return;
    }

    // ��ʼ����cluster
    result = cluster.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "message queue cluster listen failed." << std::endl;
        return;
    }
}

// ����������ģʽ������׼ģʽ��������ʾ������
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
        // ע��topicʧ��, �˳�.
        std::cout << "publisher regiuster topic failed." << std::endl;
        return;
    }

    result = publisher.publish(topic, msg);
    if (result == timax::status::succeed) {
        // publish�ɹ�, Ҳ��ʹ���첽publish�ķ�ʽ, ����ʹ�õ���ͬ���ķ�ʽ.
        std::cout << "publisher publish succeed." << std::endl;
    }
    else {
        // publishʧ��
        std::cout << "publisher publish failed." << std::endl;
    }
}

// �����߽��ն�����Ϣ�Ļص�����
void on_standard_subsciber_receive(const timax::subscriber<std::string> & subscriber,
                                   const timax::topic & topic,
                                   const timax::message<std::string> & msg,
                                   const timax::status & result) {
    if (result == timax::status::succeed) {
        // ���ճɹ�
        std::cout << "subsciber receive succeed." << std::endl;
    }
    else {
        // ����ʧ��
        std::cout << "subsciber receive failed." << std::endl;
    }

    std::cout << "topic: " << topic.to_string() << std::endl;
    std::cout << "message: " << msg.to_string() << std::endl;
}

// ����������ģʽ������׼ģʽ��������ʾ������
void pub_sub_mode_standard_subsciber_demo()
{
    timax::subscriber_config config;
    // ��Щʱ�������Ҫ������ϵ��Ϣ���м�Ⱥ, ����Ҫָ��һ�¼�Ⱥ��������IP�Ͷ˿�.
    auto remote_endpoint = timax::rpc::get_tcp_endpoint("192.168.3.180:5000");
    config.set_remote_endpoint(remote_endpoint);

    timax::subscriber<std::string> subscriber;
    subscriber.set_config(config);

    timax::topic topic("Test Topic");

    using namespace std::placeholders;
    timax::subscriber<std::string>::receive_callback_type on_receive =
        std::bind(&on_standard_subsciber_receive, subscriber, _2, _3, _4);
    subscriber.set_on_receive(on_receive);

    // Ϊ subscriber ��IP�Ͷ˿�
    timax::status result = subscriber.bind("192.168.3.200:7000");
    if (result != timax::status::succeed) {
        // ��ʧ��
        std::cout << "subscriber bind failed." << std::endl;
        return;
    }

    // subscriber ��ʼ����
    result = subscriber.listen();
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "subscriber listen failed." << std::endl;
        return;
    }

    // ���� Topic
    result = subscriber.subscribe(topic);
    if (result != timax::status::succeed) {
        // ����ʧ��
        std::cout << "subscriber subscribe failed. topic = " << topic.to_string() << std::endl;
        return;
    }

    // ���ĳɹ���, ���յ�������Ϣ���������Ļص�����.
}

// ����������ģʽ���������߷���ģʽ����Ϣ���м�Ⱥʾ������
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
