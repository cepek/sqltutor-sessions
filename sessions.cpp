#include <pqxx/pqxx>
#include "sessions.h"
#include <algorithm>
#include <utility>
#include <iostream>

using std::string;
using std::cout;
using pqxx::result;

Sessions::Sessions(std::vector<int>& s, string conpar)
{
    session_ids = std::move(s);
    db_connection = conpar;
}

void Sessions::exec()
{
    std::sort(session_ids.begin(), session_ids.end());
    pqxx::connection connection(db_connection);
    pqxx::work tran {connection};
    tran.exec("SET search_path TO sqltutor");

    for (unsigned i=0; i<session_ids.size(); i++)
    {
        string sid = std::to_string(session_ids[i]);

        result resid = tran.exec (
            "SELECT cast(stop-start AS interval(0)) AS time, "
            "       help, login, start "
            "FROM   sessions "
            "WHERE  session_id = " + sid + " "
        );
        if (resid.empty()) {
            std::cerr << "Session id " << sid << " doesn't exist. Ignored.\n";
            continue;
        }


        {
            auto s = resid.begin();


            result r = tran.exec("SELECT * FROM evaluation(" + sid + ")");
            if (r.empty()) {
                continue;
            }
            auto b = r.begin();
            cout << "\n"
                << "Session id          : " << sid
                << "      "                 << s["start"].as(string()) << "\n"
                << "\n"
                << "Login               : " << s["login"].as(string()) << "\n"
                << "Time                : " << s["time"].as(string()) << "\n"
                << "Help                : " << s["help"].as(int()) << "\n"
                << "\n"
                << "Number of questions : "	<< b["ev_total"].as(int()) << "\n"
                << "Correct answers	    : "	<< b["ev_correct"].as(int()) << "\n"
                << "Total points	    : "	<< b["ev_points"].as(int()) << "\n"
                << "Evaluation          : "	<< b["ev_evaluation"].as(int()) << "\n"
                << std::endl;
        }

        std:: string str_answers =
            "SELECT P.dataset_id, P.problem_id, points, correct, "
            "       answer AS user_answer, help, SQ.q_ord, SQ.language_id, "
            "       D.dataset, Q.question "
            "  FROM sessions AS S "
            "       JOIN sessions_questions AS SQ "
            "            ON S.session_id=SQ.session_id "
            "       JOIN problems AS P "
            "            ON SQ.dataset_id=P.dataset_id "
            "               AND SQ.problem_id=P.problem_id "
            "       JOIN datasets AS D"
            "            ON P.dataset_id=D.dataset_id "
            "       JOIN questions as Q "
            "            ON Q.dataset_id=SQ.dataset_id "
            "               AND Q.problem_id=SQ.problem_id "
            "               AND Q.q_ord=SQ.q_ord "
            "               AND Q.language_id=SQ.language_id "
            " WHERE S.session_id=" + sid + " "
            " ORDER BY time ASC ";
        result answers = tran.exec(str_answers);
        for (auto r=answers.begin(); r!=answers.end(); ++r)
        {
            string  dataset_id = r[0].as(string());
            string  problem_id = r[1].as(string());
            int         points = r[2].as(int());
            int        correct = r[3].as(int());
            string user_answer = r[4].as(string());
            // int        help = r[5].as(int());
            string       q_ord = r[6].as(string());
            string language_id = r[7].as(string());
            string     dataset = r[8].as(string());
            string    question = r[9].as(string());

            cout << "\n\n" << correct << "  "
                 << dataset << " " << problem_id << " (" << points << "):\n";

            {
                result res = tran.exec(
                    "SELECT ds_table, columns "
                    "  FROM datasets "
                    "       NATURAL JOIN dataset_tables "
                    " WHERE dataset_id = '" + dataset_id + "' "
                    " ORDER BY dt_ord; "
                    );
                for (auto i=res.begin(); i!=res.end(); ++i) {
                    cout << "   " << i[0].as(string()) << " : "
                              << i[1].as(string()) << "\n";
                }
            }

            auto strsp = [](string& s)
            {
                while (!s.empty() && std::isspace(s.front())) {
                    s.erase(0,1);
                }
                while (!s.empty() && std::isspace(s.back())) {
                    s.pop_back();
                }
                return s;
            };

            cout << "\n" << strsp(question) << "\n\n";

            strsp(user_answer);
            if (!correct) {
                if (user_answer.empty())
                   cout << "   missing answer";
                 else
                   cout << "   wrong answer\n\n";
            }

            cout << "" << user_answer << "\n";
            if (!correct) {
                result res = tran.exec
                    (
                    "SELECT answer "
                    "  FROM answers "
                    "       NATURAL JOIN "
                    "       problems "
                    " WHERE problem_id = " + problem_id +
                    "   AND dataset_id = " + dataset_id +
                    " ORDER BY priority ASC"
                    );
                    cout << "\n   correct answer\n";
                    for (auto b=res.begin(), e=res.end(); b!=e; b++)
                    {
                        string corr_answer = b[0].as(string());
                        cout << "\n" << strsp(corr_answer) << "\n";
                    }
            }

            cout << "\n";
        }
    }
}
