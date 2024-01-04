#include "dns_route.h"
#include "rins_reactor.h"
#include "subscribe.h"
#include <bits/stdint-uintn.h>
#include <cstdio>
#include <cstdlib>
#include <mysql/mysql.h>
#include <pthread.h>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

route *route::_instance = nullptr;
pthread_once_t route::_once = PTHREAD_ONCE_INIT;

route::route() {
    pthread_rwlock_init(&_map_lock, nullptr);
    _data_map = new std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;
    _temp_map = new std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;

    connect_db();
    build_maps();
}

void route::connect_db() {
    std::string db_host =
        config_file::instance()->get_string("mysql", "db_host", "172.17.0.1");
    short db_port =
        config_file::instance()->get_number("mysql", "db_port", 13306);
    std::string db_user =
        config_file::instance()->get_string("mysql", "db_user", "root");
    std::string db_passwd =
        config_file::instance()->get_string("mysql", "db_passwd", "root");
    std::string db_name =
        config_file::instance()->get_string("mysql", "db_name", "rins");
    printf("dsn:%s %s %s %d\n", db_host.c_str(), db_user.c_str(),
           db_passwd.c_str(), db_port);

    mysql_init(&_db_conn);
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");

    if (!mysql_real_connect(&_db_conn, db_host.c_str(), db_user.c_str(),
                            db_passwd.c_str(), db_name.c_str(), db_port, NULL,
                            0)) {
        fprintf(stderr, "Failed to connect mysql\n");
        exit(1);
    }
}

void route::build_maps() {
    int ret = 0;
    sql = "SELECT * FROM RouteData;";
    ret = mysql_real_query(&_db_conn, sql.c_str(), sql.length());
    sql.clear();

    if (ret != 0) {
        fprintf(stderr, "failed to find any data, error %s\n",
                mysql_error(&_db_conn));
        exit(1);
    }

    MYSQL_RES *result = mysql_store_result(&_db_conn);

    long line_num = mysql_num_rows(result);
    MYSQL_ROW row;
    for (long i = 0; i < line_num; i++) {
        row = mysql_fetch_row(result);
        int modID = atoi(row[1]);
        int cmdID = atoi(row[2]);
        unsigned ip = atoi(row[3]);
        int port = atoi(row[4]);

        uint64_t key = ((uint64_t)modID << 32) + cmdID;
        uint64_t value = ((uint64_t)ip << 32) + port;
        printf("modID = %d, cmdID = %d, ip = %ud, port = %d\n", modID, cmdID,
               ip, port);
        (*_data_map)[key].insert(value);
    }
    mysql_free_result(result);
}

std::unordered_set<uint64_t> route::get_hosts(int modid, int cmdid) {
    std::unordered_set<uint64_t> hosts;
    uint64_t key = ((uint64_t)modid << 32) + cmdid;
    pthread_rwlock_rdlock(&_map_lock);
    auto it = _data_map->find(key);
    if (it != _data_map->end()) {
        hosts = it->second;
    }
    pthread_rwlock_unlock(&_map_lock);
    return hosts;
}

int route::load_version() {
    sql = "SELECT version from RouteVersion WHERE id = 1;";
    int ret = mysql_real_query(&_db_conn, sql.c_str(), sql.length());
    sql.clear();
    if (ret) {
        fprintf(stderr, "load version error: %s\n", mysql_error(&_db_conn));
        return -1;
    }
    MYSQL_RES *result = mysql_store_result(&_db_conn);
    if (!result) {
        fprintf(stderr, "mysql store result: %s\n", mysql_error(&_db_conn));
        return -1;
    }
    long line = mysql_num_rows(result);
    if (line == 0) {
        fprintf(stderr, "No version in table RouteVersion: %s\n",
                mysql_error(&_db_conn));
        return -1;
    }
    MYSQL_ROW row = mysql_fetch_row(result);
    long new_version = atol(row[0]);
    if (new_version == this->_version) {
        return 0;
    }
    this->_version = new_version;
    printf("now route version is %ld\n", this->_version);
    mysql_free_result(result);

    return 1;
}

int route::load_route_data() {
    _temp_map->clear();
    sql = "SELECT * FROM RouteData;";
    int ret = mysql_real_query(&_db_conn, sql.c_str(), sql.length());
    sql.clear();
    if (ret) {
        fprintf(stderr, "load data error: %s\n", mysql_error(&_db_conn));
        return -1;
    }
    MYSQL_RES *result = mysql_store_result(&_db_conn);
    if (!result) {
        fprintf(stderr, "load data mysql store result error: %s\n",
                mysql_error(&_db_conn));
        return -1;
    }

    long line = mysql_num_rows(result);
    if (line == 0) {
        fprintf(stderr, "No data in table RouteData: %s\n",
                mysql_error(&_db_conn));
        return -1;
        MYSQL_ROW row = mysql_fetch_row(result);
    }
    MYSQL_ROW row;
    for (long i = 0; i < line; ++i) {
        row = mysql_fetch_row(result);
        int modid = atoi(row[1]);
        int cmdid = atoi(row[2]);
        unsigned ip = atoi(row[3]);
        int port = atoi(row[4]);

        uint64_t key = ((uint64_t)modid << 32) + cmdid;
        uint64_t value = ((uint64_t)ip << 32) + port;

        (*_temp_map)[key].insert(value);
    }
    printf("load data to tmep succ! size is %lu\n", _temp_map->size());

    mysql_free_result(result);
    return 0;
}

void route::swap() {
    pthread_rwlock_wrlock(&_map_lock);
    auto *temp = _data_map;
    _data_map = _temp_map;
    _temp_map = temp;
    pthread_rwlock_unlock(&_map_lock);
}

void route::load_changes(std::vector<uint64_t> &change_list) {
    char _sql[1000];
    sprintf(_sql, "SELECT modid,cmdid FROM RouteChange WHERE version <= %ld",
            _version);
    sql = _sql;
    int ret = mysql_real_query(&_db_conn, sql.c_str(), sql.length());
    sql.clear();
    if (ret) {
        fprintf(stderr, "mysql_real_query: %s\n", mysql_error(&_db_conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(&_db_conn);
    if (!result) {
        fprintf(stderr, "load changes mysql_store_result %s\n",
                mysql_error(&_db_conn));
        return;
    }

    long lineNum = mysql_num_rows(result);
    if (lineNum == 0) {
        fprintf(stderr, "No version in table ChangeLog: %s\n",
                mysql_error(&_db_conn));
        return;
    }
    MYSQL_ROW row;
    for (long i = 0; i < lineNum; ++i) {
        row = mysql_fetch_row(result);
        int modid = atoi(row[0]);
        int cmdid = atoi(row[1]);
        uint64_t key = (((uint64_t)modid) << 32) + cmdid;
        change_list.push_back(key);
    }
    mysql_free_result(result);
}

void route::remove_changes(bool remove_all) {
    sql = "DELETE FROM RouteChange";
    if (remove_all == false) {
        char _sql[1000];
        sprintf(_sql, "DELETE FROM RouteChange WHERE version <= %ld", _version);
        sql = _sql;
    }
    int ret = mysql_real_query(&_db_conn, sql.c_str(), sql.length());
    if (ret != 0) {
        fprintf(stderr, "delete RouteChange: %s\n", mysql_error(&_db_conn));
        return;
    }

    return;
}
