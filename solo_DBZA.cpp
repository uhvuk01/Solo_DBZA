#include <iostream>
#include <string>
#include <mariadb/conncpp.hpp>
#include <vector>
#include <fstream>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <cstdlib>

class DB{
public:
    sql::Connection* conn;
    DB()
    {
        try{
            sql::Driver* driver= sql::mariadb::get_driver_instance();
            sql::SQLString url=("jdbc:mariadb://10.10.21.122/studyakm");        
            sql::Properties properties({ {"user", "JDJ"}, {"password", "1234"} }); 
            conn = driver->connect(url, properties);
        }
        catch(sql::SQLException& e){
            std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
        }
    }
 
    ~DB(){delete conn;}
};

class Resort{     //숙박업소 (지역) 1번
    DB& Resort_DB;
public:
    Resort(DB& Handler_DB):Resort_DB(Handler_DB){}
    void showResort(std::string region) 
    {
        try{
            std::cout<<"다음은 "<<region<<"에 위치한 숙박업소 정보입니다."<<std::endl<<std::endl;
            sleep(1);
            std::unique_ptr < sql::Statement > stmnt(Resort_DB.conn -> createStatement());
            sql::ResultSet *res =stmnt->executeQuery("SELECT * FROM JOINRESORT WHERE REGION ='"+region+"'");
            while (res->next())
            {
                std::cout<<"업체 명: "<<res->getString(7)<<std::endl;
                std::cout<<"\t 현재 영업 상태: "<<res->getString(3);
                std::cout<<",\t 주소: "<<res->getString(4);
                std::cout<<",\t\t 도로명주소: "<<res->getString(5)<<std::endl<<std::endl;
            }

        }
        catch(sql::SQLException& e){
            std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
        }
    }
};

class Subway_avg{ //지하철역 혼잡도 함수 (지역) 2번
    DB& Subway_avg_DB;
public:
    Subway_avg(DB& Handler_DB):Subway_avg_DB(Handler_DB){}
    void subway_avg(std::string region) 
        {
            try
            {
                std::cout<<"탑승하실 지하철 호선을 입력해주세요 (ex. 1호선): ";
                std::string train;
                getline(std::cin,train);
                std::unique_ptr<sql::PreparedStatement> stmnt(Subway_avg_DB.conn->prepareStatement( "SELECT LINE," 
                "(CASE WHEN AVG(TIME9TO12) >300000 THEN '매우혼잡' WHEN AVG(TIME9TO12) >100000 THEN '보통' ELSE '원활' END)TIME9TO12,(CASE WHEN AVG(TIME13TO16) >300000 THEN '매우혼잡' WHEN AVG(TIME9TO12) >100000 THEN '보통' ELSE '원활'END)"
                "TIME12TO16,(CASE WHEN AVG(TIME17TO20) >300000 THEN '매우혼잡' WHEN AVG(TIME9TO12) >100000 THEN '보통' ELSE '원활' END)TIME17TO20 FROM SUBWAY WHERE REGION =? AND LINE = ?;") );
                stmnt->setString(1, region);
                stmnt->setString(2, train);
                sql::ResultSet *res = stmnt->executeQuery();
                while (res->next())
                {
                    std::cout << res->getString(1) << std::endl;
                    std::cout << "09시~12시 교통혼잡도 : " << res->getString(2) << std::endl;
                    std::cout << "13시~16시 교통혼잡도: " << res->getString(3) << std::endl;
                    std::cout << "17시~20시 교통혼잡도: " << res->getString(4) << std::endl;
                }
                std::cout << std::endl;
            }
            catch(sql::SQLException& e)
            {
                std::cerr << "Error inserting new task: " << e.what() << std::endl;
            }
        }
};

