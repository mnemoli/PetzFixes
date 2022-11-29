#pragma once
#include "Sprite_Post.h"
class Area;
class Sprite_Post_B1 : public Sprite_Post
{
private:
	Ledge* scratchLedge = nullptr;
	//Ledge* areaLedge = nullptr;
	//Area* area = nullptr;
public:
	void InitToyExtraCode(bool, Host*) override;
	void AddInHostExtraCode(Host&) override;
	void RemInHostExtraCode(Host&, const char*, const char*) override;
	void RunUpdate() override;
	XTRect<> ComputeLedgeRect(XTRect<>);
	~Sprite_Post_B1();
};

