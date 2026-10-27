#pragma once
// Engine-independent, server-owned simulation. Coordinates and ranges are metres.
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <map>
#include <string>
#include <vector>
namespace frontline {
struct Vec { float x=0, y=0; };
inline Vec operator+(Vec a,Vec b){return {a.x+b.x,a.y+b.y};}
inline Vec operator-(Vec a,Vec b){return {a.x-b.x,a.y-b.y};}
inline Vec operator*(Vec a,float b){return {a.x*b,a.y*b};}
inline float length(Vec a){return std::sqrt(a.x*a.x+a.y*a.y);}
inline float distance(Vec a,Vec b){return length(a-b);}
inline bool valid(Vec p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::abs(p.x)<=500&&std::abs(p.y)<=500;}
enum class Kind { Rifle, Recon, Tank, AntiTank, Mortar, Supply };
enum class Action { Move, Attack, AttackMove, Stop, Hold, Resupply, Smoke };
enum class Phase { Lobby, Playing, Finished };
struct Stats { const char* name; int cost; float hp,speed,range,sight,damage,reload; int ammo; bool armor,antiArmor; };
inline constexpr std::array<Stats,6> roster{{
 {"Rifle squad",100,120,5,105,180,12,1,120,false,false},
 {"Recon vehicle",150,170,15,140,280,14,1,150,true,false},
 {"Main battle tank",350,450,10,240,220,75,3,40,true,true},
 {"Anti-tank squad",150,90,4,200,190,95,5,12,false,true},
 {"Mortar team",200,85,3,350,140,32,4,35,false,false},
 {"Supply truck",125,140,12,0,160,0,1,0,true,false}
}};
inline bool valid(Kind k){return int(k)>=0&&int(k)<int(roster.size());}
inline bool valid(Action a){return int(a)>=0&&int(a)<=int(Action::Smoke);}
struct Order { Action action=Action::Stop; Vec destination; int target=-1; };
struct Unit { int id=0,team=0; Kind kind=Kind::Rifle; Vec position; float heading=0,hp=0,suppression=0,cooldown=0,smokeCooldown=0; int ammo=0; bool hold=false; std::deque<Order> orders; };
struct Objective { Vec position; int owner=-1; float capture=0; int capturing=-1; };
struct Obstacle { Vec center; float radius; bool cover=true; };
struct Smoke { Vec position; float remaining=20; };
struct Player { bool occupied=false,connected=false,ready=false; std::string token; float grace=0,credits=500; int tickets=1000; };
struct Contact { int id; Kind kind; Vec position; float age=0; }; // Historical values only.
struct View { std::vector<Unit> units; std::vector<Contact> contacts; };
class Battle {
 public:
 Phase phase=Phase::Lobby; int winner=-2; float elapsed=0; int nextId=1;
 std::array<Player,2> players{};
 std::map<int,Unit> units;
 std::array<Objective,3> objectives{{{{-220,-160}},{{0,80}},{{230,-110}}}};
 std::vector<Obstacle> obstacles{{{-110,10},32},{{100,-40},38},{{20,240},45},{{-250,180},30},{{260,180},28}};
 std::vector<Smoke> smoke;
 std::array<std::map<int,Contact>,2> history;
 float ticketFraction=0;
 int join(const std::string& token) {
  if(token.empty()) return -1;
  for(int i=0;i<2;++i) if(players[i].occupied&&players[i].token==token) {
   if(players[i].connected||phase==Phase::Finished|| (phase==Phase::Playing&&players[i].grace<=0)) return -1;
   players[i].connected=true; players[i].grace=0; return i;
  }
  if(phase!=Phase::Lobby) return -1;
  for(int i=0;i<2;++i) if(!players[i].occupied) { players[i]=Player{}; players[i].occupied=true; players[i].connected=true; players[i].token=token; return i; }
  return -1;
 }
 void disconnect(int team){if(team<0||team>1)return; if(phase==Phase::Lobby)players[team]=Player{};else {players[team].connected=false;players[team].grace=60;}}
 bool ready(int team){
  if(team<0||team>1||phase!=Phase::Lobby||!players[team].connected)return false;
  players[team].ready=true;
  if(players[0].ready&&players[1].ready&&players[0].connected&&players[1].connected){phase=Phase::Playing; for(int t=0;t<2;++t){spawn(t,Kind::Rifle);spawn(t,Kind::Recon);}}
  return true;
 }
 int count(int team)const{int n=0;for(auto& p:units)if(p.second.team==team)++n;return n;}
 int spawn(int team,Kind kind){
  int n=count(team); Unit u;u.id=nextId++;u.team=team;u.kind=kind;u.hp=roster[int(kind)].hp;u.ammo=roster[int(kind)].ammo;
  u.position={team==0?-455.f:455.f,float((n%10)*14-63)};u.heading=team==0?0:3.14159265f; units[u.id]=u;return u.id;
 }
 std::string purchase(int team,Kind kind){
  if(team<0||team>1||phase!=Phase::Playing||!players[team].connected)return "Match is not active";
  if(!valid(kind))return "Unknown unit";
  if(count(team)>=20)return "Army limit: 20";
  if(players[team].credits<roster[int(kind)].cost)return "Insufficient credits";
  players[team].credits-=roster[int(kind)].cost;spawn(team,kind);return {};
 }
 static float segmentDistance(Vec p,Vec a,Vec b){Vec v=b-a;float d=v.x*v.x+v.y*v.y;float t=d>0?std::clamp(((p.x-a.x)*v.x+(p.y-a.y)*v.y)/d,0.f,1.f):0;return distance(p,a+v*t);}
 bool clear(Vec a,Vec b)const{
  for(auto& o:obstacles)if(segmentDistance(o.center,a,b)<o.radius)return false;
  for(auto& s:smoke)if(segmentDistance(s.position,a,b)<35)return false;
  return true;
 }
 bool walkable(Vec p)const{if(!valid(p))return false;for(auto& o:obstacles)if(distance(p,o.center)<o.radius+4)return false;return true;}
 bool visible(int team,const Unit& target)const {
  if(target.team==team)return true;
  for(auto& p:units){auto& u=p.second;if(u.team==team&&distance(u.position,target.position)<=roster[int(u.kind)].sight&&clear(u.position,target.position))return true;}
  return false;
 }
 std::string command(int team,const std::vector<int>& ids,Order order,bool queue){
  if(team<0||team>1||phase!=Phase::Playing||!players[team].connected)return "Match is not active";
  if(!valid(order.action)||ids.empty()||ids.size()>20||!valid(order.destination))return "Invalid command";
  for(int id:ids){auto it=units.find(id);if(it==units.end()||it->second.team!=team)return "Unit not owned";if(queue&&it->second.orders.size()>=16)return "Order queue full";}
  if(order.action==Action::Attack||order.action==Action::Resupply){auto it=units.find(order.target);if(it==units.end())return "Target unavailable";
   if(order.action==Action::Attack&&(it->second.team==team||!visible(team,it->second)))return "Target unavailable";
   if(order.action==Action::Resupply&&(it->second.team!=team||it->second.kind!=Kind::Supply))return "Select a friendly supply truck";
  }
  if((order.action==Action::Move||order.action==Action::AttackMove)&&!walkable(order.destination))return "Destination is blocked";
  if(order.action==Action::Smoke)for(int id:ids){auto& u=units.at(id);if(u.kind!=Kind::Mortar||u.smokeCooldown>0||u.ammo<1||distance(u.position,order.destination)>350)return "Smoke unavailable or out of range";}
  int index=0;
  for(int id:ids){auto& u=units.at(id);if(order.action==Action::Smoke){smoke.push_back({order.destination,20});u.smokeCooldown=30;--u.ammo;continue;}
   if(!queue){u.orders.clear();}
   u.hold=order.action==Action::Hold;
   if(order.action!=Action::Stop&&order.action!=Action::Hold){Order individual=order;if(ids.size()>1&&(order.action==Action::Move||order.action==Action::AttackMove)){Vec p=order.destination+Vec{float(index%5)*9-18,float(index/5)*9};if(walkable(p))individual.destination=p;}u.orders.push_back(individual);}++index;
  }return {};
 }
 float armorMultiplier(const Unit& target,const Unit& shooter)const{
  if(!roster[int(target.kind)].armor)return 1;
  Vec d=shooter.position-target.position;float angle=std::cos(std::atan2(d.y,d.x)-target.heading);
  return (roster[int(shooter.kind)].antiArmor?1.f:.12f)*(angle>.5f?.45f:angle<-.5f?1.f:.7f);
 }
 void finish(int result){phase=Phase::Finished;winner=result;}
 void tick(float dt){
  if(phase!=Phase::Playing||!std::isfinite(dt)||dt<=0||dt>.25f)return;
  elapsed+=dt;
  for(int t=0;t<2;++t){auto& p=players[t];p.credits=std::min(2000.f,p.credits+8*dt);if(!p.connected){p.grace-=dt;if(p.grace<=0){finish(1-t);return;}}}
  for(auto& s:smoke)s.remaining-=dt;
  smoke.erase(std::remove_if(smoke.begin(),smoke.end(),[](const Smoke& s){return s.remaining<=0;}),smoke.end());
  for(auto& pair:units){auto& u=pair.second;auto& stat=roster[int(u.kind)];u.cooldown=std::max(0.f,u.cooldown-dt);u.smokeCooldown=std::max(0.f,u.smokeCooldown-dt);u.suppression=std::max(0.f,u.suppression-dt*.08f);
   if(u.hp<=0)continue;
   int target=-1;float nearest=stat.range;
   auto suitable=[&](const Unit& e){return e.hp>0&&e.team!=u.team&&visible(u.team,e)&&distance(u.position,e.position)<=stat.range&&(u.kind==Kind::Mortar||clear(u.position,e.position))&&std::abs(e.position.x)<420;};
   if(!u.orders.empty()&&u.orders.front().action==Action::Attack){auto it=units.find(u.orders.front().target);if(it!=units.end()&&suitable(it->second))target=it->first;}
   bool moving=!u.orders.empty()&&u.orders.front().action==Action::Move;
   if(target<0&&!moving)for(auto& ep:units)if(suitable(ep.second)){float d=distance(u.position,ep.second.position);if(d<nearest){nearest=d;target=ep.first;}}
   if(target>=0&&u.ammo>0&&u.cooldown<=0&&u.suppression<.95f){auto& enemy=units.at(target);float cover=1;for(auto& o:obstacles)if(o.cover&&distance(enemy.position,o.center)<o.radius+16)cover=.55f;
    enemy.hp-=stat.damage*armorMultiplier(enemy,u)*cover;enemy.suppression=std::min(1.f,enemy.suppression+.22f);u.cooldown=stat.reload;--u.ammo;u.heading=std::atan2(enemy.position.y-u.position.y,enemy.position.x-u.position.x);
   }
   if(!u.orders.empty()){
    auto& order=u.orders.front();Vec dest=order.destination;bool remove=false;
    if(order.action==Action::Attack||order.action==Action::Resupply){auto it=units.find(order.target);if(it==units.end()||(order.action==Action::Attack&&!visible(u.team,it->second)))remove=true;else dest=it->second.position;}
    float stopRange=order.action==Action::Attack?stat.range*.85f:order.action==Action::Resupply?25.f:3.f;
    float d=distance(dest,u.position);
    if(remove||d<stopRange){u.orders.pop_front();}
    else if(!(target>=0&&order.action==Action::AttackMove)&&!u.hold){Vec direction=(dest-u.position)*(1/std::max(d,.01f));Vec next=u.position+direction*(stat.speed*(1-u.suppression*.65f)*dt);
     if(!walkable(next)){Vec tangent{-direction.y,direction.x};next=u.position+tangent*(stat.speed*dt);if(!walkable(next))next=u.position-tangent*(stat.speed*dt);}
     if(walkable(next)){u.heading=std::atan2(next.y-u.position.y,next.x-u.position.x);u.position=next;}
    }
   }
   if(u.kind==Kind::Supply)for(auto& ally:units)if(ally.second.team==u.team&&distance(u.position,ally.second.position)<35&&ally.second.cooldown<=0){auto& a=ally.second;a.hp=std::min(roster[int(a.kind)].hp,a.hp+dt*3);if(a.ammo<roster[int(a.kind)].ammo&&int(elapsed*2)!=int((elapsed-dt)*2))++a.ammo;}
  }
  for(auto it=units.begin();it!=units.end();)if(it->second.hp<=0)it=units.erase(it);else ++it;
  for(auto& o:objectives){std::array<int,2> near{};for(auto& p:units)if(distance(p.second.position,o.position)<55)++near[p.second.team];int team=near[0]>0&&near[1]==0?0:near[1]>0&&near[0]==0?1:-1;
   if(team<0||team==o.owner){o.capture=0;o.capturing=-1;}else{if(o.capturing!=team){o.capture=0;o.capturing=team;}o.capture+=dt;if(o.capture>=8){o.owner=team;o.capture=0;o.capturing=-1;}}
  }
  ticketFraction+=dt;if(ticketFraction>=1){ticketFraction-=1;int a=0,b=0;for(auto& o:objectives){a+=o.owner==0;b+=o.owner==1;}if(a>=2)players[1].tickets=std::max(0,players[1].tickets-2);if(b>=2)players[0].tickets=std::max(0,players[0].tickets-2);}
  if(players[0].tickets==0||players[1].tickets==0||elapsed>=1500)finish(players[0].tickets==players[1].tickets?-1:players[0].tickets>players[1].tickets?0:1);
  for(int team=0;team<2;++team){for(auto& h:history[team])h.second.age+=dt;for(auto& p:units)if(p.second.team!=team&&visible(team,p.second))history[team][p.first]={p.first,p.second.kind,p.second.position,0};for(auto it=history[team].begin();it!=history[team].end();)if(it->second.age>30)it=history[team].erase(it);else ++it;}
 }
 View view(int team)const{View v;if(team<0||team>1)return v;for(auto& p:units)if(visible(team,p.second)){Unit u=p.second;if(u.team!=team){u.orders.clear();u.ammo=-1;u.cooldown=0;u.smokeCooldown=0;}v.units.push_back(u);}for(auto& h:history[team])if(h.second.age>0)v.contacts.push_back(h.second);return v;}
};
}