class ShowSubway{ //지역구 별 탈수있는 지하철호선과 역 이름  (날짜,지역) 3번
    DB& ShowSubway_DB;
public:
    ShowSubway(DB& Handler_DB):ShowSubway_DB(Handler_DB){}
    void showSubWay(std::string mydate,std::string myregion) {
        try {
            // createStaemet 객체 생성
            std::cout<<"다음은 선택하신 지역에서 탑승가능한 호선입니다.\n";
            std::unique_ptr<sql::PreparedStatement> pre_stmnt(ShowSubway_DB.conn->prepareStatement("select line from SUBWAY where region = ? group by line"));
            pre_stmnt->setString(1, myregion);
            sql::ResultSet *pre_stmnt_res = pre_stmnt->executeQuery();
            while (pre_stmnt_res->next())
            {
                std::cout << pre_stmnt_res->getString(1) << "\t\t";
            }
            std::unique_ptr<sql::Statement> ex_sway(ShowSubway_DB.conn->createStatement());

            std::cout << std::endl;
            std::string zp[2];
            std::cout<<"\n세부노선을 열람하시겠습니까? (y or n)\n";
            getline(std::cin,zp[0]);

            if(zp[0]=="y" || zp[0]=="Y"){
                sql::ResultSet *tes=ex_sway->executeQuery("SELECT DISTINCT LINE,STATION FROM SUBWAY WHERE REGION= '"+myregion+"'");
                while (tes->next())
                {
                    std::cout<<"호선: "<<tes->getString(1);
                    std::cout<<", 역이름: "<<tes->getString(2)<<std::endl;
                }
                std::cout<<"\n 혹시 찾는 역이 없습니까? y or n\n";
                getline(std::cin,zp[1]);
                
                if(zp[1]=="y" || zp[1]=="Y")
                {
                    std::string pp="NO%";
                    tes=ex_sway->executeQuery("SELECT LINE,STATION FROM SUBWAY WHERE REGION LIKE '데이터%' GROUP BY STATION");
                    
                    if(tes->rowsCount()==0)
                        std::cout<<"쿼리문제\n";
                    while (tes->next())
                    {

                    std::cout<<"호선: "<<tes->getString(1);
                    std::cout<<", 역이름: "<<tes->getString(2)<<std::endl;
                    }
                }
            
                
                std::unique_ptr<sql::Statement> stmnt(ShowSubway_DB.conn->createStatement());
                // 쿼리를 실행
                mydate=mydate.substr(0,mydate.find('-'));
                std::string mystation;
                std::cout<<"지하철역 이름을 입력하세요 ex)도림천\n";
                getline(std::cin,mystation);
                sql::SQLString aa="SELECT DISTINCT LINE FROM SUBWAY WHERE STATION = '"+ mystation+"'";
                sql::ResultSet *stmnt_res = stmnt->executeQuery(aa);
                std::string myline;
                int rowcount=stmnt_res->rowsCount();
                while (stmnt_res->next())
                {
                    std::cout << "호선 이름 = " << stmnt_res->getString(1)<<std::endl;
                    if(rowcount==1)
                    {
                        myline=stmnt_res->getString(1);
                    }
                }
                
                if(rowcount !=1)
                {
                    std::cout<<"몇 호선을 타시겠습니까? ";
                    getline(std::cin,myline);          
                }        
            
                sql::SQLString date = "SELECT LINE,STATION,(CASE WHEN AVG(TIME9TO12) >300000 THEN '매우혼잡' WHEN AVG(TIME9TO12) >100000 THEN '보통' ELSE '원활' END) AS '9~12시 탑승객 평균',"
                "(CASE WHEN AVG(TIME13TO16) >300000 THEN '매우혼잡' WHEN AVG(TIME13TO16) >100000 THEN '보통' ELSE '원활' END) AS '13~16시 탑승객 평균',"
                "(CASE WHEN AVG(TIME17TO20) >300000 THEN '매우혼잡' WHEN AVG(TIME17TO20) >100000 THEN '보통' ELSE '원활' END) AS '17~20시 탑승객 평균' FROM SUBWAY WHERE MONTH LIKE";
                sql::SQLString when =" '%"+mydate+"%'";
                sql::SQLString region = " AND LINE ='"+myline+"'";
                sql::SQLString rest =" GROUP BY STATION";
                
                stmnt_res=stmnt->executeQuery(date+when+region+rest);    
            
                if(stmnt_res->rowsCount()==0)
                    std::cout<<"데이터없음"<<std::endl;

                while (stmnt_res->next()){
                    std::cout << stmnt_res->getString(1)<<'\t';
                    std::cout << stmnt_res->getString(2)<<"\n\t";
                    std::cout<< "9~12시 = " <<stmnt_res->getString(3);
                    std::cout<< ", 13~16시 = " <<stmnt_res->getString(4);
                    std::cout<< ", 17~20시 = " <<stmnt_res->getString(5)<<std::endl<<std::endl;
                }   
            }
            else std::cout<<"초기화면으로 돌아갑니다.\n";
        // 실패시 오류 메세지 반환
        }
        catch(sql::SQLException& e){
            std::cerr << "Error selecting tasks: " << e.what() << std::endl;
        }

        }
};

