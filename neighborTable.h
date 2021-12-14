#pragma once
class neighbor {
private:
	int id;
	int level;
	int state;
public:
	neighbor* next;
	neighbor();
	neighbor(int id, int level, int state);
	int getState();
	int setState(int state);
	int getLevel();
	void setLevel(int level);
	int getId();
};

class neighborTable {
private:
	neighbor* head;
	neighbor* tool;
public:
	neighborTable();
	void addNeighbor(neighbor* n);
	void addNeighbor(int id, int level, int state);
};



neighbor::neighbor() {
	this->id = this->level = this->state = -1;
	this->next = NULL;
}
neighbor::neighbor(int id, int level, int state) {
	this->id = id;
	this->level = level;
	this->state = state;
	this->next = NULL;
}
int neighbor::getState() {
	return this->state;
}
int neighbor::setState(int state) {
	this->state = state;
}
int neighbor::getLevel() {
	return this->level;
}
void neighbor::setLevel(int level) {
	this->level=level
}
int neighbor::getId() {
	return this->id;
}


neighborTable::neighborTable() {
	this->head = new neighbor();
}
void neighborTable::addNeighbor(neighbor* n) {
	this->tool = this->head;
	while (this->tool->next != NULL)this->tool = this->tool->next;
	this->tool->next = n;
}
void neighborTable::addNeighbor(int id, int level, int state) {
	neighbor* n = new neighbor(id, level, state);
	this->tool = this->head;
	while (this->tool->next != NULL)this->tool = this->tool->next;
	this->tool->next = n;
}