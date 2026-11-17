#include "flprogScrollMenu.h"
#include <cassert>
#include <climits>
#include <iostream>
static unsigned reads=0,writes=0;
static int16_t out[5]; static String texts[5]; static int16_t bias=0;
int16_t readInt(uint16_t i) { ++reads; assert(i<100); return i+bias; }
void writeInt(int16_t v,uint16_t i) { ++writes; assert(i<5); out[i]=v; }
String readText(uint16_t i) { assert(i<100); return "Рецепт "+std::to_string(i); }
void writeText(String v,uint16_t i) { assert(i<5); texts[i]=v; }
static uint16_t lastSource,lastOutput; static unsigned wideWrites;
uint32_t readWide(uint16_t i) { lastSource=i; return i; }
void writeWide(uint32_t,uint16_t i) { lastOutput=i; ++wideWrites; }
int main(){
 FlprogScrollMenu m(5); FlprogScrollMenuInt16Column a(100,5); FlprogScrollMenuStringColumn b(100,5);
 FlprogAbstractScrollMenuColumn* cols[]={&a,&b};
 assert(!m.isReady() && !m.pool()); assert(m.setColumns(cols,2)); assert(!m.pool());
 a.setReadFunction(readInt); a.setWriteFunction(writeInt); b.setReadFunction(readText); b.setWriteFunction(writeText);
 a.setEmptyValue(-1); b.setEmptyValue("-");
 m.setCount(100); assert(m.pool()); assert(m.isReady()); assert(m.getFirstIndex()==0 && m.getSelectedRow()==0 && writes==5);
 for(int i=1;i<5;i++){m.setIndex(i);assert(!m.pool());assert(m.getSelectedRow()==i && writes==5);}
 for(int i=5;i<100;i++){m.setIndex(i);assert(m.pool()); assert(m.getFirstIndex()==i-4 && m.getSelectedRow()==4);for(int j=0;j<5;j++){assert(out[j]==i-4+j);assert(texts[j]=="Рецепт "+std::to_string(i-4+j));}}
 m.setIndex(100); assert(!m.pool() && m.getIndex()==99);
 for(int i=98;i>=95;i--){m.setIndex(i);assert(!m.pool());assert(m.getSelectedRow()==i-95);}
 for(int i=94;i>=0;i--){m.setIndex(i);assert(m.pool()); assert(m.getFirstIndex()==i && m.getSelectedRow()==0);}
 m.setIndex(75);assert(m.pool() && m.getFirstIndex()==71);
 m.setCount(3);assert(m.pool());assert(m.getIndex()==2 && m.getFirstIndex()==0 && m.getSelectedRow()==2 && m.getVisibleCount()==3);
 assert(out[3]==-1 && texts[4]=="-" && !m.isRowValid(3));
 m.setCount(0);assert(m.pool() && m.getIndex()==-1 && m.getSelectedRow()==-1);for(int i=0;i<5;i++)assert(out[i]==-1 && texts[i]=="-");
 m.setCount(-6);assert(!m.pool());m.setCount(INT32_MAX);m.setIndex(-10);assert(m.pool() && m.getCount()==100 && m.getIndex()==0);
 bias=10; assert(!m.pool() && out[0]==0); m.refresh(); assert(m.pool() && out[0]==10); m.setAutoRefresh(true);assert(m.pool());m.setAutoRefresh(false);assert(!m.pool());
 b.setWriteFunction(nullptr); assert(!m.pool()&&!m.isReady()&&m.getIndex()==-1);b.setWriteFunction(writeText);assert(m.pool());
 FlprogScrollMenuInt16Column shortcol(2,5,readInt,writeInt);FlprogAbstractScrollMenuColumn* shortcols[]={&shortcol,&b};assert(m.setColumns(shortcols,2));assert(m.pool() && m.getCount()==2);
 FlprogScrollMenuInt16Column small(100,4,readInt,writeInt);FlprogAbstractScrollMenuColumn* bad[]={&small};assert(!m.setColumns(bad,1)&&!m.pool());
 FlprogAbstractScrollMenuColumn* dup[]={&a,&a};assert(!m.setColumns(dup,2));assert(!m.setColumns(nullptr,3));FlprogAbstractScrollMenuColumn* nullcol[]={nullptr};assert(!m.setColumns(nullcol,1));
 FlprogScrollMenu zero(0);assert(!zero.setColumns(cols,2));
 FlprogScrollMenu wide(65535);FlprogScrollMenuUInt32Column w(65535,65535,readWide,writeWide);FlprogAbstractScrollMenuColumn* wc[]={&w};assert(wide.setColumns(wc,1));wide.setCount(65535);wide.setIndex(INT32_MAX);assert(wide.pool());assert(wide.getIndex()==65534&&wide.getSelectedRow()==65534&&lastSource==65534&&lastOutput==65534&&wideWrites==65535);
 FlprogScrollMenu one(1);FlprogScrollMenuUInt32Column u(65535,1,readWide,writeWide);FlprogAbstractScrollMenuColumn* uc[]={&u};assert(one.setColumns(uc,1));one.setCount(65535);one.setIndex(65534);assert(one.pool()&&one.getFirstIndex()==65534&&one.getSelectedRow()==0);
 std::cout<<"PASS: 100 forward/back, boundaries, mixed columns, callbacks, empty, resize, refresh, uint16 limits\n";
}