class Weather{ //날씨 함수 (날짜) 4번
    DB& Weather_DB;
public:
    Weather(DB& Handler_DB):Weather_DB(Handler_DB){}
    void Day_weaather(std::string mydate){ //지하철역 혼잡도 함수 (CONN,지역, 역이름)
        std::cout<<"다음은 입력하신 날짜의 당일 강수확률입니다. \n";
        std::unique_ptr<sql::Statement> stmnt(Weather_DB.conn->createStatement());
        std::string mymonth=mydate.substr(0,mydate.find('-'));
        std::string when = "'%"+mymonth+"%'";
        sql::ResultSet *res=stmnt->executeQuery("select AVG(RP_PER) from WEATHER WHERE DATE LIKE"+when);

        while (res->next()){
            std::cout<<"날짜: "+mydate<<" 강수확률: "<<res->getFloat(1)<<"%"<<std::endl;
            if(res->getFloat(1)>=50) std::cout<<"비가 올 확률이 큽니다. 가급적 실내에서 여행을 즐겨보세요.\n\n";
            if(res->getFloat(1)>=20) std::cout<<"비가 약하게 올 확률이 있습니다. 작은 우산 하나 챙겨가세요\n\n";
            else std::cout<<"구름 한 점 없이 화창한 날씨입니다. 즐거운 여행하시길 바랍니다.\n\n";
        }
    }
};

class ShowPlace{ //행사 정보 (날짜, 지역) 5번
    DB& ShoPlaceAndDust_DB;
public:
    ShowPlace(DB& Handler_DB):ShoPlaceAndDust_DB(Handler_DB){}
    void showPlaceAndDust(std::string mydate, std::string myregion) {
        try {
            // 쿼리를 실행
            sql::PreparedStatement* stmnt(ShoPlaceAndDust_DB.conn->prepareStatement("SELECT ENAME,BDATE,EDATE,PLACE,AGE FROM FESTIVAL WHERE BDATE LIKE ? OR EDATE LIKE ?;"));
            std::string when ="%" + mydate + "%";  //05-02
            stmnt->setString(1, when);
            stmnt->setString(2, when);
            sql::ResultSet *res = stmnt->executeQuery();
            // 반복문을 통해서 내부의 값을 반환
            
            while(res->rowsCount()==0){
                std::cout<<"찾은 데이터가 없습니다"<<std::endl;
                break;
            }
            std::cout<<std::endl;
            while (res->next()) 
            {
                std::cout << "행사 = " << res->getString(1)<<'\n';
                std::cout << "시작 날짜 = " << res->getString(2)<<'\n';
                std::cout << "끝나는 날짜 = " << res->getString(3)<<'\n';
                std::cout << "장소 = " << res->getString(4)<<'\n';
                std::cout << "관람가능연령 = " << res->getString(5)<<'\n'<<'\n';
            
            } 
        // 실패시 오류 메세지 반환
        } catch(sql::SQLException& e){
        std::cerr << "Error selecting tasks: " << e.what() << std::endl;
    }
    }
};

