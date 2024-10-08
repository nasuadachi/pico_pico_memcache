
#include <unordered_map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <algorithm>
#include <sstream>
#include <memory>
#include <iostream>

#define PORT 11211  // memcached のデフォルトポート
#define MAX_CLIENTS 10  // 同時接続数の上限
#define BUFFER_SIZE 1024  // 一度の命令あたりのバッファサイズ。これを超えたらエラー
#define MAX_STORE_SIZE 100  // keyの数の上限

// std::shared_ptr を使用して値を管理
std::list<std::string> key_list;
// キーと値を管理するためのマップ
std::unordered_map<std::string, std::shared_ptr<std::string>> store;

/**
    getとsetしかないmemcachedのシンプルな実装。
    しかも一度コマンドを送ったら接続を切る。連続してコマンドは受け取らない(これは意図的)
		MIT License nasu-K.Adachi
 */

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size = sizeof(client_address);
    char buffer[BUFFER_SIZE];
    int valread;

    // ソケットを作成
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        std::cerr << "socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // アドレス構造体を設定
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // ソケットにアドレスをバインド
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 接続を listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cerr << "Key-Value store listening on port " << PORT << std::endl;

    while (1) {
        // クライアントからの接続を受け入れる
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket < 0) {
            std::cerr << "accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        // クライアントからのデータを読み込む
        valread = read(client_socket, buffer, BUFFER_SIZE - 1); // NULL終端のためのスペースを残す
        buffer[valread] = '\0'; // 無理やり最後にNULLを入れてstringをぶった斬る
        if (valread >= BUFFER_SIZE - 1) {
            std::cerr << "Received command is too long" << std::endl;
            close(client_socket);   // 長すぎるコマンドは受け付けない
            continue;
        }


        // コマンドを解析
        std::stringstream ss(buffer);
        std::string command, key, value;
        ss >> command >> key >> value;

        // コマンドを実行
        if (command == "get") {
            auto it = store.find(key);
            if (it != store.end()) {
                send(client_socket, it->second->c_str(), it->second->length(), 0);
            } else {
                // キーが見つからない場合の処理は特になくて、そのまま終了
            }
       
        } else if (command == "set") {
            if (store.size() >= MAX_STORE_SIZE) {
                // 改行コードを変換
                size_t pos = value.find("\r\n");
                while (pos != std::string::npos) {
                    value.replace(pos, 2, "\n");
                    pos = value.find("\r\n", pos + 1);
                }
                std::string oldest_key = key_list.front();
                key_list.pop_front();
                store.erase(oldest_key);
            }

            // shared_ptr を使用して値を格納
            store[key] = std::make_shared<std::string>(value);
            key_list.push_back(key);
            send(client_socket, "OK", 2, 0);
        }

        // クライアントとの接続を毎回閉じる
        close(client_socket);
    }

    return 0; // ここに来ることはない
}
