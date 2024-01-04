#pragma once

#include "rins.pb.h"
#include <mysql/mysql.h>

class store_report {
  public:
    store_report();
    void store(rins::ReportStatusRequest req);

  private:
    MYSQL _db_conn;
};