class Region_dust{  //미세먼지 정보 (날짜) 6번
    DB& Region_dust_DB;
public:
    Region_dust(DB& Handler_DB):Region_dust_DB(Handler_DB){}
    void region_dust(std::string date)
    {
    std::string when = "%" + date + "%";
    std::cout<<" 여행자님의 여행일 미세먼지에 대한 정보를 알려드립니다.";
    try{
        sql::PreparedStatement* stmnt(Region_dust_DB.conn->prepareStatement("SELECT (CASE WHEN AVG(DUST)>150 THEN '매우나쁨' WHEN AVG(DUST)<150 AND AVG(DUST)>80 THEN '나쁨' WHEN AVG(DUST)<80 AND AVG(DUST)>30 THEN '보통' ELSE '좋음' END) AS 미세먼지평균,"
        " (CASE WHEN AVG(DUST)>150 THEN '매우나쁨' WHEN AVG(DUST)<150 AND AVG(DUST)>80 THEN '나쁨' WHEN AVG(DUST)<80 AND AVG(DUST)>30 THEN '보통' ELSE '좋음' END) AS 초미세먼지평균 FROM AIR WHERE DATE LIKE ?;"));
            stmnt->setString(1, when);
        sql::ResultSet *res = stmnt->executeQuery();
        while (res->next()){
            std::cout << "\n미세먼지 기준치 = " << res->getString(1)<<"\n\n";
            if(res->getString(1) == "매우나쁨") std::cout << "외출을 삼가하세요\n\n";
            else if(res->getString(1) == "나쁨") std::cout << "실내에서 여행을 즐기시길 추천합니다\n\n";
            else if(res->getString(1) == "보통") std::cout << "마스크를 구비하세요\n\n";
            else if(res->getString(1) == "좋음") std::cout << "먼지없이 화창한 날씨입니다.\n\n";
        }
    }

    catch(sql::SQLException& e){
        std::cerr << "Error selecting tasks: " << e.what() << std::endl;
    }

    }
};

class Handler{
    DB Handler_DB;
public:
    std::vector<std::vector<std::string>> vect1; 
    std::vector<std::string> row;
    std::string time;   //검색한 시간
    std::string date;   //날짜 변수
    std::string region; //지역 변수
    std::string temp_text;
    std::string choice;
    int flag;
    int count_history;
    Handler():vect1(), count_history(0),flag(0){}
    void function(){
        
        while(1){
            if(flag == 0){
                std::unique_ptr<sql::Statement> stmnt(Handler_DB.conn->createStatement());
                sql::ResultSet *res=stmnt->executeQuery("select now()");

                while (res->next()){
                    time = res->getString(1);
                }

                std::cout<<"\n여행 날짜를 입력해주세요 ex) 05-20 : \n";
                getline(std::cin,date);
                row.emplace_back(date);
                std::cout<<"방문하시는 지역구를 입력해주세요 ex) 강남구 : \n";
                getline(std::cin,region);
                row.emplace_back(region);
                // system("clear");
                std::cout<<"어느 정보를 검색하시겠습니까?\n\n";
                std::cout<<"-----------------------------------------\n\n";
                std::cout<<"1)숙박업소\n2)지하철 혼잡도\n3)지하철호선\n4)날씨\n5)행사\n6)미세먼지\n7)검색기록조회\n8)검색기록삭제\n9)종료\n\n";
                std::cout<<"-----------------------------------------\n\n";
                getline(std::cin,choice);
                row.emplace_back(choice);
                if(choice == "1"){        //숙박업소 정보 (지역)
                    Resort Resort_go(Handler_DB);
                    Resort_go.showResort(region);
                    temp_text = "숙박업소";
                    history(time,temp_text,date,region);
                }
                else if(choice == "2"){   //지하철 혼잡도 (지역, 역이름)
                    std::cout<<std::endl;
                    Subway_avg Subway_avg_go(Handler_DB);
                    Subway_avg_go.subway_avg(region); 
                    temp_text = "지하철 혼잡도";
                    history(time,temp_text,date,region);
                }
                else if(choice == "3"){   //지역구 별 탈수있는 지하철호선과 역 이름 (지역)
                    std::cout<<std::endl;
                    ShowSubway ShowSubway_go(Handler_DB);
                    ShowSubway_go.showSubWay(date,region); 
                    temp_text = "지하철호선";
                    history(time,temp_text,date,region);
                }
                else if(choice == "4"){   //선택한 날짜에 대한 날씨정보(날짜)
                    std::cout<<std::endl;
                    Weather Weather_go(Handler_DB);
                    Weather_go.Day_weaather(date); 
                    temp_text = "날씨";
                    history(time,temp_text,date,region);
                }
                else if(choice == "5"){   //선택한 날짜에 대한 행사정보(날짜, 지역)
                    std::cout<<std::endl;
                    ShowPlace ShoPlaceAndDust_go(Handler_DB);
                    ShoPlaceAndDust_go.showPlaceAndDust(date,region); 
                    temp_text = "행사";
                    history(time,temp_text,date,region);
                }
                else if(choice == "6"){   //선택한 날짜에 대한 미세먼지정보(날짜, 지역)
                    std::cout<<std::endl;
                    Region_dust Region_dust_go(Handler_DB);
                    Region_dust_go.region_dust(date); 
                    temp_text = "미세먼지";
                    history(time,temp_text,date,region);
                }
                else if(choice == "7"){
                    show_history();
                }
                else if(choice == "8"){
                    delete_history();
                }
                else if(choice == "9"){
                    std::cout<<"종료합니다\n";
                    break;
                }
            }
            if(flag == 1){
                flag = 0;
                if(choice == "1"){
                    Resort Resort_go(Handler_DB);
                    Resort_go.showResort(region);
                }
                if(choice == "2"){
                    std::cout<<std::endl;
                    Subway_avg Subway_avg_go(Handler_DB);
                    Subway_avg_go.subway_avg(region); 
                }
                if(choice == "3"){
                    std::cout<<std::endl;
                    ShowSubway ShowSubway_go(Handler_DB);
                    ShowSubway_go.showSubWay(date,region); 
                }
                if(choice == "4"){
                    std::cout<<std::endl;
                    Weather Weather_go(Handler_DB);
                    Weather_go.Day_weaather(date); 
                }
                if(choice == "5"){
                    std::cout<<std::endl;
                    ShowPlace ShoPlaceAndDust_go(Handler_DB);
                    ShoPlaceAndDust_go.showPlaceAndDust(date,region); 
                }
                if(choice == "6"){
                    std::cout<<std::endl;
                    Region_dust Region_dust_go(Handler_DB);
                    Region_dust_go.region_dust(date); 
                }
            }
        }
    }

