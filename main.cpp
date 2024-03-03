#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

// Function to split a string into vector of strings based on delimiter
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    std::string configData = "user       www www;  ## Default: nobody\n"
        "worker_processes  5;  ## Default: 1\n"
        "error_log  logs/error.log;\n"
        "pid        logs/nginx.pid;\n"
        "worker_rlimit_nofile 8192;\n"
        "\n"
        "events {\n"
        "  worker_connections  4096;  ## Default: 1024\n"
        "}\n"
        "\n"
        "http {\n"
        "  include    conf/mime.types;\n"
        "  include    /etc/nginx/proxy.conf;\n"
        "  include    /etc/nginx/fastcgi.conf;\n"
        "  index    index.html index.htm index.php;\n"
        "\n"
        "  default_type application/octet-stream;\n"
        "  log_format   main '$remote_addr - $remote_user [$time_local]  $status '\n"
        "    '\"$request\" $body_bytes_sent \"$http_referer\" '\n"
        "    '\"$http_user_agent\" \"$http_x_forwarded_for\"';\n"
        "  access_log   logs/access.log  main;\n"
        "  sendfile     on;\n"
        "  tcp_nopush   on;\n"
        "  server_names_hash_bucket_size 128; # this seems to be required for some vhosts\n"
        "\n"
        "  server { # php/fastcgi\n"
        "    listen       80;\n"
        "    server_name  domain1.com www.domain1.com;\n"
        "    access_log   logs/domain1.access.log  main;\n"
        "    root         html;\n"
        "\n"
        "    location ~ \\.php$ {\n"
        "      fastcgi_pass   127.0.0.1:1025;\n"
        "    }\n"
        "  }\n"
        "\n"
        "  server { # simple reverse-proxy\n"
        "    listen       80;\n"
        "    server_name  domain2.com www.domain2.com;\n"
        "    access_log   logs/domain2.access.log  main;\n"
        "\n"
        "    # serve static files\n"
        "    location ~ ^/(images|javascript|js|css|flash|media|static)/  {\n"
        "      root    /var/www/virtual/big.server.com/htdocs;\n"
        "      expires 30d;\n"
        "    }\n"
        "\n"
        "    # pass requests for dynamic content to rails/turbogears/zope, et al\n"
        "    location / {\n"
        "      proxy_pass      http://127.0.0.1:8080;\n"
        "    }\n"
        "  }\n"
        "\n"
        "  upstream big_server_com {\n"
        "    server 127.0.0.3:8000 weight=5;\n"
        "    server 127.0.0.3:8001 weight=5;\n"
        "    server 192.168.0.1:8000;\n"
        "    server 192.168.0.1:8001;\n"
        "  }\n"
        "\n"
        "  server { # simple load balancing\n"
        "    listen          80;\n"
        "    server_name     big.server.com;\n"
        "    access_log      logs/big.server.access.log main;\n"
        "\n"
        "    location / {\n"
        "      proxy_pass      http://big_server_com;\n"
        "    }\n"
        "  }\n"
        "}\n";

    std::vector<std::string> lines = split(configData, '\n');
    std::vector<std::map<std::string, std::string>> keyValues;

    std::string currentSection = "";
    for (const auto &line : lines) {
        // Remove leading and trailing whitespaces
        std::string trimmedLine = line;
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t\n\r\f\v"));
        trimmedLine.erase(trimmedLine.find_last_not_of(" \t\n\r\f\v") + 1);

        if (trimmedLine.empty() || trimmedLine.substr(0, 2) == "##") {
            continue;  // Skip empty lines and comments
        }

        // Check for start of a new section
        if (trimmedLine.back() == '{') {
            currentSection = trimmedLine.substr(0, trimmedLine.size() - 1);
        }
        // Check for end of a section
        else if (trimmedLine == "}") {
            size_t lastDot = currentSection.find_last_of('.');
            if (lastDot != std::string::npos) {
                currentSection = currentSection.substr(0, lastDot);
            } else {
                currentSection = "";
            }
        }
        // Regular key-value pair
        else {
            std::vector<std::string> tokens = split(trimmedLine, ' ');
            if (tokens.size() >= 2) {
                std::string key = currentSection.empty() ? tokens[0] : (currentSection + "." + tokens[0]);
                std::string value = tokens[1];
                std::map<std::string, std::string> kvPair = {{key, value}};
                keyValues.push_back(kvPair);
            }
        }
    }

    // Print the result
    for (const auto &kvMap : keyValues) {
        for (const auto &kv : kvMap) {
            std::cout << "Key: " << kv.first << ", Value: " << kv.second << std::endl;
        }
    }

    return 0;
}
