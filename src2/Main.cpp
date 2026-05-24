#include <iostream>
#include "HaloEngine.h"
#include "httplib.h"

using namespace std;
using namespace httplib;

int main() {
    HaloEngine engine;
    cout << "      HALO CYBER ACCESS ENGINE - API SERVER      " << endl;
    
    cout << "[SYSTEM] loading data..." << endl;
    engine.loadData("data/halo_dataset_1_5m.csv"); 
    
    if(engine.pool.total_logs == 0) {
        cout << "[ERROR] cannot load data" << endl;
        return 1;
    }

    Server svr;

    // API 1: Top 10 Resources
    svr.Get("/api/top", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*"); 
        try {
            long long start = 0, end = 9999999999;
            if (req.has_param("start") && !req.get_param_value("start").empty()) 
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty()) 
                end = stoll(req.get_param_value("end"));
            
            string json_result = engine.getTop10ResourcesByTimeJson(start, end);
            res.set_content(json_result, "application/json");
        } catch (const exception& e) {
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error in Top10\"}", "application/json");
        }
    });

    // API 2: User Journey
    svr.Get("/api/user", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            long long start = 0, end = 9999999999;
            string id = "";
            
            if (req.has_param("id")) id = req.get_param_value("id");
            if (req.has_param("start") && !req.get_param_value("start").empty()) 
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty()) 
                end = stoll(req.get_param_value("end"));
            
            if (id.empty()) {
                res.set_content("[]", "application/json");
                return;
            }
            
            string json_result = engine.getUserJourneyJson(id, start, end);
            res.set_content(json_result, "application/json");
        } catch (const exception& e) {
            res.status = 500;
            res.set_content("[]", "application/json");
        }
    });

    // API 3: Resource Tracking
    svr.Get("/api/resource", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            long long start = 0, end = 9999999999;
            string id = "";
            
            if (req.has_param("id")) id = req.get_param_value("id");
            if (req.has_param("start") && !req.get_param_value("start").empty()) 
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty()) 
                end = stoll(req.get_param_value("end"));
            
            if (id.empty()) {
                res.set_content("[]", "application/json");
                return;
            }
            
            string json_result = engine.getResourceJourneyJson(id, start, end);
            res.set_content(json_result, "application/json");
        } catch (const exception& e) {
            res.status = 500;
            res.set_content("[]", "application/json");
        }
    });

    cout << "[SYSTEM] System is ready!" << endl;
    cout << "[NETWORK] API Server is running at: http://localhost:8080" << endl;
    
    svr.listen("0.0.0.0", 8080);
    return 0;
}