    void history(std::string time,std::string temp_text,std::string date, std::string region){
        count_history++;
        if(count_history < 30){
            row.clear();
            row.emplace_back(time);
            row.emplace_back(date);
            row.emplace_back(region);
            row.emplace_back(temp_text);

            vect1.emplace_back(row);
        }
    }

    void show_history(){
        int up=1;
        if(count_history == 0){
            std::cout<<"조회할 수 있는 검색 기록이 없습니다.";
        }
        else{
            std::cout<<"검색 기록입니다 (최대30개)\n";
            for(int i = 0; i < vect1.size(); i++){
                std::cout<<"\n"<< up++ <<")";
                for(int j = 0 ; j < vect1[i].size(); j++)
                        std::cout<< vect1[i][j]<<" |";
            }
            std::cout<<"\n검색결과를 바탕으로 재검색 하시겠습니까? (y/n)\n";
            std::string react1;
            getline(std::cin,react1);
            if(react1 =="y"|| react1 == "Y"){
                flag = 1;
                std::cout<<"검색번호를 입력하세요.\n";
                std::string react2;
                getline(std::cin,react2);
                std::cout<<vect1[stoi(react2)-1][1]<<std::endl;
                std::cout<<vect1[stoi(react2)-1][2]<<std::endl;
                // exit(1);
                date=vect1[stoi(react2)-1][1];
                region=vect1[stoi(react2)-1][2];
                choice = react2;
                std::cout<<date<<std::endl;
                std::cout<<region<<std::endl;
            }
            else {
                std::cout<<"* 초기화면으로 돌아갑니다 *\n";
            }
        }
    }

    void delete_history(){
        std::cout<<"\n검색 기록을 삭제합니다\n";
        vect1.clear();
        count_history = 0;
    }
};

int main()
{
    Handler controler;
    controler.function();

   return 0;
}