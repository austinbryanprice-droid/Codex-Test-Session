#include "../Source/FrontlineCommand/Core/Battle.h"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace frontline;
static int checks=0;
#define CHECK(expr) do { ++checks; if(!(expr)){std::cerr<<__LINE__<<": " #expr "\n";std::exit(1);} } while(false)
Battle started(){Battle b;CHECK(b.join("alpha")==0);CHECK(b.join("bravo")==1);CHECK(b.ready(0));CHECK(b.phase==Phase::Lobby);CHECK(b.ready(1));CHECK(b.phase==Phase::Playing);return b;}
void advance(Battle& b,float seconds){for(int i=0;i<int(seconds*20);++i)b.tick(.05f);}
int main(){
 {auto b=started();CHECK(b.join("third")==-1);CHECK(b.join("alpha")==-1);CHECK(b.count(0)==2);CHECK(b.purchase(0,Kind::Tank).empty());CHECK(!b.purchase(0,Kind::Tank).empty());CHECK(b.players[0].credits==150);CHECK(!b.purchase(0,Kind(99)).empty());b.players[0].credits=99999;for(int i=0;i<17;++i)CHECK(b.purchase(0,Kind::Rifle).empty());CHECK(b.count(0)==20);CHECK(!b.purchase(0,Kind::Rifle).empty());}
 {auto b=started();int own=1,enemy=3;CHECK(!b.command(0,{enemy},{Action::Move,{0,0}},false).empty());CHECK(!b.command(0,{own},{Action::Move,{std::numeric_limits<float>::quiet_NaN(),0}},false).empty());CHECK(!b.command(0,{own},{Action(99),{}},false).empty());CHECK(!b.command(0,{own},{Action::Attack,{},enemy},false).empty());CHECK(b.command(0,{own},{Action::Move,{-400,-63}},false).empty());advance(b,2);CHECK(b.units.at(own).position.x>-455);for(int i=0;i<15;++i)CHECK(b.command(0,{own},{Action::Move,{-400,0}},true).empty());CHECK(!b.command(0,{own},{Action::Move,{-400,0}},true).empty());CHECK(b.command(0,{own},{Action::Stop,{}},false).empty());CHECK(b.units.at(own).orders.empty());}
 {auto b=started();CHECK(b.view(0).units.size()==2);b.units.at(3).position={-300,-63};advance(b,.1f);CHECK(b.view(0).units.size()==3);Vec known=b.units.at(3).position;b.units.at(3).position={400,400};advance(b,1);auto v=b.view(0);CHECK(v.units.size()==2);CHECK(v.contacts.size()==1);CHECK(distance(v.contacts[0].position,known)<.01f);advance(b,31);CHECK(b.view(0).contacts.empty());}
 {auto b=started();b.units.at(1).position={-220,-160};b.units.at(2).position={0,80};advance(b,10);CHECK(b.objectives[0].owner==0);CHECK(b.objectives[1].owner==0);CHECK(b.players[1].tickets<1000);b.elapsed=1499.9f;advance(b,.2f);CHECK(b.phase==Phase::Finished);CHECK(b.winner==0);}
 {auto b=started();b.elapsed=1499.9f;advance(b,.2f);CHECK(b.winner==-1);}
 {auto b=started();b.disconnect(0);advance(b,59);CHECK(b.phase==Phase::Playing);CHECK(b.join("alpha")==0);advance(b,2);CHECK(b.phase==Phase::Playing);b.disconnect(0);advance(b,61);CHECK(b.phase==Phase::Finished);CHECK(b.winner==1);CHECK(b.join("alpha")==-1);}
 {auto b=started();b.units.clear();int a=b.spawn(0,Kind::Tank),e=b.spawn(1,Kind::Tank);auto& attacker=b.units.at(a);auto& defender=b.units.at(e);attacker.position={100,-200};defender.position={0,-200};defender.heading=0;float front=b.armorMultiplier(defender,attacker);attacker.position={-100,-200};CHECK(b.armorMultiplier(defender,attacker)>front);float hp=defender.hp;advance(b,4);CHECK(defender.hp<hp);CHECK(attacker.ammo<roster[2].ammo);}
 {auto b=started();b.units.clear();int mortar=b.spawn(0,Kind::Mortar);b.units.at(mortar).position={-200,-200};CHECK(b.command(0,{mortar},{Action::Smoke,{-150,-200}},false).empty());CHECK(b.smoke.size()==1);CHECK(!b.clear({-200,-200},{-100,-200}));CHECK(!b.command(0,{mortar},{Action::Smoke,{-150,-200}},false).empty());advance(b,21);CHECK(b.smoke.empty());}
 {auto b=started();b.units.at(1).ammo=0;b.units.at(1).hp=50;int supply=b.spawn(0,Kind::Supply);b.units.at(supply).position=b.units.at(1).position;advance(b,2);CHECK(b.units.at(1).ammo>0);CHECK(b.units.at(1).hp>50);}
 {auto b=started();b.units.at(1).position={-220,-160};b.units.at(3).position={-210,-160};b.units.at(1).ammo=0;b.units.at(3).ammo=0;advance(b,9);CHECK(b.objectives[0].owner==-1);}
 std::cout<<checks<<" assertions passed across 10 scenarios\n";
}
