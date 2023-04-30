#pragma once
#include <includes.h>
#include "saomega.h"

using namespace SA;
namespace SA {
    class DB {
    private:
        daotk::mysql::connection db_;
    public:

        bool open() {
            daotk::mysql::connect_options options;
            options.server = config["db"]["server"];
            options.username = config["db"]["user"];
            options.password = config["db"]["pass"];
            options.dbname = config["db"]["db"];
            options.autoreconnect = true;
            options.timeout = 30;
            options.port = config["db"]["port"];

            bool result = db_.open(options);
            if (!result)
            {
                Log::GetLog()->critical("Failed to open database connection check your settings!");
            }
        }


    };
}