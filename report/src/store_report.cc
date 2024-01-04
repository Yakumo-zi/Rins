#include "store_report.h"
#include "config_file.h"
#include <mysql/mysql.h>

store_report::store_report() {
    mysql_library_init(0, nullptr, nullptr);

    mysql_init(&_db_conn);
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    std::string db_host =
        config_file::instance()->get_string("mysql", "db_host", "127.0.0.1");
    short db_port =
        config_file::instance()->get_number("mysql", "db_port", 3306);
    std::string db_user =
        config_file::instance()->get_string("mysql", "db_user", "root");
    std::string db_passwd =
        config_file::instance()->get_string("mysql", "db_passwd", "root");
    std::string db_name =
        config_file::instance()->get_string("mysql", "db_name", "rins");

    if (mysql_real_connect(&_db_conn, db_host.c_str(), db_user.c_str(),
                           db_passwd.c_str(), db_name.c_str(), db_port, NULL,
                           0) == NULL) {
        fprintf(stderr, "mysql real connect error\n");
        exit(1);
    }
}

void store_report::store(rins::ReportStatusRequest req) {
    for (int i = 0; i < req.results_size(); i++) {
        const rins::HostCallResult &result = req.results(i);
        int overload = result.overload() ? 1 : 0;
        char sql[1024];
        snprintf(
            sql, 1024,
            "INSERT INTO ServerCallStatus"
            "(modid, cmdid, ip, port, caller, succ_cnt, err_cnt, ts, overload) "
            "VALUES (%d, %d, %u, %u, %u, %u, %u, %u, %d) ON DUPLICATE KEY "
            "UPDATE succ_cnt = %u, err_cnt = %u, ts = %u, overload = %d",
            req.modid(), req.cmdid(), result.ip(), result.port(), req.caller(),
            result.succ(), result.err(), req.timestamp(), overload,
            result.succ(), result.err(), req.timestamp(), overload);

        mysql_ping(&_db_conn); // ping
                               // 测试一下，防止链接断开，会触发重新建立连接
        if (mysql_real_query(&_db_conn, sql, strlen(sql)) != 0) {
            fprintf(stderr, "Fial to Insert into ServerCallStatus %s\n",
                    mysql_error(&_db_conn));
        }
    }
}